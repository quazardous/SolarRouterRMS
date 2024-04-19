#include "ModuleOTAUpdate.h"
#include <ArduinoOTA.h>   //Modification On The Air
#include "ModuleCore.h"

namespace ModuleOTAUpdate
{
    void boot() {
        // Modification du programme par le Wifi  - OTA(On The Air)
        //***************************************************
        ArduinoOTA.setHostname(ModuleCore::getHostname());
        ArduinoOTA.begin();  //Mandatory
    }
    void loop(unsigned long msLoop) {
        ArduinoOTA.handle();
    }
} // namespace ModuleOTAUpdate