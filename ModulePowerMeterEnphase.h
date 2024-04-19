#pragma once

#include <Arduino.h>
#include <WebServer.h>

namespace ModulePowerMeterEnphase
{
    // events
    void setup();
    void loop(unsigned long msLoop); // other stuff to do in the loop
    void gauge(unsigned long msLoop);

    // getters / setters
    void setUser(const char *user);
    const char *getUser();
    void setPwd(const char *pwd);
    const char *getPwd();
    void setSerial(unsigned long serial);
    unsigned long getSerial();

    // web handlers
    void httpAjaxRMS(WebServer& server, String& S);
}