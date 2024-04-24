#pragma once

#include <Arduino.h>
#include "ModuleServer.h"

// Nb jour historique stocké (x 4 bytes)
// should not be less than 370 in case of migration from v8.06_rms
#define RMS_HISTORY_RANGE 370

#define RMS_HISTORY_5MIN_SIZE 600
#define RMS_HISTORY_2SEC_SIZE 300

namespace ModuleHistory
{
    void boot();
    void loopTimer(unsigned long msNow);
    void loop(unsigned long msLoop);
    void dayIsGone();

    // web handlers
    void httpAjaxHisto48h(AsyncWebServerRequest* request, String &S);
    void httpAjaxHisto10mn(AsyncWebServerRequest* request, String &S);
    void httpAjaxHistoriqueEnergie1An(AsyncWebServerRequest* request, String &S);
} // namespace ModuleHistory