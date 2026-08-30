#include "DisplayManager.h"

#include "BoardPins.h"
#include "CH422GManager.h"

extern "C" {

#include "esp_err.h"
#include "esp_log.h"

#include "driver/gpio.h"

#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_heap_caps.h"

}


namespace {

constexpr const char* TAG =
    "DisplayManager";


constexpr int LCD_BOUNCE_BUFFER_HEIGHT =
    10;

constexpr int LCD_BOUNCE_BUFFER_SIZE =
    BOARD_LCD_WIDTH *
    LCD_BOUNCE_BUFFER_HEIGHT;

} // namespace


bool DisplayManager::initialized =
    false;


esp_lcd_panel_handle_t DisplayManager::panel =
    nullptr;


// =====================================================
// INITIALIZE
// =====================================================

bool DisplayManager::initialize()
{
    if (initialized)
    {
        ESP_LOGI(
            TAG,
            "Display already initialized"
        );

        return true;
    }


    ESP_LOGI(
        TAG,
        "Initializing RGB LCD %dx%d",
        BOARD_LCD_WIDTH,
        BOARD_LCD_HEIGHT
    );


    // =================================================
    // CH422G
    // =================================================

    if (!CH422GManager::initialize())
    {
        ESP_LOGE(
            TAG,
            "CH422G initialization failed"
        );

        return false;
    }


    // Keep backlight off while LCD is being initialized.
    if (!setBacklight(false))
    {
        ESP_LOGE(
            TAG,
            "Failed to disable LCD backlight"
        );

        return false;
    }


    // =================================================
    // RGB PANEL CONFIGURATION
    // =================================================

    esp_lcd_rgb_panel_config_t config = {};


    config.clk_src =
        LCD_CLK_SRC_DEFAULT;


    // =================================================
    // TIMING
    // =================================================

    config.timings.pclk_hz =
        BOARD_LCD_PIXEL_CLOCK_HZ;

    config.timings.h_res =
        BOARD_LCD_WIDTH;

    config.timings.v_res =
        BOARD_LCD_HEIGHT;


    config.timings.hsync_pulse_width =
        4;

    config.timings.hsync_back_porch =
        8;

    config.timings.hsync_front_porch =
        8;


    config.timings.vsync_pulse_width =
        4;

    config.timings.vsync_back_porch =
        8;

    config.timings.vsync_front_porch =
        8;


    config.timings.flags.pclk_active_neg =
        1;


    // =================================================
    // RGB FORMAT
    // =================================================

    //
    // 16 RGB data lines.
    //
    // With the current ESP-IDF RGB driver, data_width=16
    // defaults to RGB565 when no explicit color format is
    // provided.
    //
    config.data_width =
        16;


    // One full framebuffer.
    config.num_fbs =
        1;


    // Bounce buffer in internal RAM.
    config.bounce_buffer_size_px =
        LCD_BOUNCE_BUFFER_SIZE;


    // =================================================
    // CONTROL PINS
    // =================================================

    config.hsync_gpio_num =
        static_cast<gpio_num_t>(
            BOARD_LCD_HSYNC
        );

    config.vsync_gpio_num =
        static_cast<gpio_num_t>(
            BOARD_LCD_VSYNC
        );

    config.de_gpio_num =
        static_cast<gpio_num_t>(
            BOARD_LCD_DE
        );

    config.pclk_gpio_num =
        static_cast<gpio_num_t>(
            BOARD_LCD_PCLK
        );

    config.disp_gpio_num =
        static_cast<gpio_num_t>(
            BOARD_LCD_DISP
        );


    // =================================================
    // RGB DATA PINS
    // =================================================

    config.data_gpio_nums[0] =
        static_cast<gpio_num_t>(
            BOARD_LCD_DATA0
        );

    config.data_gpio_nums[1] =
        static_cast<gpio_num_t>(
            BOARD_LCD_DATA1
        );

    config.data_gpio_nums[2] =
        static_cast<gpio_num_t>(
            BOARD_LCD_DATA2
        );

    config.data_gpio_nums[3] =
        static_cast<gpio_num_t>(
            BOARD_LCD_DATA3
        );

    config.data_gpio_nums[4] =
        static_cast<gpio_num_t>(
            BOARD_LCD_DATA4
        );

    config.data_gpio_nums[5] =
        static_cast<gpio_num_t>(
            BOARD_LCD_DATA5
        );

    config.data_gpio_nums[6] =
        static_cast<gpio_num_t>(
            BOARD_LCD_DATA6
        );

    config.data_gpio_nums[7] =
        static_cast<gpio_num_t>(
            BOARD_LCD_DATA7
        );

    config.data_gpio_nums[8] =
        static_cast<gpio_num_t>(
            BOARD_LCD_DATA8
        );

    config.data_gpio_nums[9] =
        static_cast<gpio_num_t>(
            BOARD_LCD_DATA9
        );

    config.data_gpio_nums[10] =
        static_cast<gpio_num_t>(
            BOARD_LCD_DATA10
        );

    config.data_gpio_nums[11] =
        static_cast<gpio_num_t>(
            BOARD_LCD_DATA11
        );

    config.data_gpio_nums[12] =
        static_cast<gpio_num_t>(
            BOARD_LCD_DATA12
        );

    config.data_gpio_nums[13] =
        static_cast<gpio_num_t>(
            BOARD_LCD_DATA13
        );

    config.data_gpio_nums[14] =
        static_cast<gpio_num_t>(
            BOARD_LCD_DATA14
        );

    config.data_gpio_nums[15] =
        static_cast<gpio_num_t>(
            BOARD_LCD_DATA15
        );


    // =================================================
    // FRAMEBUFFER
    // =================================================

    config.flags.fb_in_psram =
        1;


    // =================================================
    // CREATE PANEL
    // =================================================

    esp_err_t result =
        esp_lcd_new_rgb_panel(
            &config,
            &panel
        );


    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "esp_lcd_new_rgb_panel failed: %s",
            esp_err_to_name(result)
        );

        panel =
            nullptr;

        return false;
    }


    // =================================================
    // INITIALIZE PANEL
    // =================================================

    result =
        esp_lcd_panel_init(
            panel
        );


    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "esp_lcd_panel_init failed: %s",
            esp_err_to_name(result)
        );


        esp_lcd_panel_del(
            panel
        );


        panel =
            nullptr;

        return false;
    }


    initialized =
        true;


    ESP_LOGI(
        TAG,
        "RGB LCD initialized successfully"
    );


    return true;
}

