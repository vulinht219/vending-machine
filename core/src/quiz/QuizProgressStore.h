#pragma once

#include "QuizProgress.h"

#include <string>

class QuizProgressStore {
public:
    explicit QuizProgressStore(
        const std::string& filePath
    );

    bool exists() const;

    QuizProgress load() const;

    void save(
        const QuizProgress& progress
    ) const;

private:
    std::string filePath;
};