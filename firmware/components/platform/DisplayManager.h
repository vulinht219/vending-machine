#pragma once

#include "esp_lcd_panel_ops.h"



class DisplayManager
{
public:

    // Initialize the RGB LCD hardware.
    //
    // Does not initialize LVGL or touch.
    static bool initialize();


    // Run a raw RGB565 screen test.
    //
    // This test does not use LVGL.
    static bool runSelfTest();


    // Control LCD backlight through CH422G.
    static bool setBacklight(
        bool enabled
    );


    // Native ESP-IDF LCD handle.
    static esp_lcd_panel_handle_t getPanel();


    static bool isInitialized();


private:

    static bool initialized;

    static esp_lcd_panel_handle_t panel;
};