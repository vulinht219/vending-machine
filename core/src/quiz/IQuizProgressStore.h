#pragma once

#include "QuizProgress.h"

class IQuizProgressStore {
public:
    virtual ~IQuizProgressStore() = default;

    virtual bool exists() const = 0;

    virtual QuizProgress load() const = 0;

    virtual void save(
        const QuizProgress& progress
    ) const = 0;
};