// =====================================================
// SELF TEST
// =====================================================

bool DisplayManager::runSelfTest()
{
    if (!initialized)
    {
        ESP_LOGE(
            TAG,
            "Cannot run LCD self-test: display not initialized"
        );

        return false;
    }


    constexpr size_t PIXEL_COUNT =
        static_cast<size_t>(
            BOARD_LCD_WIDTH
        ) *
        static_cast<size_t>(
            BOARD_LCD_HEIGHT
        );


    constexpr size_t BUFFER_SIZE =
        PIXEL_COUNT *
        sizeof(uint16_t);


    ESP_LOGI(
        TAG,
        "Allocating %u bytes for LCD self-test",
        static_cast<unsigned int>(
            BUFFER_SIZE
        )
    );


    auto* framebuffer =
        static_cast<uint16_t*>(
            heap_caps_malloc(
                BUFFER_SIZE,
                MALLOC_CAP_SPIRAM |
                MALLOC_CAP_8BIT
            )
        );


    if (framebuffer == nullptr)
    {
        ESP_LOGE(
            TAG,
            "Failed to allocate LCD self-test framebuffer in PSRAM"
        );

        return false;
    }


    // =================================================
    // RGB565 TEST BARS
    // =================================================
    //
    // Screen:
    //
    // +------------+------------+------------+
    // |    RED     |   GREEN    |    BLUE    |
    // +------------+------------+------------+
    //
    // Bottom strip:
    //
    // WHITE | BLACK
    //

    constexpr uint16_t COLOR_RED =
        0xF800;

    constexpr uint16_t COLOR_GREEN =
        0x07E0;

    constexpr uint16_t COLOR_BLUE =
        0x001F;

    constexpr uint16_t COLOR_WHITE =
        0xFFFF;

    constexpr uint16_t COLOR_BLACK =
        0x0000;


    constexpr int BOTTOM_STRIP_HEIGHT =
        80;


    const int mainHeight =
        BOARD_LCD_HEIGHT -
        BOTTOM_STRIP_HEIGHT;


    for (
        int y = 0;
        y < BOARD_LCD_HEIGHT;
        ++y
    )
    {
        for (
            int x = 0;
            x < BOARD_LCD_WIDTH;
            ++x
        )
        {
            uint16_t color;


            if (y >= mainHeight)
            {
                if (x < BOARD_LCD_WIDTH / 2)
                {
                    color =
                        COLOR_WHITE;
                }
                else
                {
                    color =
                        COLOR_BLACK;
                }
            }
            else
            {
                const int section =
                    (
                        x * 3
                    ) /
                    BOARD_LCD_WIDTH;


                if (section == 0)
                {
                    color =
                        COLOR_RED;
                }
                else if (section == 1)
                {
                    color =
                        COLOR_GREEN;
                }
                else
                {
                    color =
                        COLOR_BLUE;
                }
            }


            framebuffer[
                static_cast<size_t>(y) *
                BOARD_LCD_WIDTH +
                x
            ] =
                color;
        }
    }


    ESP_LOGI(
        TAG,
        "Drawing LCD self-test pattern"
    );


    esp_err_t result =
        esp_lcd_panel_draw_bitmap(
            panel,
            0,
            0,
            BOARD_LCD_WIDTH,
            BOARD_LCD_HEIGHT,
            framebuffer
        );


    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "esp_lcd_panel_draw_bitmap failed: %s",
            esp_err_to_name(result)
        );


        heap_caps_free(
            framebuffer
        );


        return false;
    }


    //
    // esp_lcd_panel_draw_bitmap() has now transferred /
    // copied the test image into the RGB panel buffer.
    //
    heap_caps_free(
        framebuffer
    );


    ESP_LOGI(
        TAG,
        "LCD self-test pattern drawn successfully"
    );


    return true;
}

// =====================================================
// BACKLIGHT
// =====================================================

bool DisplayManager::setBacklight(
    bool enabled
)
{
    if (!CH422GManager::initialize())
    {
        return false;
    }


    if (!CH422GManager::setOutput(
            2,
            enabled
        ))
    {
        ESP_LOGE(
            TAG,
            "Failed to set LCD backlight"
        );

        return false;
    }


    ESP_LOGI(
        TAG,
        "LCD backlight %s",
        enabled
            ? "ON"
            : "OFF"
    );


    return true;
}


// =====================================================
// PANEL HANDLE
// =====================================================

esp_lcd_panel_handle_t DisplayManager::getPanel()
{
    return panel;
}


// =====================================================
// STATUS
// =====================================================

bool DisplayManager::isInitialized()
{
    return initialized;
}