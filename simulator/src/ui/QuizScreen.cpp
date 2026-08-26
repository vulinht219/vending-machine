#include "QuizScreen.h"

#include "HomeScreen.h"
#include "CandySelectScreen.h"
#include "theme/Theme.h"

#include <cstdint>
#include <string>


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
        lv_label_get_text(answerLabel);

    std::string answer = current;

    if (answer.length() >= 8) {
        return;
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
            lv_event_get_user_data(event)
        );

    appendToAnswer(number);
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

    if (answerLabel != nullptr) {
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

    if (answerLabel == nullptr) {
        return;
    }

    std::string answer =
        lv_label_get_text(answerLabel);

    if (!answer.empty()) {
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
                lv_event_get_user_data(event)
            )
        );

    std::string answer =
        std::to_string(selectedIndex);

    bool correct =
        currentGame->submitAnswer(answer);

    if (correct) {
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

    if (answerLabel == nullptr) {
        return;
    }

    const char* answer =
        lv_label_get_text(answerLabel);

    if (
        answer == nullptr ||
        answer[0] == '\0'
    ) {
        return;
    }

    bool correct =
        currentGame->submitAnswer(answer);

    if (correct) {
        showCorrectPopup();
    }
    else {
        showWrongPopup();
    }
}


// =====================================================
// WRONG POPUP
// =====================================================

