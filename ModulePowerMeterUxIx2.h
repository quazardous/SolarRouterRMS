// *******************************
// * Source de Mesures UI Double *
// *      Capteur JSY-MK-194     *
// *******************************
#pragma once

#include <Arduino.h>

namespace ModulePowerMeterUxIx2
{
    void setup();
    void gauge(unsigned long msLoop);

    // web handlers
    void httpAjaxRMS(WebServer& server, String& S);
} // namespace ModulePowerMeterUxIx2