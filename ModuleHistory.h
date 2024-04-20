#pragma once

#include <Arduino.h>
#include <WebServer.h>

#define RMS_HISTORY_RANGE 370 // Nb jour historique stocké (x 4 bytes)

#define RMS_HISTORY_5MIN_SIZE 600
#define RMS_HISTORY_2SEC_SIZE 300

namespace ModuleHistory
{
    void boot();
    void loopTimer(unsigned long msNow);
    void loop(unsigned long msLoop);
    void dayIsGone();

    // web handlers
    void httpAjaxHisto48h(WebServer &server, String &S);
    void httpAjaxHisto10mn(WebServer &server, String &S);
    void httpAjaxHistoriqueEnergie1An(WebServer &server, String &S);
} // namespace ModuleHistory