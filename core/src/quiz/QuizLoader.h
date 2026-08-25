#pragma once

#include "Quiz.h"

#include <string>
#include <vector>

class QuizLoader {
public:
    static std::vector<Quiz> loadFromJsonLines(
        const std::string& filePath
    );
};