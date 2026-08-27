#pragma once

#include "Date.h"

class IClock {
public:
    virtual ~IClock() = default;

    virtual Date today() const = 0;
};