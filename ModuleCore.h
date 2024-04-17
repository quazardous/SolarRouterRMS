#pragma once

#include <Arduino.h>
#include "helpers.h"

namespace ModuleCore
{
    void setup();
    void loopTimer(unsigned long mtsNow);
    void loop(unsigned long msLoop);

    // getters / setters
    const char *getHostname();
    void setRouterName(const char *n);
    const char *getRouterName();
    void setFixProbeName(const char *n);
    const char *getFixProbeName();
    void setMobileProbeName(const char *n);
    const char *getMobileProbeName();
    void setTemperatureName(const char *n);
    const char *getTemperatureName();

    // states
    const cpu_load_t *getCpuLoad1();
} // namespace ModuleCore