#include "RealDispenser.h"

bool RealDispenser::dispense(int slot)
{
    return slot >= 1 && slot <= 6;
}