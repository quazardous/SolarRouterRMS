#pragma once

#include <Arduino.h>

namespace ModuleRomMap
{
    int readEeprom(int address);
    int writeEeprom(int address);
} // namespace ModuleRomMap