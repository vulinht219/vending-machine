#include "CH422GManager.h"

#include "BoardI2CManager.h"

extern "C" {

#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c_master.h"

}


namespace {

constexpr const char* TAG =
    "CH422GManager";


// CH422G uses command-style I2C addresses.
//
// Waveshare official examples write:
//
// 0x24 -> mode/configuration
// 0x38 -> output state
//
constexpr uint8_t CH422G_MODE_ADDRESS =
    0x24;

constexpr uint8_t CH422G_OUTPUT_ADDRESS =
    0x38;


i2c_master_dev_handle_t modeDevice =
    nullptr;

i2c_master_dev_handle_t outputDevice =
    nullptr;


bool addDevice(
    uint8_t address,
    i2c_master_dev_handle_t* handle
)
{
    if (*handle != nullptr)
    {
        return true;
    }


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
            handle
        );


    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to add CH422G device 0x%02X: %s",
            address,
            esp_err_to_name(result)
        );

        return false;
    }


    return true;
}

} // namespace


bool CH422GManager::initialized =
    false;


// Initial output state.
//
// This preserves the state previously used successfully
// for SD initialization.
uint8_t CH422GManager::outputState =
    0x0A;


// =====================================================
// INITIALIZE
// =====================================================

bool CH422GManager::initialize()
{
    if (initialized)
    {
        return true;
    }


    if (!BoardI2CManager::initialize())
    {
        ESP_LOGE(
            TAG,
            "Board I2C initialization failed"
        );

        return false;
    }


    if (!addDevice(
            CH422G_MODE_ADDRESS,
            &modeDevice
        ))
    {
        return false;
    }


    if (!addDevice(
            CH422G_OUTPUT_ADDRESS,
            &outputDevice
        ))
    {
        return false;
    }


    // =================================================
    // ENABLE OUTPUT MODE
    // =================================================

    uint8_t mode =
        0x01;


    esp_err_t result =
        i2c_master_transmit(
            modeDevice,
            &mode,
            1,
            100
        );


    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to configure CH422G output mode: %s",
            esp_err_to_name(result)
        );

        return false;
    }


    // =================================================
    // INITIAL OUTPUT STATE
    // =================================================

    result =
        i2c_master_transmit(
            outputDevice,
            &outputState,
            1,
            100
        );


    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to write initial CH422G state: %s",
            esp_err_to_name(result)
        );

        return false;
    }


    initialized =
        true;


    ESP_LOGI(
        TAG,
        "CH422G initialized, output state=0x%02X",
        outputState
    );


    return true;
}


// =====================================================
// SET OUTPUT
// =====================================================

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
        ESP_LOGE(
            TAG,
            "Invalid CH422G EXIO index: %d",
            exio
        );

        return false;
    }


    if (!initialize())
    {
        return false;
    }


    const uint8_t mask =
        static_cast<uint8_t>(
            1U << exio
        );


    const uint8_t oldState =
        outputState;


    if (high)
    {
        outputState =
            static_cast<uint8_t>(
                outputState |
                mask
            );
    }
    else
    {
        outputState =
            static_cast<uint8_t>(
                outputState &
                static_cast<uint8_t>(
                    ~mask
                )
            );
    }


    if (outputState == oldState)
    {
        return true;
    }


    if (!writeOutputState())
    {
        outputState =
            oldState;

        return false;
    }


    ESP_LOGI(
        TAG,
        "EXIO%d=%d, output state=0x%02X",
        exio,
        high ? 1 : 0,
        outputState
    );


    return true;
}


// =====================================================
// SET COMPLETE OUTPUT STATE
// =====================================================

bool CH422GManager::setOutputState(
    uint8_t state
)
{
    if (!initialize())
    {
        return false;
    }


    const uint8_t oldState =
        outputState;


    outputState =
        state;


    if (!writeOutputState())
    {
        outputState =
            oldState;

        return false;
    }


    ESP_LOGI(
        TAG,
        "CH422G output state=0x%02X",
        outputState
    );


    return true;
}


// =====================================================
// WRITE CURRENT STATE
// =====================================================

bool CH422GManager::writeOutputState()
{
    esp_err_t result =
        i2c_master_transmit(
            outputDevice,
            &outputState,
            1,
            100
        );


    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to write CH422G state 0x%02X: %s",
            outputState,
            esp_err_to_name(result)
        );

        return false;
    }


    return true;
}


// =====================================================
// GET OUTPUT STATE
// =====================================================

uint8_t CH422GManager::getOutputState()
{
    return outputState;
}


// =====================================================
// STATUS
// =====================================================

bool CH422GManager::isInitialized()
{
    return initialized;
}