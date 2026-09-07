#include "HomeScreen.h"

#include "esp_log.h"


LV_IMAGE_DECLARE(home_1);
LV_IMAGE_DECLARE(home_2);
LV_IMAGE_DECLARE(home_3);
LV_IMAGE_DECLARE(home_title);
LV_IMAGE_DECLARE(home_button);


namespace {

constexpr const char* TAG =
    "HomeScreen";


lv_timer_t* homeAnimationTimer =
    nullptr;


int currentHomeFrame =
    0;


const lv_image_dsc_t* homeFrames[] = {
    &home_1,
    &home_2,
    &home_3
};


// =====================================================
// BACKGROUND ANIMATION
// =====================================================

void updateHomeAnimation(
    lv_timer_t* timer
)
{
    lv_obj_t* background =
        static_cast<lv_obj_t*>(
            lv_timer_get_user_data(
                timer
            )
        );


    if (
        background == nullptr
    ) {
        return;
    }


    currentHomeFrame =
        (
            currentHomeFrame + 1
        )
        % 3;


    lv_image_set_src(
        background,
        homeFrames[
            currentHomeFrame
        ]
    );
}

} // namespace


// =====================================================
// START BUTTON
// =====================================================

void HomeScreen::startButtonEvent(
    lv_event_t* event
)
{
    if (
        lv_event_get_code(
            event
        )
        !=
        LV_EVENT_CLICKED
    ) {
        return;
    }


    ESP_LOGI(
        TAG,
        "START BUTTON CLICKED"
    );


    // =================================================
    // TEMPORARY
    // =================================================
    //
    // Touch latency test only.
    //
    // Later this will call:
    //
    // game->startGame();
    // QuizScreen::create(*game);
    //
}


// =====================================================
// CREATE
// =====================================================

void HomeScreen::create()
{
    lv_obj_t* screen =
        lv_screen_active();


    // =================================================
    // STOP OLD TIMER
    // =================================================

    if (
        homeAnimationTimer != nullptr
    ) {
        lv_timer_delete(
            homeAnimationTimer
        );


        homeAnimationTimer =
            nullptr;
    }


    // =================================================
    // CLEAN SCREEN
    // =================================================

    lv_obj_clean(
        screen
    );


    currentHomeFrame =
        0;


    // =================================================
    // BACKGROUND
    // =================================================

    lv_obj_t* background =
        lv_image_create(
            screen
        );


    lv_image_set_src(
        background,
        &home_1
    );


    lv_obj_align(
        background,
        LV_ALIGN_CENTER,
        0,
        0
    );


    lv_obj_clear_flag(
        background,
        LV_OBJ_FLAG_CLICKABLE
    );


    // =================================================
    // BACKGROUND ANIMATION
    // =================================================
    //
    // TEMPORARILY DISABLED.
    //
    // We are testing whether the 400 ms full-screen
    // animation is causing the touch latency.
    //
    // The Home screen should remain completely static
    // on home_1 during this test.
    //

    
    homeAnimationTimer =
        lv_timer_create(
            updateHomeAnimation,
            400,
            background
        );
    


    // =================================================
    // TITLE
    // =================================================

    lv_obj_t* title =
        lv_image_create(
            screen
        );


    lv_image_set_src(
        title,
        &home_title
    );


    lv_obj_align(
        title,
        LV_ALIGN_TOP_MID,
        0,
        90
    );


    lv_obj_clear_flag(
        title,
        LV_OBJ_FLAG_CLICKABLE
    );


    // =================================================
    // START BUTTON
    // =================================================

    lv_obj_t* button =
        lv_image_create(
            screen
        );


    lv_image_set_src(
        button,
        &home_button
    );


    lv_obj_add_flag(
        button,
        LV_OBJ_FLAG_CLICKABLE
    );


    lv_obj_align(
        button,
        LV_ALIGN_CENTER,
        0,
        180
    );


    lv_obj_add_event_cb(
        button,
        startButtonEvent,
        LV_EVENT_CLICKED,
        nullptr
    );


    ESP_LOGI(
        TAG,
        "Home screen created - animation disabled for latency test"
    );
}