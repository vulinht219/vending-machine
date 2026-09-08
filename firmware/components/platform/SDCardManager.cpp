#include "SDCardManager.h"

#include "CH422GManager.h"

#include <stdio.h>

extern "C" {

#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"

#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "driver/sdspi_host.h"

#include "sdmmc_cmd.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

}


namespace {

constexpr const char* TAG =
    "SDCardManager";


// =====================================================
// WAVESHARE ESP32-S3-TOUCH-LCD-4.3
// TF / MICROSD
// =====================================================
//
// Verified with sd-test:
//
// MOSI = GPIO11
// MISO = GPIO13
// CLK  = GPIO12
//
// CS is NOT a normal ESP32 GPIO.
//
// SD_CS = CH422G EXIO4
//

constexpr gpio_num_t SD_MOSI =
    GPIO_NUM_11;

constexpr gpio_num_t SD_MISO =
    GPIO_NUM_13;

constexpr gpio_num_t SD_CLK =
    GPIO_NUM_12;


constexpr int SD_CS_EXIO =
    4;


// =====================================================
// SPI HOST
// =====================================================

constexpr spi_host_device_t SD_SPI_HOST =
    SPI2_HOST;


// =====================================================
// FILESYSTEM
// =====================================================

constexpr const char* SD_MOUNT_POINT =
    "/sdcard";


sdmmc_card_t* card =
    nullptr;


sdmmc_host_t host =
    SDSPI_HOST_DEFAULT();


bool spiBusInitialized =
    false;

} // namespace


bool SDCardManager::mounted =
    false;


// =====================================================
// MOUNT
// =====================================================

