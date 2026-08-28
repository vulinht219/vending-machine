#include "SDCardManager.h"
#include "BoardPins.h"
#include "CH422GManager.h"

#include <stdio.h>

extern "C" {

#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"

#include "driver/spi_common.h"
#include "driver/sdspi_host.h"

#include "sdmmc_cmd.h"
}


namespace {

constexpr const char* TAG =
    "SDCardManager";

sdmmc_card_t* card =
    nullptr;

sdmmc_host_t host =
    SDSPI_HOST_DEFAULT();

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


    // =================================================
    // CH422G
    // =================================================

    if (!CH422GManager::initialize())
    {
        ESP_LOGE(
            TAG,
            "CH422G initialization failed"
        );

        return false;
    }


    // =================================================
    // FAT FILESYSTEM CONFIG
    // =================================================

    esp_vfs_fat_sdmmc_mount_config_t
        mountConfig = {};

    // Never automatically format
    // production quiz cards.
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
        BOARD_SD_MOSI;

    busConfig.miso_io_num =
        BOARD_SD_MISO;

    busConfig.sclk_io_num =
        BOARD_SD_CLK;

    busConfig.quadwp_io_num =
        -1;

    busConfig.quadhd_io_num =
        -1;

    busConfig.max_transfer_sz =
        4000;


    esp_err_t result =
        spi_bus_initialize(
            static_cast<spi_host_device_t>(
                host.slot
            ),
            &busConfig,
            SDSPI_DEFAULT_DMA
        );


    if (
        result != ESP_OK &&
        result != ESP_ERR_INVALID_STATE
    )
    {
        ESP_LOGE(
            TAG,
            "SPI bus initialization failed: %s",
            esp_err_to_name(result)
        );

        return false;
    }


    // =================================================
    // SD DEVICE
    // =================================================

    sdspi_device_config_t slotConfig =
        SDSPI_DEVICE_CONFIG_DEFAULT();


    slotConfig.gpio_cs =
        static_cast<gpio_num_t>(
            BOARD_SD_CS
        );

    slotConfig.host_id =
        static_cast<spi_host_device_t>(
            host.slot
        );


    ESP_LOGI(
        TAG,
        "Mounting SD card at %s",
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


    if (result != ESP_OK)
    {
        if (result == ESP_FAIL)
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
                esp_err_to_name(result)
            );
        }


        return false;
    }


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
    if (!mounted)
    {
        return;
    }


    esp_vfs_fat_sdcard_unmount(
        BOARD_SD_MOUNT_POINT,
        card
    );


    spi_bus_free(
        static_cast<spi_host_device_t>(
            host.slot
        )
    );


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