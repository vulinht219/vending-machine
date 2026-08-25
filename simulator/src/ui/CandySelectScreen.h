#pragma once

#include <lvgl.h>
#include "game/GameManager.h"

class CandySelectScreen {
public:
    static void create(GameManager& game);

private:
    static GameManager* currentGame;

    static void candyButtonEvent(lv_event_t* event);
};