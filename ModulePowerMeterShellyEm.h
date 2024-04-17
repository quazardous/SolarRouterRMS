#pragma once

#include <Arduino.h>

namespace ModulePowerMeterShellyEm
{
    // events
    void setup();
    void gauge(unsigned long msLoop);

    // getters / setters
    void setPhasesNumber(unsigned short channel);
    unsigned short getPhasesNumber();
} // namespace ModulePowerMeterShellyEm