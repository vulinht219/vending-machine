#pragma once

#include <lvgl.h>

#include "event/SpecialEventManager.h"
#include "event/SpecialEventType.h"
#include "game/GameManager.h"


class SpecialEventScreen
{
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

    static SpecialEventType
        currentEventType;


    static void showGreeting();

    static void showCandySelection();


    static void greetingButtonEvent(
        lv_event_t* event
    );

    static void candyButtonEvent(
        lv_event_t* event
    );


    static const char* getGreetingText(
        SpecialEventType eventType
    );
};