#include "CH422GManager.h"
#include "BoardI2CManager.h"

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"

#include <cstdint>

namespace
{
    constexpr const char* TAG =
        "CH422GManager";

    constexpr uint8_t CH422G_MODE_ADDRESS =
        0x24;

    constexpr uint8_t CH422G_OUTPUT_ADDRESS =
        0x38;

    i2c_master_dev_handle_t deviceMode =
        nullptr;

    i2c_master_dev_handle_t deviceOutput =
        nullptr;


    bool addDevice(
        uint8_t address,
        i2c_master_dev_handle_t* device
    )
    {
        i2c_device_config_t config = {};

        config.dev_addr_length =
            I2C_ADDR_BIT_LEN_7;

        config.device_address =
            address;

        config.scl_speed_hz =
            400000;


        esp_err_t result =
            i2c_master_bus_add_device(
                BoardI2CManager::getBus(),
                &config,
                device
            );


        if (result != ESP_OK)
        {
            ESP_LOGE(
                TAG,
                "Failed to add CH422G address 0x%02X: %s",
                address,
                esp_err_to_name(result)
            );

            return false;
        }

        return true;
    }


    bool writeByte(
        i2c_master_dev_handle_t device,
        uint8_t value
    )
    {
        esp_err_t result =
            i2c_master_transmit(
                device,
                &value,
                1,
                100
            );

        return result == ESP_OK;
    }
}


bool CH422GManager::initialized =
    false;

unsigned char CH422GManager::outputState =
    0x00;


bool CH422GManager::initialize()
{
    if (initialized)
    {
        return true;
    }


    if (!BoardI2CManager::initialize())
    {
        return false;
    }


    if (!addDevice(
            CH422G_MODE_ADDRESS,
            &deviceMode
        ))
    {
        return false;
    }


    if (!addDevice(
            CH422G_OUTPUT_ADDRESS,
            &deviceOutput
        ))
    {
        return false;
    }


    uint8_t mode =
        0x01;


    if (!writeByte(
            deviceMode,
            mode
        ))
    {
        ESP_LOGE(
            TAG,
            "Failed to configure CH422G output mode"
        );

        return false;
    }


    // Preserve the value used by the Waveshare SD demo.
    outputState =
        0x0A;


    if (!writeByte(
            deviceOutput,
            outputState
        ))
    {
        ESP_LOGE(
            TAG,
            "Failed to initialize CH422G outputs"
        );

        return false;
    }


    initialized =
        true;


    ESP_LOGI(
        TAG,
        "CH422G initialized"
    );


    return true;
}


bool CH422GManager::setOutput(
    int exio,
    bool high
)
{
    if (
        exio < 0 ||
        exio > 7
    )
    {
        return false;
    }


    if (!initialize())
    {
        return false;
    }


    uint8_t newState =
        outputState;


    if (high)
    {
        newState |=
            static_cast<uint8_t>(
                1U << exio
            );
    }
    else
    {
        newState &=
            static_cast<uint8_t>(
                ~(1U << exio)
            );
    }


    if (!writeByte(
            deviceOutput,
            newState
        ))
    {
        ESP_LOGE(
            TAG,
            "Failed to set EXIO%d",
            exio
        );

        return false;
    }


    outputState =
        newState;


    return true;
}