
#include "ModulePowerMeter.h"
#include "ModulePowerMeterEnphase.h"
#include "ModuleWifi.h"
#include "ModuleDebug.h"
#include <UrlEncode.h>
#include <WiFiClientSecure.h>

#define RMS_POWER_METER_ENPHASE_SESSION_SERVER "enlighten.enphaseenergy.com"
#define RMS_POWER_METER_ENPHASE_TOKEN_SERVER "entrez.enphaseenergy.com"

#define RMS_POWER_METER_ENPHASE_MAX_TOKEN_SIZE 256

namespace ModulePowerMeterEnphase
{
    //Paramètres for Enphase-Envoye-Smetered
    char EnphaseToken[RMS_POWER_METER_ENPHASE_MAX_TOKEN_SIZE] = "";
    char EnphaseUser[128] = "";
    char EnphasePwd[128] = "";
    // Sert égalemnet au Shelly comme numéro de voie (refactor: plus maintenant)
    unsigned short EnphaseSerial = 0;
    char Session_id[RMS_POWER_METER_ENPHASE_MAX_TOKEN_SIZE] = "";

    WiFiClientSecure clientSecu;

    unsigned long previousTokenTock = 0;

    long LastwhDlvdCum = 0;  //Dernière valeur cumul Wh Soutire-injecté.
    int PactConso_M, PactProd;

    void setup()
    {
        String JsonToken = String("", RMS_POWER_METER_ENPHASE_MAX_TOKEN_SIZE);
        // Obtention Session ID
        //********************
        String Host = String(RMS_POWER_METER_ENPHASE_SESSION_SERVER);
        String adrEnphase = "https://" RMS_POWER_METER_ENPHASE_SESSION_SERVER "/login/login.json";
        String requestBody = "user[email]=" + String(EnphaseUser) + "&user[password]=" + urlEncode(EnphasePwd);

        if (EnphaseUser[0] != '\0' && EnphasePwd[0] != '\0')
        {
            Serial.println("Essai connexion Enlighten server 1 pour obtention session_id!");
            ModuleDebug::getDebug().println("Essai connexion Enlighten server 1 pour obtention session_id!");
            clientSecu.setInsecure(); // skip verification
            if (!clientSecu.connect(RMS_POWER_METER_ENPHASE_SESSION_SERVER, 443))
                ModuleDebug::stockMessage("Connection failed to Enlighten server: " RMS_POWER_METER_ENPHASE_SESSION_SERVER);
            else
            {
                Serial.println("Connected to Enlighten server: " RMS_POWER_METER_ENPHASE_SESSION_SERVER);
                clientSecu.println("POST " + adrEnphase + "?" + requestBody + " HTTP/1.0");
                clientSecu.println("Host: " RMS_POWER_METER_ENPHASE_SESSION_SERVER);
                clientSecu.println("Connection: close");
                clientSecu.println();
                String line = "";
                while (clientSecu.connected())
                {
                    line = clientSecu.readStringUntil('\n');
                    if (line == "\r")
                    {
                        Serial.println("headers 1 Enlighten received");
                        JsonToken = "";
                    }
                    JsonToken += line;
                }
                // if there are incoming bytes available
                // from the server, read them and print them:
                while (clientSecu.available())
                {
                    char c = clientSecu.read();
                    Serial.write(c);
                }
                clientSecu.stop();
            }
            strncpy(Session_id, StringJson("session_id", JsonToken).c_str(), sizeof(Session_id));
            String m = String("session_id :");
            m += String(Session_id);
            Serial.println(m);
            ModuleDebug::getDebug().println(m);
        }
        else
        {
            Serial.println("Connexion vers Envoy-S en firmware version 5");
            ModuleDebug::getDebug().println("Connexion vers Envoy-S en firmware version 5");
        }
        // Obtention Token
        //********************
        if (Session_id[0] != '\0' && EnphaseSerial > 0 && EnphaseUser[0] != '\0')
        {
            Host = String(RMS_POWER_METER_ENPHASE_TOKEN_SERVER);
            adrEnphase = "https://" RMS_POWER_METER_ENPHASE_TOKEN_SERVER "/tokens";
            requestBody = "{\"session_id\":\"" + String(Session_id) + "\", \"serial_num\":" + String(EnphaseSerial) + ", \"username\":\"" + String(EnphaseUser) + "\"}";
            Serial.println("Essai connexion Enlighten server 2 pour obtention token!");
            ModuleDebug::getDebug().println("Essai connexion Enlighten server 2 pour obtention token!");
            clientSecu.setInsecure(); // skip verification
            if (!clientSecu.connect(RMS_POWER_METER_ENPHASE_TOKEN_SERVER, 443))
                ModuleDebug::stockMessage("Connection failed to: " RMS_POWER_METER_ENPHASE_TOKEN_SERVER);
            else
            {
                Serial.println("Connected to: " RMS_POWER_METER_ENPHASE_TOKEN_SERVER);
                clientSecu.println("POST " + adrEnphase + " HTTP/1.0");
                clientSecu.println("Host: " RMS_POWER_METER_ENPHASE_TOKEN_SERVER);
                clientSecu.println("Content-Type: application/json");
                clientSecu.println("content-length: " + String(requestBody.length()));
                clientSecu.println("Connection: close");
                clientSecu.println();
                clientSecu.println(requestBody);
                clientSecu.println();
                Serial.println("Attente user est connecté");
                String line = "";
                JsonToken = "";
                while (clientSecu.connected())
                {
                    line = clientSecu.readStringUntil('\n');
                    if (line == "\r")
                    {
                        Serial.println("headers 2 enlighten received");
                        JsonToken = "";
                    }
                    JsonToken += line;
                }
                // if there are incoming bytes available
                // from the server, read them and print them:
                while (clientSecu.available())
                {
                    char c = clientSecu.read();
                    Serial.write(c);
                }
                clientSecu.stop();
                JsonToken.trim();
                String m = String("Token : ");
                m += JsonToken;
                Serial.println(m);
                ModuleDebug::getDebug().println(m);
                if (JsonToken.length() > 50)
                {
                    strncpy(EnphaseToken, JsonToken.c_str(), sizeof(EnphaseToken));
                    ModulePowerMeter::resetCpuLoad0();
                }
            }
        }
    }

