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

#include "app/AppController.h"


namespace {

// =====================================================
// SIMULATOR DEBUG DATE
// =====================================================
//
// Change only these three values to preview a date.
//
// Examples:
//
// New Year:
//   2026, 1, 1
//
// Valentine:
//   2026, 2, 14
//
// Birthday:
//   2026, 9, 21
//
// Birthday:
//   2026, 10, 2
//
// Halloween:
//   2026, 10, 31
//
// Christmas:
//   2026, 12, 25
//
// New Year's Eve:
//   2026, 12, 31
//
// Monthiversary:
//   2026, 8, 25
//
// Normal day:
//   2026, 8, 24
//

constexpr int DEBUG_YEAR  = 2026;
constexpr int DEBUG_MONTH = 12;
constexpr int DEBUG_DAY   = 24;

}


// =====================================================
// MAIN
// =====================================================

int main()
{
    // =================================================
    // SDL + LVGL
    // =================================================

    SDL_SetMainReady();

    lv_init();


    lv_display_t* display =
        lv_sdl_window_create(
            480,
            800
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
        "../dataset/production/quizzes_20000.jsonl";


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

    MockClock clock(
        DEBUG_YEAR,
        DEBUG_MONTH,
        DEBUG_DAY
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