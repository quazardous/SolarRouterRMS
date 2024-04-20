// ***************
// *  WEB SERVER *
// ***************

#include <WebServer.h>
#include "ModuleWifi.h"
#include "ModuleDebug.h"
#include "pages.h"

namespace ModuleServer
{
    extern WebServer server;

    void handleRoot()
    { 
        // Pages principales
        // en AP et STA mode ou Station Mode seul

        // Reset du timeout pour rester en mode AP
        ModuleWifi::resetApTimout();
        server.send(200, "text/html", pages[ModuleWifi::isStationMode() ? RMS_PAGE_MAIN_HTML : RMS_PAGE_CONNECT_HTML]);
    }

    void handleMainJS()
    {                                                               // Code Javascript
        server.send(200, "text/html", pages[RMS_PAGE_MAIN_JS]); // Javascript code
    }

    void handleBrute()
    {
        // Page données brutes
        server.send(200, "text/html", pages[RMS_PAGE_BRUTE_HTML]);
    }

    void handleBruteJS()
    {                                                                // Code Javascript
        server.send(200, "text/html", pages[RMS_PAGE_BRUTE_JS]); // Javascript code
    }

    void handleActions()
    {
        server.send(200, "text/html", pages[RMS_PAGE_ACTIONS_HTML]);
    }

    void handleActionsJS()
    {
        server.send(200, "text/html", pages[RMS_PAGE_ACTIONS_JS]);
    }

    void handlePara()
    {
        server.send(200, "text/html", pages[RMS_PAGE_PARA_HTML]);
    }

    void handleParaJS()
    {
        server.send(200, "text/html", pages[RMS_PAGE_PARA_JS]);
    }

    void handleParaRouteurJS()
    {
        server.send(200, "text/html", pages[RMS_PAGE_GLOBAL_PARA_JS]);
    }

    void handleNotFound()
    {
        // Page Web pas trouvé
        ModuleDebug::getDebug().println(F("Fichier non trouvé"));
        String message = "Fichier non trouvé\n\n";
        message += "URI: ";
        message += server.uri();
        message += "\nMethod: ";
        message += (server.method() == HTTP_GET) ? "GET" : "POST";
        message += "\nArguments: ";
        message += server.args();
        message += "\n";
        for (uint8_t i = 0; i < server.args(); i++)
        {
            message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
        }
        server.send(404, "text/plain", message);
    }
} // namespace ModuleServer