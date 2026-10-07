#include "SpecialEventScreen.h"

#include "DispensingScreen.h"
#include "SpecialDayBackground.h"

#include <cstdint>
#include <string>


// =========================================================
// EXISTING CANDY ASSETS
// =========================================================

LV_FONT_DECLARE(jersey25_85);

LV_IMAGE_DECLARE(candy_title);
LV_IMAGE_DECLARE(candy_panel);
LV_IMAGE_DECLARE(candy_button);


// =========================================================
// COLORS
// =========================================================

namespace
{

constexpr uint32_t PINK_COLOR =
    0xFF9FCF;

}


// =========================================================
// STATIC DATA
// =========================================================

SpecialEventManager*
SpecialEventScreen::currentSpecialEventManager =
    nullptr;


GameManager*
SpecialEventScreen::currentGame =
    nullptr;


SpecialEventType
SpecialEventScreen::currentEventType =
    SpecialEventType::NONE;


// =========================================================
// GREETING TEXT
// =========================================================

const char* SpecialEventScreen::getGreetingText(
    SpecialEventType eventType
)
{
    switch (
        eventType
    )
    {
        case SpecialEventType::MONTHIVERSARY:
            return "Happy monthiversary!";


        case SpecialEventType::HALLOWEEN:
            return "Trick or treat!";


        case SpecialEventType::CHRISTMAS:
            return "Merry Christmas!";


        case SpecialEventType::NEW_YEARS_EVE:
            return "One last sweet this year?";


        case SpecialEventType::NEW_YEAR:
            return "Happy New Year!";


        case SpecialEventType::VALENTINE:
            return "Happy Valentine!";


        case SpecialEventType::BIRTHDAY_SEPTEMBER:
            return "Happy Birthday!";


        case SpecialEventType::BIRTHDAY_OCTOBER:
            return "Happy Birthday!";


        case SpecialEventType::NONE:
        default:
            return "Special Day!";
    }
}


// =========================================================
// CREATE
// =========================================================

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


    currentEventType =
        eventType;


    showGreeting();
}


// =========================================================
// GREETING SCREEN
// =========================================================

void SpecialEventScreen::showGreeting()
{
    // Stop old animation first.
    SpecialDayBackground::stop();


    lv_obj_t* screen =
        lv_screen_active();


    lv_obj_clean(
        screen
    );


    // =====================================================
    // SPECIAL-DAY PNG BACKGROUND
    // =====================================================

    SpecialDayBackground::create(
        screen,
        currentEventType
    );


    // =====================================================
    // GREETING TITLE
    // =====================================================

    lv_obj_t* title =
        lv_label_create(
            screen
        );


    lv_label_set_text(
        title,
        getGreetingText(
            currentEventType
        )
    );


    lv_obj_set_width(
        title,
        420
    );


    lv_label_set_long_mode(
        title,
        LV_LABEL_LONG_WRAP
    );


    lv_obj_set_style_text_align(
        title,
        LV_TEXT_ALIGN_CENTER,
        0
    );


    lv_obj_set_style_text_color(
        title,
        lv_color_hex(
            PINK_COLOR
        ),
        0
    );


    // Uses LVGL default font.
    // No additional image/font asset required.
    lv_obj_align(
        title,
        LV_ALIGN_TOP_MID,
        0,
        105
    );


    lv_obj_remove_flag(
        title,
        LV_OBJ_FLAG_CLICKABLE
    );


    // =====================================================
    // CONTINUE BUTTON
    // =====================================================

    lv_obj_t* continueButton =
        lv_button_create(
            screen
        );


    lv_obj_set_size(
        continueButton,
        260,
        70
    );


    lv_obj_align(
        continueButton,
        LV_ALIGN_BOTTOM_MID,
        0,
        -80
    );


    // Transparent button.
    lv_obj_set_style_bg_opa(
        continueButton,
        LV_OPA_TRANSP,
        0
    );


    // Pink outline.
    lv_obj_set_style_border_width(
        continueButton,
        2,
        0
    );


    lv_obj_set_style_border_color(
        continueButton,
        lv_color_hex(
            PINK_COLOR
        ),
        0
    );


    lv_obj_set_style_radius(
        continueButton,
        18,
        0
    );


    lv_obj_set_style_shadow_width(
        continueButton,
        0,
        0
    );


    // =====================================================
    // BUTTON TEXT
    // =====================================================

    lv_obj_t* continueLabel =
        lv_label_create(
            continueButton
        );


    lv_label_set_text(
        continueLabel,
        "CHOOSE A CANDY"
    );


    lv_obj_set_style_text_color(
        continueLabel,
        lv_color_hex(
            PINK_COLOR
        ),
        0
    );


    lv_obj_center(
        continueLabel
    );


    lv_obj_remove_flag(
        continueLabel,
        LV_OBJ_FLAG_CLICKABLE
    );


    // =====================================================
    // CLICK
    // =====================================================

    lv_obj_add_event_cb(
        continueButton,
        greetingButtonEvent,
        LV_EVENT_CLICKED,
        nullptr
    );
}


// =========================================================
// GREETING -> CANDY SELECT
// =========================================================

void SpecialEventScreen::greetingButtonEvent(
    lv_event_t* event
)
{
    if (
        lv_event_get_code(
            event
        )
        != LV_EVENT_CLICKED
    ) {
        return;
    }


    if (
        currentSpecialEventManager
        == nullptr
        ||
        currentGame
        == nullptr
    ) {
        return;
    }


    showCandySelection();
}


// =========================================================
// CANDY SELECTION
// =========================================================

void SpecialEventScreen::showCandySelection()
{
    // The current background object is about to be deleted,
    // therefore stop its timer first.
    SpecialDayBackground::stop();


    lv_obj_t* screen =
        lv_screen_active();


    lv_obj_clean(
        screen
    );


    // =====================================================
    // SAME SPECIAL-DAY BACKGROUND
    // =====================================================

    SpecialDayBackground::create(
        screen,
        currentEventType
    );


    // =====================================================
    // CANDY TITLE
    // =====================================================

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


    // =====================================================
    // CANDY PANEL
    // =====================================================

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


    // =====================================================
    // BUTTON LAYER
    // =====================================================

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


    // =====================================================
    // POSITIONS
    // =====================================================

    const int xPositions[3] = {
        15,
        160,
        305
    };


    const int yPositions[2] = {
        35,
        241
    };


    // =====================================================
    // 6 CANDY BUTTONS
    // =====================================================

    for (
        int i = 0;
        i < 6;
        ++i
    )
    {
        const int candyNumber =
            i + 1;


        const int column =
            i % 3;


        const int row =
            i / 3;


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


        // =================================================
        // EXISTING CANDY BUTTON IMAGE
        // =================================================

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


        // =================================================
        // CANDY NUMBER
        // =================================================

        lv_obj_t* label =
            lv_label_create(
                button
            );


        const std::string text =
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


        // =================================================
        // CLICK EVENT
        // =================================================

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


// =========================================================
// CANDY BUTTON CLICK
// =========================================================

void SpecialEventScreen::candyButtonEvent(
    lv_event_t* event
)
{
    if (
        lv_event_get_code(
            event
        )
        != LV_EVENT_CLICKED
    ) {
        return;
    }


    if (
        currentSpecialEventManager
        == nullptr
        ||
        currentGame
        == nullptr
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


    const bool success =
        currentSpecialEventManager
            ->selectCandy(
                slot
            );


    if (
        !success
    ) {
        return;
    }


    SpecialDayBackground::stop();


    DispensingScreen::create(
        *currentGame
    );
}