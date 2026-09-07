#include "CandySelectScreen.h"

#include "DispensingScreen.h"

#include <string>

LV_FONT_DECLARE(jersey25_85);

LV_IMAGE_DECLARE(correct_1);
LV_IMAGE_DECLARE(correct_2);
LV_IMAGE_DECLARE(correct_3);

LV_IMAGE_DECLARE(candy_title);
LV_IMAGE_DECLARE(candy_panel);
LV_IMAGE_DECLARE(candy_button);


GameManager*
CandySelectScreen::currentGame =
    nullptr;


namespace {

lv_timer_t* backgroundTimer =
    nullptr;


int currentBackgroundFrame =
    0;


const lv_image_dsc_t* backgroundFrames[] = {
    &correct_1,
    &correct_2,
    &correct_3
};


void stopBackgroundAnimation()
{
    if (
        backgroundTimer != nullptr
    ) {
        lv_timer_delete(
            backgroundTimer
        );

        backgroundTimer =
            nullptr;
    }
}


void updateBackground(
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


    currentBackgroundFrame =
        (
            currentBackgroundFrame + 1
        )
        % 3;


    lv_image_set_src(
        background,
        backgroundFrames[
            currentBackgroundFrame
        ]
    );
}


void candyButtonEvent(
    lv_event_t* event
)
{
    if (
        lv_event_get_code(event)
        != LV_EVENT_CLICKED
    ) {
        return;
    }


    void* userData =
        lv_event_get_user_data(
            event
        );


    int candyIndex =
        static_cast<int>(
            reinterpret_cast<intptr_t>(
                userData
            )
        );


    if (
        CandySelectScreen::currentGame
        == nullptr
    ) {
        return;
    }


    stopBackgroundAnimation();


    // IMPORTANT:
    // If your existing GameManager uses a different
    // method name, replace this one line only.
    CandySelectScreen::currentGame
        ->selectCandy(
            candyIndex
        );


    DispensingScreen::create(
        *CandySelectScreen::currentGame
    );
}

}


// =====================================================
// CREATE CANDY BUTTON
// =====================================================

void CandySelectScreen::createCandyButton(
    lv_obj_t* parent,
    int candyNumber,
    int x,
    int y
)
{
    lv_obj_t* button =
        lv_button_create(
            parent
        );


    lv_obj_set_size(
        button,
        125,
        171
    );


    lv_obj_set_pos(
        button,
        x,
        y
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


// =====================================================
// CREATE
// =====================================================

void CandySelectScreen::create(
    GameManager& game
)
{
    currentGame =
        &game;


    stopBackgroundAnimation();


    currentBackgroundFrame =
        0;


    lv_obj_t* screen =
        lv_screen_active();


    lv_obj_clean(
        screen
    );


    // =================================================
    // FULL-SCREEN CORRECT BACKGROUND
    // =================================================

    lv_obj_t* background =
        lv_image_create(
            screen
        );


    lv_image_set_src(
        background,
        &correct_1
    );


    lv_obj_align(
        background,
        LV_ALIGN_CENTER,
        0,
        0
    );


    lv_obj_remove_flag(
        background,
        LV_OBJ_FLAG_CLICKABLE
    );


    backgroundTimer =
        lv_timer_create(
            updateBackground,
            400,
            background
        );


    // =================================================
    // TITLE
    // 394 x 60
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
    // 445 x 475
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
    //
    // We create a transparent container exactly over
    // the panel so all positions are relative to it.
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


    // =================================================
    // BUTTON POSITIONS
    //
    // Panel: 445 x 475
    // Button: 125 x 171
    //
    // Row 1:
    // 1 2 3
    //
    // Row 2:
    // 4 5 6
    // =================================================

    const int x1 = 15;
    const int x2 = 160;
    const int x3 = 305;

    const int y1 = 35;
    const int y2 = 241;


    createCandyButton(
        buttonLayer,
        1,
        x1,
        y1
    );


    createCandyButton(
        buttonLayer,
        2,
        x2,
        y1
    );


    createCandyButton(
        buttonLayer,
        3,
        x3,
        y1
    );


    createCandyButton(
        buttonLayer,
        4,
        x1,
        y2
    );


    createCandyButton(
        buttonLayer,
        5,
        x2,
        y2
    );


    createCandyButton(
        buttonLayer,
        6,
        x3,
        y2
    );
}
