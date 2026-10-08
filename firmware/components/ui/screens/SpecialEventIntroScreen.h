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
    static lv_timer_t* autoAdvanceTimer;

    static void continueButtonEvent(lv_event_t* event);
    static void autoAdvanceEvent(lv_timer_t* timer);
    static void transitionToCandySelection();
    static void stopAutoAdvanceTimer();
};