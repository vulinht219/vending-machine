#include "SDCardQuizSource.h"

#include <stdexcept>


SDCardQuizSource::SDCardQuizSource(
    const std::string& filePath
)
    : filePath(filePath)
{
}


std::vector<Quiz> SDCardQuizSource::loadAll()
{
    throw std::runtime_error(
        "SDCardQuizSource is not implemented yet."
    );
}