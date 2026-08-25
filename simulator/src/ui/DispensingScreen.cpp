#include "DispensingScreen.h"
#include "HomeScreen.h"
#include "theme/Theme.h"

GameManager* DispensingScreen::currentGame = nullptr;
int DispensingScreen::currentCandySlot = 0;
lv_timer_t* DispensingScreen::dispenseTimer = nullptr;

void DispensingScreen::create(
    GameManager& game,
    int candySlot
)
{
    currentGame = &game;
    currentCandySlot = candySlot;

    lv_obj_t* screen = lv_screen_active();
    lv_obj_clean(screen);

    Theme::applyScreen(screen);

    // Full-screen container
    lv_obj_t* content = lv_obj_create(screen);

    Theme::applyPanel(content);

    lv_obj_set_size(
        content,
        LV_PCT(100),
        LV_PCT(100)
    );

    lv_obj_set_style_border_width(
        content,
        0,
        0
    );

    // Temporary placeholder.
    // Later we replace this with GIF / sprite animation.
    lv_obj_t* label =
        lv_label_create(content);

    lv_label_set_text(
        label,
        "DISPENSING..."
    );

    Theme::applyTitle(label);

    lv_obj_center(label);

    // Start the actual dispensing operation.
    bool success =
        currentGame->selectCandy(
            currentCandySlot
        );

    if (!success) {

        lv_label_set_text(
            label,
            "DISPENSE ERROR!"
        );

        // Later:
        // ErrorScreen instead of simply returning home.
    }

    // Keep this screen visible for 5 seconds.
    dispenseTimer =
        lv_timer_create(
            dispenseFinished,
            5000,
            nullptr
        );

    lv_timer_set_repeat_count(
        dispenseTimer,
        1
    );
}

void DispensingScreen::dispenseFinished(
    lv_timer_t* timer
)
{
    dispenseTimer = nullptr;

    HomeScreen::create(
        *currentGame
    );
}