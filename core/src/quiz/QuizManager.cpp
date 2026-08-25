#include "QuizManager.h"

#include "QuizValidator.h"

#include <algorithm>
#include <numeric>
#include <random>
#include <stdexcept>


// =====================================================
// CONSTRUCTOR
// =====================================================

QuizManager::QuizManager(
    IQuizSource& quizSource,
    IQuizProgressStore& progressStore
)
    : quizSource(quizSource),
      progressStore(progressStore)
{
    // =============================================
    // LOAD DATASET
    // =============================================

    quizzes =
        quizSource.loadAll();


    // =============================================
    // VALIDATE DATASET
    // =============================================

    QuizValidator::validateDataset(
        quizzes
    );


    // =============================================
    // LOAD / CREATE PROGRESS
    // =============================================

    initializeProgress();


    // =============================================
    // RECREATE RANDOM ORDER
    // =============================================

    buildShuffleOrder();
}


// =====================================================
// GENERATE SEED
// =====================================================

std::uint32_t QuizManager::generateSeed()
{
    std::random_device rd;

    return rd();
}


// =====================================================
// INITIALIZE PROGRESS
// =====================================================

void QuizManager::initializeProgress()
{
    // =============================================
    // EXISTING PROGRESS
    // =============================================

    if (
        progressStore.exists()
    ) {
        progress =
            progressStore.load();


        // -----------------------------------------
        // DATASET VERSION CHANGED
        // -----------------------------------------

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


        // -----------------------------------------
        // INVALID SAVED POSITION
        // -----------------------------------------

        if (
            progress.currentPosition
            >= quizzes.size()
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


        // Existing progress is valid.
        return;
    }


    // =============================================
    // FIRST RUN
    // =============================================

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


// =====================================================
// BUILD SHUFFLE ORDER
// =====================================================

void QuizManager::buildShuffleOrder()
{
    shuffledOrder.resize(
        quizzes.size()
    );


    // Creates:
    //
    // 0, 1, 2, 3, ...
    //
    std::iota(
        shuffledOrder.begin(),
        shuffledOrder.end(),
        0
    );


    // Same seed creates same order.
    std::mt19937 generator(
        progress.seed
    );


    std::shuffle(
        shuffledOrder.begin(),
        shuffledOrder.end(),
        generator
    );
}


// =====================================================
// CURRENT QUIZ
// =====================================================

Quiz QuizManager::getCurrentQuiz() const
{
    if (quizzes.empty()) {

        throw std::runtime_error(
            "Quiz dataset is empty."
        );
    }


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


    return quizzes[
        quizIndex
    ];
}


// =====================================================
// NEXT QUIZ
// =====================================================

void QuizManager::moveToNextQuiz()
{
    progress.currentPosition++;


    // =============================================
    // END OF ROUND
    // =============================================

    if (
        progress.currentPosition
        >= quizzes.size()
    ) {
        startNewRound();

        return;
    }


    // =============================================
    // SAVE CURRENT POSITION
    // =============================================

    progressStore.save(
        progress
    );
}


// =====================================================
// NEW ROUND
// =====================================================

void QuizManager::startNewRound()
{
    progress.round++;

    progress.seed =
        generateSeed();

    progress.currentPosition =
        0;

    progress.datasetVersion =
        CURRENT_DATASET_VERSION;


    // New seed → completely new order.
    buildShuffleOrder();


    progressStore.save(
        progress
    );
}