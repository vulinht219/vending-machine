#pragma once

#include "time/IClock.h"


class MockClock
    : public IClock
{
public:

    MockClock(
        int year,
        int month,
        int day
    )
        : currentDate{
            year,
            month,
            day
        }
    {
    }


    Date today() const override
    {
        return currentDate;
    }


    void setDate(
        int year,
        int month,
        int day
    )
    {
        currentDate = {
            year,
            month,
            day
        };
    }


private:

    Date currentDate;
};