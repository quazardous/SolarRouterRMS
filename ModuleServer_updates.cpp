// ***************
// *  WEB SERVER *
// ***************

#include <WebServer.h>
#include "ModulePowerMeter.h"
#include "ModuleHardware.h"
#include "ModuleStockage.h"
#include "ModuleCore.h"
#include "ModuleTriggers.h"
#include "ModuleWifi.h"
#include "rms.h"
#include "version.h"

namespace ModuleServer
{
    extern WebServer server;
    String RS = RMS_RS;
    String GS = RMS_GS;

    void handleRestart()
    {
        // Eventuellement Reseter l'ESP32 à distance
        server.send(200, "text/plain", "OK Reset. Attendez.");
        ModuleCore::reboot("Reboot from Web", 1000);
    }

    void handleActionsUpdate()
    {
        String S = "";
        ModuleTriggers::httpUpdateTriggers(server, S);
        server.send(200, "text/plain", S);
    }

    void handleParaUpdate()
    {
        String S = "";
        ModuleStockage::httpUpdatePara(server, S);
        server.send(200, "text/plain", S);
    }

    void handleSetGpio()
    {
        int gpio = server.arg("gpio").toInt();
        int out = server.arg("out").toInt();
        String S = "Refut : gpio =" + String(gpio) + " out =" + String(out);
        if (gpio >= 0 && gpio <= 33 && out >= 0 && out <= 1)
        {
            pinMode(gpio, OUTPUT);
            digitalWrite(gpio, out);
            S = "OK : gpio =" + String(gpio) + " out =" + String(out);
        }
        server.send(200, "text/html", S);
    }

    void handleAP_SetWifi()
    {
        String S = "";
        ModuleWifi::httpUpdateWifi(server, S);
        server.send(200, "text/html", S);
        ModuleCore::reboot("Reboot after WiFi setup", 1000);
    }

} // namespace ModuleServer