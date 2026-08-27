#include "SpecialEventCalendar.h"


SpecialEventType SpecialEventCalendar::getEvent(
    const Date& date
)
{
    // =================================================
    // NEW YEAR
    // =================================================

    if (
        date.month == 1 &&
        date.day == 1
    ) {
        return SpecialEventType::NEW_YEAR;
    }


    // =================================================
    // VALENTINE
    // =================================================

    if (
        date.month == 2 &&
        date.day == 14
    ) {
        return SpecialEventType::VALENTINE;
    }


    // =================================================
    // SEPTEMBER 21
    // =================================================

    if (
        date.month == 9 &&
        date.day == 21
    ) {
        return SpecialEventType::SEPTEMBER_21;
    }


    // =================================================
    // OCTOBER 2
    // =================================================

    if (
        date.month == 10 &&
        date.day == 2
    ) {
        return SpecialEventType::OCTOBER_02;
    }


    // =================================================
    // CHRISTMAS
    // =================================================

    if (
        date.month == 12 &&
        (
            date.day == 24 ||
            date.day == 25 ||
            date.day == 26
        )
    ) {
        return SpecialEventType::CHRISTMAS;
    }


    // =================================================
    // EVERY 25TH DAY OF THE MONTH
    // =================================================

    if (
        date.day == 25
    ) {
        return SpecialEventType::MONTHLY_25;
    }


    return SpecialEventType::NONE;
}


bool SpecialEventCalendar::isSpecialDate(
    const Date& date
)
{
    return
        getEvent(date)
        != SpecialEventType::NONE;
}