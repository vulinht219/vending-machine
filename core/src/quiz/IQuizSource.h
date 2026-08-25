#pragma once

#include "Quiz.h"

#include <vector>


class IQuizSource {
public:
    virtual ~IQuizSource() = default;

    virtual std::vector<Quiz> loadAll() = 0;
};