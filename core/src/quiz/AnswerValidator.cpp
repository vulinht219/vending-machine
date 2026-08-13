#include "AnswerValidator.h"

bool AnswerValidator::validate(
    const Quiz& quiz,
    std::string userAnswer
) {
    return userAnswer == quiz.answer;
}