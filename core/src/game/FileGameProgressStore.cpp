#include "FileGameProgressStore.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

using json = nlohmann::json;


FileGameProgressStore::FileGameProgressStore(
    const std::string& filePath
)
    : filePath(filePath)
{
}


bool FileGameProgressStore::exists() const
{
    return std::filesystem::exists(
        filePath
    );
}


GameProgress FileGameProgressStore::load() const
{
    std::ifstream file(
        filePath
    );

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


void FileGameProgressStore::save(
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


    std::ofstream file(
        filePath
    );

    if (!file.is_open()) {
        throw std::runtime_error(
            "Could not save game progress file: "
            + filePath
        );
    }


    file << data.dump(4);
}