#pragma once

#include "StartupRoute.h"
#include "game/GameManager.h"
#include "event/SpecialEventManager.h"


class StartupRouter
{
public:

    StartupRouter(
        GameManager& game,
        SpecialEventManager& specialEventManager
    );


    StartupRoute resolve() const;


private:

    GameManager& game;
    SpecialEventManager& specialEventManager;
};