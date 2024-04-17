// ******************************
// * Client d'un Smart Gateways *
// ******************************
#include "ModulePowerMeter.h"
#include "ModulePowerMeterSmartG.h"
#include "ModuleDebug.h"
#include "ModuleWifi.h"
#include "ModuleHardware.h"
#include "helpers.h"
#include <WiFi.h>

namespace ModulePowerMeterSmartG
{
    //Paramètres for SmartGateways
    String SG_dataBrute = "";

    void setup()
    {
        // noop
    }

    void gauge(unsigned long msLoop)
    {
        String S = "";
        String SmartG_Data = "";
        String Gr[4];
        String data_[20];

        // Use WiFiClient class to create TCP connections
        WiFiClient clientESP_RMS;
        byte arr[4];
        ip_explode(ModulePowerMeter::getExtIp(), arr);

        String host = String(arr[3]) + "." + String(arr[2]) + "." + String(arr[1]) + "." + String(arr[0]);
        if (!clientESP_RMS.connect(host.c_str(), 82))
        {
            // PORT 82 pour Smlart Gateways
            ModuleDebug::stockMessage("connection to client SmartGateways failed : " + host);
            delay(200);
            ModuleWifi::incrWifiBug();
            return;
        }
        String url = "/smartmeter/api/read";
        clientESP_RMS.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
        unsigned long timeout = millis();
        while (clientESP_RMS.available() == 0)
        {
            if (millis() - timeout > 5000)
            {
                ModuleDebug::stockMessage(">>> client SmartGateways Timeout ! : " + host);
                clientESP_RMS.stop();
                return;
            }
            // TODO: add delay
        }
        timeout = millis();
        // Lecture des données brutes distantes
        while (clientESP_RMS.available() && (millis() - timeout < 5000))
        {
            SmartG_Data += clientESP_RMS.readStringUntil('\r');
        }
        int p = SmartG_Data.indexOf("{");
        SmartG_Data = SmartG_Data.substring(p + 1);
        p = SmartG_Data.indexOf("}");
        SmartG_Data = SmartG_Data.substring(0, p);
        PuissanceS_M_inst = ModulePowerMeter::PMax(ValJsonSG("PowerDelivered_total", SmartG_Data));
        PuissanceI_M_inst = ModulePowerMeter::PMax(ValJsonSG("PowerReturned_total", SmartG_Data));
        long EnergyDeliveredTariff1 = int(1000 * ValJsonSG("EnergyDeliveredTariff1", SmartG_Data));
        long EnergyDeliveredTariff2 = int(1000 * ValJsonSG("EnergyDeliveredTariff2", SmartG_Data));
        Energie_M_Soutiree = EnergyDeliveredTariff1 + EnergyDeliveredTariff2;
        long EnergyReturnedTariff1 = int(1000 * ValJsonSG("EnergyReturnedTariff1", SmartG_Data));
        long EnergyReturnedTariff2 = int(1000 * ValJsonSG("EnergyReturnedTariff2", SmartG_Data));
        Energie_M_Injectee = EnergyReturnedTariff1 + EnergyReturnedTariff2;
        SG_dataBrute = SmartG_Data;
        ModulePowerMeter::powerFilter();
        ModulePowerMeter::signalSourceValid();
        // Reset du Watchdog à chaque trame du SmartGateways reçue
        ModulePowerMeter::ping();
        ModuleHardware::resetConnectivityLed();
    }

    float ValJsonSG(String nom, String Json)
    {
        int p = Json.indexOf(nom);
        Json = Json.substring(p);
        p = Json.indexOf(":");
        Json = Json.substring(p + 2);
        p = Json.indexOf(",");
        float val = 0;
        if (p > 0)
        {
            Json = Json.substring(0, p);
            val = Json.toFloat();
        }
        return val;
    }

} // namespace ModulePowerMeterSmartG