    void gauge(unsigned long msLoop)
    {
        // Lecture des consommations
        int Num_portIQ = 443;
        String JsonEnPhase = "";
        byte IP[4];
        ip_explode(ModulePowerMeter::getExtIp(), IP);
        String host = String(IP[3]) + "." + String(IP[2]) + "." + String(IP[1]) + "." + String(IP[0]);

        if (strlen(EnphaseToken) > 50 && EnphaseUser[0] != '\0')
        {
            // Connexion por firmware V7
            clientSecu.setInsecure(); // skip verification
            if (!clientSecu.connect(host.c_str(), Num_portIQ))
            {
                ModuleDebug::stockMessage("Connection failed to Envoy-S server! : " + host);
            }
            else
            {
                // Serial.println("Connected to Envoy-S server!");
                clientSecu.println("GET https://" + host + "/ivp/meters/reports/consumption HTTP/1.0");
                clientSecu.println("Host: " + host);
                clientSecu.println("Accept: application/json");
                clientSecu.println("Authorization: Bearer " + String(EnphaseToken));
                clientSecu.println("Connection: close");
                clientSecu.println();

                String line = "";
                while (clientSecu.connected())
                {
                    line = clientSecu.readStringUntil('\n');
                    if (line == "\r")
                    {
                        // Serial.println("headers received");
                        JsonEnPhase = "";
                    }
                    JsonEnPhase += line;
                }
                // if there are incoming bytes available
                // from the server, read them and print them:
                while (clientSecu.available())
                {
                    char c = clientSecu.read();
                    Serial.write(c);
                }

                clientSecu.stop();
            }
        }
        else
        {
            // Conexion Envoy V5
            // Use WiFiClient class to create TCP connections http
            WiFiClient clientFirmV5;
            if (!clientFirmV5.connect(host.c_str(), 80))
            {
                ModuleWifi::incrWifiBug();
                ModuleDebug::stockMessage("connection to client clientFirmV5 failed (call to Envoy-S)");
                delay(200);
                return;
            }
            String url = "/ivp/meters/reports/consumption";
            clientFirmV5.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
            unsigned long timeout = millis();
            while (clientFirmV5.available() == 0)
            {
                if (millis() - timeout > 5000)
                {
                    Serial.println(">>> client clientFirmV5 Timeout !");
                    clientFirmV5.stop();
                    return;
                }
            }
            timeout = millis();
            String line;
            // Lecture des données brutes distantes
            while (clientFirmV5.available() && (millis() - timeout < 5000))
            {
                line = clientFirmV5.readStringUntil('\n');
                if (line == "\r")
                {
                    // Serial.println("headers received");
                    JsonEnPhase = "";
                }
                JsonEnPhase += line;
            }
        }

        // On utilise pas la librairie ArduinoJson.h, pour décoder message Json, qui crache sur de grosses données
        String TotConso = PrefiltreJson("total-consumption", "cumulative", JsonEnPhase);
        PactConso_M = int(ValJson("actPower", TotConso));
        String NetConso = PrefiltreJson("net-consumption", "cumulative", JsonEnPhase);
        float PactReseau = ValJson("actPower", NetConso);
        PactReseau = ModulePowerMeter::PfloatMax(PactReseau);
        if (PactReseau < 0)
        {
            PuissanceS_M_inst = 0;
            PuissanceI_M_inst = int(-PactReseau);
        }
        else
        {
            PuissanceI_M_inst = 0;
            PuissanceS_M_inst = int(PactReseau);
        }
        float PvaReseau = ValJson("apprntPwr", NetConso);
        PvaReseau = PfloatMax(PvaReseau);
        if (PvaReseau < 0)
        {
            PVAS_M_inst = 0;
            PVAI_M_inst = int(-PvaReseau);
        }
        else
        {
            PVAI_M_inst = 0;
            PVAS_M_inst = int(PvaReseau);
        }
        powerFilter();
        float PowerFactor = 0;
        if ((PVA_M_moy) != 0)
        {
            PowerFactor = floor(100 * abs(Puissance_M_moy) / PVA_M_moy) / 100;
            PowerFactor = min(PowerFactor, float(1));
        }
        PowerFactor_M = PowerFactor;
        long whDlvdCum = LongJson("whDlvdCum", NetConso);
        long DeltaWh = 0;
        if (whDlvdCum != 0)
        { // bonne donnée
            if (LastwhDlvdCum == 0)
            {
                LastwhDlvdCum = whDlvdCum;
            }
            DeltaWh = whDlvdCum - LastwhDlvdCum;
            LastwhDlvdCum = whDlvdCum;
            if (DeltaWh < 0)
            {
                Energie_M_Injectee = Energie_M_Injectee - DeltaWh;
            }
            else
            {
                Energie_M_Soutiree = Energie_M_Soutiree + DeltaWh;
            }
        }
        Tension_M = ValJson("rmsVoltage", NetConso);
        Intensite_M = ValJson("rmsCurrent", NetConso);
        PactProd = PactConso_M - int(PactReseau);
        ModulePowerMeter::signalSourceValid();
        if (PactReseau != 0 || PvaReseau != 0)
        {
            // Reset du Watchdog à chaque trame  reçue de la passerelle Envoye-S metered
            ModulePowerMeter::ping();
        }
        if (WifiLedCounter > 30)
        {
            WifiLedCounter = 4;
        }
    }

