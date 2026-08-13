#pragma once

#include "dispenser/IDispenser.h"

class MockDispenser : public IDispenser {
public:
    bool dispense(int slot) override;
};