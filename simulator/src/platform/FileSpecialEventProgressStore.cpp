#include "FileSpecialEventProgressStore.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>


FileSpecialEventProgressStore::
FileSpecialEventProgressStore(
    const std::string& filePath
)
    : filePath(filePath)
{
}


bool FileSpecialEventProgressStore::
exists() const
{
    return
        std::filesystem::exists(
            filePath
        );
}


SpecialEventProgress
FileSpecialEventProgressStore::
load() const
{
    std::ifstream file(
        filePath
    );


    if (!file.is_open()) {

        throw std::runtime_error(
            "Could not open special event progress file."
        );
    }


    SpecialEventProgress progress;

    int stateValue = 0;


    file
        >> progress.date.year
        >> progress.date.month
        >> progress.date.day
        >> stateValue
        >> progress.selectedSlot;


    if (file.fail()) {

        throw std::runtime_error(
            "Invalid special event progress file."
        );
    }


    progress.state =
        static_cast<
            SpecialRewardState
        >(
            stateValue
        );


    return progress;
}


void FileSpecialEventProgressStore::
save(
    const SpecialEventProgress& progress
) const
{
    std::filesystem::path path(
        filePath
    );


    if (
        path.has_parent_path()
    ) {

        std::filesystem::
            create_directories(
                path.parent_path()
            );
    }


    std::ofstream file(
        filePath,
        std::ios::trunc
    );


    if (!file.is_open()) {

        throw std::runtime_error(
            "Could not write special event progress file."
        );
    }


    file
        << progress.date.year
        << " "
        << progress.date.month
        << " "
        << progress.date.day
        << " "
        << static_cast<int>(
            progress.state
        )
        << " "
        << progress.selectedSlot;
}