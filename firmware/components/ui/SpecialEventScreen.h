#pragma once

#include <lvgl.h>

#include "event/SpecialEventManager.h"
#include "event/SpecialEventType.h"
#include "game/GameManager.h"


class SpecialEventScreen {
public:

    static void create(
        SpecialEventManager& specialEventManager,
        GameManager& game,
        SpecialEventType eventType
    );


private:

    static SpecialEventManager*
        currentSpecialEventManager;

    static GameManager*
        currentGame;


    static void candyButtonEvent(
        lv_event_t* event
    );
};