#include "SpecialEventManager.h"


SpecialEventManager::SpecialEventManager(
    IClock& clock,
    ISpecialEventProgressStore& progressStore,
    IDispenser& dispenser
)
    : clock(clock),
      progressStore(progressStore),
      dispenser(dispenser)
{
    loadProgress();
}


void SpecialEventManager::loadProgress()
{
    if (
        progressStore.exists()
    ) {
        progress =
            progressStore.load();

        return;
    }


    progress =
        SpecialEventProgress{};


    progressStore.save(
        progress
    );
}


bool SpecialEventManager::hasClaimedToday() const
{
    Date today =
        clock.today();


    return
        progress.date == today &&
        progress.state ==
            SpecialRewardState::CLAIMED;
}


bool SpecialEventManager::shouldShowEvent()
{
    Date today =
        clock.today();


    if (
        !SpecialEventCalendar::
            isSpecialDate(today)
    ) {
        return false;
    }


    if (
        hasClaimedToday()
    ) {
        return false;
    }


    return true;
}


SpecialEventType
SpecialEventManager::getTodayEvent()
{
    return
        SpecialEventCalendar::getEvent(
            clock.today()
        );
}


bool SpecialEventManager::selectCandy(
    int slot
)
{
    if (
        slot < 1 ||
        slot > 6
    ) {
        return false;
    }


    Date today =
        clock.today();


    if (
        !SpecialEventCalendar::
            isSpecialDate(today)
    ) {
        return false;
    }


    if (
        hasClaimedToday()
    ) {
        return false;
    }


    // =============================================
    // SAVE PENDING FIRST
    // =============================================

    progress.date =
        today;

    progress.state =
        SpecialRewardState::PENDING;

    progress.selectedSlot =
        slot;


    progressStore.save(
        progress
    );


    // =============================================
    // DISPENSE
    // =============================================

    bool success =
        dispenser.dispense(
            slot
        );


    if (!success) {

        // Keep PENDING so reward is not silently lost.
        return false;
    }


    // =============================================
    // CLAIMED
    // =============================================

    progress.state =
        SpecialRewardState::CLAIMED;


    progressStore.save(
        progress
    );


    return true;
}