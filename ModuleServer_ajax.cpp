// ***************
// *  WEB SERVER *
// ***************

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "ModulePowerMeter.h"
#include "ModuleHardware.h"
#include "ModuleHistory.h"
#include "ModuleEeprom.h"
#include "ModuleCore.h"
#include "ModuleTriggers.h"
#include "ModuleWifi.h"
#include "rms.h"
#include "version.h"

namespace ModuleServer
{
    void handleAjaxRMS(AsyncWebServerRequest *request)
    {
        String S = "";
        ModulePowerMeter::httpAjaxRMS(request, S);
        request->send(200, MIME_HTML, S);
    }

    void handleAjaxHisto48h(AsyncWebServerRequest *request)
    {
        String S = "";
        ModuleHistory::httpAjaxHisto48h(request, S);
        request->send(200, MIME_HTML, S);
    }

    void handleAjaxData10mn(AsyncWebServerRequest *request)
    {
        String S = "";
        ModuleHistory::httpAjaxHisto10mn(request, S);
        request->send(200, MIME_HTML, S);
    }

    void handleAjaxHisto1an(AsyncWebServerRequest *request)
    {
        // Envoi Historique Energie quotiiienne sur 1 an 370 points
        String S = "";
        ModuleHistory::httpAjaxHistoriqueEnergie1An(request, S);
        request->send(200, MIME_HTML, S);
    }

    void handleAjaxESP32(AsyncWebServerRequest *request)
    {
        // Envoi des dernières infos sur l'ESP32
        String S = "";
        ModuleCore::httpAjaxESP32(request, S);
        request->send(200, MIME_HTML, S);
    }

    void handleAjaxData(AsyncWebServerRequest *request)
    {
        String S;
        ModuleCore::httpAjaxData(request, S);
        request->send(200, MIME_HTML, S);
    }

    void handleAjax_etatActions(AsyncWebServerRequest *request)
    {
        String S = "";
        ModuleTriggers::httpAjaxTriggersStates(request, S);
        request->send(200, MIME_HTML, S);
    }

    void handleActionsAjax(AsyncWebServerRequest *request)
    {
        String S = "";
        ModuleTriggers::httpAjaxTriggers(request, S);
        request->send(200, MIME_HTML, S);
    }

    void handleParaAjax(AsyncWebServerRequest *request)
    {
        String S = "";
        ModuleEeprom::httpAjaxPara(request, S);
        request->send(200, MIME_HTML, S);
    }

    void handleParaRouteurAjax(AsyncWebServerRequest *request)
    {
        String S = "";
        ModuleCore::httpAjaxPara(request, S);
        request->send(200, MIME_HTML, S);
    }

    void handleAP_ScanWifi(AsyncWebServerRequest *request)
    {
        String S = "";
        ModuleWifi::httpAjaxScanWifi(request, S);
        request->send(200, MIME_HTML, S);
    }
} // namespace ModuleServer