#pragma once

#include "IQuizSource.h"

#include <string>


class FileQuizSource : public IQuizSource {
public:
    explicit FileQuizSource(
        const std::string& filePath
    );

    std::vector<Quiz> loadAll() override;

private:
    std::string filePath;
};