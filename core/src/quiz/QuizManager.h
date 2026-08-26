#pragma once

#include "Quiz.h"
#include "QuizProgress.h"
#include "IQuizSource.h"
#include "IQuizProgressStore.h"

#include <cstddef>
#include <cstdint>
#include <vector>


class QuizManager {
public:

    QuizManager(
        IQuizSource& quizSource,
        IQuizProgressStore& progressStore
    );


    Quiz getCurrentQuiz();


    void moveToNextQuiz();


private:

    static constexpr std::uint32_t
        CURRENT_DATASET_VERSION = 3;


    std::size_t quizCount = 0;

    std::vector<std::size_t> shuffledOrder;

    QuizProgress progress;

    IQuizSource& quizSource;

    IQuizProgressStore& progressStore;


    void initializeProgress();

    void buildShuffleOrder();

    void startNewRound();

    std::uint32_t generateSeed();
};