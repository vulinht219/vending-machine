#include "SpecialEventCalendar.h"


SpecialEventType SpecialEventCalendar::getEvent(
    const Date& date
)
{
    // =================================================
    // CHRISTMAS
    // =================================================
    //
    // Must be checked BEFORE monthiversary because
    // December 25 is both Christmas and the monthly 25th.
    //

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
    // NEW YEAR'S EVE
    // =================================================

    if (
        date.month == 12 &&
        date.day == 31
    ) {
        return SpecialEventType::NEW_YEARS_EVE;
    }


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
    // BIRTHDAY - SEPTEMBER
    // =================================================

    if (
        date.month == 9 &&
        date.day == 21
    ) {
        return SpecialEventType::BIRTHDAY_SEPTEMBER;
    }


    // =================================================
    // BIRTHDAY - OCTOBER
    // =================================================

    if (
        date.month == 10 &&
        date.day == 2
    ) {
        return SpecialEventType::BIRTHDAY_OCTOBER;
    }


    // =================================================
    // HALLOWEEN
    // =================================================

    if (
        date.month == 10 &&
        date.day == 31
    ) {
        return SpecialEventType::HALLOWEEN;
    }


    // =================================================
    // MONTHIVERSARY
    // =================================================
    //
    // Check this LAST because 25 December must resolve
    // to CHRISTMAS instead.
    //

    if (
        date.day == 25
    ) {
        return SpecialEventType::MONTHIVERSARY;
    }


    // =================================================
    // NORMAL DAY
    // =================================================

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