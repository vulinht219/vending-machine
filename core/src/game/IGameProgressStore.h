#pragma once

#include "GameProgress.h"

class IGameProgressStore {
public:
    virtual ~IGameProgressStore() = default;

    virtual bool exists() const = 0;

    virtual GameProgress load() const = 0;

    virtual void save(
        const GameProgress& progress
    ) const = 0;
};