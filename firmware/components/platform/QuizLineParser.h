#pragma once

#include "quiz/Quiz.h"

#include <string>

class QuizLineParser {
public:
    static Quiz parse(
        const std::string& line
    );
};