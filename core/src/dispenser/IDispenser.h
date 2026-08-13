#pragma once

class IDispenser {
public:
    virtual bool dispense(int slot) = 0;

    virtual ~IDispenser() = default;
};