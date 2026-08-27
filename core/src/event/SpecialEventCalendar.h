#pragma once

#include "time/Date.h"
#include "SpecialEventType.h"

class SpecialEventCalendar {
public:

    static SpecialEventType getEvent(
        const Date& date
    );

    static bool isSpecialDate(
        const Date& date
    );
};