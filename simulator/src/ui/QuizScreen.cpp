#include "QuizScreen.h"

#include "HomeScreen.h"
#include "CandySelectScreen.h"
#include "theme/Theme.h"

#include <cstdint>
#include <string>

LV_IMAGE_DECLARE(home_1);
LV_IMAGE_DECLARE(home_2);
LV_IMAGE_DECLARE(home_3);
LV_IMAGE_DECLARE(quiz_back_button);
LV_IMAGE_DECLARE(quiz_popup);

LV_IMAGE_DECLARE(correct_1);
LV_IMAGE_DECLARE(correct_2);
LV_IMAGE_DECLARE(correct_3);
LV_IMAGE_DECLARE(correct_title);

LV_IMAGE_DECLARE(incorrect_1);
LV_IMAGE_DECLARE(incorrect_2);
LV_IMAGE_DECLARE(incorrect_3);
LV_IMAGE_DECLARE(incorrect_title);

namespace {
lv_timer_t* quizBackgroundTimer = nullptr;
int currentQuizBackgroundFrame = 0;

const lv_image_dsc_t* quizBackgroundFrames[] = {
    &home_1, &home_2, &home_3
};

void stopQuizBackgroundAnimation()
{
    if (quizBackgroundTimer != nullptr) {
        lv_timer_delete(quizBackgroundTimer);
        quizBackgroundTimer = nullptr;
    }
}

void updateQuizBackground(lv_timer_t* timer)
{
    lv_obj_t* background =
        static_cast<lv_obj_t*>(lv_timer_get_user_data(timer));

    if (background == nullptr) return;

    currentQuizBackgroundFrame =
        (currentQuizBackgroundFrame + 1) % 3;

    lv_image_set_src(
        background,
        quizBackgroundFrames[currentQuizBackgroundFrame]
    );
}


lv_timer_t* resultAnimationTimer = nullptr;
int currentResultFrame = 0;

const lv_image_dsc_t* correctFrames[] = {
    &correct_1,
    &correct_2,
    &correct_3
};

const lv_image_dsc_t* incorrectFrames[] = {
    &incorrect_1,
    &incorrect_2,
    &incorrect_3
};

const lv_image_dsc_t** activeResultFrames = nullptr;


void stopResultAnimation()
{
    if (resultAnimationTimer != nullptr) {
        lv_timer_delete(resultAnimationTimer);
        resultAnimationTimer = nullptr;
    }
}


void updateResultAnimation(lv_timer_t* timer)
{
    lv_obj_t* image =
        static_cast<lv_obj_t*>(
            lv_timer_get_user_data(timer)
        );

    if (
        image == nullptr
        ||
        activeResultFrames == nullptr
    ) {
        return;
    }

    currentResultFrame =
        (currentResultFrame + 1) % 3;

    lv_image_set_src(
        image,
        activeResultFrames[currentResultFrame]
    );
}
}


// =====================================================
// STATIC VARIABLES
// =====================================================

GameManager* QuizScreen::currentGame = nullptr;

lv_obj_t* QuizScreen::answerLabel = nullptr;

lv_obj_t* QuizScreen::popupOverlay = nullptr;

lv_timer_t* QuizScreen::popupTimer = nullptr;

lv_obj_t* QuizScreen::cancelOverlay = nullptr;


// =====================================================
// NUMERIC INPUT HELPERS
// =====================================================

void QuizScreen::appendToAnswer(
    const char* value
)
{
    if (answerLabel == nullptr) {
        return;
    }


    const char* current =
        lv_label_get_text(
            answerLabel
        );


    std::string answer =
        current;


    if (
        answer.length() >= 8
    ) {
        return;
    }


    if (
        std::string(value) == "."
        &&
        answer.find('.') != std::string::npos
    ) {
        return;
    }


    if (
        std::string(value) == "."
        &&
        answer.empty()
    ) {
        answer = "0";
    }


    answer += value;


    lv_label_set_text(
        answerLabel,
        answer.c_str()
    );
}


