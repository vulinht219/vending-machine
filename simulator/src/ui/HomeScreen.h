#pragma once

#include <lvgl.h>
#include "game/GameManager.h"

class HomeScreen {
public:
    static void create(GameManager& game);

private:
    static void startButtonEvent(lv_event_t* event);
};