#pragma once

#include <lvgl.h>

#include "game/GameManager.h"


class DispensingScreen
{
public:

    static void create(
        GameManager& game
    );


    static GameManager* currentGame;
};