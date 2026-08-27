#pragma once

#include <lvgl.h>

#include "game/GameManager.h"


class DispensingScreen {
public:

    static void create(
        GameManager& game
    );


private:

    static GameManager*
        currentGame;

    static lv_timer_t*
        dispenseTimer;


    static void dispenseFinished(
        lv_timer_t* timer
    );
};