#pragma once

#include "AppState.h"

#include "SDCardQuizSource.h"
#include "NVSQuizProgressStore.h"
#include "NVSGameProgressStore.h"
#include "NVSSpecialEventProgressStore.h"

#include "RealDispenser.h"
#include "DS3231Clock.h"

#include "game/GameManager.h"
#include "event/SpecialEventManager.h"

#include <memory>


class AppController
{
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


    NVSSpecialEventProgressStore
        specialEventProgressStore;


    RealDispenser
        dispenser;


    std::unique_ptr<DS3231Clock>
        clock;


    std::unique_ptr<GameManager>
        game;


    std::unique_ptr<SpecialEventManager>
        specialEventManager;
};