// ***************
// *  WEB SERVER *
// ***************

#include "ModuleServer.h"
#include "ModuleCore.h"
#include "version.h"
// #include <AsyncJson.h>


namespace ModuleServer
{
    void prepareJsonDoc(JsonDocument& doc) {
        doc["version"] = RMS_VERSION;
    }

    void bootApi(AsyncWebServer& server) {

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
            String response;
            serializeJson(doc, response);
            request->send(200, "application/json", response);
        });
    }

} // namespace ModuleServer