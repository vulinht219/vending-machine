#include <SDL2/SDL.h>
#include <lvgl.h>

#include <chrono>
#include <thread>

#include "game/GameManager.h"
#include "dispenser/MockDispenser.h"

#include "ui/HomeScreen.h"
#include "ui/CandySelectScreen.h"


int main()
{
    SDL_SetMainReady();

    lv_init();


    // =================================================
    // CREATE SIMULATED DISPLAY
    // =================================================

    lv_display_t* display =
        lv_sdl_window_create(
            320,
            480
        );


    // =================================================
    // MOUSE = TOUCHSCREEN
    // =================================================

    lv_indev_t* mouse =
        lv_sdl_mouse_create();

    lv_indev_set_display(
        mouse,
        display
    );


    // =================================================
    // GAME
    // =================================================

    MockDispenser dispenser;

    GameManager game(
        dispenser
    );


    // =================================================
    // STARTUP ROUTING
    // =================================================
    //
    // If the user already solved a quiz but did not
    // receive a candy yet, restore Candy Select.
    //
    // Otherwise show normal Home screen.
    // =================================================

    if (
        game.hasPendingReward()
    ) {

        CandySelectScreen::create(
            game
        );
    }

    else {

        HomeScreen::create(
            game
        );
    }


    // =================================================
    // LVGL MAIN LOOP
    // =================================================

    while (true) {

        uint32_t wait =
            lv_timer_handler();


        if (wait < 1) {
            wait = 1;
        }


        if (wait > 10) {
            wait = 10;
        }


        std::this_thread::sleep_for(
            std::chrono::milliseconds(
                wait
            )
        );
    }


    return 0;
}