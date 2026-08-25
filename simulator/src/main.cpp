#include <SDL2/SDL.h>
#include <lvgl.h>

#include <chrono>
#include <thread>

#include "config/AppConfig.h"

#include "game/GameManager.h"
#include "game/FileGameProgressStore.h"

#include "quiz/FileQuizSource.h"
#include "quiz/FileQuizProgressStore.h"

#include "dispenser/MockDispenser.h"

#include "ui/HomeScreen.h"
#include "ui/CandySelectScreen.h"


int main()
{
    SDL_SetMainReady();

    lv_init();


    // =================================================
    // DISPLAY
    // =================================================

    lv_display_t* display =
        lv_sdl_window_create(
            320,
            480
        );


    // =================================================
    // INPUT
    // =================================================

    lv_indev_t* mouse =
        lv_sdl_mouse_create();

    lv_indev_set_display(
        mouse,
        display
    );


    // =================================================
    // APP CONFIG
    // =================================================

    AppConfig config;

    config.quizDatasetPath =
       "../dataset/production/quizzes_1000.jsonl";

    config.quizProgressPath =
        "../dataset/progress/quiz_progress.json";

    config.gameProgressPath =
        "../dataset/progress/game_progress.json";


    // =================================================
    // PLATFORM IMPLEMENTATIONS
    // =================================================

    MockDispenser dispenser;


    FileQuizSource quizSource(
        config.quizDatasetPath
    );


    FileQuizProgressStore quizProgressStore(
        config.quizProgressPath
    );


    FileGameProgressStore gameProgressStore(
        config.gameProgressPath
    );


    // =================================================
    // GAME MANAGER
    // =================================================

    GameManager game(
        dispenser,
        quizSource,
        quizProgressStore,
        gameProgressStore
    );


    // =================================================
    // STARTUP ROUTING
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
    // LVGL LOOP
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