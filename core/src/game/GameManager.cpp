#include "GameManager.h"


// =====================================================
// CONSTRUCTOR
// =====================================================

GameManager::GameManager(
    IDispenser& dispenser
)
    : state(GameState::HOME),
      dispenser(dispenser),
      progressStore(
          "../dataset/progress/game_progress.json"
      )
{
    if (progressStore.exists()) {

        progress =
            progressStore.load();

    }
    else {

        progress.pendingReward =
            false;

        progressStore.save(
            progress
        );
    }
}


// =====================================================
// START GAME
// =====================================================

void GameManager::startGame()
{
    state =
        GameState::QUIZ;
}


// =====================================================
// CANCEL GAME
// =====================================================

void GameManager::cancelGame()
{
    // User explicitly gives up the current question.
    // Consume it immediately.

    quizManager.moveToNextQuiz();

    state =
        GameState::HOME;
}


// =====================================================
// RETRY QUIZ
// =====================================================

void GameManager::retryQuiz()
{
    // Wrong answer does NOT consume question.

    state =
        GameState::QUIZ;
}


// =====================================================
// SUBMIT ANSWER
// =====================================================

bool GameManager::submitAnswer(
    const std::string& answer
)
{
    Quiz quiz =
        quizManager.getCurrentQuiz();


    bool correct =
        answerValidator.validate(
            quiz,
            answer
        );


    // =================================================
    // CORRECT
    // =================================================

    if (correct) {

    // Quiz completed.
    quizManager.moveToNextQuiz();

    // User has earned one candy.
    progress.pendingReward =
        true;

    progressStore.save(
        progress
    );

    state =
        GameState::CORRECT;

    return true;
    }

    // =================================================
    // WRONG
    // =================================================

    state =
        GameState::WRONG;

    return false;
}


// =====================================================
// SELECT CANDY
// =====================================================

bool GameManager::selectCandy(
    int slot
)
{
    // Validate candy slot.

    if (
        slot < 1 ||
        slot > 6
    ) {
        state =
            GameState::ERROR;

        return false;
    }


    state =
        GameState::DISPENSING;


    // =================================================
    // DISPENSE
    // =================================================

    bool success =
        dispenser.dispense(
            slot
        );


    if (success) {

    progress.pendingReward =
        false;

    progressStore.save(
        progress
    );

    state =
        GameState::SUCCESS;

    return true;
    }


    state =
        GameState::ERROR;

    return false;
}


// =====================================================
// GET STATE
// =====================================================

GameState GameManager::getState() const
{
    return state;
}


// =====================================================
// GET CURRENT QUIZ
// =====================================================

Quiz GameManager::getCurrentQuiz() const
{
    return quizManager.getCurrentQuiz();
}

bool GameManager::hasPendingReward() const
{
    return progress.pendingReward;
}