#pragma once

#include <Arduino.h>

namespace ModuleTime
{
    // events
    void boot();
    void loopTimer(unsigned long mtsNow);
    void loop(unsigned long msLoop);
    // setters / getters
    const char *getJourCourant();
    void setDateCeJour(const char *date);
    const char *getDateCeJour();

    // states
    bool timeIsValid();

    // helpers
    time_t JourHeureChange();
} // namespace ModuleTime
