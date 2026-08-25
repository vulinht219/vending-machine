#include "QuizLoader.h"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::vector<Quiz> QuizLoader::loadFromJsonLines(
    const std::string& filePath
)
{
    std::ifstream file(filePath);

    if (!file.is_open()) {
        throw std::runtime_error(
            "Could not open quiz file: " + filePath
        );
    }

    std::vector<Quiz> quizzes;

    std::string line;

    while (std::getline(file, line)) {

        if (line.empty()) {
            continue;
        }

        json data = json::parse(line);

        Quiz quiz;

        quiz.id =
            data.at("id").get<int>();

        quiz.category =
            data.at("category").get<std::string>();

        std::string type =
            data.at("type").get<std::string>();

        if (type == "numeric") {
            quiz.type =
                QuizType::NUMERIC;
        }
        else if (
            type == "multiple_choice"
        ) {
            quiz.type =
                QuizType::MULTIPLE_CHOICE;
        }
        else {
            throw std::runtime_error(
                "Unknown quiz type: " + type
            );
        }

        quiz.question =
            data.at("question").get<std::string>();

        quiz.options =
            data.at("options")
                .get<std::vector<std::string>>();

        quiz.answer =
            data.at("answer").get<std::string>();

        quiz.difficulty =
            data.at("difficulty").get<int>();

        quizzes.push_back(
            quiz
        );
    }

    return quizzes;
}