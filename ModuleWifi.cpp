#include "ModuleWifi.h"
#include "ModuleDebug.h"
#include "ModuleCore.h"
#include "ModuleHardware.h"
#include "config.h"

namespace ModuleWifi
{
    unsigned long IP_Fixe = 0;
    unsigned long Gateway = 0;
    unsigned long masque = 4294967040;
    unsigned long dns = 0;
    byte dhcpOn = 1;

    char ssid[33] = RMS_WIFI_SSID;
    char password[64] = RMS_WIFI_KEY;

    unsigned int WIFIbug = 0;

    void setup() {
        const char *hostname = ModuleCore::getHostname();
        // Configure WIFI
        // **************
        WiFi.setHostname(hostname);
        Serial.println(hostname);

        const char *ap_default_ssid = hostname;        // Mode Access point  IP: 192.168.4.1
        const char *ap_default_psk = NULL;  // Pas de mot de passe en AP,

        // Check WiFi connection
        // ... check mode
        if (WiFi.getMode() != WIFI_STA) {
            WiFi.mode(WIFI_STA);
            delay(10);
        }

        // WIFI
        Serial.println(String("SSID:") + String(ssid));
        Serial.println(String("Pass:") + String(password));
        if (strlen(ssid) > 0) {
            if (dhcpOn == 0) {  //Static IP
                byte arr[4];
                // Set your Static IP address
                ip_explode(IP_Fixe, arr);
                IPAddress local_IP(arr[3], arr[2], arr[1], arr[0]);
                // Set your Gateway IP address
                ip_explode(Gateway, arr);
                IPAddress gateway(arr[3], arr[2], arr[1], arr[0]);
                // Set your masque/subnet IP address
                ip_explode(masque, arr);
                IPAddress subnet(arr[3], arr[2], arr[1], arr[0]);
                // Set your DNS IP address
                ip_explode(dns, arr);
                IPAddress primaryDNS(arr[3], arr[2], arr[1], arr[0]);  //optional
                IPAddress secondaryDNS(8, 8, 4, 4);                    //optional
                if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
                    Serial.println("WIFI STA Failed to configure");
                }
            }
            ModuleDebug::stockMessage("Wifi Begin : " + String(ssid));
            WiFi.begin(ssid, password);
            unsigned long startMillis = millis();
            while (WiFi.status() != WL_CONNECTED && (millis() - startMillis < 15000)) {
                // Attente connexion au Wifi
                Serial.write('.');
                ModuleHardware::Gestion_LEDs();
                Serial.print(WiFi.status());
                delay(300);
            }
        }
        if (WiFi.status() == WL_CONNECTED) {
            // ... print IP Address
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
            Serial.println("Connected IP address: " + WiFi.localIP().toString() + " or <a href='http://" + hostname + ".local' >" + hostname + ".local</a>");
            ModuleDebug::stockMessage("Connected IP address: " + WiFi.localIP().toString());
        } else {
            ModuleDebug::stockMessage("Can not connect to WiFi station. Go into AP mode and STA mode.");
            // Go into software AP and STA modes.
            WiFi.mode(WIFI_AP_STA);
            delay(10);
            WiFi.softAP(ap_default_ssid, ap_default_psk);
            Serial.print("Access Point Mode. IP address: ");
            Serial.println(WiFi.softAPIP());
        }
    }

    void loopTimer(unsigned long mtsNow) {
        // noop
    }

    void loop(unsigned long msLoop) {
        // noop
        // WIFI check in ModuleCore
    }

    bool isWifiConnected() {
        return WiFi.isConnected();
    }

    bool isStationMode() {
        return WiFi.getMode() == WIFI_STA;
    }

    bool hasInternet() {
        return WiFi.getMode() == WIFI_STA && WiFi.isConnected();
    }   

    bool canConnectWifi(unsigned long msTimeout) {
        if (WiFi.waitForConnectResult(msTimeout) == WL_CONNECTED) {
            WIFIbug = 0;
            return true;
        }
        WIFIbug++;
        ModuleDebug::stockMessage("WIFI Connection Failed! #" + String(WIFIbug));
        return false;
    }

    void resetWifiBug() {
        WIFIbug = 0;
    }

    unsigned int incrWifiBug() {
        return ++WIFIbug;
    }

    unsigned int getWifiBug() {
        return WIFIbug;
    }

    // getters / setters
    void setWifiSsid(const char *s) {
        strncpy(ssid, s, sizeof(ssid) - 1);
        ssid[sizeof(ssid) - 1] = '\0';
    }
    const char *getWifiSsid() {
        return ssid;
    }
    void setWifiPassword(const char *p) {
        strncpy(password, p, sizeof(password) - 1);
        password[sizeof(password) - 1] = '\0';
    }
    const char *getWifiPassword() {
        return password;
    }
    void setDhcpOn(bool d) {
        dhcpOn = d;
    }
    bool getDhcpOn() {
        return dhcpOn;
    }
    void setStaticIp(unsigned long ip) {
        IP_Fixe = ip;
    }
    unsigned long getStaticIp() {
        return IP_Fixe;
    }
    void setGateway(unsigned long gw) {
        Gateway = gw;
    }
    unsigned long getGateway() {
        return Gateway;
    }
    void setNetmask(unsigned long nm) {
        masque = nm;
    }
    unsigned long getNetmask() {
        return masque;
    }
    void setDns(unsigned long d) {
        dns = d;
    }
} // namespace ModuleWifi