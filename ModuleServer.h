#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

namespace ModuleServer {
    extern const String MIME_JSON;
    extern const String MIME_HTML;
    extern const String MIME_CSS;
    extern const String MIME_JS;

    void boot();
    // needed for OTA update reboot
    void loop(unsigned long msLoop);
} // namespace ModuleServer