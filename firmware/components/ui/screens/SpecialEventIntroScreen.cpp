#include "SpecialEventIntroScreen.h"

#include "SpecialEventScreen.h"
#include "SpecialDayBackground.h"

#include <cstdint>


// =====================================================
// SPECIAL EVENT ASSETS
// =====================================================

LV_IMAGE_DECLARE(monthi_title);
LV_IMAGE_DECLARE(monthi_button);

LV_IMAGE_DECLARE(halloween_title);
LV_IMAGE_DECLARE(halloween_button);

LV_IMAGE_DECLARE(xmas_title);
LV_IMAGE_DECLARE(xmas_button);

LV_IMAGE_DECLARE(anewyear_title);
LV_IMAGE_DECLARE(anewyear_button);

LV_IMAGE_DECLARE(newyear_title);
LV_IMAGE_DECLARE(newyear_button);

LV_IMAGE_DECLARE(val_title);
LV_IMAGE_DECLARE(val_button);

LV_IMAGE_DECLARE(mb_title);
LV_IMAGE_DECLARE(mb_button);

LV_IMAGE_DECLARE(ab_title);
LV_IMAGE_DECLARE(ab_button);


// =====================================================
// STATIC DATA
// =====================================================

SpecialEventManager*
SpecialEventIntroScreen::currentSpecialEventManager =
    nullptr;


GameManager*
SpecialEventIntroScreen::currentGame =
    nullptr;


SpecialEventType
SpecialEventIntroScreen::currentEventType =
    SpecialEventType::NONE;


// =====================================================
// ASSET MAPPING
// =====================================================

namespace {

const lv_image_dsc_t*
getTitleAsset(
    SpecialEventType eventType
)
{
    switch (
        eventType
    ) {

        case SpecialEventType::MONTHIVERSARY:
            return &monthi_title;


        case SpecialEventType::HALLOWEEN:
            return &halloween_title;


        case SpecialEventType::CHRISTMAS:
            return &xmas_title;


        case SpecialEventType::NEW_YEARS_EVE:
            return &anewyear_title;


        case SpecialEventType::NEW_YEAR:
            return &newyear_title;


        case SpecialEventType::VALENTINE:
            return &val_title;


        case SpecialEventType::BIRTHDAY_SEPTEMBER:
            return &mb_title;


        case SpecialEventType::BIRTHDAY_OCTOBER:
            return &ab_title;


        case SpecialEventType::NONE:
        default:
            return nullptr;
    }
}


const lv_image_dsc_t*
getButtonAsset(
    SpecialEventType eventType
)
{
    switch (
        eventType
    ) {

        case SpecialEventType::MONTHIVERSARY:
            return &monthi_button;


        case SpecialEventType::HALLOWEEN:
            return &halloween_button;


        case SpecialEventType::CHRISTMAS:
            return &xmas_button;


        case SpecialEventType::NEW_YEARS_EVE:
            return &anewyear_button;


        case SpecialEventType::NEW_YEAR:
            return &newyear_button;


        case SpecialEventType::VALENTINE:
            return &val_button;


        case SpecialEventType::BIRTHDAY_SEPTEMBER:
            return &mb_button;


        case SpecialEventType::BIRTHDAY_OCTOBER:
            return &ab_button;


        case SpecialEventType::NONE:
        default:
            return nullptr;
    }
}


int getTitleHeight(
    SpecialEventType eventType
)
{
    if (
        eventType
        == SpecialEventType::NEW_YEARS_EVE
    ) {
        return 129;
    }


    return 85;
}

}


// =====================================================
// CONTINUE BUTTON EVENT
// =====================================================

void
SpecialEventIntroScreen::continueButtonEvent(
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


    SpecialDayBackground::stop();


    SpecialEventScreen::create(
        *currentSpecialEventManager,
        *currentGame,
        currentEventType
    );
}


// =====================================================
// CREATE
// =====================================================

void
SpecialEventIntroScreen::create(
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


    SpecialDayBackground::stop();


    lv_obj_t* screen =
        lv_screen_active();


    lv_obj_clean(
        screen
    );


    // =================================================
    // SPECIAL-DAY BACKGROUND
    // =================================================

    SpecialDayBackground::create(
        screen,
        eventType
    );


    // =================================================
    // TITLE
    // =================================================

    const lv_image_dsc_t* titleAsset =
        getTitleAsset(
            eventType
        );


    if (
        titleAsset != nullptr
    ) {

        lv_obj_t* title =
            lv_image_create(
                screen
            );


        lv_image_set_src(
            title,
            titleAsset
        );


        lv_obj_set_size(
            title,
            464,
            getTitleHeight(
                eventType
            )
        );


        lv_obj_align(
            title,
            LV_ALIGN_TOP_MID,
            0,
            95
        );


        lv_obj_remove_flag(
            title,
            LV_OBJ_FLAG_CLICKABLE
        );
    }


    // =================================================
    // BUTTON ASSET
    // =================================================

    const lv_image_dsc_t* buttonAsset =
        getButtonAsset(
            eventType
        );


    if (
        buttonAsset == nullptr
    ) {
        return;
    }


    // =================================================
    // CLICKABLE TRANSPARENT BUTTON
    // =================================================

    lv_obj_t* button =
        lv_button_create(
            screen
        );


    lv_obj_set_size(
        button,
        275,
        105
    );


    lv_obj_align(
        button,
        LV_ALIGN_BOTTOM_MID,
        0,
        -70
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
    // BUTTON IMAGE
    // =================================================

    lv_obj_t* buttonImage =
        lv_image_create(
            button
        );


    lv_image_set_src(
        buttonImage,
        buttonAsset
    );


    lv_obj_center(
        buttonImage
    );


    lv_obj_remove_flag(
        buttonImage,
        LV_OBJ_FLAG_CLICKABLE
    );


    // =================================================
    // CLICK EVENT
    // =================================================

    lv_obj_add_event_cb(
        button,
        continueButtonEvent,
        LV_EVENT_CLICKED,
        nullptr
    );
}
