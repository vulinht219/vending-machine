#pragma once

#include "dispenser/IDispenser.h"


class RealDispenser
    : public IDispenser
{
public:
    bool dispense(
        int slot
    ) override;
};