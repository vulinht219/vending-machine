#pragma once

#include "event/ISpecialEventProgressStore.h"


class MockSpecialEventProgressStore
    : public ISpecialEventProgressStore
{
public:

    bool exists() const override
    {
        return hasData;
    }


    SpecialEventProgress load() const override
    {
        return progress;
    }


    void save(
        const SpecialEventProgress& newProgress
    ) const override
    {
        progress =
            newProgress;

        hasData =
            true;
    }


private:

    mutable bool hasData =
        false;

    mutable SpecialEventProgress progress;
};