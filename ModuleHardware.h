#pragma once

#include <Arduino.h>

namespace ModuleHardware
{
    void boot();
    void loopTimer(unsigned long mtsNow);
    void loop(unsigned long msLoop);

    // getters / setters
    void setConnectivityLedCounter(int counter);
    int getConnectivityLedCounter();
    void setActivityLedCounter(int counter);
    int getActivityLedCounter();

    // helpers
    void Gestion_LEDs();
    void resetConnectivityLed();
} // namespace ModuleHardware