#include "quiz/FileQuizSource.h"
#include "quiz/Quiz.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>


namespace {

constexpr std::size_t EXPECTED_QUIZ_COUNT = 20000;


bool isValidMultipleChoiceAnswer(
    const Quiz& quiz
)
{
    if (quiz.options.empty()) {
        return false;
    }

    if (quiz.answer.empty()) {
        return false;
    }

    try {

        std::size_t consumed = 0;

        int answerIndex =
            std::stoi(
                quiz.answer,
                &consumed
            );


        if (
            consumed
            != quiz.answer.size()
        ) {
            return false;
        }


        if (
            answerIndex < 0
        ) {
            return false;
        }


        if (
            static_cast<std::size_t>(
                answerIndex
            )
            >= quiz.options.size()
        ) {
            return false;
        }


        return true;
    }
    catch (...) {
        return false;
    }
}


bool isValidNumericAnswer(
    const Quiz& quiz
)
{
    if (quiz.answer.empty()) {
        return false;
    }


    try {

        std::size_t consumed = 0;


        std::stod(
            quiz.answer,
            &consumed
        );


        return (
            consumed
            == quiz.answer.size()
        );
    }
    catch (...) {
        return false;
    }
}


void printResult(
    const std::string& name,
    bool passed
)
{
    std::cout
        << name
        << ": "
        << (
            passed
            ? "PASS"
            : "FAIL"
        )
        << '\n';
}

}


int main(
    int argc,
    char* argv[]
)
{
    std::cout
        << "\n"
        << "========================================\n"
        << " PRODUCTION DATASET INTEGRATION TEST\n"
        << "========================================\n\n";


    if (
        argc < 2
    ) {

        std::cerr
            << "Usage:\n"
            << "  quiz-dataset-test.exe "
            << "<path-to-quizzes_20000.jsonl>\n";

        return EXIT_FAILURE;
    }


    const std::string datasetPath =
        argv[1];


    bool fileLoaded =
        false;

    bool correctCount =
        true;

    bool idsContinuous =
        true;

    bool quizTypesValid =
        true;

    bool questionsValid =
        true;

    bool multipleChoiceValid =
        true;

    bool numericValid =
        true;

    bool allReadable =
        true;


    std::size_t checked =
        0;

    std::size_t multipleChoiceCount =
        0;

    std::size_t numericCount =
        0;


    try {

        // =================================================
        // LOAD THROUGH THE REAL SIMULATOR DATASET SOURCE
        // =================================================

        FileQuizSource source(
            datasetPath
        );


        fileLoaded =
            true;


        const std::size_t total =
            source.size();


        correctCount =
            (
                total
                == EXPECTED_QUIZ_COUNT
            );


        std::cout
            << "Dataset: "
            << datasetPath
            << '\n';


        std::cout
            << "Quiz count: "
            << total
            << "\n\n";


        // =================================================
        // READ EVERY SINGLE QUIZ THROUGH IQuizSource API
        // =================================================

        for (
            std::size_t i = 0;
            i < total;
            ++i
        ) {

            Quiz quiz =
                source.getQuiz(
                    i
                );


            ++checked;


            // =============================================
            // ID CHECK
            // index 0 -> ID 1
            // index 19999 -> ID 20000
            // =============================================

            const int expectedId =
                static_cast<int>(
                    i + 1
                );


            if (
                quiz.id
                != expectedId
            ) {

                idsContinuous =
                    false;


                std::cerr
                    << "[ID ERROR] index="
                    << i
                    << " expected="
                    << expectedId
                    << " actual="
                    << quiz.id
                    << '\n';
            }


            // =============================================
            // QUESTION CHECK
            // =============================================

            if (
                quiz.question.empty()
            ) {

                questionsValid =
                    false;


                std::cerr
                    << "[QUESTION ERROR] ID "
                    << quiz.id
                    << " has empty question.\n";
            }


            // =============================================
            // TYPE-SPECIFIC VALIDATION
            // =============================================

            switch (
                quiz.type
            ) {

                case QuizType::MULTIPLE_CHOICE:
                {

                    ++multipleChoiceCount;


                    if (
                        !isValidMultipleChoiceAnswer(
                            quiz
                        )
                    ) {

                        multipleChoiceValid =
                            false;


                        std::cerr
                            << "[MCQ ERROR] ID "
                            << quiz.id
                            << '\n';
                    }


                    break;
                }


                case QuizType::NUMERIC:
                {

                    ++numericCount;


                    if (
                        !isValidNumericAnswer(
                            quiz
                        )
                    ) {

                        numericValid =
                            false;


                        std::cerr
                            << "[NUMERIC ERROR] ID "
                            << quiz.id
                            << " answer=\""
                            << quiz.answer
                            << "\"\n";
                    }


                    break;
                }


                default:
                {

                    quizTypesValid =
                        false;


                    std::cerr
                        << "[TYPE ERROR] ID "
                        << quiz.id
                        << '\n';


                    break;
                }
            }
        }


        // =================================================
        // OUT-OF-RANGE BEHAVIOR
        // =================================================

        bool outOfRangeThrows =
            false;


        try {

            source.getQuiz(
                source.size()
            );

        }
        catch (
            const std::runtime_error&
        ) {

            outOfRangeThrows =
                true;

        }


        std::cout
            << "\n"
            << "----------------------------------------\n"
            << " RESULTS\n"
            << "----------------------------------------\n";


        printResult(
            "File / JSON / validator",
            fileLoaded
        );


        printResult(
            "Exactly 20000 quizzes",
            correctCount
        );


        printResult(
            "IDs 1..20000",
            idsContinuous
        );


        printResult(
            "Questions non-empty",
            questionsValid
        );


        printResult(
            "Quiz types",
            quizTypesValid
        );


        printResult(
            "MCQ answers",
            multipleChoiceValid
        );


        printResult(
            "Numeric answers",
            numericValid
        );


        printResult(
            "Sequential read",
            checked == total
        );


        printResult(
            "Out-of-range protection",
            outOfRangeThrows
        );


        std::cout
            << "\nMCQ count: "
            << multipleChoiceCount
            << '\n';


        std::cout
            << "Numeric count: "
            << numericCount
            << '\n';


        std::cout
            << "Checked: "
            << checked
            << " / "
            << total
            << '\n';


        // =================================================
        // FINAL RESULT
        // =================================================

        const bool passed =

            fileLoaded
            && correctCount
            && idsContinuous
            && questionsValid
            && quizTypesValid
            && multipleChoiceValid
            && numericValid
            && allReadable
            && (
                checked
                == total
            )
            && outOfRangeThrows;


        std::cout
            << "\n========================================\n"
            << " RESULT: "
            << (
                passed
                ? "PASS"
                : "FAIL"
            )
            << "\n"
            << "========================================\n";


        return (
            passed
            ? EXIT_SUCCESS
            : EXIT_FAILURE
        );
    }


    catch (
        const std::exception& exception
    ) {

        std::cerr
            << "\nDATASET LOAD FAILED:\n"
            << exception.what()
            << '\n';


        return EXIT_FAILURE;
    }
}