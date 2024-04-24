#pragma once

#include <Arduino.h>
#include "ModuleServer.h"

namespace ModulePowerMeterShellyEm
{
    // events
    void boot();
    void gauge(unsigned long msLoop);

    // getters / setters
    void setPhasesNumber(unsigned short channel);
    unsigned short getPhasesNumber();

    // web handlers
    void httpAjaxRMS(AsyncWebServerRequest* request, String& S);
} // namespace ModulePowerMeterShellyEm