void QuizScreen::numberButtonEvent(
    lv_event_t* event
)
{
    if (
        lv_event_get_code(event)
        != LV_EVENT_CLICKED
    ) {
        return;
    }


    const char* number =
        static_cast<const char*>(
            lv_event_get_user_data(
                event
            )
        );


    appendToAnswer(
        number
    );
}


void QuizScreen::clearButtonEvent(
    lv_event_t* event
)
{
    if (
        lv_event_get_code(event)
        != LV_EVENT_CLICKED
    ) {
        return;
    }


    if (
        answerLabel != nullptr
    ) {
        lv_label_set_text(
            answerLabel,
            ""
        );
    }
}


void QuizScreen::backspaceButtonEvent(
    lv_event_t* event
)
{
    if (
        lv_event_get_code(event)
        != LV_EVENT_CLICKED
    ) {
        return;
    }


    if (
        answerLabel == nullptr
    ) {
        return;
    }


    std::string answer =
        lv_label_get_text(
            answerLabel
        );


    if (
        !answer.empty()
    ) {
        answer.pop_back();
    }


    lv_label_set_text(
        answerLabel,
        answer.c_str()
    );
}


// =====================================================
// MULTIPLE CHOICE EVENT
// =====================================================

void QuizScreen::multipleChoiceEvent(
    lv_event_t* event
)
{
    if (
        lv_event_get_code(event)
        != LV_EVENT_CLICKED
    ) {
        return;
    }


    int selectedIndex =
        static_cast<int>(
            reinterpret_cast<intptr_t>(
                lv_event_get_user_data(
                    event
                )
            )
        );


    std::string answer =
        std::to_string(
            selectedIndex
        );


    bool correct =
        currentGame
            ->submitAnswer(
                answer
            );


    if (
        correct
    ) {
        showCorrectPopup();
    }
    else {
        showWrongPopup();
    }
}


// =====================================================
// NUMERIC SUBMIT
// =====================================================

void QuizScreen::submitButtonEvent(
    lv_event_t* event
)
{
    if (
        lv_event_get_code(event)
        != LV_EVENT_CLICKED
    ) {
        return;
    }


    if (
        answerLabel == nullptr
    ) {
        return;
    }


    const char* answer =
        lv_label_get_text(
            answerLabel
        );


    if (
        answer == nullptr ||
        answer[0] == '\0'
    ) {
        return;
    }


    bool correct =
        currentGame
            ->submitAnswer(
                answer
            );


    if (
        correct
    ) {
        showCorrectPopup();
    }
    else {
        showWrongPopup();
    }
}


// =====================================================
// WRONG RESULT SCREEN
// Full-screen 3-frame animation for 5 seconds.
// =====================================================

void QuizScreen::showWrongPopup()
{
    if (
        popupOverlay != nullptr
    ) {
        return;
    }

    stopQuizBackgroundAnimation();
    stopResultAnimation();

    lv_obj_t* screen =
        lv_screen_active();

    currentResultFrame = 0;
    activeResultFrames = incorrectFrames;

    popupOverlay =
        lv_image_create(
            screen
        );

    lv_image_set_src(
        popupOverlay,
        &incorrect_1
    );

    lv_obj_align(
        popupOverlay,
        LV_ALIGN_CENTER,
        0,
        0
    );


    // Incorrect title image on top of the animation.
    lv_obj_t* title =
        lv_image_create(
            screen
        );

    lv_image_set_src(
        title,
        &incorrect_title
    );

    lv_obj_align(
        title,
        LV_ALIGN_TOP_MID,
        0,
        90
    );

    lv_obj_remove_flag(
        title,
        LV_OBJ_FLAG_CLICKABLE
    );


    lv_obj_add_flag(
        popupOverlay,
        LV_OBJ_FLAG_CLICKABLE
    );

    resultAnimationTimer =
        lv_timer_create(
            updateResultAnimation,
            400,
            popupOverlay
        );

    popupTimer =
        lv_timer_create(
            wrongPopupFinished,
            5000,
            nullptr
        );

    lv_timer_set_repeat_count(
        popupTimer,
        1
    );
}


// =====================================================
// CORRECT RESULT SCREEN
// Full-screen 3-frame animation for 5 seconds.
// =====================================================

