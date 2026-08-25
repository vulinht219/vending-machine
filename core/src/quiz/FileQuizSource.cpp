#include "FileQuizSource.h"

#include "QuizLoader.h"


FileQuizSource::FileQuizSource(
    const std::string& filePath
)
    : filePath(filePath)
{
}


std::vector<Quiz> FileQuizSource::loadAll()
{
    return QuizLoader::loadFromJsonLines(
        filePath
    );
}