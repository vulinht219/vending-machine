#pragma once

#include "GameState.h"
#include "../quiz/QuizManager.h"
#include "../quiz/AnswerValidator.h"
#include "../dispenser/IDispenser.h"

#include <string>

class GameManager {
public:
    GameManager(IDispenser& dispenser);

    void startGame();
    void cancelGame();

    bool submitAnswer(const std::string& answer);
    bool selectCandy(int slot);

    GameState getState() const;
    Quiz getCurrentQuiz();

private:
    GameState state;

    QuizManager quizManager;
    AnswerValidator answerValidator;

    IDispenser& dispenser;
};