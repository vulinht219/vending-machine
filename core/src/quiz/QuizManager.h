#pragma once

#include "Quiz.h"
#include <vector>

class QuizManager {
public:
    QuizManager();

    Quiz getCurrentQuiz();
    void moveToNextQuiz();

private:
    std::vector<Quiz> quizzes;
    int currentIndex;
};