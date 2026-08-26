#include "QuizManager.h"

#include <algorithm>
#include <numeric>
#include <random>
#include <stdexcept>


QuizManager::QuizManager(
    IQuizSource& quizSource,
    IQuizProgressStore& progressStore
)
    : quizSource(quizSource),
      progressStore(progressStore)
{
    quizCount =
        quizSource.size();


    if (quizCount == 0) {
        throw std::runtime_error(
            "Quiz dataset is empty."
        );
    }


    initializeProgress();

    buildShuffleOrder();
}


std::uint32_t QuizManager::generateSeed()
{
    std::random_device rd;

    return rd();
}


void QuizManager::initializeProgress()
{
    if (
        progressStore.exists()
    ) {
        progress =
            progressStore.load();


        if (
            progress.datasetVersion
            != CURRENT_DATASET_VERSION
        ) {
            progress.seed =
                generateSeed();

            progress.currentPosition =
                0;

            progress.round =
                1;

            progress.datasetVersion =
                CURRENT_DATASET_VERSION;

            progressStore.save(
                progress
            );

            return;
        }


        if (
            progress.currentPosition
            >= quizCount
        ) {
            progress.seed =
                generateSeed();

            progress.currentPosition =
                0;

            progress.round =
                1;

            progress.datasetVersion =
                CURRENT_DATASET_VERSION;

            progressStore.save(
                progress
            );
        }

        return;
    }


    progress.seed =
        generateSeed();

    progress.currentPosition =
        0;

    progress.round =
        1;

    progress.datasetVersion =
        CURRENT_DATASET_VERSION;


    progressStore.save(
        progress
    );
}


void QuizManager::buildShuffleOrder()
{
    shuffledOrder.resize(
        quizCount
    );


    std::iota(
        shuffledOrder.begin(),
        shuffledOrder.end(),
        0
    );


    std::mt19937 generator(
        progress.seed
    );


    std::shuffle(
        shuffledOrder.begin(),
        shuffledOrder.end(),
        generator
    );
}


Quiz QuizManager::getCurrentQuiz()
{
    if (
        progress.currentPosition
        >= shuffledOrder.size()
    ) {
        throw std::runtime_error(
            "Quiz progress position is invalid."
        );
    }


    std::size_t quizIndex =
        shuffledOrder[
            progress.currentPosition
        ];


    return quizSource.getQuiz(
        quizIndex
    );
}


void QuizManager::moveToNextQuiz()
{
    progress.currentPosition++;


    if (
        progress.currentPosition
        >= quizCount
    ) {
        startNewRound();

        return;
    }


    progressStore.save(
        progress
    );
}


void QuizManager::startNewRound()
{
    progress.round++;

    progress.seed =
        generateSeed();

    progress.currentPosition =
        0;

    progress.datasetVersion =
        CURRENT_DATASET_VERSION;


    buildShuffleOrder();


    progressStore.save(
        progress
    );
}