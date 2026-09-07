#pragma once

#include <lvgl.h>

#include "game/GameManager.h"


class CandySelectScreen
{
public:

    static void create(
        GameManager& game
    );


    static GameManager* currentGame;


private:

    static void createCandyButton(
        lv_obj_t* parent,
        int candyNumber,
        int x,
        int y
    );
};