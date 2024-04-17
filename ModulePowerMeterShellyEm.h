#pragma once

#include <Arduino.h>

namespace ModulePowerMeterShellyEm
{
    // events
    void setup();
    void gauge(unsigned long msLoop);

    // getters / setters
    void setChannel(unsigned short channel);
    unsigned short getChannel();
} // namespace ModulePowerMeterShellyEm