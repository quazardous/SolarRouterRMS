#pragma once

#include <Arduino.h>
#include <WebServer.h>

namespace ModulePowerMeterShellyEm
{
    // events
    void setup();
    void gauge(unsigned long msLoop);

    // getters / setters
    void setPhasesNumber(unsigned short channel);
    unsigned short getPhasesNumber();

    // web handlers
    void httpAjaxRMS(WebServer& server, String& S);
} // namespace ModulePowerMeterShellyEm