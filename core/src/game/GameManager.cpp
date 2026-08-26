#include "GameManager.h"


// =====================================================
// CONSTRUCTOR
// =====================================================

GameManager::GameManager(
    IDispenser& dispenser,
    IQuizSource& quizSource,
    IQuizProgressStore& quizProgressStore,
    IGameProgressStore& gameProgressStore
)
    : state(GameState::HOME),
      quizManager(
          quizSource,
          quizProgressStore
      ),
      dispenser(dispenser),
      progressStore(gameProgressStore)
{
    if (
        progressStore.exists()
    ) {
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
    // User gives up this question.
    // Consume it and go to the next one.

    quizManager.moveToNextQuiz();

    state =
        GameState::HOME;
}


// =====================================================
// RETRY QUIZ
// =====================================================

void GameManager::retryQuiz()
{
    // Wrong answer does NOT consume the quiz.

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


    if (correct) {

        // Question is completed immediately.
        quizManager.moveToNextQuiz();


        // The player has earned one candy.
        progress.pendingReward =
            true;

        progressStore.save(
            progress
        );


        state =
            GameState::CORRECT;

        return true;
    }


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


    bool success =
        dispenser.dispense(
            slot
        );


    if (success) {

        // The earned reward has now been used.
        progress.pendingReward =
            false;

        progressStore.save(
            progress
        );


        state =
            GameState::SUCCESS;

        return true;
    }


    // Dispensing failed.
    //
    // pendingReward remains TRUE because
    // the user has not received their candy.

    state =
        GameState::ERROR;

    return false;
}


// =====================================================
// PENDING REWARD
// =====================================================

bool GameManager::hasPendingReward() const
{
    return progress.pendingReward;
}


// =====================================================
// GET STATE
// =====================================================

GameState GameManager::getState() const
{
    return state;
}


// =====================================================
// CURRENT QUIZ
// =====================================================

Quiz GameManager::getCurrentQuiz()
{
    return quizManager.getCurrentQuiz();
}