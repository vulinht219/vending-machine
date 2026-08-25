#include "CandySelectScreen.h"
#include "DispensingScreen.h"
#include "theme/Theme.h"

GameManager* CandySelectScreen::currentGame = nullptr;


void CandySelectScreen::candyButtonEvent(lv_event_t* event)
{
    if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
        return;
    }

    int slot =
        static_cast<int>(
            reinterpret_cast<intptr_t>(
                lv_event_get_user_data(event)
            )
        );

    DispensingScreen::create(
        *currentGame,
        slot
    );
}


void CandySelectScreen::create(GameManager& game)
{
    currentGame = &game;

    lv_obj_t* screen =
        lv_screen_active();

    lv_obj_clean(screen);

    Theme::applyScreen(screen);


    // =================================================
    // MAIN CONTENT
    // =================================================

    lv_obj_t* content =
        lv_obj_create(screen);

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

    lv_obj_set_style_pad_row(
        content,
        Theme::SPACING_LG,
        0
    );

    lv_obj_set_flex_flow(
        content,
        LV_FLEX_FLOW_COLUMN
    );

    lv_obj_set_flex_align(
        content,
        LV_FLEX_ALIGN_START,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER
    );


    // =================================================
    // TITLE
    // =================================================

    lv_obj_t* title =
        lv_label_create(content);

    lv_label_set_text(
        title,
        "CHOOSE YOUR CANDY"
    );

    Theme::applyTitle(title);


    // =================================================
    // SUBTITLE
    // =================================================

    lv_obj_t* subtitle =
        lv_label_create(content);

    lv_label_set_text(
        subtitle,
        "Pick one reward"
    );

    Theme::applyNormalText(subtitle);


    // =================================================
    // CANDY GRID
    // =================================================

    lv_obj_t* grid =
        lv_obj_create(content);

    lv_obj_set_width(
        grid,
        LV_PCT(95)
    );

    lv_obj_set_height(
        grid,
        LV_PCT(65)
    );

    Theme::applyPanel(grid);

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

    for (int i = 0; i < 6; i++) {

        int row =
            i / 3;

        int column =
            i % 3;


        lv_obj_t* button =
            lv_button_create(grid);

        lv_obj_set_grid_cell(
            button,

            LV_GRID_ALIGN_STRETCH,
            column,
            1,

            LV_GRID_ALIGN_STRETCH,
            row,
            1
        );

        Theme::applyPrimaryButton(button);


        lv_obj_t* label =
            lv_label_create(button);

        char text[16];

        snprintf(
            text,
            sizeof(text),
            "CANDY %d",
            i + 1
        );

        lv_label_set_text(
            label,
            text
        );

        Theme::applyButtonText(label);

        lv_obj_set_style_text_align(
            label,
            LV_TEXT_ALIGN_CENTER,
            0
        );

        lv_obj_center(label);


        lv_obj_add_event_cb(
            button,
            candyButtonEvent,
            LV_EVENT_CLICKED,
            reinterpret_cast<void*>(
                static_cast<intptr_t>(i + 1)
            )
        );
    }
}