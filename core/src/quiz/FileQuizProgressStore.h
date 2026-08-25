#pragma once

#include "IQuizProgressStore.h"

#include <string>

class FileQuizProgressStore
    : public IQuizProgressStore
{
public:
    explicit FileQuizProgressStore(
        const std::string& filePath
    );

    bool exists() const override;

    QuizProgress load() const override;

    void save(
        const QuizProgress& progress
    ) const override;

private:
    std::string filePath;
};