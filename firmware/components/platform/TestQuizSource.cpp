#include "TestQuizSource.h"

#include <stdexcept>


TestQuizSource::TestQuizSource()
{
    quizzes = {
        {
            1,
            "math",
            QuizType::NUMERIC,
            "What is 12 * 8?",
            {},
            "96",
            1
        },

        {
            2,
            "physics",
            QuizType::MULTIPLE_CHOICE,
            "Why do car tires have grooves?",
            {
                "To make the car lighter",
                "To improve grip on wet roads",
                "To cool the engine",
                "To increase tire pressure"
            },
            "1",
            1
        }
    };
}


std::size_t TestQuizSource::size() const
{
    return quizzes.size();
}


Quiz TestQuizSource::getQuiz(
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