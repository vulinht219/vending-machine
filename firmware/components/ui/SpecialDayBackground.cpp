#include "SpecialDayBackground.h"


// =========================================================
// ASSETS
// =========================================================

LV_IMAGE_DECLARE(ab);

LV_IMAGE_DECLARE(anewyear_1);
LV_IMAGE_DECLARE(anewyear_2);
LV_IMAGE_DECLARE(anewyear_3);

LV_IMAGE_DECLARE(halloween_1);
LV_IMAGE_DECLARE(halloween_2);
LV_IMAGE_DECLARE(halloween_3);
LV_IMAGE_DECLARE(halloween_4);

LV_IMAGE_DECLARE(mb);

LV_IMAGE_DECLARE(monthi_1);
LV_IMAGE_DECLARE(monthi_2);
LV_IMAGE_DECLARE(monthi_3);
LV_IMAGE_DECLARE(monthi_4);

LV_IMAGE_DECLARE(newyear_1);
LV_IMAGE_DECLARE(newyear_2);
LV_IMAGE_DECLARE(newyear_3);

LV_IMAGE_DECLARE(val_1);
LV_IMAGE_DECLARE(val_2);
LV_IMAGE_DECLARE(val_3);
LV_IMAGE_DECLARE(val_4);

LV_IMAGE_DECLARE(xmas_1);
LV_IMAGE_DECLARE(xmas_2);
LV_IMAGE_DECLARE(xmas_3);
LV_IMAGE_DECLARE(xmas_4);


namespace
{

lv_timer_t* animationTimer = nullptr;

lv_obj_t* backgroundImage = nullptr;

const lv_image_dsc_t* const* activeFrames = nullptr;

int activeFrameCount = 0;

int activeFrameIndex = 0;


// =========================================================
// FRAME SETS
// =========================================================

const lv_image_dsc_t* anewyearFrames[] = {
    &anewyear_1,
    &anewyear_2,
    &anewyear_3
};

const lv_image_dsc_t* halloweenFrames[] = {
    &halloween_1,
    &halloween_2,
    &halloween_3,
    &halloween_4
};

const lv_image_dsc_t* monthiFrames[] = {
    &monthi_1,
    &monthi_2,
    &monthi_3,
    &monthi_4
};

const lv_image_dsc_t* newyearFrames[] = {
    &newyear_1,
    &newyear_2,
    &newyear_3
};

const lv_image_dsc_t* valFrames[] = {
    &val_1,
    &val_2,
    &val_3,
    &val_4
};

const lv_image_dsc_t* xmasFrames[] = {
    &xmas_1,
    &xmas_2,
    &xmas_3,
    &xmas_4
};


// =========================================================
// ANIMATION CALLBACK
// =========================================================

void animationTick(
    lv_timer_t*
)
{
    if (
        backgroundImage == nullptr ||
        activeFrames == nullptr ||
        activeFrameCount <= 1
    ) {
        return;
    }

    activeFrameIndex =
        (activeFrameIndex + 1)
        % activeFrameCount;

    lv_image_set_src(
        backgroundImage,
        activeFrames[activeFrameIndex]
    );
}

}


// =========================================================
// STOP
// =========================================================

void SpecialDayBackground::stop()
{
    if (
        animationTimer != nullptr
    ) {
        lv_timer_delete(
            animationTimer
        );

        animationTimer =
            nullptr;
    }

    backgroundImage =
        nullptr;

    activeFrames =
        nullptr;

    activeFrameCount =
        0;

    activeFrameIndex =
        0;
}


// =========================================================
// CREATE
// =========================================================

void SpecialDayBackground::create(
    lv_obj_t* parent,
    SpecialEventType eventType
)
{
    stop();

    backgroundImage =
        lv_image_create(
            parent
        );

    lv_obj_align(
        backgroundImage,
        LV_ALIGN_CENTER,
        0,
        0
    );

    lv_obj_remove_flag(
        backgroundImage,
        LV_OBJ_FLAG_CLICKABLE
    );

    activeFrameIndex =
        0;


    // =====================================================
    // CHOOSE SPECIAL-DAY BACKGROUND
    // =====================================================

    switch (
        eventType
    )
    {
        // -------------------------------------------------
        // 25th - Monthiversary
        // -------------------------------------------------

        case SpecialEventType::MONTHIVERSARY:

            activeFrames =
                monthiFrames;

            activeFrameCount =
                4;

            break;


        // -------------------------------------------------
        // Halloween
        // -------------------------------------------------

        case SpecialEventType::HALLOWEEN:

            activeFrames =
                halloweenFrames;

            activeFrameCount =
                4;

            break;


        // -------------------------------------------------
        // Christmas
        // -------------------------------------------------

        case SpecialEventType::CHRISTMAS:

            activeFrames =
                xmasFrames;

            activeFrameCount =
                4;

            break;


        // -------------------------------------------------
        // 31 December
        // -------------------------------------------------

        case SpecialEventType::NEW_YEARS_EVE:

            activeFrames =
                anewyearFrames;

            activeFrameCount =
                3;

            break;


        // -------------------------------------------------
        // 1 January
        // -------------------------------------------------

        case SpecialEventType::NEW_YEAR:

            activeFrames =
                newyearFrames;

            activeFrameCount =
                3;

            break;


        // -------------------------------------------------
        // Valentine
        // -------------------------------------------------

        case SpecialEventType::VALENTINE:

            activeFrames =
                valFrames;

            activeFrameCount =
                4;

            break;


        // -------------------------------------------------
        // 21 September birthday
        // static
        // -------------------------------------------------

        case SpecialEventType::BIRTHDAY_SEPTEMBER:

            lv_image_set_src(
                backgroundImage,
                &mb
            );

            return;


        // -------------------------------------------------
        // 2 October birthday
        // static
        // -------------------------------------------------

        case SpecialEventType::BIRTHDAY_OCTOBER:

            lv_image_set_src(
                backgroundImage,
                &ab
            );

            return;


        case SpecialEventType::NONE:
        default:

            lv_obj_delete(
                backgroundImage
            );

            backgroundImage =
                nullptr;

            return;
    }


    // =====================================================
    // FIRST FRAME
    // =====================================================

    lv_image_set_src(
        backgroundImage,
        activeFrames[0]
    );


    // =====================================================
    // START TIMER
    // =====================================================

    animationTimer =
        lv_timer_create(
            animationTick,
            400,
            nullptr
        );
}
