#pragma once

#include "Quiz.h"
#include <string>

class AnswerValidator {
public:
    bool validate(const Quiz& quiz, std::string userAnswer);
};