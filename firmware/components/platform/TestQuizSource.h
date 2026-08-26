#pragma once

#include "quiz/IQuizSource.h"

#include <vector>


class TestQuizSource
    : public IQuizSource
{
public:

    TestQuizSource();

    std::size_t size() const override;

    Quiz getQuiz(
        std::size_t index
    ) override;


private:

    std::vector<Quiz> quizzes;
};