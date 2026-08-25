#include "QuizManager.h"

#include "QuizLoader.h"
#include "QuizValidator.h"

#include <algorithm>
#include <numeric>
#include <random>
#include <stdexcept>


// =====================================================
// CONSTRUCTOR
// =====================================================

QuizManager::QuizManager()
    : progressStore(
        "../dataset/progress/quiz_progress.json"
    )
{
    // =============================================
    // LOAD DATASET
    // =============================================

    quizzes =
        QuizLoader::loadFromJsonLines(
            "../dataset/sample/quizzes.jsonl"
        );


    // =============================================
    // VALIDATE DATASET
    // =============================================

    QuizValidator::validateDataset(
        quizzes
    );


    // =============================================
    // LOAD OR CREATE PROGRESS
    // =============================================

    initializeProgress();


    // =============================================
    // RECREATE SHUFFLE
    // =============================================

    buildShuffleOrder();
}


// =====================================================
// GENERATE RANDOM SEED
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
    if (
        progressStore.exists()
    ) {
        progress =
            progressStore.load();


        // =============================================
        // DATASET VERSION CHANGED
        // =============================================

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


        // =============================================
        // INVALID POSITION
        // =============================================

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
    }

    else {

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
}


// =====================================================
// BUILD SAME RANDOM ORDER FROM SEED
// =====================================================

void QuizManager::buildShuffleOrder()
{
    shuffledOrder.resize(
        quizzes.size()
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


// =====================================================
// GET CURRENT QUIZ
// =====================================================

Quiz QuizManager::getCurrentQuiz() const
{
    if (quizzes.empty()) {

        throw std::runtime_error(
            "Quiz dataset is empty."
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
// MOVE TO NEXT QUIZ
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
    // SAVE PROGRESS
    // =============================================

    progressStore.save(
        progress
    );
}


// =====================================================
// START NEW ROUND
// =====================================================

void QuizManager::startNewRound()
{
    progress.round++;

    progress.seed =
        generateSeed();

    progress.currentPosition =
        0;


    // New seed = new shuffled order.
    buildShuffleOrder();


    progressStore.save(
        progress
    );
}