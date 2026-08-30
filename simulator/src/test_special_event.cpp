#include <iostream>
#include <stdexcept>

#include "platform/MockClock.h"
#include "platform/MockSpecialEventProgressStore.h"

#include "event/SpecialEventCalendar.h"
#include "event/SpecialEventManager.h"

#include "dispenser/MockDispenser.h"


static void require(
    bool condition,
    const std::string& message
)
{
    if (!condition) {
        throw std::runtime_error(
            message
        );
    }
}


int main()
{
    try {

        // =================================================
        // BASIC CALENDAR TESTS
        // =================================================

        require(
            SpecialEventCalendar::isSpecialDate(
                {2026, 1, 1}
            ),
            "01/01 should be special."
        );


        require(
            SpecialEventCalendar::getEvent(
                {2026, 1, 1}
            )
            == SpecialEventType::NEW_YEAR,
            "01/01 should be NEW_YEAR."
        );


        require(
            SpecialEventCalendar::getEvent(
                {2026, 2, 14}
            )
            == SpecialEventType::VALENTINE,
            "14/02 should be VALENTINE."
        );


        require(
            SpecialEventCalendar::getEvent(
                {2026, 9, 21}
            )
            == SpecialEventType::BIRTHDAY_SEPTEMBER,
            "21/09 should be BIRTHDAY_SEPTEMBER."
        );


        require(
            SpecialEventCalendar::getEvent(
                {2026, 10, 2}
            )
            == SpecialEventType::BIRTHDAY_OCTOBER,
            "02/10 should be BIRTHDAY_OCTOBER."
        );


        require(
            SpecialEventCalendar::getEvent(
                {2026, 10, 31}
            )
            == SpecialEventType::HALLOWEEN,
            "31/10 should be HALLOWEEN."
        );


        require(
            SpecialEventCalendar::getEvent(
                {2026, 12, 24}
            )
            == SpecialEventType::CHRISTMAS,
            "24/12 should be CHRISTMAS."
        );


        require(
            SpecialEventCalendar::getEvent(
                {2026, 12, 25}
            )
            == SpecialEventType::CHRISTMAS,
            "25/12 should prefer CHRISTMAS over MONTHIVERSARY."
        );


        require(
            SpecialEventCalendar::getEvent(
                {2026, 12, 26}
            )
            == SpecialEventType::CHRISTMAS,
            "26/12 should be CHRISTMAS."
        );


        require(
            SpecialEventCalendar::getEvent(
                {2026, 12, 31}
            )
            == SpecialEventType::NEW_YEARS_EVE,
            "31/12 should be NEW_YEARS_EVE."
        );


        require(
            SpecialEventCalendar::getEvent(
                {2026, 8, 25}
            )
            == SpecialEventType::MONTHIVERSARY,
            "25/08 should be MONTHIVERSARY."
        );


        require(
            !SpecialEventCalendar::isSpecialDate(
                {2026, 8, 24}
            ),
            "24/08 should not be special."
        );


        std::cout
            << "Calendar rules PASSED.\n";


        // =================================================
        // REWARD FLOW TEST
        // =================================================

        MockClock clock(
            2026,
            12,
            25
        );


        MockSpecialEventProgressStore
            progressStore;


        MockDispenser dispenser;


        SpecialEventManager manager(
            clock,
            progressStore,
            dispenser
        );


        // First boot on 25/12:
        // should show special event.
        require(
            manager.shouldShowEvent(),
            "Christmas event should appear before claim."
        );


        require(
            manager.getTodayEvent()
            == SpecialEventType::CHRISTMAS,
            "Today should be CHRISTMAS."
        );


        std::cout
            << "25/12 event available PASSED.\n";


        // User chooses candy #3.
        bool dispenseSuccess =
            manager.selectCandy(3);


        require(
            dispenseSuccess,
            "Free candy dispense should succeed."
        );


        // Same day: event must disappear.
        require(
            !manager.shouldShowEvent(),
            "Event should disappear after reward is claimed."
        );


        std::cout
            << "Same-day claim protection PASSED.\n";


        // =================================================
        // SAME DAY REBOOT SIMULATION
        // =================================================
        //
        // Create another manager using the SAME store.
        // This simulates rebooting while persistent
        // storage still contains today's claim.
        // =================================================

        SpecialEventManager
            rebootedManager(
                clock,
                progressStore,
                dispenser
            );


        require(
            !rebootedManager.shouldShowEvent(),
            "Reboot on same day must not give another candy."
        );


        std::cout
            << "Same-day reboot protection PASSED.\n";


        // =================================================
        // NEXT CHRISTMAS DAY
        // =================================================

        clock.setDate(
            2026,
            12,
            26
        );


        SpecialEventManager
            nextDayManager(
                clock,
                progressStore,
                dispenser
            );


        require(
            nextDayManager.shouldShowEvent(),
            "26/12 should give a new free candy."
        );


        require(
            nextDayManager.selectCandy(5),
            "26/12 candy dispense should succeed."
        );


        require(
            !nextDayManager.shouldShowEvent(),
            "26/12 should be claimed after dispense."
        );


        std::cout
            << "Next-day Christmas reward PASSED.\n";


        // =================================================
        // NORMAL DAY
        // =================================================

        clock.setDate(
            2026,
            12,
            27
        );


        SpecialEventManager
            normalDayManager(
                clock,
                progressStore,
                dispenser
            );


        require(
            !normalDayManager.shouldShowEvent(),
            "27/12 should go directly to Home."
        );


        std::cout
            << "Normal day routing PASSED.\n";


        // =================================================
        // NEXT MONTH'S 25TH
        // =================================================

        clock.setDate(
            2027,
            1,
            25
        );


        SpecialEventManager
            monthlyEventManager(
                clock,
                progressStore,
                dispenser
            );


        require(
            monthlyEventManager.shouldShowEvent(),
            "25/01 should be MONTHIVERSARY."
        );


        require(
            monthlyEventManager.getTodayEvent()
            == SpecialEventType::MONTHIVERSARY,
            "25/01 should use MONTHIVERSARY event."
        );


        std::cout
            << "Monthly 25 event PASSED.\n";


        std::cout
            << "\n====================================\n";

        std::cout
            << "ALL SPECIAL EVENT TESTS PASSED\n";

        std::cout
            << "====================================\n";


        return 0;
    }

    catch (
        const std::exception& exception
    ) {

        std::cerr
            << "\nSPECIAL EVENT TEST FAILED:\n"
            << exception.what()
            << "\n";


        return 1;
    }
}