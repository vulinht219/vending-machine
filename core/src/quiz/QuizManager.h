#pragma once

#include "Quiz.h"
#include "QuizProgress.h"
#include "QuizProgressStore.h"

#include <cstddef>
#include <cstdint>
#include <vector>


class QuizManager {
public:
    QuizManager();

    Quiz getCurrentQuiz() const;

    void moveToNextQuiz();

private:
    static constexpr std::uint32_t CURRENT_DATASET_VERSION = 1;

    std::vector<Quiz> quizzes;

    std::vector<std::size_t> shuffledOrder;

    QuizProgress progress;

    QuizProgressStore progressStore;


    void initializeProgress();

    void buildShuffleOrder();

    void startNewRound();

    std::uint32_t generateSeed();
    
};