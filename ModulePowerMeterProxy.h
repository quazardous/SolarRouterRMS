#pragma once

#include <Arduino.h>
#include <WebServer.h>

namespace ModulePowerMeterProxy
{
    // events
    void boot();
    void gauge(unsigned long msLoop);

    // getters
    const ModulePowerMeter::source_t getProxySource();

    // web handlers
    void httpAjaxRMS(WebServer& server, String& S);
} // namespace ModulePowerMeterProxy