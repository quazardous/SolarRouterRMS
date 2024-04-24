#pragma once

#include <Arduino.h>
#include "ModuleServer.h"

namespace ModulePowerMeterSmartG
{
    void boot();
    void gauge(unsigned long msLoop);

    // web handlers
    void httpAjaxRMS(AsyncWebServerRequest* request, String& S);
} // namespace ModulePowerMeterSmartG