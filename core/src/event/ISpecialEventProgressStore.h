#pragma once

#include "SpecialEventProgress.h"

class ISpecialEventProgressStore {
public:
    virtual ~ISpecialEventProgressStore() = default;

    virtual bool exists() const = 0;

    virtual SpecialEventProgress load() const = 0;

    virtual void save(
        const SpecialEventProgress& progress
    ) const = 0;
};