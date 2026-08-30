#pragma once

#include <cstdint>

#include "esp_lcd_touch.h"


class TouchManager
{
public:

    // Initialize GT911 touch controller.
    static bool initialize();


    // Poll touch controller.
    //
    // Returns true when at least one point is pressed.
    static bool read(
        uint16_t& x,
        uint16_t& y
    );

    static void runSelfTest();


    static bool isInitialized();


    static esp_lcd_touch_handle_t getHandle();


private:

    static bool initialized;

    static esp_lcd_touch_handle_t touchHandle;

    static bool resetController();
};