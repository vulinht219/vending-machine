#include "SDCardManager.h"
#include "BoardPins.h"

#include <stdio.h>

extern "C" {

#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"

#include "driver/spi_common.h"
#include "driver/sdspi_host.h"

#include "sdmmc_cmd.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
}


namespace {

constexpr const char* TAG =
    "SDCardManager";


constexpr spi_host_device_t SD_SPI_HOST =
    SPI2_HOST;


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
    if (mounted)
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
    // SPI HOST
    // =================================================

    host.slot =
        SD_SPI_HOST;


    // =================================================
    // FAT FILESYSTEM CONFIG
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
    // SPI BUS CONFIG
    // =================================================

    spi_bus_config_t busConfig = {};


    busConfig.mosi_io_num =
        BOARD_SD_MOSI;


    busConfig.miso_io_num =
        BOARD_SD_MISO;


    busConfig.sclk_io_num =
        BOARD_SD_CLK;


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


    // =================================================
    // DEBUG INFO
    // =================================================

    ESP_LOGI(
        TAG,
        "SD pins: MOSI=%d MISO=%d CLK=%d CS=%d",
        BOARD_SD_MOSI,
        BOARD_SD_MISO,
        BOARD_SD_CLK,
        BOARD_SD_CS
    );


    ESP_LOGI(
        TAG,
        "SPI host=%d",
        static_cast<int>(
            SD_SPI_HOST
        )
    );


    // =================================================
    // INITIALIZE SPI BUS
    // =================================================

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
            "SPI bus initialization failed: %s",
            esp_err_to_name(
                result
            )
        );


        return false;
    }


    // =================================================
    // SHORT DELAY
    // =================================================

    vTaskDelay(
        pdMS_TO_TICKS(
            20
        )
    );


    // =================================================
    // SD DEVICE CONFIG
    // =================================================

    sdspi_device_config_t slotConfig =
        SDSPI_DEVICE_CONFIG_DEFAULT();


    slotConfig.gpio_cs =
        static_cast<gpio_num_t>(
            BOARD_SD_CS
        );


    slotConfig.host_id =
        SD_SPI_HOST;


    // =================================================
    // MOUNT
    // =================================================

    ESP_LOGI(
        TAG,
        "Calling esp_vfs_fat_sdspi_mount at %s...",
        BOARD_SD_MOUNT_POINT
    );


    result =
        esp_vfs_fat_sdspi_mount(
            BOARD_SD_MOUNT_POINT,
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
        BOARD_SD_MOUNT_POINT,
        card
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