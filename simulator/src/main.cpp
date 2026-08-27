#include <SDL2/SDL.h>
#include <lvgl.h>

#include <chrono>
#include <thread>

#include "config/AppConfig.h"

#include "game/GameManager.h"

#include "quiz/FileQuizSource.h"
#include "quiz/FileQuizProgressStore.h"

#include "game/FileGameProgressStore.h"

#include "dispenser/MockDispenser.h"

#include "event/SpecialEventManager.h"

#include "platform/MockClock.h"
#include "platform/FileSpecialEventProgressStore.h"
#include "ui/HomeScreen.h"
#include "ui/CandySelectScreen.h"
#include "ui/SpecialEventScreen.h"

#include "app/AppController.h"


int main()
{
    // =================================================
    // SDL + LVGL
    // =================================================

    SDL_SetMainReady();

    lv_init();


    lv_display_t* display =
        lv_sdl_window_create(
            320,
            480
        );


    lv_indev_t* mouse =
        lv_sdl_mouse_create();


    lv_indev_set_display(
        mouse,
        display
    );


    // =================================================
    // CONFIG
    // =================================================

    AppConfig config;


    config.quizDatasetPath =
        "../dataset/production/quizzes_1000.jsonl";


    config.quizProgressPath =
        "../dataset/progress/quiz_progress.json";


    config.gameProgressPath =
        "../dataset/progress/game_progress.json";


    // =================================================
    // GAME PLATFORM
    // =================================================

    MockDispenser dispenser;


    FileQuizSource quizSource(
        config.quizDatasetPath
    );


    FileQuizProgressStore
        quizProgressStore(
            config.quizProgressPath
        );


    FileGameProgressStore
        gameProgressStore(
            config.gameProgressPath
        );


    GameManager game(
        dispenser,
        quizSource,
        quizProgressStore,
        gameProgressStore
    );


    // =================================================
    // SPECIAL EVENT PLATFORM
    // =================================================
    //
    // TEMP TEST DATE:
    //
    // 25 December 2026
    //
    // Later simulator can use SystemClock
    // and firmware will use DS3231Clock.
    // =================================================

    MockClock clock(
        2026,
        12,
        25
    );


    FileSpecialEventProgressStore
        specialEventProgressStore(
            "../dataset/progress/special_event_progress.txt"
        );


    SpecialEventManager
        specialEventManager(
            clock,
            specialEventProgressStore,
            dispenser
        );


    // =================================================
    // STARTUP ROUTING
    // =================================================
    //
    // Priority:
    //
    // 1. Pending quiz reward
    // 2. Special event
    // 3. Home
    //
    // =================================================

    AppController app(
        game,
        specialEventManager
    );

    app.start();


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