#include "ui/screens/SpecialEventIntroScreen.h"

#include "ui/SpecialEventScreen.h"
#include "ui/theme/Theme.h"


SpecialEventManager*
SpecialEventIntroScreen::currentSpecialEventManager =
    nullptr;


GameManager*
SpecialEventIntroScreen::currentGame =
    nullptr;


SpecialEventType
SpecialEventIntroScreen::currentEventType =
    SpecialEventType::NONE;



const char*
SpecialEventIntroScreen::getTitle(
    SpecialEventType eventType
)
{
    switch (
        eventType
    ) {

        case SpecialEventType::MONTHIVERSARY:
            return "Happy monthiversaryy";


        case SpecialEventType::NEW_YEAR:
            return "happy new yearr";


        case SpecialEventType::VALENTINE:
            return "Happy Valentine";


        case SpecialEventType::BIRTHDAY_SEPTEMBER:
            return "It's my birthday, have a sweet";


        case SpecialEventType::BIRTHDAY_OCTOBER:
            return "Happy birthdayy";


        case SpecialEventType::HALLOWEEN:
            return "Trick or treat!";


        case SpecialEventType::CHRISTMAS:
            return "Merry Xmas";


        case SpecialEventType::NEW_YEARS_EVE:
            return "One last sweet this year?";


        case SpecialEventType::NONE:
        default:
            return "";
    }
}



const char*
SpecialEventIntroScreen::getButtonText(
    SpecialEventType eventType
)
{
    switch (
        eventType
    ) {

        case SpecialEventType::MONTHIVERSARY:
            return "HAPPY MONTHIVERSARY";


        case SpecialEventType::NEW_YEAR:
            return "HAPPY NEW YEAR";


        case SpecialEventType::VALENTINE:
            return "HAPPY VALENTINE";


        case SpecialEventType::BIRTHDAY_SEPTEMBER:
            return "HAPPY BIRTHDAY";


        case SpecialEventType::BIRTHDAY_OCTOBER:
            return "HAPPY BIRTHDAY";


        case SpecialEventType::HALLOWEEN:
            return "TRICK OR TREAT";


        case SpecialEventType::CHRISTMAS:
            return "MERRY XMAS";


        case SpecialEventType::NEW_YEARS_EVE:
            return "ONE LAST SWEET";


        case SpecialEventType::NONE:
        default:
            return "CONTINUE";
    }
}



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


    SpecialEventScreen::create(
        *currentSpecialEventManager,
        *currentGame,
        currentEventType
    );
}



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


    lv_obj_t* screen =
        lv_screen_active();


    lv_obj_clean(
        screen
    );


    Theme::applyScreen(
        screen
    );


    // =====================================================
    // MAIN CONTENT
    // =====================================================

    lv_obj_t* content =
        lv_obj_create(
            screen
        );


    lv_obj_remove_style_all(
        content
    );


    lv_obj_set_size(
        content,
        LV_PCT(100),
        LV_PCT(100)
    );


    lv_obj_set_style_pad_all(
        content,
        Theme::SPACING_LG,
        0
    );


    lv_obj_clear_flag(
        content,
        LV_OBJ_FLAG_SCROLLABLE
    );


    // =====================================================
    // CENTER AREA
    // =====================================================

    lv_obj_t* centerArea =
        lv_obj_create(
            content
        );


    lv_obj_remove_style_all(
        centerArea
    );


    lv_obj_set_size(
        centerArea,
        LV_PCT(100),
        LV_PCT(100)
    );


    lv_obj_set_flex_flow(
        centerArea,
        LV_FLEX_FLOW_COLUMN
    );


    lv_obj_set_flex_align(
        centerArea,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER
    );


    lv_obj_clear_flag(
        centerArea,
        LV_OBJ_FLAG_SCROLLABLE
    );


    // =====================================================
    // EVENT CONTENT
    // =====================================================

    lv_obj_t* eventContent =
        lv_obj_create(
            centerArea
        );


    lv_obj_remove_style_all(
        eventContent
    );


    lv_obj_set_width(
        eventContent,
        LV_PCT(100)
    );


    lv_obj_set_height(
        eventContent,
        LV_SIZE_CONTENT
    );


    lv_obj_set_flex_flow(
        eventContent,
        LV_FLEX_FLOW_COLUMN
    );


    lv_obj_set_flex_align(
        eventContent,
        LV_FLEX_ALIGN_START,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER
    );


    lv_obj_set_style_pad_row(
        eventContent,
        Theme::SPACING_LG,
        0
    );


    lv_obj_clear_flag(
        eventContent,
        LV_OBJ_FLAG_SCROLLABLE
    );


    // =====================================================
    // TITLE
    // =====================================================

    lv_obj_t* title =
        lv_label_create(
            eventContent
        );


    lv_label_set_text(
        title,
        getTitle(
            eventType
        )
    );


    Theme::applyTitle(
        title
    );


    lv_obj_set_width(
        title,
        LV_PCT(90)
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


    // =====================================================
    // CONTINUE BUTTON
    // =====================================================

    lv_obj_t* button =
        lv_button_create(
            eventContent
        );


    Theme::applyPrimaryButton(
        button
    );


    lv_obj_set_size(
        button,
        300,
        80
    );


    lv_obj_t* buttonLabel =
        lv_label_create(
            button
        );


    lv_label_set_text(
        buttonLabel,
        getButtonText(
            eventType
        )
    );


    Theme::applyButtonText(
        buttonLabel
    );


    lv_obj_center(
        buttonLabel
    );


    lv_obj_add_event_cb(
        button,
        continueButtonEvent,
        LV_EVENT_CLICKED,
        nullptr
    );
}