void QuizScreen::showCorrectPopup()
{
    if (
        popupOverlay != nullptr
    ) {
        return;
    }

    stopQuizBackgroundAnimation();
    stopResultAnimation();

    lv_obj_t* screen =
        lv_screen_active();

    currentResultFrame = 0;
    activeResultFrames = correctFrames;

    popupOverlay =
        lv_image_create(
            screen
        );

    lv_image_set_src(
        popupOverlay,
        &correct_1
    );

    lv_obj_align(
        popupOverlay,
        LV_ALIGN_CENTER,
        0,
        0
    );


    // Correct title image on top of the animation.
    lv_obj_t* title =
        lv_image_create(
            screen
        );

    lv_image_set_src(
        title,
        &correct_title
    );

    lv_obj_align(
        title,
        LV_ALIGN_TOP_MID,
        0,
        90
    );

    lv_obj_remove_flag(
        title,
        LV_OBJ_FLAG_CLICKABLE
    );


    lv_obj_add_flag(
        popupOverlay,
        LV_OBJ_FLAG_CLICKABLE
    );

    resultAnimationTimer =
        lv_timer_create(
            updateResultAnimation,
            400,
            popupOverlay
        );

    popupTimer =
        lv_timer_create(
            correctPopupFinished,
            5000,
            nullptr
        );

    lv_timer_set_repeat_count(
        popupTimer,
        1
    );
}


// =====================================================
// WRONG FINISHED
// =====================================================

void QuizScreen::wrongPopupFinished(
    lv_timer_t* timer
)
{
    popupTimer = nullptr;

    stopResultAnimation();

    popupOverlay = nullptr;
    activeResultFrames = nullptr;

    currentGame
        ->retryQuiz();

    // Rebuild QuizScreen completely.
    // This restores the normal Home background
    // and shows the same quiz again.
    QuizScreen::create(
        *currentGame
    );
}


// =====================================================
// CORRECT FINISHED
// =====================================================

void QuizScreen::correctPopupFinished(
    lv_timer_t* timer
)
{
    popupTimer = nullptr;

    stopResultAnimation();

    popupOverlay = nullptr;
    activeResultFrames = nullptr;

    CandySelectScreen::create(
        *currentGame
    );
}


// =====================================================
// CANCEL BUTTON
// =====================================================

void QuizScreen::cancelButtonEvent(
    lv_event_t* event
)
{
    if (
        lv_event_get_code(event)
        != LV_EVENT_CLICKED
    ) {
        return;
    }


    showCancelConfirmation();
}


// =====================================================
// CANCEL CONFIRMATION POPUP
// =====================================================

