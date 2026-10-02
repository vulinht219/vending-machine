#pragma once

#include <cstdint>

#include "time/IClock.h"
#include "driver/i2c_master.h"


class DS3231Clock : public IClock
{
public:

    explicit DS3231Clock(
        i2c_master_dev_handle_t device
    );


    Date today() const override;


private:

    static constexpr uint8_t DATE_REGISTER = 0x04;


    i2c_master_dev_handle_t device;


    static int bcdToDecimal(
        uint8_t value
    );
};