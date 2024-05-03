// ***************
// *  WEB SERVER *
// ***************

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "ModulePowerMeter.h"
#include "ModuleHardware.h"
#include "ModuleEeprom.h"
#include "ModuleCore.h"
#include "ModuleTriggers.h"
#include "ModuleWifi.h"
#include "rms.h"
#include "version.h"

namespace ModuleServer
{
    void handleRestart(AsyncWebServerRequest *request)
    {
        // Eventuellement Reseter l'ESP32 à distance
        request->send(200, "text/plain", "OK Reset. Attendez.");
        ModuleCore::panic("Reboot from Web", 1000);
    }

    void handleActionsUpdate(AsyncWebServerRequest *request)
    {
        String S = "";
        ModuleTriggers::httpUpdateTriggers(request, S);
        request->send(200, "text/plain", S);
    }

    void handleParaUpdate(AsyncWebServerRequest *request)
    {
        String S = "";
        ModuleEeprom::httpUpdatePara(request, S);
        request->send(200, "text/plain", S);
    }

    void handleSetGpio(AsyncWebServerRequest *request)
    {
        int gpio = request->arg("gpio").toInt();
        int out = request->arg("out").toInt();
        String S = "Refut : gpio =" + String(gpio) + " out =" + String(out);
        if (gpio >= 0 && gpio <= 33 && out >= 0 && out <= 1)
        {
            pinMode(gpio, OUTPUT);
            digitalWrite(gpio, out);
            S = "OK : gpio =" + String(gpio) + " out =" + String(out);
        }
        request->send(200, "text/html", S);
    }

    void handleAP_SetWifi(AsyncWebServerRequest *request)
    {
        String S = "";
        ModuleWifi::httpUpdateWifi(request, S);
        request->send(200, "text/html", S);
        ModuleCore::panic("Reboot after WiFi setup", 1000);
    }

} // namespace ModuleServer