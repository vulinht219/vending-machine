#pragma once

#include <lvgl.h>

#include "game/GameManager.h"


class QuizScreen {
public:
    static void create(GameManager& game);

private:

    // =================================================
    // CURRENT STATE
    // =================================================

    static GameManager* currentGame;

    static lv_obj_t* answerLabel;

    static lv_obj_t* popupOverlay;
    static lv_timer_t* popupTimer;

    static lv_obj_t* cancelOverlay;


    // =================================================
    // NUMERIC INPUT
    // =================================================

    static void appendToAnswer(
        const char* value
    );

    static void numberButtonEvent(
        lv_event_t* event
    );

    static void clearButtonEvent(
        lv_event_t* event
    );

    static void backspaceButtonEvent(
        lv_event_t* event
    );

    static void submitButtonEvent(
        lv_event_t* event
    );


    // =================================================
    // MULTIPLE CHOICE
    // =================================================

    static void multipleChoiceEvent(
        lv_event_t* event
    );


    // =================================================
    // INPUT LAYOUT
    // =================================================

    static void createNumericInput(
        lv_obj_t* parent
    );

    static void createMultipleChoiceInput(
        lv_obj_t* parent,
        const Quiz& quiz
    );


    // =================================================
    // WRONG / CORRECT
    // =================================================

    static void showWrongPopup();
    static void showCorrectPopup();

    static void wrongPopupFinished(
        lv_timer_t* timer
    );

    static void correctPopupFinished(
        lv_timer_t* timer
    );


    // =================================================
    // CANCEL
    // =================================================

    static void cancelButtonEvent(
        lv_event_t* event
    );

    static void showCancelConfirmation();

    static void cancelYesEvent(
        lv_event_t* event
    );

    static void cancelNoEvent(
        lv_event_t* event
    );
};