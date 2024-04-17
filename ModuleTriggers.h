#pragma once

#include <Arduino.h>
#include "Actions.h"

//Nombre Actions Max
#define RMS_TRIGGERS_MAX 20

namespace ModuleTriggers
{
    void setup();
    void loopTimer(unsigned long mtsNow);
    void loop(unsigned long msLoop);
    void startIntTimers();

    // helpers
    void resetGpioActions();
    byte incrTriggersCount();

    // setters / getters
    Action *getTriggers();
    Action *getTrigger(byte i);
    void setTriggersCount(byte nbActions);
    byte getTriggersCount();
    int getDelay(byte i);
} // namespace ModuleTriggers