#include "HomeScreen.h"

#include "esp_log.h"


LV_IMAGE_DECLARE(home_1);
LV_IMAGE_DECLARE(home_2);
LV_IMAGE_DECLARE(home_3);
LV_IMAGE_DECLARE(home_title);
LV_IMAGE_DECLARE(home_button);


namespace {

constexpr const char* TAG =
    "HomeScreen";


constexpr int SCREEN_WIDTH =
    480;

constexpr int SCREEN_HEIGHT =
    800;


constexpr int ANIMATION_INTERVAL_MS =
    400;


// =====================================================
// HOME FRAMES
// =====================================================

const lv_image_dsc_t* homeFrames[] = {
    &home_1,
    &home_2,
    &home_3
};


constexpr int HOME_FRAME_COUNT =
    3;


int currentHomeFrame =
    0;


lv_timer_t* homeAnimationTimer =
    nullptr;


// =====================================================
// DIRTY REGIONS
// =====================================================
//
// Generated from the difference between:
//
// home_1
// home_2
// home_3
//
// Original dirty tiles:
//
//     83 x 32x32
//
// Adjacent horizontal tiles have been merged into
// 29 regions to reduce LVGL object overhead.
//
// Each region clips a full-size frame image so LVGL
// only redraws the pixels inside that region.
//

struct DirtyRegion
{
    int x;
    int y;
    int width;
    int height;

    lv_obj_t* container;
    lv_obj_t* image;
};


DirtyRegion dirtyRegions[] = {

    {288,   0,  96, 32, nullptr, nullptr},
    {288,  32,  96, 32, nullptr, nullptr},

    {160,  64,  96, 32, nullptr, nullptr},
    {288,  64,  96, 32, nullptr, nullptr},

    { 96,  96, 160, 32, nullptr, nullptr},
    { 96, 128, 128, 32, nullptr, nullptr},
    {128, 160,  64, 32, nullptr, nullptr},

    {  0, 192,  64, 32, nullptr, nullptr},
    {128, 192,  32, 32, nullptr, nullptr},

    {  0, 224,  96, 32, nullptr, nullptr},
    {160, 224,  64, 32, nullptr, nullptr},
    {288, 224,  32, 32, nullptr, nullptr},

    {  0, 256,  96, 32, nullptr, nullptr},
    {160, 256,  64, 32, nullptr, nullptr},
    {256, 256,  64, 32, nullptr, nullptr},

    {  0, 288,  32, 32, nullptr, nullptr},
    {192, 288,  96, 32, nullptr, nullptr},

    {256, 544,  96, 32, nullptr, nullptr},

    {224, 576, 128, 32, nullptr, nullptr},
    {224, 608, 128, 32, nullptr, nullptr},

    {320, 640,  64, 32, nullptr, nullptr},
    {416, 640,  64, 32, nullptr, nullptr},

    {320, 672, 160, 32, nullptr, nullptr},

    {  0, 704,  96, 32, nullptr, nullptr},
    {352, 704, 128, 32, nullptr, nullptr},

    {  0, 736, 128, 32, nullptr, nullptr},
    {384, 736,  64, 32, nullptr, nullptr},

    {  0, 768, 160, 32, nullptr, nullptr},
    {384, 768,  64, 32, nullptr, nullptr}
};


constexpr int DIRTY_REGION_COUNT =
    sizeof(dirtyRegions) /
    sizeof(dirtyRegions[0]);


// =====================================================
// UPDATE ANIMATION
// =====================================================

void updateHomeAnimation(
    lv_timer_t*
)
{
    currentHomeFrame =
        (
            currentHomeFrame + 1
        )
        %
        HOME_FRAME_COUNT;


    const lv_image_dsc_t* frame =
        homeFrames[
            currentHomeFrame
        ];


    // =================================================
    // UPDATE ONLY DIRTY REGIONS
    // =================================================

    for (
        int i = 0;
        i < DIRTY_REGION_COUNT;
        ++i
    ) {
        DirtyRegion& region =
            dirtyRegions[i];


        if (
            region.image == nullptr
        ) {
            continue;
        }


        lv_image_set_src(
            region.image,
            frame
        );
    }
}


// =====================================================
// CREATE ONE CLIPPED REGION
// =====================================================

void createDirtyRegion(
    lv_obj_t* screen,
    DirtyRegion& region
)
{
    // =================================================
    // CONTAINER
    // =================================================
    //
    // This object defines the visible clipped area.
    //

    region.container =
        lv_obj_create(
            screen
        );


    lv_obj_remove_style_all(
        region.container
    );


    lv_obj_set_size(
        region.container,
        region.width,
        region.height
    );


    lv_obj_set_pos(
        region.container,
        region.x,
        region.y
    );


    lv_obj_clear_flag(
        region.container,
        LV_OBJ_FLAG_SCROLLABLE
    );


    lv_obj_clear_flag(
        region.container,
        LV_OBJ_FLAG_CLICKABLE
    );


    // =================================================
    // FULL-SIZE IMAGE INSIDE CLIPPED CONTAINER
    // =====================================================
    //
    // The image is still 480x800.
    //
    // It is shifted so that the required portion of the
    // full frame appears inside the small container.
    //

    region.image =
        lv_image_create(
            region.container
        );


    lv_image_set_src(
        region.image,
        &home_1
    );


    lv_obj_set_pos(
        region.image,
        -region.x,
        -region.y
    );


    lv_obj_clear_flag(
        region.image,
        LV_OBJ_FLAG_CLICKABLE
    );
}


} // namespace


