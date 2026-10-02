#include "DS3231Clock.h"

#include "esp_err.h"
#include "esp_log.h"


namespace
{

constexpr char TAG[] =
    "DS3231Clock";

}


// =====================================================
// CONSTRUCTOR
// =====================================================

DS3231Clock::DS3231Clock(
    i2c_master_dev_handle_t device
)
    : device(device)
{
    if (
        device == nullptr
    )
    {
        ESP_LOGE(
            TAG,
            "DS3231 device handle is null"
        );
    }
}


// =====================================================
// TODAY
// =====================================================

Date DS3231Clock::today() const
{
    if (
        device == nullptr
    )
    {
        ESP_LOGE(
            TAG,
            "DS3231 device is not initialized"
        );


        return {
            0,
            0,
            0
        };
    }


    uint8_t registerAddress =
        DATE_REGISTER;


    uint8_t data[3] = {};


    const esp_err_t result =
        i2c_master_transmit_receive(
            device,
            &registerAddress,
            1,
            data,
            sizeof(data),
            100
        );


    if (
        result != ESP_OK
    )
    {
        ESP_LOGE(
            TAG,
            "Failed to read date from DS3231: %s",
            esp_err_to_name(result)
        );


        return {
            0,
            0,
            0
        };
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
        bcdToDecimal(
            rawYear
        );


    const bool century =
        (rawMonth & 0x80) != 0;


    const int year =
        2000
        +
        shortYear
        +
        (
            century
                ? 100
                : 0
        );


    ESP_LOGI(
        TAG,
        "today() -> %04d-%02d-%02d",
        year,
        month,
        day
    );


    return {
        year,
        month,
        day
    };
}


// =====================================================
// BCD
// =====================================================

int DS3231Clock::bcdToDecimal(
    uint8_t value
)
{
    return
        (
            (value >> 4)
            *
            10
        )
        +
        (
            value & 0x0F
        );
}