#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

namespace ModuleServer {
    void boot();
    // needed for OTA update reboot
    void loop(unsigned long msLoop);
} // namespace ModuleServer