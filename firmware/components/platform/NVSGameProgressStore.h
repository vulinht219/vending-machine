#pragma once

#include "game/IGameProgressStore.h"


class NVSGameProgressStore
    : public IGameProgressStore
{
public:
    bool exists() const override;

    GameProgress load() const override;

    void save(
        const GameProgress& progress
    ) const override;
};