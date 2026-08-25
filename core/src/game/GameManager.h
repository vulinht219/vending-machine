#pragma once

#include <string>

#include "quiz/Quiz.h"
#include "quiz/QuizManager.h"
#include "quiz/IQuizSource.h"
#include "quiz/IQuizProgressStore.h"
#include "quiz/AnswerValidator.h"

#include "dispenser/IDispenser.h"

#include "GameProgress.h"
#include "IGameProgressStore.h"


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

    GameManager(
        IDispenser& dispenser,
        IQuizSource& quizSource,
        IQuizProgressStore& quizProgressStore,
        IGameProgressStore& gameProgressStore
    );


    void startGame();

    void cancelGame();

    void retryQuiz();


    bool submitAnswer(
        const std::string& answer
    );


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

    IGameProgressStore& progressStore;
};