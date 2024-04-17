#pragma once

#include <Arduino.h>

namespace ModuleStockage
{
    String GS = String((char)29);  //Group Separator
    String RS = String((char)30);  //Record Separator

    void setup();
    void loopTimer(unsigned long mtsNow);
    void loop(unsigned long msLoop);
    // events
    void onNewDay();

    // setters / getters
    void setEepromKey(unsigned long key);
    unsigned long getEepromKey();

} // namespace ModuleStockage
