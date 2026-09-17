#pragma once

#include "dispenser/IDispenser.h"

class RealDispenser
    : public IDispenser
{
public:
    bool initializeServo0Neutral();

    // Temporary hardware test.
    // Runs CH0 once, then returns to neutral.
    bool testServo0Once();

    bool dispense(int slot) override;
};