bool SDCardManager::mount()
{
    if (
        mounted
    )
    {
        ESP_LOGI(
            TAG,
            "SD card already mounted"
        );


        return true;
    }


    ESP_LOGI(
        TAG,
        "Starting SD initialization"
    );


    // =================================================
    // CH422G
    // =================================================
    //
    // BoardI2CManager + CH422G have already been
    // initialized by DisplayManager / TouchManager.
    //
    // initialize() is safe to call again because the
    // manager guards its initialized state.
    //

    if (
        !CH422GManager::initialize()
    )
    {
        ESP_LOGE(
            TAG,
            "CH422G initialization failed"
        );


        return false;
    }


    // =================================================
    // SD HOST
    // =================================================

    host =
        SDSPI_HOST_DEFAULT();


    host.slot =
        SD_SPI_HOST;


    // =================================================
    // FAT CONFIG
    // =================================================

    esp_vfs_fat_sdmmc_mount_config_t
        mountConfig = {};


    mountConfig.format_if_mount_failed =
        false;


    mountConfig.max_files =
        5;


    mountConfig.allocation_unit_size =
        16 * 1024;


    // =================================================
    // SPI BUS
    // =================================================

    spi_bus_config_t busConfig = {};


    busConfig.mosi_io_num =
        SD_MOSI;


    busConfig.miso_io_num =
        SD_MISO;


    busConfig.sclk_io_num =
        SD_CLK;


    busConfig.quadwp_io_num =
        -1;


    busConfig.quadhd_io_num =
        -1;


    busConfig.data4_io_num =
        -1;


    busConfig.data5_io_num =
        -1;


    busConfig.data6_io_num =
        -1;


    busConfig.data7_io_num =
        -1;


    busConfig.max_transfer_sz =
        4096;


    ESP_LOGI(
        TAG,
        "SD pins: MOSI=%d MISO=%d CLK=%d",
        static_cast<int>(
            SD_MOSI
        ),
        static_cast<int>(
            SD_MISO
        ),
        static_cast<int>(
            SD_CLK
        )
    );


    ESP_LOGI(
        TAG,
        "SD CS = CH422G EXIO%d",
        SD_CS_EXIO
    );


    ESP_LOGI(
        TAG,
        "Calling spi_bus_initialize..."
    );


    esp_err_t result =
        spi_bus_initialize(
            SD_SPI_HOST,
            &busConfig,
            SPI_DMA_CH_AUTO
        );


    ESP_LOGI(
        TAG,
        "spi_bus_initialize returned: %s",
        esp_err_to_name(
            result
        )
    );


    if (
        result == ESP_OK
    )
    {
        spiBusInitialized =
            true;
    }
    else if (
        result == ESP_ERR_INVALID_STATE
    )
    {
        // SPI2 was already initialized elsewhere.
        //
        // Continue without claiming ownership of the bus.

        ESP_LOGW(
            TAG,
            "SPI bus already initialized"
        );


        spiBusInitialized =
            false;
    }
    else
    {
        ESP_LOGE(
            TAG,
            "SPI initialization failed: %s",
            esp_err_to_name(
                result
            )
        );


        return false;
    }


    // =================================================
    // ACTIVATE SD CARD
    // =================================================
    //
    // Waveshare connects SD_CS to CH422G EXIO4.
    //
    // LOW = active.
    //

    if (
        !CH422GManager::setOutput(
            SD_CS_EXIO,
            false
        )
    )
    {
        ESP_LOGE(
            TAG,
            "Failed to activate SD CS through EXIO4"
        );


        if (
            spiBusInitialized
        )
        {
            spi_bus_free(
                SD_SPI_HOST
            );


            spiBusInitialized =
                false;
        }


        return false;
    }


    ESP_LOGI(
        TAG,
        "SD CS EXIO4 LOW"
    );


    vTaskDelay(
        pdMS_TO_TICKS(
            20
        )
    );


    // =================================================
    // SDSPI DEVICE
    // =================================================
    //
    // CS is handled externally by CH422G.
    //
    // Therefore SDSPI must NOT try to configure a
    // normal ESP32 GPIO as chip-select.
    //

    sdspi_device_config_t slotConfig =
        SDSPI_DEVICE_CONFIG_DEFAULT();


    slotConfig.host_id =
        SD_SPI_HOST;


    slotConfig.gpio_cs =
        SDSPI_SLOT_NO_CS;


    // =================================================
    // MOUNT
    // =================================================

    ESP_LOGI(
        TAG,
        "Mounting SD card at %s...",
        SD_MOUNT_POINT
    );


    result =
        esp_vfs_fat_sdspi_mount(
            SD_MOUNT_POINT,
            &host,
            &slotConfig,
            &mountConfig,
            &card
        );


    ESP_LOGI(
        TAG,
        "esp_vfs_fat_sdspi_mount returned: %s",
        esp_err_to_name(
            result
        )
    );


    if (
        result != ESP_OK
    )
    {
        if (
            result == ESP_FAIL
        )
        {
            ESP_LOGE(
                TAG,
                "Failed to mount FAT filesystem"
            );
        }
        else
        {
            ESP_LOGE(
                TAG,
                "Failed to initialize SD card: %s",
                esp_err_to_name(
                    result
                )
            );
        }


        // Deselect card.

        CH422GManager::setOutput(
            SD_CS_EXIO,
            true
        );


        if (
            spiBusInitialized
        )
        {
            spi_bus_free(
                SD_SPI_HOST
            );


            spiBusInitialized =
                false;
        }


        card =
            nullptr;


        return false;
    }


    // =================================================
    // SUCCESS
    // =================================================

    mounted =
        true;


    ESP_LOGI(
        TAG,
        "SD card mounted successfully"
    );


    sdmmc_card_print_info(
        stdout,
        card
    );


    return true;
}


// =====================================================
// UNMOUNT
// =====================================================

void SDCardManager::unmount()
{
    if (
        !mounted
    )
    {
        return;
    }


    esp_vfs_fat_sdcard_unmount(
        SD_MOUNT_POINT,
        card
    );


    // Deselect SD card through CH422G.

    CH422GManager::setOutput(
        SD_CS_EXIO,
        true
    );


    if (
        spiBusInitialized
    )
    {
        spi_bus_free(
            SD_SPI_HOST
        );


        spiBusInitialized =
            false;
    }


    card =
        nullptr;


    mounted =
        false;


    ESP_LOGI(
        TAG,
        "SD card unmounted"
    );
}


// =====================================================
// STATUS
// =====================================================

bool SDCardManager::isMounted()
{
    return mounted;
}