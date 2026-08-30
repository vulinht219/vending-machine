#include "HomeScreen.h"
#include "QuizScreen.h"
#include "theme/Theme.h"


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


    // =================================================
    // TITLE
    // =================================================

    lv_obj_t* title =
        lv_label_create(
            screen
        );


    lv_label_set_text(
        title,
        "CANDY QUEST"
    );


    Theme::applyTitle(
        title
    );


    lv_obj_set_width(
        title,
        LV_PCT(90)
    );


    lv_obj_set_style_text_align(
        title,
        LV_TEXT_ALIGN_CENTER,
        0
    );


    lv_obj_align(
        title,
        LV_ALIGN_CENTER,
        0,
        -180
    );


    // =================================================
    // START BUTTON
    // =================================================

    lv_obj_t* button =
        lv_button_create(
            screen
        );


    Theme::applyPrimaryButton(
        button
    );


    lv_obj_set_size(
        button,
        220,
        80
    );


    lv_obj_align(
        button,
        LV_ALIGN_CENTER,
        0,
        80
    );


    lv_obj_t* label =
        lv_label_create(
            button
        );


    lv_label_set_text(
        label,
        "START"
    );


    Theme::applyButtonText(
        label
    );


    lv_obj_center(
        label
    );


    // =================================================
    // CLICK EVENT
    // =================================================

    lv_obj_add_event_cb(
        button,
        startButtonEvent,
        LV_EVENT_CLICKED,
        &game
    );
}