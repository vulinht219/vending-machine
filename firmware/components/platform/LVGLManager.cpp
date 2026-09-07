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


constexpr int LOGICAL_WIDTH =
    480;

constexpr int LOGICAL_HEIGHT =
    800;

constexpr int PHYSICAL_WIDTH =
    800;

constexpr int PHYSICAL_HEIGHT =
    480;


// =====================================================
// FRAME SIZE
// =====================================================

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
// TILED ROTATION
// =====================================================
//
// Process the frame in small blocks instead of writing
// pixels across the entire PSRAM framebuffer with a very
// large stride.
//
// 16x16 is a good first test size for cache locality.
//

constexpr int TILE_SIZE =
    16;


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
// PROFILER
// =====================================================

int64_t previousFlushEndUs =
    0;


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
// ROTATE TILE
// =====================================================
//
// Logical:
//
//     x = 0..479
//     y = 0..799
//
// Physical:
//
//     X = 799 - y
//     Y = x
//
// Processing in tiles improves locality compared with
// traversing the complete frame with strided PSRAM writes.
//

inline void rotateTile(
    const uint16_t* source,
    uint16_t* destination,
    int tileX,
    int tileY,
    int tileWidth,
    int tileHeight
)
{
    for (
        int localY = 0;
        localY < tileHeight;
        ++localY
    ) {
        const int sourceY =
            tileY +
            localY;


        const size_t sourceRow =
            static_cast<size_t>(
                sourceY
            ) *
            LOGICAL_WIDTH;


        const int physicalX =
            PHYSICAL_WIDTH -
            1 -
            sourceY;


        for (
            int localX = 0;
            localX < tileWidth;
            ++localX
        ) {
            const int sourceX =
                tileX +
                localX;


            const size_t sourceIndex =
                sourceRow +
                static_cast<size_t>(
                    sourceX
                );


            const size_t destinationIndex =
                static_cast<size_t>(
                    sourceX
                ) *
                PHYSICAL_WIDTH +
                static_cast<size_t>(
                    physicalX
                );


            destination[
                destinationIndex
            ] =
                source[
                    sourceIndex
                ];
        }
    }
}


// =====================================================
// DISPLAY FLUSH
// =====================================================

void flushCallback(
    lv_display_t* lvDisplay,
    const lv_area_t* area,
    uint8_t* pixelMap
)
{
    const int64_t flushStartUs =
        esp_timer_get_time();


    int64_t gapBeforeFlushUs =
        0;


    if (
        previousFlushEndUs != 0
    ) {
        gapBeforeFlushUs =
            flushStartUs -
            previousFlushEndUs;
    }


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


        previousFlushEndUs =
            esp_timer_get_time();


        return;
    }


    auto* source =
        reinterpret_cast<uint16_t*>(
            pixelMap
        );


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
    // ROTATION PROFILE START
    // =================================================

    const int64_t rotateStartUs =
        esp_timer_get_time();


    // =================================================
    // TILED ROTATION
    // =================================================

    for (
        int tileY = 0;
        tileY < LOGICAL_HEIGHT;
        tileY += TILE_SIZE
    ) {
        const int tileHeight =
            (
                tileY +
                TILE_SIZE
                <=
                LOGICAL_HEIGHT
            )
            ?
            TILE_SIZE
            :
            LOGICAL_HEIGHT -
            tileY;


        for (
            int tileX = 0;
            tileX < LOGICAL_WIDTH;
            tileX += TILE_SIZE
        ) {
            const int tileWidth =
                (
                    tileX +
                    TILE_SIZE
                    <=
                    LOGICAL_WIDTH
                )
                ?
                TILE_SIZE
                :
                LOGICAL_WIDTH -
                tileX;


            rotateTile(
                source,
                rotatedBuffer,
                tileX,
                tileY,
                tileWidth,
                tileHeight
            );
        }


        // Let FreeRTOS schedule other work occasionally.
        //
        // With TILE_SIZE=16 this happens every logical
        // tile-row rather than every pixel row.

        taskYIELD();
    }


    const int64_t rotateEndUs =
        esp_timer_get_time();


    // =================================================
    // DRAW PROFILE START
    // =================================================

    const int64_t drawStartUs =
        esp_timer_get_time();


    esp_err_t result =
        esp_lcd_panel_draw_bitmap(
            panel,
            0,
            0,
            PHYSICAL_WIDTH,
            PHYSICAL_HEIGHT,
            rotatedBuffer
        );


    const int64_t drawEndUs =
        esp_timer_get_time();


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


    const int64_t flushEndUs =
        esp_timer_get_time();


    // =================================================
    // PROFILE OUTPUT
    // =================================================


    previousFlushEndUs =
        flushEndUs;


    taskYIELD();
}


// =====================================================
// TOUCH READ
// =====================================================

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


    int logicalX =
        rawY;


    int logicalY =
        PHYSICAL_WIDTH -
        1 -
        rawX;


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
        "Initializing LVGL tiled-rotation renderer"
    );


    if (
        !DisplayManager::isInitialized()
    ) {
        ESP_LOGE(
            TAG,
            "DisplayManager not initialized"
        );


        return false;
    }


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
    // FULL FRAME BUFFERS
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
            "Failed to allocate LVGL framebuffers"
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
        "Framebuffer size: %u bytes each",
        static_cast<unsigned>(
            FRAME_BYTES
        )
    );


    ESP_LOGI(
        TAG,
        "Rotation tile size: %dx%d",
        TILE_SIZE,
        TILE_SIZE
    );


    // =================================================
    // DISPLAY
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


    previousFlushEndUs =
        0;


    ESP_LOGI(
        TAG,
        "LVGL tiled-rotation renderer initialized"
    );


    return true;
}