void QuizScreen::showCancelConfirmation()
{
    if (
        cancelOverlay != nullptr
    ) {
        return;
    }


    lv_obj_t* screen =
        lv_screen_active();


    // =================================================
    // FULL SCREEN DARK OVERLAY
    // =================================================

    cancelOverlay =
        lv_obj_create(
            screen
        );


    lv_obj_set_size(
        cancelOverlay,
        LV_PCT(100),
        LV_PCT(100)
    );


    lv_obj_set_style_bg_color(
        cancelOverlay,
        lv_color_hex(
            0x000000
        ),
        0
    );


    lv_obj_set_style_bg_opa(
        cancelOverlay,
        LV_OPA_50,
        0
    );


    lv_obj_set_style_border_width(
        cancelOverlay,
        0,
        0
    );


    lv_obj_set_style_pad_all(
        cancelOverlay,
        0,
        0
    );


    lv_obj_add_flag(
        cancelOverlay,
        LV_OBJ_FLAG_CLICKABLE
    );


    // =================================================
    // POPUP BOX
    // =================================================

    lv_obj_t* popup =
        lv_obj_create(
            cancelOverlay
        );


    lv_obj_set_width(
        popup,
        LV_PCT(80)
    );


    lv_obj_set_height(
        popup,
        220
    );


    lv_obj_center(
        popup
    );


    Theme::applyPanel(
        popup
    );


    lv_obj_set_style_pad_all(
        popup,
        Theme::SPACING_LG,
        0
    );


    lv_obj_set_style_pad_row(
        popup,
        Theme::SPACING_MD,
        0
    );


    lv_obj_set_flex_flow(
        popup,
        LV_FLEX_FLOW_COLUMN
    );


    lv_obj_set_flex_align(
        popup,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER
    );


    // =================================================
    // TEXT
    // =================================================

    lv_obj_t* question =
        lv_label_create(
            popup
        );


    lv_label_set_text(
        question,
        "Are you sure you want to leave the quiz?"
    );


    lv_obj_set_width(
        question,
        LV_PCT(95)
    );


    lv_label_set_long_mode(
        question,
        LV_LABEL_LONG_WRAP
    );


    lv_obj_set_style_text_align(
        question,
        LV_TEXT_ALIGN_CENTER,
        0
    );


    Theme::applyNormalText(
        question
    );


    // =================================================
    // YES / NO ROW
    // =================================================

    lv_obj_t* buttonRow =
        lv_obj_create(
            popup
        );


    lv_obj_set_size(
        buttonRow,
        LV_PCT(100),
        60
    );


    lv_obj_set_style_bg_opa(
        buttonRow,
        LV_OPA_TRANSP,
        0
    );


    lv_obj_set_style_border_width(
        buttonRow,
        0,
        0
    );


    lv_obj_set_style_pad_all(
        buttonRow,
        0,
        0
    );


    lv_obj_set_style_pad_column(
        buttonRow,
        Theme::SPACING_MD,
        0
    );


    lv_obj_set_flex_flow(
        buttonRow,
        LV_FLEX_FLOW_ROW
    );


    lv_obj_set_flex_align(
        buttonRow,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER
    );


    // =================================================
    // NO
    // =================================================

    lv_obj_t* noButton =
        lv_button_create(
            buttonRow
        );


    lv_obj_set_size(
        noButton,
        105,
        45
    );


    Theme::applySecondaryButton(
        noButton
    );


    lv_obj_t* noLabel =
        lv_label_create(
            noButton
        );


    lv_label_set_text(
        noLabel,
        "NO"
    );


    Theme::applyButtonText(
        noLabel
    );


    lv_obj_center(
        noLabel
    );


    lv_obj_add_event_cb(
        noButton,
        cancelNoEvent,
        LV_EVENT_CLICKED,
        nullptr
    );


    // =================================================
    // YES
    // =================================================

    lv_obj_t* yesButton =
        lv_button_create(
            buttonRow
        );


    lv_obj_set_size(
        yesButton,
        105,
        45
    );


    Theme::applyPrimaryButton(
        yesButton
    );


    lv_obj_t* yesLabel =
        lv_label_create(
            yesButton
        );


    lv_label_set_text(
        yesLabel,
        "YES"
    );


    Theme::applyButtonText(
        yesLabel
    );


    lv_obj_center(
        yesLabel
    );


    lv_obj_add_event_cb(
        yesButton,
        cancelYesEvent,
        LV_EVENT_CLICKED,
        nullptr
    );
}


// =====================================================
// CANCEL → NO
// =====================================================

void QuizScreen::cancelNoEvent(
    lv_event_t* event
)
{
    if (
        lv_event_get_code(event)
        != LV_EVENT_CLICKED
    ) {
        return;
    }


    if (
        cancelOverlay != nullptr
    ) {

        lv_obj_delete(
            cancelOverlay
        );


        cancelOverlay =
            nullptr;
    }
}


// =====================================================
// CANCEL → YES
// =====================================================

void QuizScreen::cancelYesEvent(
    lv_event_t* event
)
{
    if (
        lv_event_get_code(event)
        != LV_EVENT_CLICKED
    ) {
        return;
    }


    cancelOverlay =
        nullptr;


    currentGame
        ->cancelGame();

    stopQuizBackgroundAnimation();

    HomeScreen::create(
        *currentGame
    );
}


// =====================================================
// CREATE NUMERIC INPUT
// =====================================================

