#pragma once

#include <Arduino.h>
#include <RemoteDebug.h>

// Debug via Wifi and logs
namespace ModuleDebug
{
    // events
    void setup();
    void loop(unsigned long msLoop);

    // getters / setters
    String* getMessages();
    RemoteDebug &getDebug();

    // helpers
    void stockMessage(const char *m);
    void stockMessage(const String &m);
    void comboLog(const char *m);
    void comboLog(const String &m);
} // namespace ModuleDebug
