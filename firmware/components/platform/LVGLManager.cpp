#include "LVGLManager.h"

#include "DisplayManager.h"
#include "TouchManager.h"

#include "esp_log.h"
#include "esp_lcd_panel_ops.h"
#include "esp_heap_caps.h"

#include "lvgl.h"

#include <cstdint>


extern "C" {

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_timer.h"

}


namespace {

constexpr const char* TAG =
    "LVGLManager";


// =====================================================
// DISPLAY DIMENSIONS
// =====================================================
//
// LVGL/UI:
//     480 x 800 portrait
//
// Physical Waveshare RGB LCD:
//     800 x 480 landscape
//

constexpr int LOGICAL_WIDTH =
    480;

constexpr int LOGICAL_HEIGHT =
    800;

constexpr int PHYSICAL_WIDTH =
    800;

constexpr int PHYSICAL_HEIGHT =
    480;


// =====================================================
// FULL FRAME SIZE
// =====================================================
//
// RGB565:
//
// 480 * 800 * 2
// = 768000 bytes
//
// We allocate:
//
// - one full LVGL portrait framebuffer
// - one full rotated physical framebuffer
//
// Total:
// ~1.5 MB PSRAM
//

constexpr size_t FRAME_PIXELS =
    static_cast<size_t>(
        LOGICAL_WIDTH
    ) *
    static_cast<size_t>(
        LOGICAL_HEIGHT
    );


constexpr size_t FRAME_BYTES =
    FRAME_PIXELS *
    sizeof(uint16_t);


// =====================================================
// LVGL OBJECTS
// =====================================================

lv_display_t* display =
    nullptr;

lv_indev_t* touchInput =
    nullptr;


// =====================================================
// FRAME BUFFERS
// =====================================================

uint16_t* drawBuffer =
    nullptr;

uint16_t* rotatedBuffer =
    nullptr;


// =====================================================
// LVGL TICK
// =====================================================

uint32_t getTickMilliseconds()
{
    return static_cast<uint32_t>(
        esp_timer_get_time() /
        1000ULL
    );
}


// =====================================================
// DISPLAY FLUSH
// =====================================================
//
// LVGL produces one complete:
//     480 x 800
//
// framebuffer.
//
// We rotate that complete frame once:
//
// logical:
//     x = 0..479
//     y = 0..799
//
// physical:
//     X = 799 - y
//     Y = x
//
// Then send ONE complete:
//     800 x 480
//
// framebuffer to the RGB LCD.
//
// This avoids dozens of partial flushes per animation
// frame.
//

void flushCallback(
    lv_display_t* lvDisplay,
    const lv_area_t* area,
    uint8_t* pixelMap
)
{
    esp_lcd_panel_handle_t panel =
        DisplayManager::getPanel();


    if (
        panel == nullptr
    ) {
        ESP_LOGE(
            TAG,
            "LCD panel handle is null"
        );


        lv_display_flush_ready(
            lvDisplay
        );


        return;
    }


    auto* source =
        reinterpret_cast<uint16_t*>(
            pixelMap
        );


    // =================================================
    // EXPECT FULL FRAME
    // =================================================
    //
    // LV_DISPLAY_RENDER_MODE_FULL should normally give
    // the complete logical display.
    //

    const int width =
        area->x2 -
        area->x1 +
        1;


    const int height =
        area->y2 -
        area->y1 +
        1;


    if (
        width != LOGICAL_WIDTH ||
        height != LOGICAL_HEIGHT
    ) {
        ESP_LOGW(
            TAG,
            "Unexpected LVGL flush area: %dx%d",
            width,
            height
        );
    }


    // =================================================
    // ROTATE COMPLETE FRAME 90 DEGREES CLOCKWISE
    // =================================================
    //
    // Source index:
    //
    //     y * 480 + x
    //
    // Physical:
    //
    //     physicalX = 799 - y
    //     physicalY = x
    //
    // Destination index:
    //
    //     physicalY * 800 + physicalX
    //

    for (
        int y = 0;
        y < LOGICAL_HEIGHT;
        ++y
    ) {
        const size_t sourceRow =
            static_cast<size_t>(
                y
            ) *
            LOGICAL_WIDTH;


        const int physicalX =
            PHYSICAL_WIDTH -
            1 -
            y;


        for (
            int x = 0;
            x < LOGICAL_WIDTH;
            ++x
        ) {
            const size_t sourceIndex =
                sourceRow +
                static_cast<size_t>(
                    x
                );


            const size_t destinationIndex =
                static_cast<size_t>(
                    x
                ) *
                PHYSICAL_WIDTH +
                static_cast<size_t>(
                    physicalX
                );


            rotatedBuffer[
                destinationIndex
            ] =
                source[
                    sourceIndex
                ];
        }


        // -------------------------------------------------
        // DO NOT STARVE FREERTOS
        // -------------------------------------------------
        //
        // Rotating 384,000 pixels is CPU work.
        //
        // Yield periodically instead of sleeping after
        // every small display strip like the previous
        // partial-render implementation.
        //

        if (
            (y % 32) == 31
        ) {
            taskYIELD();
        }
    }


    // =================================================
    // DRAW COMPLETE PHYSICAL FRAME
    // =================================================

    esp_err_t result =
        esp_lcd_panel_draw_bitmap(
            panel,
            0,
            0,
            PHYSICAL_WIDTH,
            PHYSICAL_HEIGHT,
            rotatedBuffer
        );


    if (
        result != ESP_OK
    ) {
        ESP_LOGE(
            TAG,
            "esp_lcd_panel_draw_bitmap failed: %s",
            esp_err_to_name(
                result
            )
        );
    }


    lv_display_flush_ready(
        lvDisplay
    );


    taskYIELD();
}


// =====================================================
// TOUCH READ
// =====================================================
//
// GT911 physical coordinates:
//
//     rawX = 0..799
//     rawY = 0..479
//
// Screen is mounted portrait.
//
// Convert to LVGL:
//
//     x = 0..479
//     y = 0..799
//

void touchReadCallback(
    lv_indev_t*,
    lv_indev_data_t* data
)
{
    uint16_t rawX =
        0;

    uint16_t rawY =
        0;


    if (
        !TouchManager::read(
            rawX,
            rawY
        )
    ) {
        data->state =
            LV_INDEV_STATE_RELEASED;


        return;
    }


    // =================================================
    // PHYSICAL -> PORTRAIT
    // =================================================

    int logicalX =
        rawY;


    int logicalY =
        PHYSICAL_WIDTH -
        1 -
        rawX;


    // =================================================
    // CLAMP X
    // =================================================

    if (
        logicalX < 0
    ) {
        logicalX =
            0;
    }


    if (
        logicalX >= LOGICAL_WIDTH
    ) {
        logicalX =
            LOGICAL_WIDTH -
            1;
    }


    // =================================================
    // CLAMP Y
    // =================================================

    if (
        logicalY < 0
    ) {
        logicalY =
            0;
    }


    if (
        logicalY >= LOGICAL_HEIGHT
    ) {
        logicalY =
            LOGICAL_HEIGHT -
            1;
    }


    data->point.x =
        logicalX;


    data->point.y =
        logicalY;


    data->state =
        LV_INDEV_STATE_PRESSED;
}


} // namespace


