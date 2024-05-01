// ***************
// *  WEB SERVER *
// ***************

#include "ModuleServer.h"
#include "ModuleCore.h"
#include "ModuleConfig.h"
#include "version.h"
#include <AsyncJson.h>


namespace ModuleServer
{
    void prepareJsonDoc(JsonDocument& doc) {
        doc["version"] = RMS_VERSION;
    }

    void bootApi(AsyncWebServer& server) {

        server.on("/api/hello", HTTP_GET, [](AsyncWebServerRequest *request) {
            JsonDocument doc;
            prepareJsonDoc(doc);
            ModuleCore::apiHello(request, doc);
            String jsonStr;
            serializeJson(doc, jsonStr);
            request->send(200, MIME_JSON, jsonStr);
        });

        server.on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request) {
            JsonDocument doc;
            prepareJsonDoc(doc);
            ModuleConfig::apiGetConfig(request, doc);
            String jsonStr;
            serializeJson(doc, jsonStr);
            request->send(200, MIME_JSON, jsonStr);
        });

        AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/api/config", [](AsyncWebServerRequest *request, JsonVariant &json) {
            ModuleCore::log("POST /api/config");
            JsonDocument in;
            JsonDocument out;
            if (json.is<JsonObject>())
            {
                in = json.as<JsonObject>();
                String response;
                prepareJsonDoc(out);
                ModuleConfig::apiPostConfig(request, in, out);
                serializeJson(out, response);
                request->send(200, "application/json", response);
                return;
            }
            // Handle error 400
            String response;
            out["error"] = "Bad Request";
            serializeJson(out, response);
            request->send(400, "application/json", response);
        });

        server.addHandler(handler);
    }

} // namespace ModuleServer