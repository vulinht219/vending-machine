#pragma once

#include <lvgl.h>

#include "game/GameManager.h"


class IncorrectScreen
{
public:

    static void create(
        GameManager& game
    );


private:

    static GameManager* currentGame;

    static lv_timer_t* animationTimer;

    static lv_timer_t* finishTimer;

    static int currentFrame;


    static void animationTick(
        lv_timer_t* timer
    );


    static void finished(
        lv_timer_t* timer
    );
};