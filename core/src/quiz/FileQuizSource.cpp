#include "FileQuizSource.h"

#include "QuizLoader.h"
#include "QuizValidator.h"

#include <stdexcept>


FileQuizSource::FileQuizSource(
    const std::string& filePath
)
{
    quizzes =
        QuizLoader::loadFromJsonLines(
            filePath
        );


    QuizValidator::validateDataset(
        quizzes
    );
}


std::size_t FileQuizSource::size() const
{
    return quizzes.size();
}


Quiz FileQuizSource::getQuiz(
    std::size_t index
)
{
    if (
        index >= quizzes.size()
    ) {
        throw std::runtime_error(
            "Quiz index out of range."
        );
    }


    return quizzes[index];
}