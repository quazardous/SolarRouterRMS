#pragma once

#include <Arduino.h>

namespace ModuleSensor
{
    // events
    void setup();
    void loopTimer(unsigned long mtsNow);
    void loop(unsigned long msLoop);

    // getters
    float getTemperature();
} // namespace ModuleSensor
