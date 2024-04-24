#pragma once

#include <Arduino.h>
#include "ModuleServer.h"

namespace ModulePowerMeterLinky
{
    void boot();
    void gauge(unsigned long msLoop);

    // web handlers
    void httpAjaxRMS(AsyncWebServerRequest* request, String& S);
} // namespace ModulePowerMeterLinky
