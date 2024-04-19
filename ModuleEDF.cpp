// *******************************************************
// * Recherche Info Tempo EDF pour les sources non Linky *
// *******************************************************
#include "ModuleEDF.h"
#include <WiFiClientSecure.h>
#include "ModuleDebug.h"
#include "ModuleTime.h"
#include "ModuleWifi.h"
#include "ModulePowerMeter.h"
#include <ArduinoJson.h>
#include "HelperJson.h"
#include "helpers.h"

namespace ModuleEDF {

    WiFiClientSecure clientSecuEDF;
    int LastHeureEDF = -1;
    bool TempoEDFon = 0;
    String LTARF = ""; //Option tarifaire EDF
    String STGE = ""; //Status Tempo uniquement EDF

    // Paramètres pour EDF
    int LTARFbin = 0;  // Code binaire  des tarifs EDF

    unsigned long previousEdfTock = 0;

    void setup() {
        // noop
    }

    void setupTimer(unsigned long msNow) {
        previousEdfTock = msNow;
    }

    void loop(unsigned long msLoop) {
        unsigned long msNow = millis();
        if (TICKTOCK(msNow, previousEdfTock, 30000)) { 
            if (ModuleWifi::isWifiConnected()) {
                Call_EDF_data();
                int Ltarf = 0;  //Code binaire Tarif
                if (LTARF.indexOf("PLEINE") >= 0) Ltarf += 1;
                if (LTARF.indexOf("CREUSE") >= 0) Ltarf += 2;
                if (LTARF.indexOf("BLEU") >= 0) Ltarf += 4;
                if (LTARF.indexOf("BLANC") >= 0) Ltarf += 8;
                if (LTARF.indexOf("ROUGE") >= 0) Ltarf += 16;
                LTARFbin = Ltarf;
            } else {
                ModuleDebug::stockMessage("Pas de connexion WIFI pour EDF");
            }
        }
    }

    void Call_EDF_data()
    {
        time_t now = time(NULL);

        String DateEDF = String(ts2str(now, "%Y-%m-%d"));
        const char *adr_EDF_Host = "particulier.edf.fr";
        String Host = String(adr_EDF_Host);
        String urlJSON = "/services/rest/referentiel/searchTempoStore?dateRelevant=" + DateEDF;
        String EDFdata = "";
        String line = "";
        float HeureCouranteDeci = decimal_hour(now) * 100;
        int Hcour = HeureCouranteDeci / 2; // Par pas de 72secondes pour faire 2 appels si un bug
        int LastH = LastHeureEDF / 2;

        if ((LastH != Hcour) && (Hcour == 300 || Hcour == 310 || Hcour == 530 || Hcour == 560 || Hcour == 600 || Hcour == 900 || Hcour == 1150) || LastHeureEDF < 0)
        {
            if (TempoEDFon >= 1)
            {
                // Use clientSecu class to create TCP connections
                clientSecuEDF.setInsecure(); // skip verification
                if (!clientSecuEDF.connect(adr_EDF_Host, 443))
                {
                    ModuleDebug::stockMessage("Connection failed to EDF server :" + Host);
                }
                else
                {

                    clientSecuEDF.print(String("GET ") + urlJSON + " HTTP/1.1\r\n" + "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\n" + "Host: " + Host + "\r\n" + "Connection: keep-alive\r\n\r\n");
                    Serial.println("Request vers EDF Sent");
                    unsigned long timeout = millis();
                    while (clientSecuEDF.available() == 0)
                    {
                        if (millis() - timeout > 5000)
                        {
                            ModuleDebug::stockMessage(">>> clientSecuEDF EDF Timeout !");
                            clientSecuEDF.stop();
                            return;
                        }
                        // TODO add yield
                    }
                    timeout = millis();
                    // Lecture des données brutes distantes
                    int fin = 0;
                    while (clientSecuEDF.connected() && (millis() - timeout < 5000) && fin < 2)
                    {
                        line = clientSecuEDF.readStringUntil('\n');
                        EDFdata += line;
                        if (line == "\r")
                        {
                            ModuleDebug::stockMessage("EnTetes EDF reçues");
                            EDFdata = "";
                            fin = 1;
                        }
                        if (fin == 1 && line.indexOf("}") >= 0)
                            fin = 2;

                        // TODO add yield
                    }
                    clientSecuEDF.stop();

                    // C'est EDF qui donne la couleur
                    String LTARFrecu = HelperJson::StringJson("couleurJourJ", EDFdata); // Remplace code du Linky
                    if (LTARFrecu.indexOf("TEMPO") >= 0)
                    {
                        // here we have a valid tempo color
                        LTARF = LTARFrecu;
                        String couleurJourJ1 = HelperJson::StringJson("couleurJourJ1", EDFdata);
                        line = "0";
                        if (couleurJourJ1 == "TEMPO_BLEU")
                            line = "4";
                        if (couleurJourJ1 == "TEMPO_BLANC")
                            line = "8";
                        if (couleurJourJ1 == "TEMPO_ROUGE")
                            line = "C";
                        STGE = line; // Valeur Hexa code du Linky
                        ModuleDebug::stockMessage(DateEDF + " : " + EDFdata);
                        EDFdata = "";
                        LastHeureEDF = HeureCouranteDeci; // Heure lecture Tempo EDF
                    }
                    else
                    {
                        ModuleDebug::stockMessage(DateEDF + " : Pas de données EDF valides");
                    }
                }
            }
            else
            {
                ModulePowerMeter::source_t source = ModulePowerMeter::getSource();
                if (source != ModulePowerMeter::SOURCE_LINKY && source != ModulePowerMeter::SOURCE_PROXY)
                {
                    LTARF = "";
                    STGE = "0";
                }
            }
        }
    }

    // setters / getters
    void setTempo(bool tempo)
    {
        if (TempoEDFon != tempo) {
            TempoEDFon = tempo;
            // force a refresh if changed
            LastHeureEDF = -1;
        }
    }
    bool getTempo()
    {
        return TempoEDFon;
    }

    void setLTARF(const char *ltarf)
    {
        LTARF = String(ltarf);
    }
    const char *getLTARF()
    {
        return LTARF.c_str();
    }
    unsigned int getBinaryLTARF()
    {
        return LTARFbin;
    }

    void setSTGE(const char *stge)
    {
        STGE = String(stge);
    }
    const char *getSTGE()
    {
        return STGE.c_str();
    }
} // namespace ModuleEDF
