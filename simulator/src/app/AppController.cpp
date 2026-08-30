#include "AppController.h"

#include "ui/HomeScreen.h"
#include "ui/CandySelectScreen.h"
#include "ui/SpecialEventScreen.h"


AppController::AppController(
    GameManager& game,
    SpecialEventManager& specialEventManager
)
    : game(game),
      specialEventManager(specialEventManager),
      startupRouter(
          game,
          specialEventManager
      )
{
}


void AppController::start()
{
    switch (startupRouter.resolve())
    {
        case StartupRoute::PENDING_QUIZ_REWARD:

            CandySelectScreen::create(
                game
            );

            break;


        case StartupRoute::SPECIAL_EVENT:

            SpecialEventScreen::create(
                specialEventManager,
                game,
                specialEventManager.getTodayEvent()
            );

            break;


        case StartupRoute::HOME:

        default:

            HomeScreen::create(
                game
            );

            break;
    }
}