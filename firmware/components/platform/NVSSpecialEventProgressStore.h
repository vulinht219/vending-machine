#pragma once

#include "event/ISpecialEventProgressStore.h"


class NVSSpecialEventProgressStore
    : public ISpecialEventProgressStore
{
public:

    bool exists() const override;


    SpecialEventProgress
    load() const override;


    void save(
        const SpecialEventProgress& progress
    ) const override;
};