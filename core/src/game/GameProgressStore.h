#pragma once

#include "GameProgress.h"

#include <string>

class GameProgressStore {
public:
    explicit GameProgressStore(
        const std::string& filePath
    );

    bool exists() const;

    GameProgress load() const;

    void save(
        const GameProgress& progress
    ) const;

private:
    std::string filePath;
};