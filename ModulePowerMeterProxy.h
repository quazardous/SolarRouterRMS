#pragma once

#include <Arduino.h>

namespace ModulePowerMeterProxy
{
    // events
    void setup();
    void gauge(unsigned long msLoop);

    // getters
    const ModulePowerMeter::source_t getProxySource();
} // namespace ModulePowerMeterProxy