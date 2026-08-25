#include "QuizProgressStore.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

using json = nlohmann::json;


QuizProgressStore::QuizProgressStore(
    const std::string& filePath
)
    : filePath(filePath)
{
}


// =====================================================
// EXISTS
// =====================================================

bool QuizProgressStore::exists() const
{
    return std::filesystem::exists(
        filePath
    );
}


// =====================================================
// LOAD
// =====================================================

QuizProgress QuizProgressStore::load() const
{
    std::ifstream file(
        filePath
    );

    if (!file.is_open()) {
        throw std::runtime_error(
            "Could not open quiz progress file: "
            + filePath
        );
    }

    json data;

    file >> data;

    QuizProgress progress;

    progress.seed =
        data.at("seed")
            .get<std::uint32_t>();

    progress.currentPosition =
        data.at("currentPosition")
            .get<std::size_t>();

    progress.round =
        data.at("round")
            .get<std::uint32_t>();

    progress.datasetVersion =
        data.value(
            "datasetVersion",
            1
        );

    return progress;
}


// =====================================================
// SAVE
// =====================================================

void QuizProgressStore::save(
    const QuizProgress& progress
) const
{
    std::filesystem::path path(
        filePath
    );

    // Create parent directory if needed.
    if (
        path.has_parent_path()
    ) {
        std::filesystem::create_directories(
            path.parent_path()
        );
    }


    json data = {
        {
            "seed",
            progress.seed
        },

        {
            "currentPosition",
            progress.currentPosition
        },

        {
            "round",
            progress.round
        }, 
        {
            "datasetVersion",
            progress.datasetVersion
        }
    };


    std::ofstream file(
        filePath
    );

    if (!file.is_open()) {

        throw std::runtime_error(
            "Could not save quiz progress file: "
            + filePath
        );
    }


    file << data.dump(4);
}