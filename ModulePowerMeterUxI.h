#pragma once

#include <Arduino.h>
#include "ModuleServer.h"

namespace ModulePowerMeterUxI
{
    // events
    void boot();
    void gauge(unsigned long msLoop);

    // web handlers
    void httpAjaxRMS(AsyncWebServerRequest* request, String& S);
}