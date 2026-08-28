#pragma once

#include "driver/i2c_master.h"

class BoardI2CManager
{
public:
    static bool initialize();

    static i2c_master_bus_handle_t getBus();

private:
    static i2c_master_bus_handle_t bus;
};