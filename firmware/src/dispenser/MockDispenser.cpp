#include "MockDispenser.h"

#include <iostream>

bool MockDispenser::dispense(int slot) {
    std::cout
        << "[MOCK] Dispensing candy #"
        << slot
        << std::endl;

    return true;
}