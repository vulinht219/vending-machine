#include "GameProgressStore.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

using json = nlohmann::json;


GameProgressStore::GameProgressStore(
    const std::string& filePath
)
    : filePath(filePath)
{
}


bool GameProgressStore::exists() const
{
    return std::filesystem::exists(
        filePath
    );
}


GameProgress GameProgressStore::load() const
{
    std::ifstream file(filePath);

    if (!file.is_open()) {
        throw std::runtime_error(
            "Could not open game progress file: "
            + filePath
        );
    }

    json data;

    file >> data;

    GameProgress progress;

    progress.pendingReward =
        data.value(
            "pendingReward",
            false
        );

    return progress;
}


void GameProgressStore::save(
    const GameProgress& progress
) const
{
    std::filesystem::path path(
        filePath
    );

    if (path.has_parent_path()) {
        std::filesystem::create_directories(
            path.parent_path()
        );
    }

    json data = {
        {
            "pendingReward",
            progress.pendingReward
        }
    };

    std::ofstream file(filePath);

    if (!file.is_open()) {
        throw std::runtime_error(
            "Could not save game progress file: "
            + filePath
        );
    }

    file << data.dump(4);
}