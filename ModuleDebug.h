#pragma once

#include <Arduino.h>
#include <RemoteDebug.h>

#define RMS_DEBUG_STOCK_MESSAGES 4

// Debug via Wifi and logs
namespace ModuleDebug
{
    // events
    void boot();
    void loop(unsigned long msLoop);

    // getters / setters
    String* getMessages();
    int getMessageIdx();
    RemoteDebug &getDebug();

    // helpers
    void stockMessage(const char *m);
    void stockMessage(const String &m);
    void comboLog(const char *m);
    void comboLog(const String &m);
} // namespace ModuleDebug
