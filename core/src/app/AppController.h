#pragma once

#include "game/GameManager.h"
#include "event/SpecialEventManager.h"


class AppController {
public:

    AppController(
        GameManager& game,
        SpecialEventManager& specialEventManager
    );


    void start();


private:

    GameManager& game;

    SpecialEventManager&
        specialEventManager;
};