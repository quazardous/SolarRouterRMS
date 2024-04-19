// ***************
// *  WEB SERVER *
// ***************

#include <WebServer.h>
#include "ModuleDebug.h"

#define RMS_WEB_SERVER_PORT 80

namespace ModuleServer
{
    // Simple Web Server on port 80
    WebServer server(RMS_WEB_SERVER_PORT);

    void setup()
    {
        // Init Web Server on port 80
        server.on("/", handleRoot);
        server.on("/MainJS", handleMainJS);
        server.on("/Para", handlePara);
        server.on("/ParaJS", handleParaJS);
        server.on("/ParaRouteurJS", handleParaRouteurJS);
        server.on("/ParaAjax", handleParaAjax);
        server.on("/ParaRouteurAjax", handleParaRouteurAjax);
        server.on("/ParaUpdate", handleParaUpdate);
        server.on("/Actions", handleActions);
        server.on("/ActionsJS", handleActionsJS);
        server.on("/ActionsUpdate", handleActionsUpdate);
        server.on("/ActionsAjax", handleActionsAjax);
        server.on("/Brute", handleBrute);
        server.on("/BruteJS", handleBruteJS);
        server.on("/ajax_histo48h", handleAjaxHisto48h);
        server.on("/ajax_histo1an", handleAjaxHisto1an);
        server.on("/ajax_dataRMS", handleAjaxRMS);
        server.on("/ajax_dataESP32", handleAjaxESP32);
        server.on("/ajax_data", handleAjaxData);
        server.on("/ajax_data10mn", handleAjaxData10mn);
        server.on("/ajax_etatActions", handleAjax_etatActions);
        server.on("/SetGPIO", handleSetGpio);
        server.on("/restart", handleRestart);
        server.on("/AP_ScanWifi", handleAP_ScanWifi);
        server.on("/AP_SetWifi", handleAP_SetWifi);
        server.onNotFound(handleNotFound);
        server.begin();
        ModuleDebug::getDebug().println("HTTP server started");
    }

    void loop(unsigned long msLoop)
    {
        server.handleClient();
    }

    // in file ModuleServer_pages.cpp
    void handleRoot();
    void handleMainJS();
    void handleBrute();
    void handleBruteJS();
    void handleActions();
    void handleActionsJS();
    void handlePara();
    void handleParaJS();
    void handleParaRouteurJS();
    void handleNotFound();

    // in file ModuleServer_ajax.cpp
    void handleParaAjax();
    void handleParaRouteurAjax();
    void handleActionsAjax();
    void handleAjaxHisto48h();
    void handleAjaxHisto1an();
    void handleAjaxRMS();
    void handleAjaxESP32();
    void handleAjaxData();
    void handleAjaxData10mn();
    void handleAjax_etatActions();
    void handleAP_ScanWifi();

    // in file ModuleServer_updates.cpp
    void handleParaUpdate();
    void handleActionsUpdate();
    void handleSetGpio();
    void handleRestart();
    void handleAP_SetWifi();

} // namespace ModuleServer