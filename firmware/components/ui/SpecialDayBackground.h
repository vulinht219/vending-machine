#pragma once

#include <lvgl.h>

#include "event/SpecialEventType.h"


class SpecialDayBackground
{
public:
    static void create(
        lv_obj_t* parent,
        SpecialEventType eventType
    );

    static void stop();
};
