#pragma once

#include "time/Date.h"

enum class SpecialRewardState {
    NONE,
    PENDING,
    CLAIMED
};


struct SpecialEventProgress {

    Date date {
        0,
        0,
        0
    };

    SpecialRewardState state =
        SpecialRewardState::NONE;

    int selectedSlot = 0;
};