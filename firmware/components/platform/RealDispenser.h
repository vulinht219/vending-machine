#pragma once

#include "dispenser/IDispenser.h"
#include <cstdint>

// The result of the physical cycle, NOT the return value of startDispense().
enum class DispenseStatus : uint8_t
{
    Idle = 0,
    Running,
    CandyDetected,  // sensor observed a beam interruption
    TimedOut,       // stopped without verified candy detection
    SensorBlocked,  // beam was broken before motor activation
    Stopped,        // stop was requested
    Fault           // motor, sensor or I2C failure
};

class RealDispenser : public IDispenser
{
public:
    // Existing AppController entrypoint; initializes PCA9685 FULL OFF.
    bool initializeServo0Neutral();

    // Existing Home TEST SERVO button opens calibration.
    bool testServo0Once();

    // Stage-1 motor backend: accepted != physically dispensed.
    // Do not use until GameManager/UI is integrated with completion.
    bool startDispense(int slot); // 1..6, maps to CH0..CH5
    DispenseStatus getDispenseStatus() const;
    int getLastDispenseSlot() const;
    bool isBusy() const;
    bool requestStop(); // asynchronous request, not physical power cutoff

    // Existing game API remains locked until its bool semantics are reviewed.
    bool dispense(int slot) override;
};