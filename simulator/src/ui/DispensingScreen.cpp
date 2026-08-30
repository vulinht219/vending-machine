#include "DispensingScreen.h"

#include "HomeScreen.h"
#include "theme/Theme.h"


// =====================================================
// STATIC DATA
// =====================================================

GameManager*
    DispensingScreen::currentGame =
        nullptr;


lv_timer_t*
    DispensingScreen::dispenseTimer =
        nullptr;


// =====================================================
// CREATE
// =====================================================

void DispensingScreen::create(
    GameManager& game
)
{
    currentGame =
        &game;


    lv_obj_t* screen =
        lv_screen_active();


    lv_obj_clean(
        screen
    );


    Theme::applyScreen(
        screen
    );


    // =================================================
    // FULL SCREEN CONTENT
    // =================================================

    lv_obj_t* content =
        lv_obj_create(
            screen
        );


    lv_obj_set_size(
        content,
        LV_PCT(100),
        LV_PCT(100)
    );


    lv_obj_set_style_bg_opa(
        content,
        LV_OPA_TRANSP,
        0
    );


    lv_obj_set_style_border_width(
        content,
        0,
        0
    );


    lv_obj_set_style_pad_all(
        content,
        Theme::SPACING_LG,
        0
    );


    lv_obj_set_scrollbar_mode(
        content,
        LV_SCROLLBAR_MODE_OFF
    );


    lv_obj_remove_flag(
        content,
        LV_OBJ_FLAG_SCROLLABLE
    );


    // =================================================
    // CENTER AREA
    // =================================================

    lv_obj_t* centerArea =
        lv_obj_create(
            content
        );


    lv_obj_set_size(
        centerArea,
        LV_PCT(100),
        LV_PCT(100)
    );


    lv_obj_set_style_bg_opa(
        centerArea,
        LV_OPA_TRANSP,
        0
    );


    lv_obj_set_style_border_width(
        centerArea,
        0,
        0
    );


    lv_obj_set_style_pad_all(
        centerArea,
        0,
        0
    );


    lv_obj_set_scrollbar_mode(
        centerArea,
        LV_SCROLLBAR_MODE_OFF
    );


    lv_obj_remove_flag(
        centerArea,
        LV_OBJ_FLAG_SCROLLABLE
    );


    lv_obj_set_flex_flow(
        centerArea,
        LV_FLEX_FLOW_COLUMN
    );


    lv_obj_set_flex_align(
        centerArea,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER
    );


    // =================================================
    // TEMPORARY DISPENSING PLACEHOLDER
    // =================================================

    lv_obj_t* label =
        lv_label_create(
            centerArea
        );


    lv_label_set_text(
        label,
        "DISPENSING..."
    );


    Theme::applyTitle(
        label
    );


    lv_obj_set_style_text_align(
        label,
        LV_TEXT_ALIGN_CENTER,
        0
    );


    // =================================================
    // TIMER
    // =================================================

    if (
        dispenseTimer != nullptr
    ) {

        lv_timer_delete(
            dispenseTimer
        );


        dispenseTimer =
            nullptr;
    }


    dispenseTimer =
        lv_timer_create(
            dispenseFinished,
            5000,
            nullptr
        );


    lv_timer_set_repeat_count(
        dispenseTimer,
        1
    );
}


// =====================================================
// FINISHED
// =====================================================

void DispensingScreen::dispenseFinished(
    lv_timer_t* timer
)
{
    dispenseTimer =
        nullptr;


    if (
        currentGame == nullptr
    ) {
        return;
    }


    HomeScreen::create(
        *currentGame
    );
}