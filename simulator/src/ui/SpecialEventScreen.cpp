#include "SpecialEventScreen.h"

#include "DispensingScreen.h"

#include "theme/Theme.h"

#include "event/SpecialEventType.h"

#include <cstdint>
#include <string>


namespace {

// =====================================================
// SPECIAL EVENT TITLE
// =====================================================

const char* getSpecialEventTitle(
    SpecialEventType eventType
)
{
    switch (eventType)
    {
        case SpecialEventType::MONTHIVERSARY:
            return "Happy monthiversaryy";

        case SpecialEventType::HALLOWEEN:
            return "Trick or treat!";

        case SpecialEventType::CHRISTMAS:
            return "Merry Christmas";

        case SpecialEventType::NEW_YEARS_EVE:
            return "One last sweet this year?";

        case SpecialEventType::NEW_YEAR:
            return "happy new yearr";

        case SpecialEventType::VALENTINE:
            return "Happy Valentine";

        case SpecialEventType::BIRTHDAY_SEPTEMBER:
            return "It's my birthday, have a sweet";

        case SpecialEventType::BIRTHDAY_OCTOBER:
            return "Happy birthdayy";

        case SpecialEventType::NONE:
        default:
            return "Special Day!";
    }
}

} // namespace


// =====================================================
// STATIC DATA
// =====================================================

SpecialEventManager*
    SpecialEventScreen::currentSpecialEventManager =
        nullptr;


GameManager*
    SpecialEventScreen::currentGame =
        nullptr;


// =====================================================
// CANDY BUTTON EVENT
// =====================================================

void SpecialEventScreen::candyButtonEvent(
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
        currentSpecialEventManager
        == nullptr ||
        currentGame
        == nullptr
    ) {
        return;
    }


    int slot =
        static_cast<int>(
            reinterpret_cast<intptr_t>(
                lv_event_get_user_data(
                    event
                )
            )
        );


    // =================================================
    // CLAIM SPECIAL EVENT REWARD
    // =================================================

    bool success =
        currentSpecialEventManager
            ->selectCandy(
                slot
            );


    if (!success) {

        // Later:
        // show dispenser error / retry popup.

        return;
    }


    // =================================================
    // REUSE NORMAL DISPENSING SCREEN
    // =================================================

    DispensingScreen::create(
        *currentGame
    );
}