void QuizScreen::createNumericInput(
    lv_obj_t* parent
)
{
    // =================================================
    // INPUT ROW
    // Answer box + backspace on the same row
    // =================================================

    lv_obj_t* inputRow =
        lv_obj_create(
            parent
        );

    lv_obj_set_width(
        inputRow,
        LV_PCT(100)
    );

    lv_obj_set_height(
        inputRow,
        75
    );

    lv_obj_set_style_bg_opa(
        inputRow,
        LV_OPA_TRANSP,
        0
    );

    lv_obj_set_style_border_width(
        inputRow,
        0,
        0
    );

    lv_obj_set_style_pad_all(
        inputRow,
        0,
        0
    );

    lv_obj_set_style_pad_column(
        inputRow,
        10,
        0
    );

    lv_obj_set_flex_flow(
        inputRow,
        LV_FLEX_FLOW_ROW
    );

    lv_obj_set_flex_align(
        inputRow,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER
    );


    // =================================================
    // ANSWER BOX
    // =================================================

    lv_obj_t* answerBox =
        lv_obj_create(
            inputRow
        );

    lv_obj_set_width(
        answerBox,
        310
    );

    lv_obj_set_height(
        answerBox,
        75
    );

    lv_obj_set_style_bg_color(
        answerBox,
        lv_color_hex(0xF7D79A),
        0
    );

    lv_obj_set_style_bg_opa(
        answerBox,
        LV_OPA_COVER,
        0
    );

    lv_obj_set_style_border_width(
        answerBox,
        0,
        0
    );

    lv_obj_set_style_radius(
        answerBox,
        14,
        0
    );

    lv_obj_set_style_shadow_width(
        answerBox,
        0,
        0
    );

    answerLabel =
        lv_label_create(
            answerBox
        );

    lv_label_set_text(
        answerLabel,
        ""
    );

    Theme::applyNormalText(
        answerLabel
    );

    lv_obj_set_style_text_font(
        answerLabel,
        &lv_font_montserrat_28,
        0
    );

    lv_obj_set_style_text_color(
        answerLabel,
        lv_color_hex(0x000000),
        0
    );

    lv_obj_center(
        answerLabel
    );


    // =================================================
    // BACKSPACE BUTTON
    // =================================================

    lv_obj_t* backspace =
        lv_button_create(
            inputRow
        );

    lv_obj_set_size(
        backspace,
        65,
        75
    );

    lv_obj_set_style_bg_color(
        backspace,
        lv_color_hex(0xF7D79A),
        0
    );

    lv_obj_set_style_bg_opa(
        backspace,
        LV_OPA_COVER,
        0
    );

    lv_obj_set_style_border_width(
        backspace,
        0,
        0
    );

    lv_obj_set_style_radius(
        backspace,
        14,
        0
    );

    lv_obj_set_style_shadow_width(
        backspace,
        0,
        0
    );

    lv_obj_t* backspaceLabel =
        lv_label_create(
            backspace
        );

    lv_label_set_text(
        backspaceLabel,
        "<"
    );

    Theme::applyButtonText(
        backspaceLabel
    );

    lv_obj_set_style_text_font(
        backspaceLabel,
        &lv_font_montserrat_18,
        0
    );

    lv_obj_set_style_text_color(
        backspaceLabel,
        lv_color_hex(0x000000),
        0
    );

    lv_obj_center(
        backspaceLabel
    );

    lv_obj_add_event_cb(
        backspace,
        backspaceButtonEvent,
        LV_EVENT_CLICKED,
        nullptr
    );


    // =================================================
    // KEYPAD
    // 4 rows x 3 columns
    //
    // 1 2 3
    // 4 5 6
    // 7 8 9
    // C 0 .
    // =================================================

    lv_obj_t* keypad =
        lv_obj_create(
            parent
        );

    lv_obj_set_width(
        keypad,
        LV_PCT(100)
    );

    lv_obj_set_height(
        keypad,
        280
    );

    lv_obj_set_style_bg_opa(
        keypad,
        LV_OPA_TRANSP,
        0
    );

    lv_obj_set_style_border_width(
        keypad,
        0,
        0
    );

    lv_obj_set_style_radius(
        keypad,
        0,
        0
    );

    lv_obj_set_style_pad_all(
        keypad,
        0,
        0
    );

    lv_obj_set_style_pad_row(
        keypad,
        10,
        0
    );

    lv_obj_set_style_pad_column(
        keypad,
        10,
        0
    );

    static int32_t columns[] = {
        LV_GRID_FR(1),
        LV_GRID_FR(1),
        LV_GRID_FR(1),
        LV_GRID_TEMPLATE_LAST
    };

    static int32_t rows[] = {
        LV_GRID_FR(1),
        LV_GRID_FR(1),
        LV_GRID_FR(1),
        LV_GRID_FR(1),
        LV_GRID_TEMPLATE_LAST
    };

    lv_obj_set_grid_dsc_array(
        keypad,
        columns,
        rows
    );

    lv_obj_set_layout(
        keypad,
        LV_LAYOUT_GRID
    );

    const char* numbers[12] = {
        "1", "2", "3",
        "4", "5", "6",
        "7", "8", "9",
        "C", "0", "."
    };

    for (
        int i = 0;
        i < 12;
        i++
    ) {
        int row = i / 3;
        int column = i % 3;

        lv_obj_t* button =
            lv_button_create(
                keypad
            );

        lv_obj_set_grid_cell(
            button,
            LV_GRID_ALIGN_STRETCH,
            column,
            1,
            LV_GRID_ALIGN_STRETCH,
            row,
            1
        );

        lv_obj_set_style_bg_color(
            button,
            lv_color_hex(0xF7D79A),
            0
        );

        lv_obj_set_style_bg_opa(
            button,
            LV_OPA_COVER,
            0
        );

        lv_obj_set_style_border_width(
            button,
            0,
            0
        );

        lv_obj_set_style_radius(
            button,
            12,
            0
        );

        lv_obj_set_style_shadow_width(
            button,
            0,
            0
        );

        lv_obj_t* label =
            lv_label_create(
                button
            );

        lv_label_set_text(
            label,
            numbers[i]
        );

        Theme::applyButtonText(
            label
        );

        lv_obj_set_style_text_font(
            label,
            &lv_font_montserrat_18,
            0
        );

        lv_obj_set_style_text_color(
            label,
            lv_color_hex(0x000000),
            0
        );

        lv_obj_center(
            label
        );

        if (
            i == 9
        ) {
            lv_obj_add_event_cb(
                button,
                clearButtonEvent,
                LV_EVENT_CLICKED,
                nullptr
            );
        }
        else {
            lv_obj_add_event_cb(
                button,
                numberButtonEvent,
                LV_EVENT_CLICKED,
                (void*)numbers[i]
            );
        }
    }


    // =================================================
    // SUBMIT
    // =================================================

    lv_obj_t* submit =
        lv_button_create(
            parent
        );

    lv_obj_set_width(
        submit,
        LV_PCT(70)
    );

    lv_obj_set_height(
        submit,
        55
    );

    lv_obj_set_style_bg_color(
        submit,
        lv_color_hex(0xF7D79A),
        0
    );

    lv_obj_set_style_bg_opa(
        submit,
        LV_OPA_COVER,
        0
    );

    lv_obj_set_style_border_width(
        submit,
        0,
        0
    );

    lv_obj_set_style_radius(
        submit,
        14,
        0
    );

    lv_obj_set_style_shadow_width(
        submit,
        0,
        0
    );

    lv_obj_t* submitLabel =
        lv_label_create(
            submit
        );

    lv_label_set_text(
        submitLabel,
        "SUBMIT"
    );

    Theme::applyButtonText(
        submitLabel
    );

    lv_obj_set_style_text_font(
        submitLabel,
        &lv_font_montserrat_18,
        0
    );

    lv_obj_set_style_text_color(
        submitLabel,
        lv_color_hex(0x000000),
        0
    );

    lv_obj_center(
        submitLabel
    );

    lv_obj_add_event_cb(
        submit,
        submitButtonEvent,
        LV_EVENT_CLICKED,
        nullptr
    );
}


