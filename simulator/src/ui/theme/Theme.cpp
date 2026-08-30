#include "Theme.h"

LV_FONT_DECLARE(quiz_symbols_18);

// =========================================
// COLORS
// =========================================

static const uint32_t COLOR_BACKGROUND = 0x171525;
static const uint32_t COLOR_PANEL = 0x242136;
static const uint32_t COLOR_PRIMARY = 0xFF5C9A;
static const uint32_t COLOR_SECONDARY = 0x514B6B;
static const uint32_t COLOR_KEYPAD = 0x332E49;
static const uint32_t COLOR_TEXT = 0xFFFFFF;
static const uint32_t COLOR_BORDER = 0x81789E;


// =========================================
// FONT
// =========================================

static const lv_font_t* getQuizTextFont()
{
    static lv_font_t quizTextFont;
    static bool initialized = false;

    if (!initialized) {
        quizTextFont = *LV_FONT_DEFAULT;

        // Normal characters:
        //   LV_FONT_DEFAULT
        //
        // Missing quiz symbols:
        //   € £ ° × ÷
        //       ↓
        //   quiz_symbols_18

        quizTextFont.fallback = &quiz_symbols_18;

        initialized = true;
    }

    return &quizTextFont;
}


// =========================================
// SCREEN
// =========================================

void Theme::applyScreen(lv_obj_t* screen)
{
    lv_obj_set_style_bg_color(
        screen,
        lv_color_hex(COLOR_BACKGROUND),
        0
    );

    lv_obj_set_style_bg_opa(
        screen,
        LV_OPA_COVER,
        0
    );
}


// =========================================
// TITLE
// =========================================

void Theme::applyTitle(lv_obj_t* label)
{
    lv_obj_set_style_text_color(
        label,
        lv_color_hex(COLOR_TEXT),
        0
    );

    lv_obj_set_style_text_font(
        label,
        getQuizTextFont(),
        0
    );
}


// =========================================
// NORMAL TEXT
// =========================================

void Theme::applyNormalText(lv_obj_t* label)
{
    lv_obj_set_style_text_color(
        label,
        lv_color_hex(COLOR_TEXT),
        0
    );

    lv_obj_set_style_text_font(
        label,
        getQuizTextFont(),
        0
    );
}


// =========================================
// BUTTON TEXT
// =========================================

void Theme::applyButtonText(lv_obj_t* label)
{
    lv_obj_set_style_text_color(
        label,
        lv_color_hex(COLOR_TEXT),
        0
    );

    lv_obj_set_style_text_font(
        label,
        getQuizTextFont(),
        0
    );
}


// =========================================
// PANEL
// =========================================

void Theme::applyPanel(lv_obj_t* panel)
{
    lv_obj_set_style_bg_color(
        panel,
        lv_color_hex(COLOR_PANEL),
        0
    );

    lv_obj_set_style_border_width(
        panel,
        0,
        0
    );

    lv_obj_set_style_radius(
        panel,
        RADIUS_MD,
        0
    );
}


// =========================================
// ANSWER BOX
// =========================================

void Theme::applyAnswerBox(lv_obj_t* box)
{
    lv_obj_set_style_bg_color(
        box,
        lv_color_hex(COLOR_PANEL),
        0
    );

    lv_obj_set_style_border_color(
        box,
        lv_color_hex(COLOR_BORDER),
        0
    );

    lv_obj_set_style_border_width(
        box,
        2,
        0
    );

    lv_obj_set_style_radius(
        box,
        RADIUS_MD,
        0
    );
}


// =========================================
// PRIMARY BUTTON
// =========================================

void Theme::applyPrimaryButton(lv_obj_t* button)
{
    lv_obj_set_style_bg_color(
        button,
        lv_color_hex(COLOR_PRIMARY),
        0
    );

    lv_obj_set_style_radius(
        button,
        RADIUS_MD,
        0
    );

    lv_obj_set_style_border_width(
        button,
        0,
        0
    );
}


// =========================================
// SECONDARY BUTTON
// =========================================

void Theme::applySecondaryButton(lv_obj_t* button)
{
    lv_obj_set_style_bg_color(
        button,
        lv_color_hex(COLOR_SECONDARY),
        0
    );

    lv_obj_set_style_radius(
        button,
        RADIUS_MD,
        0
    );

    lv_obj_set_style_border_width(
        button,
        0,
        0
    );
}


// =========================================
// KEYPAD BUTTON
// =========================================

void Theme::applyKeypadButton(lv_obj_t* button)
{
    lv_obj_set_style_bg_color(
        button,
        lv_color_hex(COLOR_KEYPAD),
        0
    );

    lv_obj_set_style_radius(
        button,
        RADIUS_SM,
        0
    );

    lv_obj_set_style_border_width(
        button,
        0,
        0
    );
}