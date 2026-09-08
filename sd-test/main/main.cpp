#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>

extern "C" {

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_vfs_fat.h"

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/spi_common.h"
#include "driver/sdspi_host.h"

#include "sdmmc_cmd.h"

}


// =====================================================
// CONSTANTS
// =====================================================

namespace {

constexpr const char* TAG =
    "SD_TEST";


// =====================================================
// WAVESHARE ESP32-S3-TOUCH-LCD-4.3
// MICROSD / TF CARD
// =====================================================

constexpr gpio_num_t SD_MOSI =
    GPIO_NUM_11;

constexpr gpio_num_t SD_MISO =
    GPIO_NUM_13;

constexpr gpio_num_t SD_CLK =
    GPIO_NUM_12;


// SD_CS is controlled through:
//
// CH422G EXIO4
//
// Active LOW.
//

constexpr int SD_CS_EXIO =
    4;


// =====================================================
// BOARD I2C
// =====================================================

constexpr gpio_num_t I2C_SDA =
    GPIO_NUM_8;

constexpr gpio_num_t I2C_SCL =
    GPIO_NUM_9;


constexpr i2c_port_num_t I2C_PORT =
    I2C_NUM_0;


// =====================================================
// CH422G ADDRESSES
// =====================================================
//
// 0x24:
// system control
//
// 0x38:
// IO output register
//

constexpr uint8_t CH422G_ADDR_SYSTEM =
    0x24;

constexpr uint8_t CH422G_ADDR_IO =
    0x38;


// =====================================================
// SPI
// =====================================================

constexpr spi_host_device_t SD_HOST =
    SPI2_HOST;


// =====================================================
// FILESYSTEM
// =====================================================

constexpr const char* MOUNT_POINT =
    "/sdcard";

constexpr const char* QUIZ_PATH =
    "/sdcard/quizzes.jsonl";


// =====================================================
// GLOBAL HANDLES
// =====================================================

sdmmc_card_t* card =
    nullptr;


i2c_master_bus_handle_t i2cBus =
    nullptr;


i2c_master_dev_handle_t ch422SystemDevice =
    nullptr;


i2c_master_dev_handle_t ch422IoDevice =
    nullptr;


// =====================================================
// CH422G OUTPUT STATE
// =====================================================

uint8_t ch422OutputState =
    0xFF;


// =====================================================
// INITIALIZE I2C
// =====================================================

bool initializeI2C()
{
    ESP_LOGI(
        TAG,
        "Initializing I2C for CH422G..."
    );


    i2c_master_bus_config_t busConfig = {};


    busConfig.i2c_port =
        I2C_PORT;


    busConfig.sda_io_num =
        I2C_SDA;


    busConfig.scl_io_num =
        I2C_SCL;


    busConfig.clk_source =
        I2C_CLK_SRC_DEFAULT;


    busConfig.glitch_ignore_cnt =
        7;


    busConfig.flags.enable_internal_pullup =
        true;


    esp_err_t result =
        i2c_new_master_bus(
            &busConfig,
            &i2cBus
        );


    if (
        result != ESP_OK
    )
    {
        ESP_LOGE(
            TAG,
            "i2c_new_master_bus failed: %s",
            esp_err_to_name(
                result
            )
        );


        return false;
    }


    // =================================================
    // CH422G SYSTEM DEVICE
    // =================================================

    i2c_device_config_t systemConfig = {};


    systemConfig.dev_addr_length =
        I2C_ADDR_BIT_LEN_7;


    systemConfig.device_address =
        CH422G_ADDR_SYSTEM;


    systemConfig.scl_speed_hz =
        400000;


    result =
        i2c_master_bus_add_device(
            i2cBus,
            &systemConfig,
            &ch422SystemDevice
        );


    if (
        result != ESP_OK
    )
    {
        ESP_LOGE(
            TAG,
            "Failed to add CH422G system device: %s",
            esp_err_to_name(
                result
            )
        );


        return false;
    }


    // =================================================
    // CH422G IO DEVICE
    // =================================================

    i2c_device_config_t ioConfig = {};


    ioConfig.dev_addr_length =
        I2C_ADDR_BIT_LEN_7;


    ioConfig.device_address =
        CH422G_ADDR_IO;


    ioConfig.scl_speed_hz =
        400000;


    result =
        i2c_master_bus_add_device(
            i2cBus,
            &ioConfig,
            &ch422IoDevice
        );


    if (
        result != ESP_OK
    )
    {
        ESP_LOGE(
            TAG,
            "Failed to add CH422G IO device: %s",
            esp_err_to_name(
                result
            )
        );


        return false;
    }


    ESP_LOGI(
        TAG,
        "I2C initialized"
    );


    return true;
}


// =====================================================
// ENABLE CH422G OUTPUT MODE
// =====================================================

bool enableCH422OutputMode()
{
    uint8_t value =
        0x01;


    esp_err_t result =
        i2c_master_transmit(
            ch422SystemDevice,
            &value,
            1,
            100
        );


    if (
        result != ESP_OK
    )
    {
        ESP_LOGE(
            TAG,
            "Failed to enable CH422G output mode: %s",
            esp_err_to_name(
                result
            )
        );


        return false;
    }


    ESP_LOGI(
        TAG,
        "CH422G output mode enabled"
    );


    return true;
}


// =====================================================
// WRITE CH422G OUTPUTS
// =====================================================

bool writeCH422Outputs(
    uint8_t value
)
{
    esp_err_t result =
        i2c_master_transmit(
            ch422IoDevice,
            &value,
            1,
            100
        );


    if (
        result != ESP_OK
    )
    {
        ESP_LOGE(
            TAG,
            "CH422G output write failed: %s",
            esp_err_to_name(
                result
            )
        );


        return false;
    }


    ch422OutputState =
        value;


    ESP_LOGI(
        TAG,
        "CH422G output state=0x%02X",
        ch422OutputState
    );


    return true;
}


// =====================================================
// SD CHIP SELECT
// =====================================================

bool setSDChipSelect(
    bool active
)
{
    uint8_t newState =
        ch422OutputState;


    if (
        active
    )
    {
        // Active LOW.

        newState &=
            static_cast<uint8_t>(
                ~(
                    1U
                    <<
                    SD_CS_EXIO
                )
            );
    }
    else
    {
        newState |=
            static_cast<uint8_t>(
                1U
                <<
                SD_CS_EXIO
            );
    }


    if (
        !writeCH422Outputs(
            newState
        )
    )
    {
        return false;
    }


    ESP_LOGI(
        TAG,
        "SD_CS EXIO4 = %s",
        active
            ? "LOW (ACTIVE)"
            : "HIGH (INACTIVE)"
    );


    return true;
}


// =====================================================
// BUFFERED INDEX BENCHMARK
// =====================================================

void runIndexBenchmark()
{
    ESP_LOGI(
        TAG,
        "Starting buffered index benchmark..."
    );


    FILE* quizFile =
        fopen(
            QUIZ_PATH,
            "rb"
        );


    if (
        quizFile == nullptr
    )
    {
        ESP_LOGE(
            TAG,
            "Could not open dataset for benchmark"
        );


        return;
    }


    // =================================================
    // READ 16 KiB AT A TIME
    // =====================================================

    constexpr size_t BUFFER_SIZE =
        16 * 1024;


    unsigned char buffer[
        BUFFER_SIZE
    ];


    bool lookingForLineStart =
        true;


    size_t quizCount =
        0;


    long totalBytes =
        0;


    const int64_t startTime =
        esp_timer_get_time();


    while (
        true
    )
    {
        const size_t bytesRead =
            fread(
                buffer,
                1,
                BUFFER_SIZE,
                quizFile
            );


        if (
            bytesRead == 0
        )
        {
            if (
                ferror(
                    quizFile
                )
            )
            {
                ESP_LOGE(
                    TAG,
                    "Read error during benchmark"
                );
            }


            break;
        }


        for (
            size_t i = 0;
            i < bytesRead;
            ++i
        )
        {
            const unsigned char c =
                buffer[i];


            if (
                lookingForLineStart
            )
            {
                if (
                    c == '\n'
                    ||
                    c == '\r'
                )
                {
                    continue;
                }


                ++quizCount;


                lookingForLineStart =
                    false;
            }


            if (
                c == '\n'
            )
            {
                lookingForLineStart =
                    true;
            }
        }


        totalBytes +=
            static_cast<long>(
                bytesRead
            );
    }


    const int64_t endTime =
        esp_timer_get_time();


    fclose(
        quizFile
    );


    const long long elapsedMs =
        static_cast<long long>(
            (
                endTime
                -
                startTime
            )
            /
            1000
        );


    ESP_LOGI(
        TAG,
        "Buffered index benchmark finished"
    );


    ESP_LOGI(
        TAG,
        "Quiz count: %u",
        static_cast<unsigned>(
            quizCount
        )
    );


    ESP_LOGI(
        TAG,
        "Bytes scanned: %ld",
        totalBytes
    );


    ESP_LOGI(
        TAG,
        "Index time: %lld ms",
        elapsedMs
    );
}


} // namespace


