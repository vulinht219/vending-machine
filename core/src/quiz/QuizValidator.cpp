#include "QuizValidator.h"

#include <stdexcept>
#include <unordered_set>


bool QuizValidator::isAllowedCategory(
    const std::string& category
)
{
    static const std::unordered_set<std::string>
        allowedCategories = {

            "math",
            "physics",
            "animals",
            "science",
            "world_history",
            "dad_jokes"
        };

    return
        allowedCategories.find(category)
        != allowedCategories.end();
}


void QuizValidator::validateQuiz(
    const Quiz& quiz
)
{
    // =========================================
    // ID
    // =========================================

    if (quiz.id <= 0) {
        throw std::runtime_error(
            "Quiz ID must be greater than 0."
        );
    }


    // =========================================
    // CATEGORY
    // =========================================

    if (!isAllowedCategory(quiz.category)) {
        throw std::runtime_error(
            "Invalid category in quiz ID "
            + std::to_string(quiz.id)
            + ": "
            + quiz.category
        );
    }


    // =========================================
    // QUESTION
    // =========================================

    if (quiz.question.empty()) {
        throw std::runtime_error(
            "Question is empty in quiz ID "
            + std::to_string(quiz.id)
        );
    }


    // =========================================
    // DIFFICULTY
    // =========================================

    if (
        quiz.difficulty < 1 ||
        quiz.difficulty > 3
    ) {
        throw std::runtime_error(
            "Difficulty must be 1-3 in quiz ID "
            + std::to_string(quiz.id)
        );
    }


    // =========================================
    // MULTIPLE CHOICE
    // =========================================

    if (
        quiz.type
        == QuizType::MULTIPLE_CHOICE
    ) {
        if (quiz.options.size() != 4) {

            throw std::runtime_error(
                "Multiple choice quiz must have "
                "exactly 4 options. Quiz ID "
                + std::to_string(quiz.id)
            );
        }

        if (
            quiz.answer != "0" &&
            quiz.answer != "1" &&
            quiz.answer != "2" &&
            quiz.answer != "3"
        ) {
            throw std::runtime_error(
                "Multiple choice answer must be "
                "0, 1, 2 or 3. Quiz ID "
                + std::to_string(quiz.id)
            );
        }

        for (
            const std::string& option :
            quiz.options
        ) {
            if (option.empty()) {

                throw std::runtime_error(
                    "Empty answer option in quiz ID "
                    + std::to_string(quiz.id)
                );
            }
        }
    }


    // =========================================
    // NUMERIC
    // =========================================

    else if (
        quiz.type
        == QuizType::NUMERIC
    ) {
        if (!quiz.options.empty()) {

            throw std::runtime_error(
                "Numeric quiz must not contain "
                "ABCD options. Quiz ID "
                + std::to_string(quiz.id)
            );
        }

        if (quiz.answer.empty()) {

            throw std::runtime_error(
                "Numeric quiz has empty answer. "
                "Quiz ID "
                + std::to_string(quiz.id)
            );
        }

        // Only math is allowed to use numeric input
        if (quiz.category != "math") {

            throw std::runtime_error(
                "Numeric quiz must belong to "
                "math category. Quiz ID "
                + std::to_string(quiz.id)
            );
        }
    }
}


void QuizValidator::validateDataset(
    const std::vector<Quiz>& quizzes
)
{
    if (quizzes.empty()) {
        throw std::runtime_error(
            "Quiz dataset is empty."
        );
    }


    std::unordered_set<int> ids;


    for (const Quiz& quiz : quizzes) {

        validateQuiz(quiz);


        // =====================================
        // DUPLICATE ID
        // =====================================

        if (
            ids.find(quiz.id)
            != ids.end()
        ) {
            throw std::runtime_error(
                "Duplicate quiz ID: "
                + std::to_string(quiz.id)
            );
        }

        ids.insert(quiz.id);
    }
}