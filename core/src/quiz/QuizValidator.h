#pragma once

#include "Quiz.h"

#include <string>
#include <vector>

class QuizValidator {
public:
    static void validateDataset(
        const std::vector<Quiz>& quizzes
    );

private:
    static void validateQuiz(
        const Quiz& quiz
    );

    static bool isAllowedCategory(
        const std::string& category
    );
};