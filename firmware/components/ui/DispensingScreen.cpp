#include "DispensingScreen.h"



#include "HomeScreen.h"


LV_IMAGE_DECLARE(dispensing_1);
LV_IMAGE_DECLARE(dispensing_2);
LV_IMAGE_DECLARE(dispensing_3);
LV_IMAGE_DECLARE(dispensing_title);


GameManager*
DispensingScreen::currentGame =
    nullptr;


namespace {

lv_timer_t* animationTimer =
    nullptr;

lv_timer_t* finishTimer =
    nullptr;

int currentFrame =
    0;


const lv_image_dsc_t* frames[] = {
    &dispensing_1,
    &dispensing_2,
    &dispensing_3
};


void stopAnimation()
{
    if (
        animationTimer != nullptr
    ) {
        lv_timer_delete(
            animationTimer
        );

        animationTimer =
            nullptr;
    }
}


void animationTick(
    lv_timer_t* timer
)
{
    lv_obj_t* image =
        static_cast<lv_obj_t*>(
            lv_timer_get_user_data(
                timer
            )
        );


    if (
        image == nullptr
    ) {
        return;
    }


    currentFrame =
        (
            currentFrame + 1
        )
        % 3;


    lv_image_set_src(
        image,
        frames[
            currentFrame
        ]
    );
}


void finishDispensing(
    lv_timer_t* timer
)
{
    finishTimer =
        nullptr;


    stopAnimation();


    HomeScreen::create(
        *DispensingScreen::currentGame
    );
}

}


// =====================================================
// CREATE
// =====================================================

void DispensingScreen::create(
    GameManager& game
)
{
    currentGame =
        &game;


    stopAnimation();


    if (
        finishTimer != nullptr
    ) {
        lv_timer_delete(
            finishTimer
        );

        finishTimer =
            nullptr;
    }


    currentFrame =
        0;


    lv_obj_t* screen =
        lv_screen_active();


    lv_obj_clean(
        screen
    );


    // =================================================
    // FULL-SCREEN DISPENSING ANIMATION
    // =================================================

    lv_obj_t* image =
        lv_image_create(
            screen
        );


    lv_image_set_src(
        image,
        &dispensing_1
    );


    lv_obj_align(
        image,
        LV_ALIGN_CENTER,
        0,
        0
    );

    lv_obj_t* title =
        lv_image_create(
            screen
    );

    lv_image_set_src(
        title,
        &dispensing_title
    );

    lv_obj_align(
        title,
        LV_ALIGN_TOP_MID,
        0,
        80
    );

    // =================================================
    // FRAME ANIMATION
    // =================================================

    animationTimer =
        lv_timer_create(
            animationTick,
            400,
            image
        );


    // =================================================
    // AFTER 5 SECONDS -> HOME
    // =================================================

    finishTimer =
        lv_timer_create(
            finishDispensing,
            5000,
            nullptr
        );


    lv_timer_set_repeat_count(
        finishTimer,
        1
    );
}
