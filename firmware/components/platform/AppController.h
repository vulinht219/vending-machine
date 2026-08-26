#pragma once

#include "AppState.h"

#include "SDCardQuizSource.h"
#include "NVSQuizProgressStore.h"
#include "NVSGameProgressStore.h"
#include "RealDispenser.h"

#include "game/GameManager.h"

#include <memory>


class AppController {
public:
    AppController();

    void start();

    AppState getState() const;

    GameManager* getGame();


private:
    AppState state;


    std::unique_ptr<SDCardQuizSource>
        quizSource;


    NVSQuizProgressStore
        quizProgressStore;


    NVSGameProgressStore
        gameProgressStore;


    RealDispenser
        dispenser;


    std::unique_ptr<GameManager>
        game;
};