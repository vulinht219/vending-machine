#pragma once

#include <string>

#include "quiz/Quiz.h"
#include "quiz/QuizManager.h"
#include "quiz/AnswerValidator.h"
#include "dispenser/IDispenser.h"

#include "GameProgress.h"
#include "GameProgressStore.h"


enum class GameState {
    HOME,
    QUIZ,
    WRONG,
    CORRECT,
    DISPENSING,
    SUCCESS,
    ERROR
};


class GameManager {
public:
    explicit GameManager(
        IDispenser& dispenser
    );


    // Start quiz flow
    void startGame();


    // User confirms cancel:
    // current quiz is consumed
    void cancelGame();


    // Wrong answer:
    // stay on same quiz
    void retryQuiz();


    // Check submitted answer
    bool submitAnswer(
        const std::string& answer
    );


    // Dispense selected candy
    bool selectCandy(
        int slot
    );

    bool hasPendingReward() const;


    GameState getState() const;


    Quiz getCurrentQuiz() const;


private:
    GameState state;

    QuizManager quizManager;

    AnswerValidator answerValidator;

    IDispenser& dispenser;

    GameProgress progress;

    GameProgressStore progressStore;
};