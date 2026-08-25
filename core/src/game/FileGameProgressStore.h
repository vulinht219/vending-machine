#pragma once

#include "IGameProgressStore.h"

#include <string>

class FileGameProgressStore
    : public IGameProgressStore
{
public:
    explicit FileGameProgressStore(
        const std::string& filePath
    );

    bool exists() const override;

    GameProgress load() const override;

    void save(
        const GameProgress& progress
    ) const override;

private:
    std::string filePath;
};