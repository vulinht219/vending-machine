#pragma once

#include <string>
#include <vector>

enum class QuizType {
    MULTIPLE_CHOICE,
    NUMERIC
};

struct Quiz {
    int id;

    std::string category;

    QuizType type;

    std::string question;

    std::vector<std::string> options;

    std::string answer;

    int difficulty;
};