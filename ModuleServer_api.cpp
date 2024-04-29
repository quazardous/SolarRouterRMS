// ***************
// *  WEB SERVER *
// ***************

#include "ModuleServer.h"
#include "ModuleCore.h"
#include "ModuleConfig.h"
#include "version.h"
// #include <AsyncJson.h>


namespace ModuleServer
{
    void prepareJsonDoc(JsonDocument& doc) {
        doc["version"] = RMS_VERSION;
    }

    void bootApi(AsyncWebServer& server) {
        // DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

        // AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/rest/endpoint", [](AsyncWebServerRequest *request, JsonVariant &json) {
        //     StaticJsonDocument<200> data;
        //     if (json.is<JsonArray>())
        //     {
        //         data = json.as<JsonArray>();
        //     }
        //     else if (json.is<JsonObject>())
        //     {
        //         data = json.as<JsonObject>();
        //     }
        //     String response;
        //     serializeJson(data, response);
        //     request->send(200, "application/json", response);
        //     Serial.println(response);
        // });
        server.on("/api/hello", HTTP_GET, [](AsyncWebServerRequest *request) {
            JsonDocument doc;
            prepareJsonDoc(doc);
            ModuleCore::apiHello(request, doc);
            String jsonStr;
            serializeJson(doc, jsonStr);
            request->send(200, MIME_JSON, jsonStr);
            // AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonStr);
            // response->addHeader("Access-Control-Allow-Origin", "*");
            // request->send(response);
        });

        server.on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request) {
            JsonDocument doc;
            prepareJsonDoc(doc);
            ModuleConfig::apiGetConfig(request, doc);
            String jsonStr;
            serializeJson(doc, jsonStr);
            request->send(200, MIME_JSON, jsonStr);
            // AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonStr);
            // response->addHeader("Access-Control-Allow-Origin", "*");
            // request->send(response);
        });
    }

} // namespace ModuleServer