#include "StartupRouter.h"


StartupRouter::StartupRouter(
    GameManager& game,
    SpecialEventManager& specialEventManager
)
    : game(game),
      specialEventManager(specialEventManager)
{
}


StartupRoute StartupRouter::resolve() const
{
    if (game.hasPendingReward())
    {
        return StartupRoute::PENDING_QUIZ_REWARD;
    }


    if (specialEventManager.shouldShowEvent())
    {
        return StartupRoute::SPECIAL_EVENT;
    }


    return StartupRoute::HOME;
}