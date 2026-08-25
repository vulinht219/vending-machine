#pragma once

#include <lvgl.h>

class Theme {
public:
    // ---------- Design tokens ----------
    static constexpr int SPACING_XS = 4;
    static constexpr int SPACING_SM = 8;
    static constexpr int SPACING_MD = 12;
    static constexpr int SPACING_LG = 16;
    static constexpr int SPACING_XL = 24;

    static constexpr int RADIUS_SM = 4;
    static constexpr int RADIUS_MD = 8;
    static constexpr int RADIUS_LG = 12;

    // ---------- Screen ----------
    static void applyScreen(lv_obj_t* screen);

    // ---------- Text ----------
    static void applyTitle(lv_obj_t* label);
    static void applyNormalText(lv_obj_t* label);
    static void applyButtonText(lv_obj_t* label);

    // ---------- Containers ----------
    static void applyPanel(lv_obj_t* panel);
    static void applyAnswerBox(lv_obj_t* box);

    // ---------- Buttons ----------
    static void applyPrimaryButton(lv_obj_t* button);
    static void applySecondaryButton(lv_obj_t* button);
    static void applyKeypadButton(lv_obj_t* button);
};