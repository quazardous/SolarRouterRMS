// ***************
// *  WEB SERVER *
// ***************

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "ModuleWifi.h"
#include "ModuleDebug.h"
#include "ModuleCore.h"
#include "pages.h"

namespace ModuleServer
{
    void handleRoot(AsyncWebServerRequest *request)
    { 
        // Pages principales
        // en AP et STA mode ou Station Mode seul

        // Reset du timeout pour rester en mode AP
        ModuleWifi::resetApTimout();
        request->send(200, "text/html", pages[ModuleWifi::isStationMode() ? RMS_PAGE_MAIN_HTML : RMS_PAGE_CONNECT_HTML]);
    }

    void handleMainJS(AsyncWebServerRequest *request)
    {                                                               // Code Javascript
        request->send(200, "text/html", pages[RMS_PAGE_MAIN_JS]); // Javascript code
    }

    void handleBrute(AsyncWebServerRequest *request)
    {
        // Page données brutes
        request->send(200, "text/html", pages[RMS_PAGE_BRUTE_HTML]);
    }

    void handleBruteJS(AsyncWebServerRequest *request)
    {                                                                // Code Javascript
        request->send(200, "text/html", pages[RMS_PAGE_BRUTE_JS]); // Javascript code
    }

    void handleActions(AsyncWebServerRequest *request)
    {
        request->send(200, "text/html", pages[RMS_PAGE_ACTIONS_HTML]);
    }

    void handleActionsJS(AsyncWebServerRequest *request)
    {
        request->send(200, "text/html", pages[RMS_PAGE_ACTIONS_JS]);
    }

    void handlePara(AsyncWebServerRequest *request)
    {
        request->send(200, "text/html", pages[RMS_PAGE_PARA_HTML]);
    }

    void handleParaJS(AsyncWebServerRequest *request)
    {
        request->send(200, "text/html", pages[RMS_PAGE_PARA_JS]);
    }

    void handleParaRouteurJS(AsyncWebServerRequest *request)
    {
        request->send(200, "text/html", pages[RMS_PAGE_GLOBAL_PARA_JS]);
    }

    void handleNotFound(AsyncWebServerRequest *request)
    {
        // Page Web pas trouvé
        ModuleCore::log("Not Found");
        String message = "Not Found\n\n";
        message += "URI: ";
        message += request->url();
        message += "\nMethod: ";
        message += (request->method() == HTTP_GET) ? "GET" : "POST";
        message += "\nArguments: ";
        message += String(request->args());
        message += "\n";
        for (uint8_t i = 0; i < request->args(); i++)
        {
            message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
        }
        request->send(404, "text/plain", message);
    }
} // namespace ModuleServer