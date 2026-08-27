#pragma once

#include "SpecialEventCalendar.h"
#include "SpecialEventProgress.h"
#include "ISpecialEventProgressStore.h"

#include "time/IClock.h"

#include "dispenser/IDispenser.h"


class SpecialEventManager {
public:

    SpecialEventManager(
        IClock& clock,
        ISpecialEventProgressStore& progressStore,
        IDispenser& dispenser
    );


    bool shouldShowEvent();


    SpecialEventType getTodayEvent();


    bool selectCandy(
        int slot
    );


private:

    IClock& clock;

    ISpecialEventProgressStore&
        progressStore;

    IDispenser& dispenser;


    SpecialEventProgress
        progress;


    void loadProgress();

    bool hasClaimedToday() const;
};