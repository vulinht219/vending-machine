#include "QuizManager.h"

QuizManager::QuizManager() {
    currentIndex = 0;

    quizzes.push_back({
        1,
        "What is 12 + 7?",
        "19",
        1
    });

    quizzes.push_back({
        2,
        "What is 8 * 4?",
        "32",
        1
    });

    quizzes.push_back({
        3,
        "What is 100 / 5?",
        "20",
        1
    });
}

Quiz QuizManager::getCurrentQuiz() {
    return quizzes[currentIndex];
}

void QuizManager::moveToNextQuiz() {
    currentIndex++;

    if (currentIndex >= quizzes.size()) {
        currentIndex = 0;
    }
}