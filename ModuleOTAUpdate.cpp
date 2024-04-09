#pragma once

namespace ModuleOTAUpdate
{
    void setup() {
        // Modification du programme par le Wifi  - OTA(On The Air)
        //***************************************************
        ArduinoOTA.setHostname((const char *)hostname.c_str());
        ArduinoOTA.begin();  //Mandatory
    }
    void loop() {
        ArduinoOTA.handle();
    }
} // namespace ModuleOTAUpdate