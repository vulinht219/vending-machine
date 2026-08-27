#include "DS3231Clock.h"

#include "esp_err.h"
#include "esp_log.h"


namespace
{
    constexpr char TAG[] = "DS3231Clock";
}


DS3231Clock::DS3231Clock(
    i2c_master_bus_handle_t bus
)
    : device(nullptr)
{
    i2c_device_config_t deviceConfig = {};

    deviceConfig.dev_addr_length =
        I2C_ADDR_BIT_LEN_7;

    deviceConfig.device_address =
        DEVICE_ADDRESS;

    deviceConfig.scl_speed_hz =
        100000;


    esp_err_t result =
        i2c_master_bus_add_device(
            bus,
            &deviceConfig,
            &device
        );


    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to add DS3231 device: %s",
            esp_err_to_name(result)
        );

        device = nullptr;
    }
}


DS3231Clock::~DS3231Clock()
{
    if (device != nullptr)
    {
        i2c_master_bus_rm_device(device);

        device = nullptr;
    }
}


Date DS3231Clock::today() const
{
    if (device == nullptr)
    {
        ESP_LOGE(
            TAG,
            "DS3231 device is not initialized"
        );

        return {0, 0, 0};
    }


    uint8_t registerAddress =
        DATE_REGISTER;

    uint8_t data[3] = {};


    esp_err_t result =
        i2c_master_transmit_receive(
            device,
            &registerAddress,
            1,
            data,
            sizeof(data),
            100
        );


    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to read date from DS3231: %s",
            esp_err_to_name(result)
        );

        return {0, 0, 0};
    }


    const uint8_t rawDay =
        data[0];

    const uint8_t rawMonth =
        data[1];

    const uint8_t rawYear =
        data[2];


    const int day =
        bcdToDecimal(
            rawDay & 0x3F
        );


    const int month =
        bcdToDecimal(
            rawMonth & 0x1F
        );


    const int shortYear =
        bcdToDecimal(rawYear);


    const bool century =
        (rawMonth & 0x80) != 0;


    const int year =
        2000
        + shortYear
        + (century ? 100 : 0);


    return {
        year,
        month,
        day
    };
}


int DS3231Clock::bcdToDecimal(
    uint8_t value
)
{
    return
        ((value >> 4) * 10)
        + (value & 0x0F);
}