void QuizScreen::showWrongPopup()
{
    if (popupOverlay != nullptr) {
        return;
    }

    lv_obj_t* screen =
        lv_screen_active();

    popupOverlay =
        lv_obj_create(screen);

    lv_obj_set_size(
        popupOverlay,
        LV_PCT(100),
        LV_PCT(100)
    );

    lv_obj_align(
        popupOverlay,
        LV_ALIGN_CENTER,
        0,
        0
    );

    lv_obj_add_flag(
        popupOverlay,
        LV_OBJ_FLAG_CLICKABLE
    );

    Theme::applyPanel(
        popupOverlay
    );

    lv_obj_t* label =
        lv_label_create(
            popupOverlay
        );

    lv_label_set_text(
        label,
        "WRONG!"
    );

    Theme::applyTitle(label);

    lv_obj_center(label);

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
// CORRECT POPUP
// =====================================================

void QuizScreen::showCorrectPopup()
{
    if (popupOverlay != nullptr) {
        return;
    }

    lv_obj_t* screen =
        lv_screen_active();

    popupOverlay =
        lv_obj_create(screen);

    lv_obj_set_size(
        popupOverlay,
        LV_PCT(100),
        LV_PCT(100)
    );

    lv_obj_align(
        popupOverlay,
        LV_ALIGN_CENTER,
        0,
        0
    );

    lv_obj_add_flag(
        popupOverlay,
        LV_OBJ_FLAG_CLICKABLE
    );

    Theme::applyPanel(
        popupOverlay
    );

    lv_obj_t* label =
        lv_label_create(
            popupOverlay
        );

    lv_label_set_text(
        label,
        "CORRECT!"
    );

    Theme::applyTitle(label);

    lv_obj_center(label);

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
    currentGame->retryQuiz();

    if (popupOverlay != nullptr) {

        lv_obj_delete(
            popupOverlay
        );

        popupOverlay = nullptr;
    }

    if (answerLabel != nullptr) {

        lv_label_set_text(
            answerLabel,
            ""
        );
    }

    popupTimer = nullptr;
}


// =====================================================
// CORRECT FINISHED
// =====================================================

void QuizScreen::correctPopupFinished(
    lv_timer_t* timer
)
{
    popupTimer = nullptr;
    popupOverlay = nullptr;

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
    if (cancelOverlay != nullptr) {
        return;
    }

    lv_obj_t* screen =
        lv_screen_active();


    // =================================================
    // FULL SCREEN DARK OVERLAY
    // =================================================

    cancelOverlay =
        lv_obj_create(screen);

    lv_obj_set_size(
        cancelOverlay,
        LV_PCT(100),
        LV_PCT(100)
    );

    lv_obj_set_style_bg_color(
        cancelOverlay,
        lv_color_hex(0x000000),
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
        lv_obj_create(cancelOverlay);

    lv_obj_set_width(
        popup,
        LV_PCT(85)
    );

    lv_obj_set_height(
        popup,
        190
    );

    lv_obj_center(popup);

    Theme::applyPanel(popup);

    lv_obj_set_style_pad_all(
        popup,
        Theme::SPACING_LG,
        0
    );

    lv_obj_set_style_pad_row(
        popup,
        Theme::SPACING_LG,
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
        lv_label_create(popup);

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
        lv_obj_create(popup);

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
        lv_button_create(buttonRow);

    lv_obj_set_size(
        noButton,
        90,
        44
    );

    Theme::applySecondaryButton(
        noButton
    );

    lv_obj_t* noLabel =
        lv_label_create(noButton);

    lv_label_set_text(
        noLabel,
        "NO"
    );

    Theme::applyButtonText(
        noLabel
    );

    lv_obj_center(noLabel);

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
        lv_button_create(buttonRow);

    lv_obj_set_size(
        yesButton,
        90,
        44
    );

    Theme::applyPrimaryButton(
        yesButton
    );

    lv_obj_t* yesLabel =
        lv_label_create(yesButton);

    lv_label_set_text(
        yesLabel,
        "YES"
    );

    Theme::applyButtonText(
        yesLabel
    );

    lv_obj_center(yesLabel);

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

    if (cancelOverlay != nullptr) {

        lv_obj_delete(
            cancelOverlay
        );

        cancelOverlay = nullptr;
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

    cancelOverlay = nullptr;

    currentGame->cancelGame();

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
    // ANSWER BOX
    // =================================================

    lv_obj_t* answerBox =
        lv_obj_create(parent);

    lv_obj_set_width(
        answerBox,
        LV_PCT(70)
    );

    lv_obj_set_height(
        answerBox,
        45
    );

    Theme::applyAnswerBox(
        answerBox
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

    lv_obj_center(answerLabel);


    // =================================================
    // KEYPAD
    // =================================================

    lv_obj_t* keypad =
        lv_obj_create(parent);

    lv_obj_set_width(
        keypad,
        LV_PCT(95)
    );

    lv_obj_set_height(
        keypad,
        175
    );

    Theme::applyPanel(
        keypad
    );

    lv_obj_set_style_pad_all(
        keypad,
        Theme::SPACING_SM,
        0
    );

    lv_obj_set_style_pad_row(
        keypad,
        Theme::SPACING_SM,
        0
    );

    lv_obj_set_style_pad_column(
        keypad,
        Theme::SPACING_SM,
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
        "C", "0", "<"
    };


    for (int i = 0; i < 12; i++) {

        int row =
            i / 3;

        int column =
            i % 3;


        lv_obj_t* button =
            lv_button_create(keypad);


        lv_obj_set_grid_cell(
            button,

            LV_GRID_ALIGN_STRETCH,
            column,
            1,

            LV_GRID_ALIGN_STRETCH,
            row,
            1
        );


        Theme::applyKeypadButton(
            button
        );


        lv_obj_t* label =
            lv_label_create(button);

        lv_label_set_text(
            label,
            numbers[i]
        );

        Theme::applyButtonText(
            label
        );

        lv_obj_center(label);


        if (i == 9) {

            lv_obj_add_event_cb(
                button,
                clearButtonEvent,
                LV_EVENT_CLICKED,
                nullptr
            );
        }

        else if (i == 11) {

            lv_obj_add_event_cb(
                button,
                backspaceButtonEvent,
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
        lv_button_create(parent);

    lv_obj_set_width(
        submit,
        LV_PCT(55)
    );

    lv_obj_set_height(
        submit,
        40
    );

    Theme::applyPrimaryButton(
        submit
    );

    lv_obj_t* submitLabel =
        lv_label_create(submit);

    lv_label_set_text(
        submitLabel,
        "SUBMIT"
    );

    Theme::applyButtonText(
        submitLabel
    );

    lv_obj_center(submitLabel);

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
    answerLabel = nullptr;


    // =================================================
    // OPTIONS CONTAINER
    // =================================================

    lv_obj_t* optionsContainer =
        lv_obj_create(parent);


    lv_obj_set_width(
        optionsContainer,
        LV_PCT(95)
    );


    // Fixed viewport height.
    // If the answers need more space,
    // this container becomes vertically scrollable.
    lv_obj_set_height(
        optionsContainer,
        LV_SIZE_CONTENT
    );


    Theme::applyPanel(
        optionsContainer
    );


    lv_obj_set_style_pad_all(
        optionsContainer,
        Theme::SPACING_SM,
        0
    );


    lv_obj_set_style_pad_row(
        optionsContainer,
        Theme::SPACING_SM,
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
    // ENABLE VERTICAL SCROLL
    // =================================================

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

    for (int i = 0; i < 4; i++) {

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


        // IMPORTANT:
        //
        // Do NOT use a fixed 42px height.
        // The button now grows depending
        // on how many lines the answer needs.
        lv_obj_set_height(
            button,
            LV_SIZE_CONTENT
        );


        // Give wrapped text enough vertical space.
        lv_obj_set_style_pad_top(
            button,
            10,
            0
        );

        lv_obj_set_style_pad_bottom(
            button,
            10,
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


        Theme::applySecondaryButton(
            button
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


        // Almost entire button width.
        // This gives LVGL a known width at which
        // it can wrap the text.
        lv_obj_set_width(
            label,
            LV_PCT(95)
        );


        // Long answers continue on another line.
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


        // Center the entire wrapped label
        // vertically/horizontally inside the button.
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
                static_cast<intptr_t>(i)
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
    currentGame = &game;

    answerLabel = nullptr;
    popupOverlay = nullptr;
    popupTimer = nullptr;
    cancelOverlay = nullptr;


    lv_obj_t* screen =
        lv_screen_active();

    lv_obj_clean(screen);

    Theme::applyScreen(screen);


    Quiz quiz =
        game.getCurrentQuiz();


    // =================================================
    // MAIN CONTENT
    // =====================================================

    lv_obj_t* content =
        lv_obj_create(screen);

    lv_obj_set_size(
        content,
        LV_PCT(100),
        LV_PCT(100)
    );

    lv_obj_set_style_bg_opa(
        content,
        LV_OPA_TRANSP,
        0
    );

    lv_obj_set_style_border_width(
        content,
        0,
        0
    );

    lv_obj_set_style_pad_all(
        content,
        Theme::SPACING_SM,
        0
    );

    lv_obj_set_style_pad_row(
        content,
        Theme::SPACING_SM,
        0
    );

    lv_obj_set_flex_flow(
        content,
        LV_FLEX_FLOW_COLUMN
    );

    lv_obj_set_flex_align(
        content,

        LV_FLEX_ALIGN_START,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER
    );


    // =================================================
    // HEADER
    // =====================================================

    lv_obj_t* header =
        lv_obj_create(content);

    lv_obj_set_width(
        header,
        LV_PCT(100)
    );

    lv_obj_set_height(
        header,
        48
    );

    lv_obj_set_style_bg_opa(
        header,
        LV_OPA_TRANSP,
        0
    );

    lv_obj_set_style_border_width(
        header,
        0,
        0
    );

    lv_obj_set_style_pad_all(
        header,
        0,
        0
    );

    lv_obj_set_flex_flow(
        header,
        LV_FLEX_FLOW_ROW
    );

    lv_obj_set_flex_align(
        header,

        LV_FLEX_ALIGN_SPACE_BETWEEN,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER
    );


    // =================================================
    // TITLE
    // =====================================================

    lv_obj_t* title =
        lv_label_create(header);

    lv_label_set_text(
        title,
        "QUIZ TIME!"
    );

    Theme::applyTitle(
        title
    );


    // =================================================
    // CANCEL IN HEADER
    // =====================================================

    lv_obj_t* cancel =
        lv_button_create(header);

    lv_obj_set_size(
        cancel,
        85,
        34
    );

    Theme::applySecondaryButton(
        cancel
    );

    lv_obj_t* cancelLabel =
        lv_label_create(cancel);

    lv_label_set_text(
        cancelLabel,
        "LEAVE"
    );

    Theme::applyButtonText(
        cancelLabel
    );

    lv_obj_center(cancelLabel);

    lv_obj_add_event_cb(
        cancel,
        cancelButtonEvent,
        LV_EVENT_CLICKED,
        nullptr
    );


    // =================================================
    // QUESTION AREA
    // =====================================================

    lv_obj_t* questionArea =
        lv_obj_create(content);

    lv_obj_set_width(
        questionArea,
        LV_PCT(95)
    );

    lv_obj_set_height(
        questionArea,
        70
    );

    lv_obj_set_style_bg_opa(
        questionArea,
        LV_OPA_TRANSP,
        0
    );

    lv_obj_set_style_border_width(
        questionArea,
        0,
        0
    );

    lv_obj_set_style_pad_all(
        questionArea,
        Theme::SPACING_XS,
        0
    );

    lv_obj_add_flag(
        questionArea,
        LV_OBJ_FLAG_SCROLLABLE
    );

    lv_obj_set_scroll_dir(
        questionArea,
        LV_DIR_VER
    );

    lv_obj_set_scrollbar_mode(
        questionArea,
        LV_SCROLLBAR_MODE_AUTO
    );


    // =================================================
    // QUESTION TEXT
    // =====================================================

    lv_obj_t* question =
        lv_label_create(
            questionArea
        );

    lv_label_set_text(
        question,
        quiz.question.c_str()
    );

    lv_obj_set_width(
        question,
        LV_PCT(100)
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
    // INPUT TYPE
    // =====================================================

    if (
        quiz.type
        == QuizType::NUMERIC
    ) {

        createNumericInput(
            content
        );
    }

    else if (
        quiz.type
        == QuizType::MULTIPLE_CHOICE
    ) {

        createMultipleChoiceInput(
            content,
            quiz
        );
    }
}
