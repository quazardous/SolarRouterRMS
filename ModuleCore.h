#pragma once

#include <Arduino.h>
#include "helpers.h"
#include "ModuleServer.h"

namespace ModuleCore
{
    void boot();
    void loopTimer(unsigned long mtsNow);
    void loop(unsigned long msLoop);

    // getters / setters
    const char *getVersion();
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
    // bool isUp();
    // // declare RMS UP and ready
    // void upAndReady(bool up = true);

    // helpers

    // instant reboot (panic)
    // should not be used (blocks the loop)
    void panic(String m = "", int delay = 0);
    void log(const char *m);
    void log(const String &m);
    void checkup();
    // reboot in loop
    // modules can call this function again to add more delay
    // (does not block the loop)
    void reboot(String m = "", int msDelay = 3000);

    // web handlers
    void httpAjaxESP32(AsyncWebServerRequest* request, String& S);
    void httpAjaxData(AsyncWebServerRequest* request, String& S);
    void httpAjaxPara(AsyncWebServerRequest* request, String& S);

    // API handlers
    void apiHello(AsyncWebServerRequest* request, JsonDocument& doc);
    void apiReboot(AsyncWebServerRequest* request, JsonDocument& doc);
} // namespace ModuleCore