// =====================================================
// INITIALIZE
// =====================================================

bool LVGLManager::initialize()
{
    ESP_LOGI(
        TAG,
        "Initializing LVGL full-frame renderer"
    );


    // =================================================
    // REQUIRE DISPLAY
    // =================================================

    if (
        !DisplayManager::isInitialized()
    ) {
        ESP_LOGE(
            TAG,
            "DisplayManager not initialized"
        );


        return false;
    }


    // =================================================
    // REQUIRE TOUCH
    // =================================================

    if (
        !TouchManager::isInitialized()
    ) {
        ESP_LOGE(
            TAG,
            "TouchManager not initialized"
        );


        return false;
    }


    // =================================================
    // LVGL CORE
    // =================================================

    lv_init();


    lv_tick_set_cb(
        getTickMilliseconds
    );


    // =================================================
    // ALLOCATE FULL FRAME BUFFERS IN PSRAM
    // =================================================

    drawBuffer =
        static_cast<uint16_t*>(
            heap_caps_malloc(
                FRAME_BYTES,
                MALLOC_CAP_SPIRAM |
                MALLOC_CAP_8BIT
            )
        );


    rotatedBuffer =
        static_cast<uint16_t*>(
            heap_caps_malloc(
                FRAME_BYTES,
                MALLOC_CAP_SPIRAM |
                MALLOC_CAP_8BIT
            )
        );


    if (
        drawBuffer == nullptr ||
        rotatedBuffer == nullptr
    ) {
        ESP_LOGE(
            TAG,
            "Failed to allocate full LVGL framebuffers"
        );


        if (
            drawBuffer != nullptr
        ) {
            heap_caps_free(
                drawBuffer
            );


            drawBuffer =
                nullptr;
        }


        if (
            rotatedBuffer != nullptr
        ) {
            heap_caps_free(
                rotatedBuffer
            );


            rotatedBuffer =
                nullptr;
        }


        return false;
    }


    ESP_LOGI(
        TAG,
        "Full LVGL framebuffer: %u bytes",
        static_cast<unsigned>(
            FRAME_BYTES
        )
    );


    ESP_LOGI(
        TAG,
        "Total UI framebuffer PSRAM: %u bytes",
        static_cast<unsigned>(
            FRAME_BYTES *
            2
        )
    );


    // =================================================
    // CREATE LVGL DISPLAY
    // =================================================

    display =
        lv_display_create(
            LOGICAL_WIDTH,
            LOGICAL_HEIGHT
        );


    if (
        display == nullptr
    ) {
        ESP_LOGE(
            TAG,
            "lv_display_create failed"
        );


        return false;
    }


    lv_display_set_color_format(
        display,
        LV_COLOR_FORMAT_RGB565
    );


    // =================================================
    // FULL FRAME RENDER MODE
    // =================================================

    lv_display_set_buffers(
        display,
        drawBuffer,
        nullptr,
        FRAME_BYTES,
        LV_DISPLAY_RENDER_MODE_FULL
    );


    lv_display_set_flush_cb(
        display,
        flushCallback
    );


    // =================================================
    // TOUCH
    // =================================================

    touchInput =
        lv_indev_create();


    if (
        touchInput == nullptr
    ) {
        ESP_LOGE(
            TAG,
            "lv_indev_create failed"
        );


        return false;
    }


    lv_indev_set_type(
        touchInput,
        LV_INDEV_TYPE_POINTER
    );


    lv_indev_set_read_cb(
        touchInput,
        touchReadCallback
    );


    lv_indev_set_display(
        touchInput,
        display
    );


    // =================================================
    // READY
    // =================================================

    ESP_LOGI(
        TAG,
        "LVGL full-frame renderer initialized"
    );


    ESP_LOGI(
        TAG,
        "Logical display: %dx%d portrait",
        LOGICAL_WIDTH,
        LOGICAL_HEIGHT
    );


    ESP_LOGI(
        TAG,
        "Physical display: %dx%d landscape",
        PHYSICAL_WIDTH,
        PHYSICAL_HEIGHT
    );


    return true;
}