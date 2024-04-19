// ****************************************************
// * Client d'un Shelly Em sur voie 0 ou 1 ou triphasé*
// ****************************************************
#include "ModulePowerMeter.h"
#include "ModulePowerMeterShellyEm.h"
#include "ModuleDebug.h"
#include "ModuleWifi.h"
#include "ModuleHardware.h"
#include "HelperJson.h"
#include "rms.h"

namespace ModulePowerMeterShellyEm
{
    //Paramètres for Shelly Em
    String ShEm_dataBrute = "";
    int ShEm_comptage_appels = 0;
    float PwMoy2 = 0;  //Moyenne voie secondsaire
    float pfMoy2 = 1;  //pf Voie secondaire
    unsigned short phasesNumber = 0;

    void setup()
    {
        // noop
    }

    void gauge(unsigned long msLoop)
    {
        String S = "";
        String Shelly_Data = "";
        float Pw = 0;
        float voltage = 0;
        float pf = 0;

        // Use WiFiClient class to create TCP connections
        WiFiClient clientESP_RMS;
        byte arr[4];
        ip_explode(ModulePowerMeter::getExtIp(), arr);

        String host = String(arr[3]) + "." + String(arr[2]) + "." + String(arr[1]) + "." + String(arr[0]);
        if (!clientESP_RMS.connect(host.c_str(), 80))
        {
            ModuleDebug::stockMessage("connection to client Shelly Em failed: " + host);
            delay(200);
            ModuleWifi::incrWifiBug();
            return;
        }
        int voie = phasesNumber;
        int Voie = voie % 2;

        if (ShEm_comptage_appels == 1)
        {
            Voie = (Voie + 1) % 2;
        }
        String url = "/emeter/" + String(Voie);
        if (voie == 3)
            url = "/status";                                   // Triphasé
        ShEm_comptage_appels = (ShEm_comptage_appels + 1) % 5; // 1 appel sur 6 vers la deuxième voie qui ne sert pas au routeur
        clientESP_RMS.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
        unsigned long timeout = millis();
        while (clientESP_RMS.available() == 0)
        {
            if (millis() - timeout > 5000)
            {
                ModuleDebug::stockMessage("client Shelly Em Timeout ! : " + host);
                clientESP_RMS.stop();
                return;
            }
        }
        timeout = millis();
        // Lecture des données brutes distantes
        while (clientESP_RMS.available() && (millis() - timeout < 5000))
        {
            Shelly_Data += clientESP_RMS.readStringUntil('\r');
        }
        int p = Shelly_Data.indexOf("{");
        Shelly_Data = Shelly_Data.substring(p);
        ModulePowerMeter::electric_data_t *elecDataHouse = ModulePowerMeter::getElectricData();
        if (voie == 3)
        {
            // Triphasé
            ShEm_dataBrute = "<strong>Triphasé</strong><br>" + Shelly_Data;
            p = Shelly_Data.indexOf("emeters");
            Shelly_Data = Shelly_Data.substring(p + 10);
            Pw = ModulePowerMeter::PMax(HelperJson::ValJson("power", Shelly_Data)); // Phase 1
            pf = HelperJson::ValJson("pf", Shelly_Data);
            pf = abs(pf);
            float total_Pw = Pw;
            float total_Pva = 0;
            if (pf > 0)
            {
                total_Pva = abs(Pw) / pf;
            }
            float total_E_soutire = HelperJson::ValJson("total\"", Shelly_Data);
            float total_E_injecte = HelperJson::ValJson("total_returned", Shelly_Data);
            p = Shelly_Data.indexOf("}");
            Shelly_Data = Shelly_Data.substring(p + 1);
            float total_E_injecte = HelperJson::ValJson("total_returned", Shelly_Data);
            Pw = ModulePowerMeter::PMax(HelperJson::ValJson("power", Shelly_Data)); // Phase 2
            pf = HelperJson::ValJson("pf", Shelly_Data);
            pf = abs(pf);
            total_Pw += Pw;
            if (pf > 0)
            {
                total_Pva += abs(Pw) / pf;
            }
            total_E_soutire += HelperJson::ValJson("total\"", Shelly_Data);
            total_E_injecte += HelperJson::ValJson("total_returned", Shelly_Data);
            p = Shelly_Data.indexOf("}");
            Shelly_Data = Shelly_Data.substring(p + 1);
            Pw = ModulePowerMeter::PMax(HelperJson::ValJson("power", Shelly_Data)); // Phase 3
            pf = HelperJson::ValJson("pf", Shelly_Data);
            pf = abs(pf);
            total_Pw += Pw;
            if (pf > 0)
            {
                total_Pva += abs(Pw) / pf;
            }
            total_E_soutire += HelperJson::ValJson("total\"", Shelly_Data);
            total_E_injecte += HelperJson::ValJson("total_returned", Shelly_Data);
            elecDataHouse->energyIn = int(total_E_soutire);
            elecDataHouse->energyOut = int(total_E_injecte);
            if (total_Pw == 0)
            {
                total_Pva = 0;
            }
            if (total_Pw > 0)
            {
                elecDataHouse->instPowerIn = total_Pw;
                elecDataHouse->instPowerOut = 0;
                elecDataHouse->instVaPowerIn = total_Pva;
                elecDataHouse->instVaPowerOut = 0;
            }
            else
            {
                elecDataHouse->instPowerIn = 0;
                elecDataHouse->instPowerOut = -total_Pw;
                elecDataHouse->instVaPowerIn = 0;
                elecDataHouse->instVaPowerOut = total_Pva; // NB no negative sign
            }
        }
        else
        {
            // Monophasé
            ShEm_dataBrute = "<strong>Voie: " + String(voie) + "</strong><br>" + Shelly_Data;
            Shelly_Data = Shelly_Data + ",";
            if (Shelly_Data.indexOf("true") > 0)
            {
                // Donnée valide
                Pw = ModulePowerMeter::PMax(HelperJson::ValJson("power", Shelly_Data));
                voltage = HelperJson::ValJson("voltage", Shelly_Data);
                pf = HelperJson::ValJson("pf", Shelly_Data);
                pf = abs(pf);
                if (pf > 1)
                    pf = 1;
                if (Voie == voie)
                {
                    // voie du routeur
                    if (Pw >= 0)
                    {
                        elecDataHouse->instPowerIn = Pw;
                        elecDataHouse->instPowerOut = 0;
                        if (pf > 0.01)
                        {
                            elecDataHouse->instVaPowerIn = ModulePowerMeter::PMax(Pw / pf);
                        }
                        else
                        {
                            elecDataHouse->instVaPowerIn = 0;
                        }
                        elecDataHouse->instVaPowerOut = 0;
                    }
                    else
                    {
                        elecDataHouse->instPowerIn = 0;
                        elecDataHouse->instPowerOut = -Pw;
                        elecDataHouse->instVaPowerIn = 0;
                        if (pf > 0.01)
                        {
                            elecDataHouse->instVaPowerOut = ModulePowerMeter::PMax(-Pw / pf);
                        }
                        else
                        {
                            elecDataHouse->instVaPowerOut = 0;
                        }
                    }
                    elecDataHouse->energyIn = int(HelperJson::ValJson("total\"", Shelly_Data));
                    elecDataHouse->energyOut = int(HelperJson::ValJson("total_returned", Shelly_Data));
                    elecDataHouse->powerFactor = pf;
                    elecDataHouse->voltage = voltage;
                }
                else
                {
                    // voie secondaire
                    if (ModulePowerMeter::getSlowSmoothing())
                    {
                        // Lissage car moins de mesure sur voie secondaire
                        PwMoy2 = 0.2 * Pw + 0.8 * PwMoy2;
                        pfMoy2 = 0.2 * pf + 0.8 * pfMoy2;
                        Pw = PwMoy2;
                        pf = pfMoy2;
                    }
                    if (Pw >= 0)
                    {
                        elecDataHouse->instPowerIn = Pw;
                        elecDataHouse->instPowerOut = 0;
                        if (pf > 0.01)
                        {
                            elecDataHouse->instVaPowerIn = ModulePowerMeter::PMax(Pw / pf);
                        }
                        else
                        {
                            elecDataHouse->instVaPowerIn = 0;
                        }
                        elecDataHouse->instVaPowerOut = 0;
                    }
                    else
                    {
                        elecDataHouse->instPowerIn = 0;
                        elecDataHouse->instPowerOut = -Pw;
                        elecDataHouse->instVaPowerIn = 0;
                        if (pf > 0.01)
                        {
                            elecDataHouse->instVaPowerOut = ModulePowerMeter::PMax(-Pw / pf);
                        }
                        else
                        {
                            elecDataHouse->instVaPowerOut = 0;
                        }
                    }
                    elecDataHouse->energyIn = int(HelperJson::ValJson("total\"", Shelly_Data));
                    elecDataHouse->energyOut = int(HelperJson::ValJson("total_returned", Shelly_Data));
                    elecDataHouse->powerFactor = pf;
                    elecDataHouse->voltage = voltage;
                }
            }
        }
        ModulePowerMeter::powerFilter();
        // Reset du Watchdog à chaque trame du Shelly reçue
        ModulePowerMeter::ping();
        if (ShEm_comptage_appels > 1)
            ModulePowerMeter::signalSourceValid();
        ModuleHardware::resetConnectivityLed();
    }

    // getters / setters
    void setPhasesNumber(unsigned short number)
    {
        phasesNumber = number;
    }
    unsigned short getPhasesNumber()
    {
        return phasesNumber;
    }

    // web handlers
    void httpAjaxRMS(WebServer& server, String& S) {
        String GS = RMS_GS;
        S += GS + ShEm_dataBrute;
    }
} // namespace ModulePowerMeterShellyEm