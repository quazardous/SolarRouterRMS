#include "ModuleWifi.h"

namespace ModuleWifi
{
    void setup() {
        // Configure WIFI
        // **************
        String hostname(HOSTNAME);
        uint32_t chipId = 0;
        for (int i = 0; i < 17; i = i + 8) {
            chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
        }
        hostname += String(chipId);  //Add chip ID to hostname
        WiFi.hostname(hostname);
        Serial.println(hostname);
        ap_default_ssid = (const char *)hostname.c_str();
        // Check WiFi connection
        // ... check mode
        if (WiFi.getMode() != WIFI_STA) {
            WiFi.mode(WIFI_STA);
            delay(10);
        }

        //WIFI
        Serial.println("SSID:" + ssid);
        Serial.println("Pass:" + password);
        if (ssid.length() > 0) {
            if (dhcpOn == 0) {  //Static IP
            byte arr[4];
            arr[0] = IP_Fixe & 0xFF;          // 0x78
            arr[1] = (IP_Fixe >> 8) & 0xFF;   // 0x56
            arr[2] = (IP_Fixe >> 16) & 0xFF;  // 0x34
            arr[3] = (IP_Fixe >> 24) & 0xFF;  // 0x12
            // Set your Static IP address
            IPAddress local_IP(arr[3], arr[2], arr[1], arr[0]);
            // Set your Gateway IP address
            arr[0] = Gateway & 0xFF;          // 0x78
            arr[1] = (Gateway >> 8) & 0xFF;   // 0x56
            arr[2] = (Gateway >> 16) & 0xFF;  // 0x34
            arr[3] = (Gateway >> 24) & 0xFF;  // 0x12
            IPAddress gateway(arr[3], arr[2], arr[1], arr[0]);
            // Set your masque/subnet IP address
            arr[0] = masque & 0xFF;
            arr[1] = (masque >> 8) & 0xFF;
            arr[2] = (masque >> 16) & 0xFF;
            arr[3] = (masque >> 24) & 0xFF;
            IPAddress subnet(arr[3], arr[2], arr[1], arr[0]);
            // Set your DNS IP address
            arr[0] = dns & 0xFF;
            arr[1] = (dns >> 8) & 0xFF;
            arr[2] = (dns >> 16) & 0xFF;
            arr[3] = (dns >> 24) & 0xFF;
            IPAddress primaryDNS(arr[3], arr[2], arr[1], arr[0]);  //optional
            IPAddress secondaryDNS(8, 8, 4, 4);                    //optional
            if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
                Serial.println("WIFI STA Failed to configure");
            }
            }
            StockMessage("Wifi Begin : " + ssid);
            WiFi.begin(ssid.c_str(), password.c_str());
            while (WiFi.status() != WL_CONNECTED && (millis() - startMillis < 15000)) {  // Attente connexion au Wifi
            Serial.write('.');
            Gestion_LEDs();
            Serial.print(WiFi.status());
            delay(300);
            }
        }
        if (WiFi.status() == WL_CONNECTED) {
            // ... print IP Address
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
            Serial.println("Connected IP address: " + WiFi.localIP().toString() + " or <a href='http://" + hostname + ".local' >" + hostname + ".local</a>");
            StockMessage("Connected IP address: " + WiFi.localIP().toString());
        } else {
            StockMessage("Can not connect to WiFi station. Go into AP mode and STA mode.");
            // Go into software AP and STA modes.
            WiFi.mode(WIFI_AP_STA);
            delay(10);
            WiFi.softAP(ap_default_ssid, ap_default_psk);
            Serial.print("Access Point Mode. IP address: ");
            Serial.println(WiFi.softAPIP());
        }
    }

    void loop() {
        //Vérification du WIFI
        //********************
        if (tps - previousWifiMillis > 30000) {  //Test présence WIFI toutes les 30s et autres
            previousWifiMillis = tps;
            if (WiFi.waitForConnectResult(10000) != WL_CONNECTED) {
                StockMessage("WIFI Connection Failed! #" + String(WIFIbug));
                WIFIbug++;
                if (WIFIbug > 2) {
                    ESP.restart();
                }
            } else {
                WIFIbug = 0;
            }

            if (WiFi.getMode() != WIFI_STA) {
                Serial.print("Access Point Mode. IP address: ");
                Serial.println(WiFi.softAPIP());
            } else {
                Serial.print("Niveau Signal WIFI:");
                Serial.println(WiFi.RSSI());
                Serial.print("IP address: ");
                Serial.println(WiFi.localIP());
                Serial.print("WIFIbug:");
                Serial.println(WIFIbug);
                Debug.print("Niveau Signal WIFI:");
                Debug.println(WiFi.RSSI());
                Debug.print("WIFIbug:");
                Debug.println(WIFIbug);
                Serial.println("Charge Lecture RMS (coeur 0) en ms - Min : " + String(int(previousTimeRMSMin)) + " Moy : " + String(int(previousTimeRMSMoy)) + "  Max : " + String(int(previousTimeRMSMax)));
                Debug.println("Charge Lecture RMS (coeur 0) en ms - Min : " + String(int(previousTimeRMSMin)) + " Moy : " + String(int(previousTimeRMSMoy)) + "  Max : " + String(int(previousTimeRMSMax)));
                Serial.println("Charge Boucle générale (coeur 1) en ms - Min : " + String(int(previousLoopMin)) + " Moy : " + String(int(previousLoopMoy)) + "  Max : " + String(int(previousLoopMax)));
                Debug.println("Charge Boucle générale (coeur 1) en ms - Min : " + String(int(previousLoopMin)) + " Moy : " + String(int(previousLoopMoy)) + "  Max : " + String(int(previousLoopMax)));
            }
            int T = int(millis() / 1000);
            float DureeOn = float(T) / 3600;
            Serial.println("ESP32 ON depuis : " + String(DureeOn) + " heures");
            ModuleDebug::Debug.println("ESP32 ON depuis : " + String(DureeOn) + " heures");
            
            ModuleSensor::loop();
            ModuleTime::loop();
            
            Call_EDF_data();
            int Ltarf = 0;  //Code binaire Tarif
            if (LTARF.indexOf("PLEINE") >= 0) Ltarf += 1;
            if (LTARF.indexOf("CREUSE") >= 0) Ltarf += 2;
            if (LTARF.indexOf("BLEU") >= 0) Ltarf += 4;
            if (LTARF.indexOf("BLANC") >= 0) Ltarf += 8;
            if (LTARF.indexOf("ROUGE") >= 0) Ltarf += 16;
            LTARFbin = Ltarf;
        }
        if ((tps - startMillis) > 180000 && WiFi.getMode() != WIFI_STA) {  //Connecté en  Access Point depuis 3mn. Pas normal
            Serial.println("Pas connecté en WiFi mode Station. Redémarrage");
            delay(5000);
            ESP.restart();
        }
    }
} // namespace ModuleWifi