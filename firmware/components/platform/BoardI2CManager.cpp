#include "BoardI2CManager.h"
#include "BoardPins.h"

#include "esp_err.h"
#include "esp_log.h"

namespace
{
    constexpr const char* TAG =
        "BoardI2CManager";
}

i2c_master_bus_handle_t
    BoardI2CManager::bus = nullptr;


bool BoardI2CManager::initialize()
{
    if (bus != nullptr)
    {
        return true;
    }

    i2c_master_bus_config_t config = {};

    config.i2c_port =
        static_cast<i2c_port_num_t>(
            BOARD_I2C_PORT
        );

    config.sda_io_num =
        static_cast<gpio_num_t>(
            BOARD_I2C_SDA
        );

    config.scl_io_num =
        static_cast<gpio_num_t>(
            BOARD_I2C_SCL
        );

    config.clk_source =
        I2C_CLK_SRC_DEFAULT;

    config.glitch_ignore_cnt =
        7;

    config.flags.enable_internal_pullup =
        true;


    esp_err_t result =
        i2c_new_master_bus(
            &config,
            &bus
        );


    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to initialize I2C bus: %s",
            esp_err_to_name(result)
        );

        bus = nullptr;

        return false;
    }


    ESP_LOGI(
        TAG,
        "I2C initialized: SDA=%d SCL=%d",
        BOARD_I2C_SDA,
        BOARD_I2C_SCL
    );


    return true;
}


i2c_master_bus_handle_t
BoardI2CManager::getBus()
{
    return bus;
}