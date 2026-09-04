#include "IncorrectScreen.h"

#include "QuizScreen.h"


LV_IMAGE_DECLARE(incorrect_1);
LV_IMAGE_DECLARE(incorrect_2);
LV_IMAGE_DECLARE(incorrect_3);


GameManager*
IncorrectScreen::currentGame =
    nullptr;


lv_timer_t*
IncorrectScreen::animationTimer =
    nullptr;


lv_timer_t*
IncorrectScreen::finishTimer =
    nullptr;


int
IncorrectScreen::currentFrame =
    0;


namespace {

const lv_image_dsc_t* incorrectFrames[] = {
    &incorrect_1,
    &incorrect_2,
    &incorrect_3
};

}


void IncorrectScreen::animationTick(
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
        incorrectFrames[
            currentFrame
        ]
    );
}


void IncorrectScreen::finished(
    lv_timer_t* timer
)
{
    animationTimer =
        nullptr;


    finishTimer =
        nullptr;


    currentGame
        ->retryQuiz();


    QuizScreen::create(
        *currentGame
    );
}


void IncorrectScreen::create(
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
        &incorrect_1
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