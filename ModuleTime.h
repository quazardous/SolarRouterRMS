#pragma once

#include <Arduino.h>

namespace ModuleTime
{
    // events
    void setup();
    void loopTimer(unsigned long mtsNow);
    void loop(unsigned long msLoop);
    // getters
    // states
    bool timeIsValid();
} // namespace ModuleTime
