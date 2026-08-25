#include "NVSGameProgressStore.h"

#include <stdexcept>


bool NVSGameProgressStore::exists() const
{
    return false;
}


GameProgress NVSGameProgressStore::load() const
{
    throw std::runtime_error(
        "NVSGameProgressStore is not implemented yet."
    );
}


void NVSGameProgressStore::save(
    const GameProgress& progress
) const
{
    // ESP32 NVS implementation comes later.
}