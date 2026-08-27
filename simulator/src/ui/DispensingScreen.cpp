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
    // =====================================================

    lv_obj_t* content =
        lv_obj_create(
            screen
        );


    Theme::applyPanel(
        content
    );


    lv_obj_set_size(
        content,
        LV_PCT(100),
        LV_PCT(100)
    );


    lv_obj_set_style_border_width(
        content,
        0,
        0
    );


    // =================================================
    // TEMPORARY DISPENSING PLACEHOLDER
    // =================================================
    //
    // Later:
    //
    // GIF
    // sprite animation
    // pixel-art dispensing scene
    //
    // =================================================

    lv_obj_t* label =
        lv_label_create(
            content
        );


    lv_label_set_text(
        label,
        "DISPENSING..."
    );


    Theme::applyTitle(
        label
    );


    lv_obj_center(
        label
    );


    // =================================================
    // TIMER
    // =====================================================

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