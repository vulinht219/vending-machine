#include "HomeScreen.h"
#include "QuizScreen.h"
#include "theme/Theme.h"


LV_IMAGE_DECLARE(home_1);
LV_IMAGE_DECLARE(home_2);
LV_IMAGE_DECLARE(home_3);
LV_IMAGE_DECLARE(home_title);
LV_IMAGE_DECLARE(home_button);


namespace {

lv_timer_t* homeAnimationTimer = nullptr;

int currentHomeFrame = 0;


const lv_image_dsc_t* homeFrames[] = {
    &home_1,
    &home_2,
    &home_3
};


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

}


void HomeScreen::startButtonEvent(
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
        homeAnimationTimer
        != nullptr
    ) {

        lv_timer_delete(
            homeAnimationTimer
        );

        homeAnimationTimer =
            nullptr;
    }


    GameManager* game =
        static_cast<GameManager*>(
            lv_event_get_user_data(
                event
            )
        );


    game->startGame();


    QuizScreen::create(
        *game
    );
}


void HomeScreen::create(
    GameManager& game
)
{
    lv_obj_t* screen =
        lv_screen_active();


    lv_obj_clean(
        screen
    );


    Theme::applyScreen(
        screen
    );


    currentHomeFrame =
        0;


    // =====================================================
    // BACKGROUND
    // =====================================================

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


    // =====================================================
    // BACKGROUND ANIMATION
    // =====================================================

    if (
        homeAnimationTimer
        != nullptr
    ) {

        lv_timer_delete(
            homeAnimationTimer
        );

        homeAnimationTimer =
            nullptr;
    }


    homeAnimationTimer =
        lv_timer_create(
            updateHomeAnimation,
            400,
            background
        );


    // =====================================================
    // TITLE
    // =====================================================

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


    // =====================================================
    // START BUTTON
    // =====================================================

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
        &game
    );
}