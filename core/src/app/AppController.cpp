#include "AppController.h"

#include "ui/HomeScreen.h"
#include "ui/CandySelectScreen.h"
#include "ui/SpecialEventScreen.h"


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
}