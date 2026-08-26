#include <iostream>
#include <stdexcept>

#include "../../firmware/components/platform/SDCardQuizSource.h"


int main()
{
    try {

        SDCardQuizSource source(
            "../dataset/production/quizzes_1000.jsonl"
        );


        std::size_t count =
            source.size();


        std::cout
            << "Quiz count: "
            << count
            << "\n\n";


        if (count != 1000) {

            throw std::runtime_error(
                "Expected exactly 1000 quizzes."
            );
        }


        // =============================================
        // TEST EVERY SINGLE QUIZ
        // =============================================

        for (
            std::size_t i = 0;
            i < count;
            ++i
        ) {

            Quiz quiz =
                source.getQuiz(i);


            // -----------------------------------------
            // ID
            // -----------------------------------------

            int expectedId =
                static_cast<int>(i + 1);


            if (
                quiz.id != expectedId
            ) {

                throw std::runtime_error(
                    "Unexpected quiz ID at index "
                    + std::to_string(i)
                    + ". Expected "
                    + std::to_string(expectedId)
                    + ", got "
                    + std::to_string(quiz.id)
                );
            }


            // -----------------------------------------
            // QUESTION
            // -----------------------------------------

            if (
                quiz.question.empty()
            ) {

                throw std::runtime_error(
                    "Empty question at quiz ID "
                    + std::to_string(quiz.id)
                );
            }


            // -----------------------------------------
            // ANSWER
            // -----------------------------------------

            if (
                quiz.answer.empty()
            ) {

                throw std::runtime_error(
                    "Empty answer at quiz ID "
                    + std::to_string(quiz.id)
                );
            }


            // -----------------------------------------
            // MULTIPLE CHOICE
            // -----------------------------------------

            if (
                quiz.type ==
                QuizType::MULTIPLE_CHOICE
            ) {

                if (
                    quiz.options.size() != 4
                ) {

                    throw std::runtime_error(
                        "Quiz ID "
                        + std::to_string(quiz.id)
                        + " does not have exactly 4 options."
                    );
                }


                if (
                    quiz.answer != "0" &&
                    quiz.answer != "1" &&
                    quiz.answer != "2" &&
                    quiz.answer != "3"
                ) {

                    throw std::runtime_error(
                        "Invalid multiple-choice answer at quiz ID "
                        + std::to_string(quiz.id)
                    );
                }
            }


            // -----------------------------------------
            // NUMERIC
            // -----------------------------------------

            if (
                quiz.type ==
                QuizType::NUMERIC
            ) {

                if (
                    !quiz.options.empty()
                ) {

                    throw std::runtime_error(
                        "Numeric quiz ID "
                        + std::to_string(quiz.id)
                        + " unexpectedly contains options."
                    );
                }
            }


            // Show progress every 100 questions.
            if (
                (i + 1) % 100 == 0
            ) {

                std::cout
                    << "Validated "
                    << (i + 1)
                    << " / "
                    << count
                    << "\n";
            }
        }


        std::cout
            << "\n==============================\n";

        std::cout
            << "ALL "
            << count
            << " QUIZZES PASSED\n";

        std::cout
            << "==============================\n";


        return 0;
    }

    catch (
        const std::exception& e
    ) {

        std::cerr
            << "\nTEST FAILED:\n"
            << e.what()
            << "\n";


        return 1;
    }
}