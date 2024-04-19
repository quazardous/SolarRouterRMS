#pragma once

#include <Arduino.h>
#include "helpers.h"
#include <WebServer.h>

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
    bool isUp();
    // declare RMS UP and ready
    void upAndReady(bool up = true);

    // helpers
    void reboot(String $m = "", int $delay = 0);
    void log(const char *m);
    void log(const String &m);

    // web handlers
    void httpAjaxESP32(WebServer& server, String& S);
    void httpAjaxData(WebServer& server, String& S);
    void httpAjaxPara(WebServer& server, String& S);
} // namespace ModuleCore