#include "GameManager.h"

GameManager::GameManager(IDispenser& dispenser)
    : dispenser(dispenser)
{
    state = GameState::HOME;
}

void GameManager::startGame() {
    state = GameState::QUIZ;
}

bool GameManager::submitAnswer(
    const std::string& answer
) {
    Quiz quiz = quizManager.getCurrentQuiz();

    bool correct =
        answerValidator.validate(quiz, answer);

    if (correct) {
        state = GameState::CORRECT;
        state = GameState::CANDY_SELECT;
        return true;
    }

    state = GameState::WRONG;
    return false;
}

bool GameManager::selectCandy(int slot) {
    if (slot < 1 || slot > 6) {
        state = GameState::ERROR;
        return false;
    }

    state = GameState::DISPENSING;

    bool success =
        dispenser.dispense(slot);

    if (success) {
        state = GameState::SUCCESS;

        quizManager.moveToNextQuiz();

        return true;
    }

    state = GameState::ERROR;
    return false;
}

GameState GameManager::getState() const {
    return state;
}

Quiz GameManager::getCurrentQuiz() {
    return quizManager.getCurrentQuiz();
}

void GameManager::cancelGame() {
    state = GameState::HOME;
}