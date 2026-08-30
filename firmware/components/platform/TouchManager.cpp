#include "TouchManager.h"

#include "BoardPins.h"
#include "BoardI2CManager.h"
#include "CH422GManager.h"

extern "C" {

#include "esp_err.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/i2c_master.h"

#include "esp_lcd_io_i2c.h"
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_gt911.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

}


namespace {

constexpr const char* TAG =
    "TouchManager";


esp_lcd_panel_io_handle_t touchIO =
    nullptr;

} // namespace


bool TouchManager::initialized =
    false;


esp_lcd_touch_handle_t TouchManager::touchHandle =
    nullptr;


// =====================================================
// RESET GT911
// =====================================================

bool TouchManager::resetController()
{
    if (!CH422GManager::initialize())
    {
        ESP_LOGE(
            TAG,
            "CH422G initialization failed"
        );

        return false;
    }


    // =================================================
    // GPIO4 OUTPUT
    // =================================================

    gpio_config_t gpioConfig = {};

    gpioConfig.pin_bit_mask =
        1ULL <<
        BOARD_TOUCH_INT;

    gpioConfig.mode =
        GPIO_MODE_OUTPUT;

    gpioConfig.pull_up_en =
        GPIO_PULLUP_DISABLE;

    gpioConfig.pull_down_en =
        GPIO_PULLDOWN_DISABLE;

    gpioConfig.intr_type =
        GPIO_INTR_DISABLE;


    esp_err_t result =
        gpio_config(
            &gpioConfig
        );


    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to configure touch GPIO4: %s",
            esp_err_to_name(result)
        );

        return false;
    }


    // =================================================
    // WAVESHARE GT911 RESET SEQUENCE
    // =================================================

    //
    // Official sequence:
    //
    // CH422G EXIO1 LOW
    // wait
    //
    // GPIO4 LOW
    // wait
    //
    // CH422G EXIO1 HIGH
    // wait
    //

    if (!CH422GManager::setOutput(
            BOARD_TOUCH_RST_EXIO,
            false
        ))
    {
        return false;
    }


    vTaskDelay(
        pdMS_TO_TICKS(
            100
        )
    );


    result =
        gpio_set_level(
            static_cast<gpio_num_t>(
                BOARD_TOUCH_INT
            ),
            0
        );


    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to drive touch INT low: %s",
            esp_err_to_name(result)
        );

        return false;
    }


    vTaskDelay(
        pdMS_TO_TICKS(
            100
        )
    );


    if (!CH422GManager::setOutput(
            BOARD_TOUCH_RST_EXIO,
            true
        ))
    {
        return false;
    }


    vTaskDelay(
        pdMS_TO_TICKS(
            200
        )
    );


    // =================================================
    // RELEASE GPIO4
    // =================================================
    //
    // After address-selection/reset, GT911 INT should no
    // longer be actively driven by ESP32.
    //

    result =
        gpio_reset_pin(
            static_cast<gpio_num_t>(
                BOARD_TOUCH_INT
            )
        );


    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to release touch INT GPIO: %s",
            esp_err_to_name(result)
        );

        return false;
    }


    ESP_LOGI(
        TAG,
        "GT911 reset sequence completed"
    );


    return true;
}


// =====================================================
// INITIALIZE
// =====================================================

bool TouchManager::initialize()
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


    if (!resetController())
    {
        ESP_LOGE(
            TAG,
            "GT911 reset failed"
        );

        return false;
    }


    // =================================================
    // GT911 I2C PANEL IO
    // =================================================

    esp_lcd_panel_io_i2c_config_t ioConfig = {};

    ioConfig.dev_addr =
        ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS;

    ioConfig.scl_speed_hz =
        100000;

    ioConfig.control_phase_bytes =
        1;

    ioConfig.dc_bit_offset =
        0;

    ioConfig.lcd_cmd_bits =
        16;

    ioConfig.flags.disable_control_phase =
        1;


    esp_err_t result =
        esp_lcd_new_panel_io_i2c(
            BoardI2CManager::getBus(),
            &ioConfig,
            &touchIO
        );


    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to create GT911 panel IO: %s",
            esp_err_to_name(result)
        );

        return false;
    }


    // =================================================
    // TOUCH CONFIG
    // =================================================

    esp_lcd_touch_config_t touchConfig = {};


    touchConfig.x_max =
        BOARD_TOUCH_WIDTH;

    touchConfig.y_max =
        BOARD_TOUCH_HEIGHT;


    //
    // Reset is handled manually through CH422G.
    //
    touchConfig.rst_gpio_num =
        GPIO_NUM_NC;


    //
    // We will poll GT911 initially.
    //
    // No interrupt pin is required for the first test.
    //
    touchConfig.int_gpio_num =
        GPIO_NUM_NC;


    touchConfig.levels.reset =
        0;

    touchConfig.levels.interrupt =
        0;


    touchConfig.flags.swap_xy =
        0;

    touchConfig.flags.mirror_x =
        0;

    touchConfig.flags.mirror_y =
        0;

    esp_lcd_touch_io_gt911_config_t gt911Config = {};

    gt911Config.dev_addr =
        ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS;


    touchConfig.driver_data =
        &gt911Config;

    result =
        esp_lcd_touch_new_i2c_gt911(
            touchIO,
            &touchConfig,
            &touchHandle
        );


    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "GT911 initialization failed: %s",
            esp_err_to_name(result)
        );

        touchHandle =
            nullptr;

        return false;
    }


    initialized =
        true;


    ESP_LOGI(
        TAG,
        "GT911 initialized successfully"
    );


    return true;
}


// =====================================================
// READ
// =====================================================

bool TouchManager::read(
    uint16_t& x,
    uint16_t& y
)
{
    if (!initialized)
    {
        return false;
    }


    esp_err_t result =
        esp_lcd_touch_read_data(
            touchHandle
        );


    if (result != ESP_OK)
    {
        ESP_LOGW(
            TAG,
            "GT911 read failed: %s",
            esp_err_to_name(result)
        );

        return false;
    }


    esp_lcd_touch_point_data_t pointData[1] = {};

    uint8_t touchCount =
        0;


    result =
        esp_lcd_touch_get_data(
            touchHandle,
            pointData,
            &touchCount,
            1
        );


    if (result != ESP_OK)
    {
        ESP_LOGW(
            TAG,
            "Failed to get GT911 touch data: %s",
            esp_err_to_name(result)
        );

        return false;
    }


    if (touchCount == 0)
    {
        return false;
    }


    x =
        pointData[0].x;

    y =
        pointData[0].y;


    return true;
}


// =====================================================
// HANDLE
// =====================================================

esp_lcd_touch_handle_t TouchManager::getHandle()
{
    return touchHandle;
}


// =====================================================
// STATUS
// =====================================================

bool TouchManager::isInitialized()
{
    return initialized;
}


// =====================================================
// SELF TEST
// =====================================================

void TouchManager::runSelfTest()
{
    if (!initialized)
    {
        ESP_LOGE(
            TAG,
            "Cannot run touch self-test: GT911 not initialized"
        );

        return;
    }


    ESP_LOGI(
        TAG,
        "GT911 touch self-test started"
    );


    while (true)
    {
        uint16_t x = 0;
        uint16_t y = 0;


        if (read(x, y))
        {
            ESP_LOGI(
                TAG,
                "TOUCH x=%u y=%u",
                static_cast<unsigned int>(x),
                static_cast<unsigned int>(y)
            );
        }


        vTaskDelay(
            pdMS_TO_TICKS(20)
        );
    }
}