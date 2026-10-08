#include "SpecialEventScreen.h"

#include "DispensingScreen.h"
#include "SpecialDayBackground.h"

#include "esp_log.h"

extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
}

#include <atomic>
#include <cstdint>
#include <new>
#include <string>


LV_FONT_DECLARE(jersey25_85);

LV_IMAGE_DECLARE(candy_title);
LV_IMAGE_DECLARE(candy_panel);
LV_IMAGE_DECLARE(candy_button);


// =====================================================
// STATIC DATA
// =====================================================

SpecialEventManager*
SpecialEventScreen::currentSpecialEventManager =
    nullptr;

GameManager*
SpecialEventScreen::currentGame =
    nullptr;


namespace
{

constexpr const char* TAG =
    "SpecialEventScreen";

std::atomic<bool> specialDispenseTaskRunning{
    false
};

struct SpecialDispenseTaskContext
{
    SpecialEventManager* manager;
    int slot;
};


void specialDispenseTask(
    void* parameter
)
{
    auto* context =
        static_cast<SpecialDispenseTaskContext*>(
            parameter
        );

    bool success =
        false;

    if (
        context != nullptr
        && context->manager != nullptr
    ) {
        success =
            context->manager->selectCandy(
                context->slot
            );

        ESP_LOGI(
            TAG,
            "Special dispense finished: slot=%d success=%d",
            context->slot,
            static_cast<int>(success)
        );
    }

    delete context;

    specialDispenseTaskRunning.store(
        false
    );

    vTaskDelete(
        nullptr
    );
}

} // namespace


// =====================================================
// CANDY BUTTON EVENT
// =====================================================

void SpecialEventScreen::candyButtonEvent(
    lv_event_t* event
)
{
    if (
        lv_event_get_code(event)
        != LV_EVENT_CLICKED
    ) {
        return;
    }

    if (
        currentSpecialEventManager == nullptr
        || currentGame == nullptr
    ) {
        return;
    }

    if (
        specialDispenseTaskRunning.exchange(true)
    ) {
        return;
    }

    const int slot =
        static_cast<int>(
            reinterpret_cast<intptr_t>(
                lv_event_get_user_data(
                    event
                )
            )
        );

    auto* context =
        new (std::nothrow) SpecialDispenseTaskContext{
            currentSpecialEventManager,
            slot
        };

    if (
        context == nullptr
    ) {
        specialDispenseTaskRunning.store(false);
        ESP_LOGE(TAG, "Unable to allocate special dispense task context");
        return;
    }

    const BaseType_t taskResult =
        xTaskCreate(
            specialDispenseTask,
            "special_dispense",
            4096,
            context,
            4,
            nullptr
        );

    if (
        taskResult != pdPASS
    ) {
        delete context;
        specialDispenseTaskRunning.store(false);
        ESP_LOGE(TAG, "Unable to create special dispense task");
        return;
    }

    // Keep the LVGL task free. The special-event manager waits for
    // sensor-confirmed dispensing in the background worker instead.
    SpecialDayBackground::stop();

    DispensingScreen::create(
        *currentGame
    );
}


// =====================================================
// CREATE
// =====================================================

