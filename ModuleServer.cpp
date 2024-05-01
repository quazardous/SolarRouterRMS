// ***************
// *  WEB SERVER *
// ***************

#include "ModuleServer.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include "ModuleDebug.h"
#include "pages.h"
#include "config.h"

#define RMS_WEB_SERVER_PORT 80

namespace ModuleServer
{
    // Simple Web Server on port 80
    AsyncWebServer server(RMS_WEB_SERVER_PORT);

    void bootApi(AsyncWebServer& server);

    // in file ModuleServer_pages.cpp
    void handleRoot(AsyncWebServerRequest *request);
    void handleMainJS(AsyncWebServerRequest *request);
    void handleBrute(AsyncWebServerRequest *request);
    void handleBruteJS(AsyncWebServerRequest *request);
    void handleActions(AsyncWebServerRequest *request);
    void handleActionsJS(AsyncWebServerRequest *request);
    void handlePara(AsyncWebServerRequest *request);
    void handleParaJS(AsyncWebServerRequest *request);
    void handleParaRouteurJS(AsyncWebServerRequest *request);
    void handleNotFound(AsyncWebServerRequest *request);

    // in file ModuleServer_ajax.cpp
    void handleParaAjax(AsyncWebServerRequest *request);
    void handleParaRouteurAjax(AsyncWebServerRequest *request);
    void handleActionsAjax(AsyncWebServerRequest *request);
    void handleAjaxHisto48h(AsyncWebServerRequest *request);
    void handleAjaxHisto1an(AsyncWebServerRequest *request);
    void handleAjaxRMS(AsyncWebServerRequest *request);
    void handleAjaxESP32(AsyncWebServerRequest *request);
    void handleAjaxData(AsyncWebServerRequest *request);
    void handleAjaxData10mn(AsyncWebServerRequest *request);
    void handleAjax_etatActions(AsyncWebServerRequest *request);
    void handleAP_ScanWifi(AsyncWebServerRequest *request);

    // in file ModuleServer_updates.cpp
    void handleParaUpdate(AsyncWebServerRequest *request);
    void handleActionsUpdate(AsyncWebServerRequest *request);
    void handleSetGpio(AsyncWebServerRequest *request);
    void handleRestart(AsyncWebServerRequest *request);
    void handleAP_SetWifi(AsyncWebServerRequest *request);

    const String MIME_JSON = "application/json";
    const String MIME_HTML = "text/html";
    const String MIME_PLAIN = "text/plain";
    const String MIME_CSS = "text/css";
    const String MIME_JS = "text/javascript";

    void boot()
    {
        #if RMS_WEB_SERVER_ALLOW_OFFSHORE == 1
        // Allow CORS for Offshore mode
        DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
        #endif

        // server.enableCORS(true);
        // Init Web Server on port 80
        bootApi(server);

        // Serving minified files on non minified URL
        // libraries
        server.on("/lib/simple.css", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send_P(200, MIME_CSS, pages[RMS_PAGE_SIMPLE_MIN_CSS]);
        });
        server.on("/lib/reef.js", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send_P(200, MIME_JS, pages[RMS_PAGE_REEF_MIN_JS]);
        });

        // core files
        server.on("/css/rms.css", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send_P(200, MIME_CSS, pages[RMS_PAGE_RMS_CSS]);
        });
        server.on("/js/rms.js", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send_P(200, MIME_JS, pages[RMS_PAGE_RMS_JS]);
        });
        server.on("/js/helpers.js", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send_P(200, MIME_JS, pages[RMS_PAGE_HELPERS_JS]);
        });
        server.on("/js/ui_helpers.js", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send_P(200, MIME_JS, pages[RMS_PAGE_UI_HELPERS_JS]);
        });
        server.on("/js/ui.js", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send_P(200, MIME_JS, pages[RMS_PAGE_UI_JS]);
        });
        server.on("/js/app.js", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send_P(200, MIME_JS, pages[RMS_PAGE_APP_JS]);
        });
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send_P(200, MIME_HTML, pages[RMS_PAGE_INDEX_HTML]);
        });

        // server.on("/", HTTP_GET, handleRoot);
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

        ElegantOTA.begin(&server); // Start ElegantOTA
        server.begin();
    }

    void loop(unsigned long msLoop)
    {
        // handles reboot after update
        ElegantOTA.loop();
    }
} // namespace ModuleServer