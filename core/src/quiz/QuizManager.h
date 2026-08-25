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


    Quiz getCurrentQuiz() const;


    void moveToNextQuiz();


private:

    // Increase this whenever the production dataset
    // changes in a way that should reset quiz progress.
    static constexpr std::uint32_t
        CURRENT_DATASET_VERSION = 2;


    std::vector<Quiz> quizzes;

    std::vector<std::size_t> shuffledOrder;


    QuizProgress progress;


    IQuizSource& quizSource;

    IQuizProgressStore& progressStore;


    void initializeProgress();

    void buildShuffleOrder();

    void startNewRound();

    std::uint32_t generateSeed();
};