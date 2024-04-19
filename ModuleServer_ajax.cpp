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

    void handleAjaxRMS()
    {
        String S = "";
        ModulePowerMeter::httpAjaxRMS(server, S);
        server.send(200, "text/html", S);
    }

    void handleAjaxHisto48h()
    {
        String S = "";
        ModuleStockage::httpAjaxHisto48h(server, S);
        server.send(200, "text/html", S);
    }

    void handleAjaxData10mn()
    {
        String S = "";
        ModuleStockage::httpAjaxHisto10mn(server, S);
        server.send(200, "text/html", S);
    }

    void handleAjaxHisto1an()
    {
        // Envoi Historique Energie quotiiienne sur 1 an 370 points
        String S = "";
        ModuleStockage::httpAjaxHistoriqueEnergie1An(server, S);
        server.send(200, "text/html", S);
    }

    void handleAjaxESP32()
    {
        // Envoi des dernières infos sur l'ESP32
        String S = "";
        ModuleCore::httpAjaxESP32(server, S);
        server.send(200, "text/html", S);
    }

    void handleAjaxData()
    {
        String S;
        ModuleCore::httpAjaxData(server, S);
        server.send(200, "text/html", S);
    }

    void handleAjax_etatActions()
    {
        String S = "";
        ModuleTriggers::httpAjaxTriggersStates(server, S);
        server.send(200, "text/html", S);
    }

    void handleActionsAjax()
    {
        String S = "";
        ModuleTriggers::httpAjaxTriggers(server, S);
        server.send(200, "text/html", S);
    }

    void handleParaAjax()
    {
        String S = "";
        ModuleStockage::httpAjaxPara(server, S);
        server.send(200, "text/html", S);
    }

    void handleParaRouteurAjax()
    {
        String S = "";
        ModuleCore::httpAjaxPara(server, S);
        server.send(200, "text/html", S);
    }

    void handleAP_ScanWifi()
    {
        String S = "";
        ModuleWifi::httpAjaxScanWifi(server, S);
        server.send(200, "text/html", S);
    }
} // namespace ModuleServer