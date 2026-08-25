#pragma once

#include <cstddef>
#include <cstdint>

struct QuizProgress {
    std::uint32_t seed = 0;

    std::size_t currentPosition = 0;

    std::uint32_t round = 1;

    std::uint32_t datasetVersion = 1;
};