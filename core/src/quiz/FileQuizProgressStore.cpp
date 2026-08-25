#include "FileQuizProgressStore.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

using json = nlohmann::json;


FileQuizProgressStore::FileQuizProgressStore(
    const std::string& filePath
)
    : filePath(filePath)
{
}


bool FileQuizProgressStore::exists() const
{
    return std::filesystem::exists(
        filePath
    );
}


QuizProgress FileQuizProgressStore::load() const
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


void FileQuizProgressStore::save(
    const QuizProgress& progress
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