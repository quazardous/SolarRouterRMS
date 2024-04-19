#pragma once

#include <Arduino.h>
#include <WebServer.h>

namespace ModulePowerMeterUxI
{
    // events
    void setup();
    void gauge(unsigned long msLoop);

    // web handlers
    void httpAjaxRMS(WebServer& server, String& S);
}