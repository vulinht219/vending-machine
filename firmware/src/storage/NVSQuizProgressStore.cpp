#include "NVSQuizProgressStore.h"

#include <stdexcept>


bool NVSQuizProgressStore::exists() const
{
    return false;
}


QuizProgress NVSQuizProgressStore::load() const
{
    throw std::runtime_error(
        "NVSQuizProgressStore is not implemented yet."
    );
}


void NVSQuizProgressStore::save(
    const QuizProgress& progress
) const
{
    // ESP32 NVS implementation comes later.
}