#pragma once

#include "quiz/IQuizSource.h"

#include <string>

class SDCardQuizSource
    : public IQuizSource
{
public:
    explicit SDCardQuizSource(
        const std::string& filePath
    );

    std::vector<Quiz> loadAll() override;

private:
    std::string filePath;
};