// =====================================================
// START BUTTON
// =====================================================

void HomeScreen::startButtonEvent(
    lv_event_t* event
)
{
    if (
        lv_event_get_code(
            event
        )
        !=
        LV_EVENT_CLICKED
    ) {
        return;
    }


    ESP_LOGI(
        TAG,
        "START BUTTON CLICKED"
    );


    // =================================================
    // TEMPORARY
    // =================================================
    //
    // Later:
    //
    // game->startGame();
    // QuizScreen::create(*game);
    //
}


// =====================================================
// CREATE
// =====================================================

void HomeScreen::create()
{
    lv_obj_t* screen =
        lv_screen_active();


    // =================================================
    // STOP PREVIOUS TIMER
    // =================================================

    if (
        homeAnimationTimer != nullptr
    ) {
        lv_timer_delete(
            homeAnimationTimer
        );


        homeAnimationTimer =
            nullptr;
    }


    // =================================================
    // RESET REGIONS
    // =================================================

    for (
        int i = 0;
        i < DIRTY_REGION_COUNT;
        ++i
    ) {
        dirtyRegions[i].container =
            nullptr;

        dirtyRegions[i].image =
            nullptr;
    }


    // =================================================
    // CLEAN SCREEN
    // =================================================

    lv_obj_clean(
        screen
    );


    currentHomeFrame =
        0;


    // =================================================
    // STATIC BACKGROUND
    // =================================================
    //
    // home_1 is rendered once as the base.
    //

    lv_obj_t* background =
        lv_image_create(
            screen
        );


    lv_image_set_src(
        background,
        &home_1
    );


    lv_obj_align(
        background,
        LV_ALIGN_CENTER,
        0,
        0
    );


    lv_obj_clear_flag(
        background,
        LV_OBJ_FLAG_CLICKABLE
    );


    // =================================================
    // DIRTY ANIMATION REGIONS
    // =================================================

    for (
        int i = 0;
        i < DIRTY_REGION_COUNT;
        ++i
    ) {
        createDirtyRegion(
            screen,
            dirtyRegions[i]
        );
    }


    // =================================================
    // TITLE
    // =================================================

    lv_obj_t* title =
        lv_image_create(
            screen
        );


    lv_image_set_src(
        title,
        &home_title
    );


    lv_obj_align(
        title,
        LV_ALIGN_TOP_MID,
        0,
        90
    );


    lv_obj_clear_flag(
        title,
        LV_OBJ_FLAG_CLICKABLE
    );


    // =================================================
    // START BUTTON
    // =================================================

    lv_obj_t* button =
        lv_image_create(
            screen
        );


    lv_image_set_src(
        button,
        &home_button
    );


    lv_obj_add_flag(
        button,
        LV_OBJ_FLAG_CLICKABLE
    );


    lv_obj_align(
        button,
        LV_ALIGN_CENTER,
        0,
        180
    );


    lv_obj_add_event_cb(
        button,
        startButtonEvent,
        LV_EVENT_CLICKED,
        nullptr
    );


    // =================================================
    // START ANIMATION
    // =================================================

    homeAnimationTimer =
        lv_timer_create(
            updateHomeAnimation,
            ANIMATION_INTERVAL_MS,
            nullptr
        );


    ESP_LOGI(
        TAG,
        "Home screen created with dirty-region animation"
    );


    ESP_LOGI(
        TAG,
        "Dirty regions: %d",
        DIRTY_REGION_COUNT
    );
}