// =====================================================
// CREATE MULTIPLE CHOICE INPUT
// =====================================================

void QuizScreen::createMultipleChoiceInput(
    lv_obj_t* parent,
    const Quiz& quiz
)
{
    answerLabel =
        nullptr;


    // =================================================
    // OPTIONS CONTAINER
    // =================================================

    lv_obj_t* optionsContainer =
        lv_obj_create(
            parent
        );


    lv_obj_set_width(
        optionsContainer,
        LV_PCT(100)
    );


    lv_obj_set_height(
        optionsContainer,
        LV_SIZE_CONTENT
    );


    lv_obj_set_style_bg_opa(
        optionsContainer,
        LV_OPA_TRANSP,
        0
    );

    lv_obj_set_style_border_width(
        optionsContainer,
        0,
        0
    );

    lv_obj_set_style_radius(
        optionsContainer,
        0,
        0
    );


    lv_obj_set_style_pad_all(
        optionsContainer,
        0,
        0
    );


    lv_obj_set_style_pad_row(
        optionsContainer,
        10,
        0
    );


    // =================================================
    // VERTICAL FLEX LAYOUT
    // =================================================

    lv_obj_set_flex_flow(
        optionsContainer,
        LV_FLEX_FLOW_COLUMN
    );


    lv_obj_set_flex_align(
        optionsContainer,

        LV_FLEX_ALIGN_START,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER
    );


    // =================================================
    // OPTIONS DO NOT SCROLL SEPARATELY
    // =================================================
    //
    // Question and options belong to the same card.
    // We do not want a second independent scroll area.
    //

    lv_obj_set_scrollbar_mode(
        optionsContainer,
        LV_SCROLLBAR_MODE_OFF
    );


    lv_obj_remove_flag(
        optionsContainer,
        LV_OBJ_FLAG_SCROLLABLE
    );


    // =================================================
    // OPTION LETTERS
    // =================================================

    const char* letters[] = {

        "A",
        "B",
        "C",
        "D"
    };


    // =================================================
    // CREATE ANSWER BUTTONS
    // =================================================

    for (
        int i = 0;
        i < 4;
        i++
    ) {

        if (
            i >=
            static_cast<int>(
                quiz.options.size()
            )
        ) {
            break;
        }


        // =============================================
        // BUTTON
        // =============================================

        lv_obj_t* button =
            lv_button_create(
                optionsContainer
            );


        lv_obj_set_width(
            button,
            LV_PCT(100)
        );


        // Dynamic height so long answers can wrap.
        lv_obj_set_height(
            button,
            LV_SIZE_CONTENT
        );


        lv_obj_set_style_pad_top(
            button,
            13,
            0
        );


        lv_obj_set_style_pad_bottom(
            button,
            13,
            0
        );


        lv_obj_set_style_pad_left(
            button,
            10,
            0
        );


        lv_obj_set_style_pad_right(
            button,
            10,
            0
        );


        lv_obj_set_style_bg_color(
            button,
            lv_color_hex(0xF7D79A),
            0
        );

        lv_obj_set_style_bg_opa(
            button,
            LV_OPA_COVER,
            0
        );

        lv_obj_set_style_border_width(
            button,
            0,
            0
        );

        lv_obj_set_style_radius(
            button,
            14,
            0
        );

        lv_obj_set_style_shadow_width(
            button,
            0,
            0
        );


        // =============================================
        // LABEL
        // =============================================

        lv_obj_t* label =
            lv_label_create(
                button
            );


        std::string text =
            std::string(
                letters[i]
            )
            + ". "
            + quiz.options[i];


        lv_label_set_text(
            label,
            text.c_str()
        );


        lv_obj_set_width(
            label,
            LV_PCT(96)
        );


        lv_label_set_long_mode(
            label,
            LV_LABEL_LONG_WRAP
        );


        lv_obj_set_style_text_align(
            label,
            LV_TEXT_ALIGN_LEFT,
            0
        );


        Theme::applyButtonText(
            label
        );

        lv_obj_set_style_text_font(
            label,
            &lv_font_montserrat_18,
            0
        );

        lv_obj_set_style_text_color(
            label,
            lv_color_hex(0x000000),
            0
        );


        lv_obj_center(
            label
        );


        // =============================================
        // CLICK EVENT
        // =============================================

        lv_obj_add_event_cb(
            button,
            multipleChoiceEvent,
            LV_EVENT_CLICKED,

            reinterpret_cast<void*>(
                static_cast<intptr_t>(
                    i
                )
            )
        );
    }
}


