#pragma once

#include <Arduino.h>
#include "ModuleServer.h"

namespace ModulePowerMeterProxy
{
    // events
    void boot();
    void gauge(unsigned long msLoop);

    // getters
    const ModulePowerMeter::source_t getProxySource();

    // web handlers
    void httpAjaxRMS(AsyncWebServerRequest* request, String& S);
} // namespace ModulePowerMeterProxy