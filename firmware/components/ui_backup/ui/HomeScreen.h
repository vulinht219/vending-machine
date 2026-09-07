#pragma once

#include "lvgl.h"


class HomeScreen
{
public:

    static void create();


private:

    static void startButtonEvent(
        lv_event_t* event
    );
};