// =====================================================
// CREATE QUIZ SCREEN
// =====================================================

void QuizScreen::create(
    GameManager& game
)
{

    stopResultAnimation();
    activeResultFrames = nullptr;
    currentGame = &game;
    answerLabel = nullptr;
    popupOverlay = nullptr;
    popupTimer = nullptr;
    cancelOverlay = nullptr;

    stopQuizBackgroundAnimation();

    lv_obj_t* screen = lv_screen_active();
    lv_obj_clean(screen);
    Theme::applyScreen(screen);

    Quiz quiz = game.getCurrentQuiz();

    // Animated background: reuse Home frames.
    currentQuizBackgroundFrame = 0;

    lv_obj_t* background = lv_image_create(screen);
    lv_image_set_src(background, &home_1);
    lv_obj_align(background, LV_ALIGN_CENTER, 0, 0);
    lv_obj_remove_flag(background, LV_OBJ_FLAG_CLICKABLE);

    quizBackgroundTimer =
        lv_timer_create(
            updateQuizBackground,
            400,
            background
        );

    // Figma popup image: 445 x 664.
    lv_obj_t* popupImage = lv_image_create(screen);
    lv_image_set_src(popupImage, &quiz_popup);
    lv_obj_align(popupImage, LV_ALIGN_CENTER, 0, 40);
    lv_obj_remove_flag(popupImage, LV_OBJ_FLAG_CLICKABLE);

    // Transparent content layer over the popup.
    lv_obj_t* quizCard = lv_obj_create(screen);
    lv_obj_set_size(quizCard, 425, 610);
    lv_obj_align(quizCard, LV_ALIGN_CENTER, 0, 40);

    lv_obj_set_style_bg_opa(quizCard, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(quizCard, 0, 0);

    lv_obj_set_style_pad_top(quizCard, 22, 0);
    lv_obj_set_style_pad_bottom(quizCard, 18, 0);
    lv_obj_set_style_pad_left(quizCard, 12, 0);
    lv_obj_set_style_pad_right(quizCard, 12, 0);
    lv_obj_set_style_pad_row(quizCard, Theme::SPACING_MD, 0);

    lv_obj_set_flex_flow(quizCard, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(
        quizCard,
        LV_FLEX_ALIGN_START,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER
    );

    lv_obj_set_scrollbar_mode(
        quizCard,
        LV_SCROLLBAR_MODE_OFF
    );
    lv_obj_remove_flag(
        quizCard,
        LV_OBJ_FLAG_SCROLLABLE
    );

    // Dynamic question from the dataset.
    lv_obj_t* question = lv_label_create(quizCard);
    lv_label_set_text(question, quiz.question.c_str());
    lv_obj_set_width(question, LV_PCT(94));
    lv_label_set_long_mode(question, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(
        question,
        LV_TEXT_ALIGN_CENTER,
        0
    );
    Theme::applyNormalText(question);

    lv_obj_set_style_text_font(
        question,
        &lv_font_montserrat_20,
        0
    );

    lv_obj_set_style_text_color(
        question,
        lv_color_hex(0xFFFFFF),
        0
    );

    // Preserve existing numeric / MCQ logic.
    if (quiz.type == QuizType::NUMERIC) {
        createNumericInput(quizCard);
    }
    else if (quiz.type == QuizType::MULTIPLE_CHOICE) {
        createMultipleChoiceInput(quizCard, quiz);
    }

    // Figma back button: 56 x 56.
    // Created last so it stays above the other layers.
    lv_obj_t* cancel = lv_image_create(screen);
    lv_image_set_src(cancel, &quiz_back_button);
    lv_obj_add_flag(cancel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(cancel, LV_ALIGN_TOP_LEFT, 18, 28);

    // Keep the existing cancel-confirmation flow.
    lv_obj_add_event_cb(
        cancel,
        cancelButtonEvent,
        LV_EVENT_CLICKED,
        nullptr
    );
}
