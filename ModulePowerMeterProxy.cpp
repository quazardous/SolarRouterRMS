// ***************************************************************
// * Client d'un autre ESP32 en charge de mesurer les puissances *
// ***************************************************************
#include "ModulePowerMeter.h"
#include "ModulePowerMeterProxy.h"
#include "ModuleWifi.h"
#include "ModuleEDF.h"
#include "ModuleHardware.h"
#include "ModuleDebug.h"
#include "rms.h"

namespace ModulePowerMeterProxy
{
    ModulePowerMeter::source_t proxySource;

    void boot()
    {
    }

    void gauge(unsigned long msLoop)
    {
        String GS = RMS_GS;
        String RS = RMS_RS;
        String S = "";
        String RMSExtDataB = "";
        String Gr[4];
        String data_[21];

        // Use WiFiClient class to create TCP connections
        WiFiClient clientESP_RMS;
        byte arr[4];
        ip_explode(ModulePowerMeter::getExtIp(), arr);

        String host = String(arr[3]) + "." + String(arr[2]) + "." + String(arr[1]) + "." + String(arr[0]);
        if (!clientESP_RMS.connect(host.c_str(), 80))
        {
            ModuleDebug::stockMessage("connection to client ESP_RMS failed : " + host);
            delay(200);
            ModuleWifi::incrWifiBug();
            return;
        }
        String url = "/ajax_data";
        clientESP_RMS.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
        unsigned long timeout = millis();
        while (clientESP_RMS.available() == 0)
        {
            if (millis() - timeout > 5000)
            {
                ModuleDebug::stockMessage("client ESP_RMS Timeout !" + host);
                clientESP_RMS.stop();
                return;
            }
        }
        timeout = millis();
        // Lecture des données brutes distantes
        while (clientESP_RMS.available() && (millis() - timeout < 5000))
        {
            RMSExtDataB += clientESP_RMS.readStringUntil('\r');
        }
        if (RMSExtDataB.length() > 300)
        {
            RMSExtDataB = "";
        }
        if (RMSExtDataB.indexOf("Deb") >= 0 && RMSExtDataB.indexOf("Fin") > 0)
        { // Trame complète reçue
            RMSExtDataB = RMSExtDataB.substring(RMSExtDataB.indexOf("Deb") + 4);
            RMSExtDataB = RMSExtDataB.substring(0, RMSExtDataB.indexOf("Fin") + 3);
            String Sval = "";
            int idx = 0;
            while (RMSExtDataB.indexOf(GS) > 0)
            {
                Sval = RMSExtDataB.substring(0, RMSExtDataB.indexOf(GS));
                RMSExtDataB = RMSExtDataB.substring(RMSExtDataB.indexOf(GS) + 1);
                Gr[idx] = Sval;
                idx++;
            }
            Gr[idx] = RMSExtDataB;
            idx = 0;
            for (int i = 0; i < 3; i++)
            {
                while (Gr[i].indexOf(RS) >= 0)
                {
                    Sval = Gr[i].substring(0, Gr[i].indexOf(RS));
                    Gr[i] = Gr[i].substring(Gr[i].indexOf(RS) + 1);
                    data_[idx] = Sval;
                    idx++;
                }
                data_[idx] = Gr[i];
                idx++;
            }
            ModulePowerMeter::electric_data_t *elecDataHouse = ModulePowerMeter::getElectricData(ModulePowerMeter::DOMAIN_HOUSE);
            ModulePowerMeter::electric_data_t *elecDataTriac = ModulePowerMeter::getElectricData(ModulePowerMeter::DOMAIN_TRIAC);
            for (int i = 0; i <= idx; i++)
            {
                switch (i)
                {

                case 1:
                    proxySource = ModulePowerMeter::getSourceFromName(data_[i].c_str());
                    break;
                case 2:
                    if (!ModuleEDF::getTempo())
                        ModuleEDF::setLTARF(data_[i].c_str());
                    break;
                case 3:
                    if (!ModuleEDF::getTempo())
                        ModuleEDF::setSTGE(data_[i].c_str());
                    break;
                case 4:
                    // Temperature non utilisé
                    break;
                case 5:
                    elecDataHouse->powerIn = ModulePowerMeter::PMax(data_[i].toInt());
                    break;
                case 6:
                    elecDataHouse->powerOut = ModulePowerMeter::PMax(data_[i].toInt());
                    break;
                case 7:
                    elecDataHouse->vaPowerIn = ModulePowerMeter::PMax(data_[i].toInt());
                    break;
                case 8:
                    elecDataHouse->vaPowerOut = ModulePowerMeter::PMax(data_[i].toInt());
                    break;
                case 9:
                    elecDataHouse->energyDayIn = data_[i].toInt();
                    break;
                case 10:
                    elecDataHouse->energyDayOut = data_[i].toInt();
                    break;
                case 11:
                    elecDataHouse->energyIn = data_[i].toInt();
                    break;
                case 12:
                    elecDataHouse->energyOut = data_[i].toInt();
                    // Reset du Watchdog à chaque trame du RMS reçue
                    ModulePowerMeter::signalSourceValid();
                    ModuleHardware::resetConnectivityLed();
                    ModulePowerMeter::ping();
                    break;
                case 13:
                    // CAS UxIx2 avec une deuxieme sonde
                    elecDataTriac->powerIn = data_[i].toInt();
                    break;
                case 14:
                    elecDataTriac->powerOut = data_[i].toInt();
                    break;
                case 15:
                    elecDataTriac->vaPowerIn = data_[i].toInt();
                    break;
                case 16:
                    elecDataTriac->vaPowerOut = data_[i].toInt();
                    break;
                case 17:
                    elecDataTriac->energyDayIn = data_[i].toInt();
                    break;
                case 18:
                    elecDataTriac->energyDayOut = data_[i].toInt();
                    break;
                case 19:
                    elecDataTriac->energyIn = data_[i].toInt();
                    break;
                case 20:
                    elecDataTriac->energyOut = data_[i].toInt();
                    break;
                }
            }
            RMSExtDataB = "";
        }
    }

    const ModulePowerMeter::source_t getProxySource()
    {
        return proxySource;
    }

    // http handlers
    void httpAjaxRMS(AsyncWebServerRequest* request, String& S)
    {
        int LastIdx = request->arg((size_t)0).toInt();

        // Use WiFiClient class to create TCP connections
        WiFiClient clientESP_RMS;
        byte arr[4];
        ip_explode(ModulePowerMeter::getExtIp(), arr);

        String host = String(arr[3]) + "." + String(arr[2]) + "." + String(arr[1]) + "." + String(arr[0]);
        if (!clientESP_RMS.connect(host.c_str(), 80))
        {
            ModuleDebug::stockMessage("connection to client ESP_RMS external failed (call from  handleAjaxRMS)");
            return;
        }
        String url = "/ajax_dataRMS?idx=" + String(LastIdx);
        clientESP_RMS.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
        unsigned long timeout = millis();
        while (clientESP_RMS.available() == 0)
        {
            if (millis() - timeout > 5000)
            {
                ModuleDebug::stockMessage(">>> clientESP_RMS Timeout !");
                clientESP_RMS.stop();
                return;
            }
        }
        String RMSExtDataB = "";
        // Lecture des données brutes distantes
        while (clientESP_RMS.available())
        {
            RMSExtDataB += clientESP_RMS.readStringUntil('\r');
        }
        S = RMSExtDataB.substring(RMSExtDataB.indexOf("\n\n") + 2);
    }
} // namespace ModulePowerMeterProxy