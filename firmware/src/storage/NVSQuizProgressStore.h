#pragma once

#include "quiz/IQuizProgressStore.h"


class NVSQuizProgressStore
    : public IQuizProgressStore
{
public:
    bool exists() const override;

    QuizProgress load() const override;

    void save(
        const QuizProgress& progress
    ) const override;
};