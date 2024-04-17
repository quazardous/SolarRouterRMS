#pragma once

#include <Arduino.h>

namespace ModuleRomMap
{
    int readParameters(int address);
    int writeParameters(int address, bool commit = true);
} // namespace ModuleRomMap