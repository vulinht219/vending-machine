#pragma once

struct Date {
    int year;
    int month;
    int day;

    bool operator==(const Date& other) const
    {
        return
            year == other.year &&
            month == other.month &&
            day == other.day;
    }

    bool operator!=(const Date& other) const
    {
        return !(*this == other);
    }
};