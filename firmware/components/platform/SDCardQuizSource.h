#pragma once

#include "quiz/IQuizSource.h"

#include <cstddef>
#include <string>
#include <vector>


class SDCardQuizSource
    : public IQuizSource
{
public:

    explicit SDCardQuizSource(
        const std::string& filePath
    );


    std::size_t size() const override;


    Quiz getQuiz(
        std::size_t index
    ) override;


private:

    std::string filePath;

    // Byte offset of each JSONL line.
    std::vector<long> offsets;


    void buildIndex();


    Quiz parseLine(
        const std::string& line
    );
};