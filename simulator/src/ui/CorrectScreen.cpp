#include "CorrectScreen.h"

#include "CandySelectScreen.h"


LV_IMAGE_DECLARE(correct_1);
LV_IMAGE_DECLARE(correct_2);
LV_IMAGE_DECLARE(correct_3);


GameManager*
CorrectScreen::currentGame =
    nullptr;


lv_timer_t*
CorrectScreen::animationTimer =
    nullptr;


lv_timer_t*
CorrectScreen::finishTimer =
    nullptr;


int
CorrectScreen::currentFrame =
    0;


namespace {

const lv_image_dsc_t* correctFrames[] = {
    &correct_1,
    &correct_2,
    &correct_3
};

}


void CorrectScreen::animationTick(
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
        correctFrames[
            currentFrame
        ]
    );
}


void CorrectScreen::finished(
    lv_timer_t* timer
)
{
    animationTimer =
        nullptr;


    finishTimer =
        nullptr;


    CandySelectScreen::create(
        *currentGame
    );
}


void CorrectScreen::create(
    GameManager& game
)
{
    currentGame =
        &game;


    currentFrame =
        0;


    lv_obj_t* screen =
        lv_screen_active();


    lv_obj_clean(
        screen
    );


    lv_obj_t* image =
        lv_image_create(
            screen
        );


    lv_image_set_src(
        image,
        &correct_1
    );


    lv_obj_align(
        image,
        LV_ALIGN_CENTER,
        0,
        0
    );


    animationTimer =
        lv_timer_create(
            animationTick,
            400,
            image
        );


    finishTimer =
        lv_timer_create(
            finished,
            3000,
            nullptr
        );


    lv_timer_set_repeat_count(
        finishTimer,
        1
    );
}