    void loop(unsigned long msLoop)
    {
        // stuff to do on the normal loop
        unsigned long msNow = millis();
        if (TICKTOCK(msNow, previousTokenTock, 2592000000))
        {
            // Connexion firmware V7
            if (strlen(EnphaseToken) > 50 && EnphaseUser[0] != '\0')
            {
                // Tout les 30 jours on recherche un nouveau Token
                setup();
            }
        }
    }

    String PrefiltreJson(String F1, String F2, String Json)
    {
        int p = Json.indexOf(F1);
        Json = Json.substring(p);
        p = Json.indexOf(F2);
        Json = Json.substring(p);
        return Json;
    }

    float ValJson(String nom, String Json)
    {
        int p = Json.indexOf(nom);
        Json = Json.substring(p);
        p = Json.indexOf(":");
        Json = Json.substring(p + 1);
        int q = Json.indexOf(",");
        p = Json.indexOf("}");
        p = min(p, q);
        float val = 0;
        if (p > 0)
        {
            Json = Json.substring(0, p);
            val = Json.toFloat();
        }
        return val;
    }

    long LongJson(String nom, String Json)
    {
        // Pour éviter des problèmes d'overflow
        int p = Json.indexOf(nom);
        Json = Json.substring(p);
        p = Json.indexOf(":");
        Json = Json.substring(p + 1);
        int q = Json.indexOf(".");
        p = Json.indexOf("}");
        p = min(p, q);
        long val = 0;
        if (p > 0)
        {
            Json = Json.substring(0, p);
            val = Json.toInt();
        }
        return val;
    }

    long myLongJson(String nom, String Json)
    { 
        // Alternative a LongJson au dessus pour extraire chez EDF nb jour Tempo  https://particulier.edf.fr/services/rest/referentiel/getNbTempoDays?TypeAlerte=TEMPO
        int p = Json.indexOf(nom);
        Json = Json.substring(p);
        p = Json.indexOf(":");
        Json = Json.substring(p + 1);
        int q = Json.indexOf(","); //<==== Recherche d'une virgule et non d'un point
        if (q == -1)
            q = 999; //  /<==== Ajout de ces 2 lignes pour que la ligne p = min(p, q); ci dessous donne le bon résultat
        p = Json.indexOf("}");
        p = min(p, q);
        long val = 0;
        if (p > 0)
        {
            Json = Json.substring(0, p);
            val = Json.toInt();
        }
        return val;
    }

    // setter / getter
    void setUser(const char *user)
    {
        strncpy(EnphaseUser, user, sizeof(EnphaseUser) - 1);
        EnphaseUser[sizeof(EnphaseUser) - 1] = '\0';
    }
    const char *getUser()
    {
        return EnphaseUser;
    }
    void setPwd(const char *pwd)
    {
        strncpy(EnphasePwd, pwd, sizeof(EnphasePwd) - 1);
        EnphasePwd[sizeof(EnphasePwd) - 1] = '\0';
    }
    const char *getPwd()
    {
        return EnphasePwd;
    }
    void setSerial(unsigned long serial)
    {
        EnphaseSerial = serial;
    }
    unsigned long getSerial()
    {
        return EnphaseSerial;
    }
} // namespace ModulePowerMeterEnphase