void SpecialEventScreen::create(
    SpecialEventManager& specialEventManager,
    GameManager& game,
    SpecialEventType eventType
)
{
    currentSpecialEventManager =
        &specialEventManager;


    currentGame =
        &game;


    // Stop previous greeting animation first.
    SpecialDayBackground::stop();


    lv_obj_t* screen =
        lv_screen_active();


    lv_obj_clean(
        screen
    );


    // =================================================
    // SPECIAL-DAY BACKGROUND
    // =================================================

    SpecialDayBackground::create(
        screen,
        eventType
    );


    // =================================================
    // TITLE
    // Same asset as normal CandySelectScreen
    // =================================================

    lv_obj_t* title =
        lv_image_create(
            screen
        );


    lv_image_set_src(
        title,
        &candy_title
    );


    lv_obj_align(
        title,
        LV_ALIGN_TOP_MID,
        0,
        80
    );


    lv_obj_remove_flag(
        title,
        LV_OBJ_FLAG_CLICKABLE
    );


    // =================================================
    // PANEL
    // Same asset as normal CandySelectScreen
    // =================================================

    lv_obj_t* panel =
        lv_image_create(
            screen
        );


    lv_image_set_src(
        panel,
        &candy_panel
    );


    lv_obj_align(
        panel,
        LV_ALIGN_TOP_MID,
        0,
        185
    );


    lv_obj_remove_flag(
        panel,
        LV_OBJ_FLAG_CLICKABLE
    );


    // =================================================
    // INVISIBLE BUTTON LAYER
    // =================================================

    lv_obj_t* buttonLayer =
        lv_obj_create(
            screen
        );


    lv_obj_set_size(
        buttonLayer,
        445,
        475
    );


    lv_obj_align(
        buttonLayer,
        LV_ALIGN_TOP_MID,
        0,
        185
    );


    lv_obj_set_style_bg_opa(
        buttonLayer,
        LV_OPA_TRANSP,
        0
    );


    lv_obj_set_style_border_width(
        buttonLayer,
        0,
        0
    );


    lv_obj_set_style_shadow_width(
        buttonLayer,
        0,
        0
    );


    lv_obj_set_style_pad_all(
        buttonLayer,
        0,
        0
    );


    lv_obj_set_style_radius(
        buttonLayer,
        0,
        0
    );


    lv_obj_clear_flag(
        buttonLayer,
        LV_OBJ_FLAG_SCROLLABLE
    );


    // =================================================
    // BUTTON POSITIONS
    // =================================================

    const int xPositions[3] = {
        15,
        160,
        305
    };


    const int yPositions[2] = {
        35,
        241
    };


    // =================================================
    // CREATE 6 CANDY BUTTONS
    // =================================================

    for (
        int i = 0;
        i < 6;
        ++i
    ) {

        int candyNumber =
            i + 1;


        int column =
            i % 3;


        int row =
            i / 3;


        // =============================================
        // BUTTON
        // =============================================

        lv_obj_t* button =
            lv_button_create(
                buttonLayer
            );


        lv_obj_set_size(
            button,
            125,
            171
        );


        lv_obj_set_pos(
            button,
            xPositions[column],
            yPositions[row]
        );


        lv_obj_set_style_bg_opa(
            button,
            LV_OPA_TRANSP,
            0
        );


        lv_obj_set_style_border_width(
            button,
            0,
            0
        );


        lv_obj_set_style_shadow_width(
            button,
            0,
            0
        );


        lv_obj_set_style_radius(
            button,
            0,
            0
        );


        lv_obj_set_style_pad_all(
            button,
            0,
            0
        );


        // =============================================
        // CANDY BUTTON IMAGE
        // =============================================

        lv_obj_t* buttonImage =
            lv_image_create(
                button
            );


        lv_image_set_src(
            buttonImage,
            &candy_button
        );


        lv_obj_center(
            buttonImage
        );


        lv_obj_remove_flag(
            buttonImage,
            LV_OBJ_FLAG_CLICKABLE
        );


        // =============================================
        // NUMBER
        // =============================================

        lv_obj_t* label =
            lv_label_create(
                button
            );


        std::string text =
            std::to_string(
                candyNumber
            );


        lv_label_set_text(
            label,
            text.c_str()
        );


        lv_obj_set_style_text_color(
            label,
            lv_color_hex(
                0xE8FFFF
            ),
            0
        );


        lv_obj_set_style_text_font(
            label,
            &jersey25_85,
            0
        );


        lv_obj_center(
            label
        );


        lv_obj_remove_flag(
            label,
            LV_OBJ_FLAG_CLICKABLE
        );


        // =============================================
        // CLICK EVENT
        // =============================================

        lv_obj_add_event_cb(
            button,
            candyButtonEvent,
            LV_EVENT_CLICKED,
            reinterpret_cast<void*>(
                static_cast<intptr_t>(
                    candyNumber
                )
            )
        );
    }
}