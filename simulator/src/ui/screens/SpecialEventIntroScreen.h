#pragma once

#include <lvgl.h>

#include "game/GameManager.h"
#include "event/SpecialEventManager.h"
#include "event/SpecialEventType.h"


class SpecialEventIntroScreen
{
public:

    static void create(
        SpecialEventManager& specialEventManager,
        GameManager& game,
        SpecialEventType eventType
    );


private:

    static SpecialEventManager* currentSpecialEventManager;

    static GameManager* currentGame;

    static SpecialEventType currentEventType;


    static void continueButtonEvent(
        lv_event_t* event
    );


    static const char* getTitle(
        SpecialEventType eventType
    );


    static const char* getButtonText(
        SpecialEventType eventType
    );
};