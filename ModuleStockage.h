#pragma once

#include <Arduino.h>

namespace ModuleStockage
{
    void setup();
    void loopTimer(unsigned long mtsNow);
    void loop(unsigned long msLoop);
    // events
    void onNewDay();

    // setters / getters
    void setEepromKey(unsigned long key);
    unsigned long getEepromKey();

} // namespace ModuleStockage