// =====================================================
// CREATE SPECIAL EVENT SCREEN
// =====================================================

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


    lv_obj_t* screen =
        lv_screen_active();


    lv_obj_clean(
        screen
    );


    Theme::applyScreen(
        screen
    );


    // =================================================
    // MAIN CONTENT
    // =================================================

    lv_obj_t* content =
        lv_obj_create(
            screen
        );


    lv_obj_set_size(
        content,
        LV_PCT(100),
        LV_PCT(100)
    );


    lv_obj_set_style_bg_opa(
        content,
        LV_OPA_TRANSP,
        0
    );


    lv_obj_set_style_border_width(
        content,
        0,
        0
    );


    lv_obj_set_style_pad_all(
        content,
        Theme::SPACING_LG,
        0
    );


    // =================================================
    // CENTER AREA
    // =================================================
    //
    // Fills the whole screen.
    // The complete special-event block is centered here.
    //
    // =================================================

    lv_obj_t* centerArea =
        lv_obj_create(
            content
        );


    lv_obj_set_size(
        centerArea,
        LV_PCT(100),
        LV_PCT(100)
    );


    lv_obj_set_style_bg_opa(
        centerArea,
        LV_OPA_TRANSP,
        0
    );


    lv_obj_set_style_border_width(
        centerArea,
        0,
        0
    );


    lv_obj_set_style_pad_all(
        centerArea,
        0,
        0
    );


    lv_obj_set_scrollbar_mode(
        centerArea,
        LV_SCROLLBAR_MODE_OFF
    );


    lv_obj_remove_flag(
        centerArea,
        LV_OBJ_FLAG_SCROLLABLE
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


    // =================================================
    // SPECIAL EVENT CONTENT BLOCK
    // =================================================
    //
    // Title + subtitle + grid are one centered block.
    //
    // =================================================

    lv_obj_t* eventContent =
        lv_obj_create(
            centerArea
        );


    lv_obj_set_width(
        eventContent,
        LV_PCT(100)
    );


    lv_obj_set_height(
        eventContent,
        LV_SIZE_CONTENT
    );


    lv_obj_set_style_bg_opa(
        eventContent,
        LV_OPA_TRANSP,
        0
    );


    lv_obj_set_style_border_width(
        eventContent,
        0,
        0
    );


    lv_obj_set_style_pad_all(
        eventContent,
        0,
        0
    );


    lv_obj_set_style_pad_row(
        eventContent,
        Theme::SPACING_LG,
        0
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


    // =================================================
    // TITLE
    // =================================================

    lv_obj_t* title =
        lv_label_create(
            eventContent
        );


    lv_label_set_text(
        title,
        getSpecialEventTitle(
            eventType
        )
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


    Theme::applyTitle(
        title
    );


    // =================================================
    // SUBTITLE
    // =================================================

    lv_obj_t* subtitle =
        lv_label_create(
            eventContent
        );


    lv_label_set_text(
        subtitle,
        "Pick one free candy!"
    );


    lv_obj_set_style_text_align(
        subtitle,
        LV_TEXT_ALIGN_CENTER,
        0
    );


    Theme::applyNormalText(
        subtitle
    );


    // =================================================
    // CANDY GRID
    // =================================================

    lv_obj_t* grid =
        lv_obj_create(
            eventContent
        );


    lv_obj_set_width(
        grid,
        LV_PCT(95)
    );


    // Bigger than the old 280 px so touch targets
    // are more comfortable on 480x800 portrait.
    lv_obj_set_height(
        grid,
        500
    );


    Theme::applyPanel(
        grid
    );


    lv_obj_set_style_pad_all(
        grid,
        Theme::SPACING_MD,
        0
    );


    lv_obj_set_style_pad_row(
        grid,
        Theme::SPACING_MD,
        0
    );


    lv_obj_set_style_pad_column(
        grid,
        Theme::SPACING_MD,
        0
    );


    static int32_t columns[] = {

        LV_GRID_FR(1),
        LV_GRID_FR(1),
        LV_GRID_FR(1),

        LV_GRID_TEMPLATE_LAST
    };


    static int32_t rows[] = {

        LV_GRID_FR(1),
        LV_GRID_FR(1),

        LV_GRID_TEMPLATE_LAST
    };


    lv_obj_set_grid_dsc_array(
        grid,
        columns,
        rows
    );


    lv_obj_set_layout(
        grid,
        LV_LAYOUT_GRID
    );


    // =================================================
    // 6 CANDY BUTTONS
    // =================================================

    for (
        int i = 0;
        i < 6;
        ++i
    ) {

        int slot =
            i + 1;


        int row =
            i / 3;


        int column =
            i % 3;


        lv_obj_t* button =
            lv_button_create(
                grid
            );


        lv_obj_set_grid_cell(
            button,

            LV_GRID_ALIGN_STRETCH,
            column,
            1,

            LV_GRID_ALIGN_STRETCH,
            row,
            1
        );


        Theme::applyPrimaryButton(
            button
        );


        lv_obj_t* label =
            lv_label_create(
                button
            );


        std::string labelText =
            "CANDY "
            + std::to_string(
                slot
            );


        lv_label_set_text(
            label,
            labelText.c_str()
        );


        Theme::applyButtonText(
            label
        );


        lv_obj_set_style_text_align(
            label,
            LV_TEXT_ALIGN_CENTER,
            0
        );


        lv_obj_center(
            label
        );


        lv_obj_add_event_cb(
            button,
            candyButtonEvent,
            LV_EVENT_CLICKED,

            reinterpret_cast<void*>(
                static_cast<intptr_t>(
                    slot
                )
            )
        );
    }
}