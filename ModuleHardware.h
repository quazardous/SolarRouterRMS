#pragma once

#include <Arduino.h>

namespace ModuleHardware
{
    void setup();
    void loopTimer(unsigned long mtsNow);
    void loop(unsigned long msLoop);

    // helpers
    void Gestion_LEDs();
} // namespace ModuleHardware