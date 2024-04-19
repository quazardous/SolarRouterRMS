#pragma once

#include <Arduino.h>
#include "Actions.h"
#include <WebServer.h>

//Nombre Actions Max
#define RMS_TRIGGERS_MAX 20

namespace ModuleTriggers
{
    // only for stats
    struct it_counters_infos_s {
        int it_10ms = 0; // Interruption avant deglitch
        int it_10ms_in = 0; // Interruption apres deglitch
        int it_mode = 0; // IT externe Triac ou interne [-5,+5]
        bool triac = 0; // Triac connected
        bool synchronized = 0; // ESP synchronized with current
    };

    void boot();
    void loopTimer(unsigned long mtsNow);
    void loop(unsigned long msLoop);
    void startIntTimers();

    // helpers
    void resetGpioActions();
    byte incrTriggersCount();
    void checkItStatus();

    // setters / getters
    Action *getTriggers();
    Action *getTrigger(byte i);
    void setTriggersCount(byte nbActions);
    byte getTriggersCount();
    int getDelay(byte i);
    it_counters_infos_s *getItCountersInfos(bool $check = false);

    // handlers
    void httpAjaxTriggersStates(WebServer& server, String& S);
    void httpAjaxTriggers(WebServer& server, String& S);
    void httpUpdateTriggers(WebServer& server, String& S);
} // namespace ModuleTriggers