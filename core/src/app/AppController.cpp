#include "AppController.h"

#include "ui/HomeScreen.h"
#include "ui/CandySelectScreen.h"
#include "ui/SpecialEventScreen.h"
#include "ui/screens/SpecialEventIntroScreen.h"
#include "DisplayManager.h"


AppController::AppController(
    GameManager& game,
    SpecialEventManager& specialEventManager
)
    : game(game),
      specialEventManager(
          specialEventManager
      )
{
}


void AppController::start()
{
    // =================================================
    // PRIORITY 1:
    // QUIZ REWARD THAT WAS NOT YET CLAIMED
    // =================================================

    if (
        game.hasPendingReward()
    ) {

        CandySelectScreen::create(
            game
        );

        return;
    }


    // =================================================
    // PRIORITY 2:
    // SPECIAL DAY FREE CANDY
    // =================================================

    if (
        specialEventManager
            .shouldShowEvent()
    ) {

        SpecialEventScreen::create(
            specialEventManager,
            game
        );

        return;
    }


    // =================================================
    // PRIORITY 3:
    // NORMAL HOME
    // =================================================

    HomeScreen::create(
        game
    );

    // =====================================================
    // TEMPORARY LCD HARDWARE SELF TEST
    // =====================================================

    if (!DisplayManager::initialize())
    {
        ESP_LOGE(
            TAG,
            "LCD initialization failed"
        );

        return false;
    }


    if (!DisplayManager::runSelfTest())
    {
       ESP_LOGE(
            TAG,
            "LCD self-test failed"
        );

        return false;
    }


    if (!DisplayManager::setBacklight(true))
    {
        ESP_LOGE(
            TAG,
            "LCD backlight enable failed"
       );

        return false;
    }
}