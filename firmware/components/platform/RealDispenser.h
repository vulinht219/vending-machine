#pragma once

#include "dispenser/IDispenser.h"

class RealDispenser : public IDispenser
{
public:
    // Compatibility name retained for AppController.
    // Starts PCA9685 with every channel FULL OFF.
    bool initializeServo0Neutral();

    // Starts one non-blocking CH0 test cycle:
    // run -> calibrated neutral -> FULL OFF.
    // Returns true only when the cycle was accepted.
    bool testServo0Once();

    // Locked until the loaded spiral mechanism is verified.
    bool dispense(int slot) override;
};
