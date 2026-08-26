#pragma once

#include "Quiz.h"

#include <cstddef>


class IQuizSource {
public:
    virtual ~IQuizSource() = default;

    virtual std::size_t size() const = 0;

    virtual Quiz getQuiz(
        std::size_t index
    ) = 0;
};