// =====================================================
// APP MAIN
// =====================================================

extern "C" void app_main()
{
    ESP_LOGI(
        TAG,
        "================================"
    );


    ESP_LOGI(
        TAG,
        "Waveshare SD-only test started"
    );


    ESP_LOGI(
        TAG,
        "Official SD pins: MOSI=%d MISO=%d CLK=%d",
        SD_MOSI,
        SD_MISO,
        SD_CLK
    );


    ESP_LOGI(
        TAG,
        "SD CS = CH422G EXIO4"
    );


    // =================================================
    // INITIALIZE I2C + CH422G
    // =================================================

    if (
        !initializeI2C()
    )
    {
        return;
    }


    if (
        !enableCH422OutputMode()
    )
    {
        return;
    }


    // Start with all outputs HIGH.

    if (
        !writeCH422Outputs(
            0xFF
        )
    )
    {
        return;
    }


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
        "Calling spi_bus_initialize..."
    );


    esp_err_t result =
        spi_bus_initialize(
            SD_HOST,
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
        result != ESP_OK
    )
    {
        ESP_LOGE(
            TAG,
            "SPI initialization failed"
        );


        return;
    }


    // =================================================
    // ACTIVATE SD CS
    // =================================================

    if (
        !setSDChipSelect(
            true
        )
    )
    {
        return;
    }


    vTaskDelay(
        pdMS_TO_TICKS(
            20
        )
    );


    // =================================================
    // SD HOST CONFIG
    // =================================================

    sdmmc_host_t host =
        SDSPI_HOST_DEFAULT();


    host.slot =
        SD_HOST;


    sdspi_device_config_t slotConfig =
        SDSPI_DEVICE_CONFIG_DEFAULT();


    slotConfig.host_id =
        SD_HOST;


    // CS is controlled externally by CH422G.

    slotConfig.gpio_cs =
        SDSPI_SLOT_NO_CS;


    // =================================================
    // FAT CONFIG
    // =================================================

    esp_vfs_fat_sdmmc_mount_config_t mountConfig = {};


    mountConfig.format_if_mount_failed =
        false;


    mountConfig.max_files =
        5;


    mountConfig.allocation_unit_size =
        16 * 1024;


    // =================================================
    // MOUNT SD
    // =================================================

    ESP_LOGI(
        TAG,
        "Calling esp_vfs_fat_sdspi_mount..."
    );


    result =
        esp_vfs_fat_sdspi_mount(
            MOUNT_POINT,
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
        ESP_LOGE(
            TAG,
            "SD mount failed"
        );


        return;
    }


    // =================================================
    // CARD INFO
    // =================================================

    ESP_LOGI(
        TAG,
        "SD CARD MOUNTED SUCCESSFULLY"
    );


    sdmmc_card_print_info(
        stdout,
        card
    );


    // =================================================
    // CHECK DATASET
    // =================================================

    struct stat fileStat;


    if (
        stat(
            QUIZ_PATH,
            &fileStat
        )
        ==
        0
    )
    {
        ESP_LOGI(
            TAG,
            "DATASET FOUND"
        );


        ESP_LOGI(
            TAG,
            "Path: %s",
            QUIZ_PATH
        );


        ESP_LOGI(
            TAG,
            "Size: %ld bytes",
            static_cast<long>(
                fileStat.st_size
            )
        );
    }
    else
    {
        ESP_LOGE(
            TAG,
            "Dataset NOT found at %s",
            QUIZ_PATH
        );


        return;
    }


    // =================================================
    // BENCHMARK INDEX BUILD
    // =================================================

    runIndexBenchmark();


    // =================================================
    // DONE
    // =================================================

    ESP_LOGI(
        TAG,
        "SD TEST COMPLETE"
    );


    while (
        true
    )
    {
        vTaskDelay(
            pdMS_TO_TICKS(
                1000
            )
        );
    }
}