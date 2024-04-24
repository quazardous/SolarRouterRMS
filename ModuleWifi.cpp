#include "ModuleWifi.h"
#include "ModuleDebug.h"
#include "ModuleCore.h"
#include "ModuleHardware.h"
#include "ModuleEeprom.h"
#include "config.h"
#include "rms.h"

void wifiUp();

namespace ModuleWifi
{
    void wifiSteps(); // wifi connect steps (in loop)
    IPAddress ip2ip(unsigned long ip);
    bool startScanWifi();
    bool checkScanWifi();

    unsigned long staticIp = 0;
    unsigned long gateway = 0;
    unsigned long netmask = 4294967040;
    unsigned long dns = 0;
    byte dhcpOn = 1;

    char ssid[33] = RMS_WIFI_SSID;
    char password[64] = RMS_WIFI_KEY;

    unsigned int WIFIbug = 0;

    wifi_step_t wifiStep = WIFI_STEP_BOOT;

    unsigned long lastWifiStepsTock = 0;
    unsigned long lastCheckTock = 0;
    unsigned long beginWifiConnect = 0;
    unsigned long beginWifiAP = 0;
    unsigned long lastWifiScan = 0;
    unsigned long lastCheckWifiScan = 0;
    unsigned long doWifiScan = 0;
    bool scanningWifi = false;

    void boot() {
        const char *hostname = ModuleCore::getHostname();
        // Configure WIFI
        // **************
        WiFi.setHostname(hostname);
        ModuleCore::log(String("Hostname: ") + String(hostname));

        // WIFI
        if (strlen(ssid) > 0) {
            ModuleCore::log(String("SSID: ") + String(ssid));
            ModuleCore::log(String("Pass: ") + String(password));
        } else {
            ModuleCore::log("No Wifi SSID");
        }

    }

    void loopTimer(unsigned long mtsNow) {
        lastWifiStepsTock = mtsNow;
        lastCheckTock = mtsNow;
    }

    void loop(unsigned long msLoop) {
        unsigned long msNow = millis();

        if (wifiStep != WIFI_STEP_STA_FINAL && wifiStep != WIFI_STEP_AP_FINAL) {
            // try to init WiFi by steps
            if (TICKTOCK(msNow, lastWifiStepsTock, 100)) {
                wifiSteps();
            }
            return;
        }

        // Async WiFi scan
        if (lastWifiScan < doWifiScan) {
            if (scanningWifi) {
                if (TICKTOCK(msNow, lastCheckWifiScan, 1000))
                {
                    scanningWifi = checkScanWifi();
                    if (!scanningWifi) {
                        // scanning done
                        lastWifiScan = millis();
                    }
                }
            } else {
                scanningWifi = startScanWifi();
            }
        }

        if (TICKTOCK(msNow, lastCheckTock, 30000))
        {
            // Check WIFI
            // int wifiBug = ModuleWifi::getWifiBug();
            // //Test présence WIFI toutes les 30s et autres
            // if (!ModuleWifi::canConnectWifi(10000)) {
            //     if (wifiBug > 2) {
            //         reboot("No WIFI !!!", 1000);
            //         return;
            //     }
            // }

            if (WiFi.getMode() == WIFI_STA) {
                // Wifi status
                String m = "IP address: " + String(WiFi.localIP());
                ModuleCore::log(m);
                ModuleDebug::getDebug().println(m);
                m = "WIFI signal:" + String(WiFi.RSSI());
                ModuleCore::log(m);
                ModuleDebug::getDebug().println(m);

                if (!WiFi.isConnected()) {
                    ModuleCore::log("WIFI: Not connected");
                    ModuleDebug::getDebug().println("WIFI: Not connected");
                }
                // m = "WIFIbug:" + String(wifiBug);
                // ModuleCore::log(m);
                // ModuleDebug::getDebug().println(m);
            } else {
                ModuleCore::log("AP Mode. IP address: " + WiFi.softAPIP().toString());
            }

            if (wifiStep == WIFI_STEP_AP_FINAL && (!scanningWifi) && (doWifiScan == 0 || (doWifiScan < lastWifiScan && (millis() - doWifiScan) > 300000))) {
                // AP mode: doing wifi scan every 5 minutes
                ModuleCore::log("WIFI: AP mode => auto wifi scan");
                doWifiScan = millis();
            }

            if (wifiStep == WIFI_STEP_AP_FINAL && strlen(ssid) > 0 && (millis() - beginWifiAP > 300000)) {
                // AP mode for 5 minutes but we have a SSID => trying STA mode again (maybe WIFI router was down or something)
                ModuleCore::log("WIFI: AP mode for 5 minutes but we have a SSID => trying STA mode again");
                wifiStep = WIFI_STEP_BOOT;
            }
        }


    }

    // states
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

    // helpers
    bool startScanWifi() {
        ModuleCore::log("WIFI: Starting WiFi scan");
        WiFi.scanNetworks(true, false, false, 2000U);
        return true;
    }

    // Check if scan is done (true: running, false: done/failed)
    bool checkScanWifi() {
        int status = WiFi.scanComplete();
        if (status == WIFI_SCAN_RUNNING) {
            return true;
        }
        if (status == WIFI_SCAN_FAILED) {
            ModuleCore::log("WIFI: Scan failed");
            return false;
        }
        ModuleCore::log("WIFI: Scan done in " + String((millis() - doWifiScan) / 1000) + " seconds");
        if (status == 0) {
            ModuleCore::log("WIFI: No networks found :/");
        } else {
            ModuleCore::log("WIFI: " + String(status) + " networks found");
            for (int i = 0; i < status; ++i) {
                ModuleCore::log(" - " + WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ")");
            }
        }
        return false;
    }

    void wifiSteps() {
        switch (wifiStep)
        {
            case WIFI_STEP_BOOT:
                if (strlen(ssid) > 0) {
                    ModuleCore::log("Wifi Step BOOT: SSID OK => STA mode");
                    wifiStep = WIFI_STEP_STA;
                } else {
                    ModuleCore::log("Wifi Step BOOT: no SSDI => AP mode");
                    wifiStep = WIFI_STEP_AP;
                }
                break;
            case WIFI_STEP_STA:
                ModuleCore::log("Wifi Step STA: waiting STA mode");
                if (WiFi.getMode() != WIFI_STA) {
                    ModuleCore::log("Switching to STA mode");
                    WiFi.mode(WIFI_STA);
                }
                wifiStep = WIFI_STEP_STA_WAITING_STA;
                break;
            case WIFI_STEP_STA_WAITING_STA:
                ModuleCore::log("Wifi Step STA WAITING STA: waiting STA mode");
                if (WiFi.getMode() == WIFI_STA) {
                    wifiStep = WIFI_STEP_STA_BEGIN;
                }
                break;
            case WIFI_STEP_STA_BEGIN:
                ModuleCore::log("Wifi Step STA BEGIN: begin WiFi");
                if (dhcpOn == 0) {  //Static IP
                    // Set your Static IP address
                    IPAddress ip_staticIp = ip2ip(staticIp);
                    ModuleCore::log("Static IP: " + ip_staticIp.toString());
                    // Set your gateway IP address
                    IPAddress ip_gateway = ip2ip(gateway);
                    ModuleCore::log("Gateway: " + ip_gateway.toString());
                    // Set your netmask/subnet IP address
                    IPAddress ip_netmask = ip2ip(netmask);
                    ModuleCore::log("Netmask: " + ip_netmask.toString());
                    // Set your DNS IP address
                    IPAddress ip_dns1 = ip2ip(dns); //optional
                    ModuleCore::log("DNS1: " + ip_dns1.toString());
                    IPAddress ip_dns2(8, 8, 4, 4); //optional
                    ModuleCore::log("DNS2: " + ip_dns2.toString());

                    if (!WiFi.config(ip_staticIp, ip_gateway, ip_netmask, ip_dns1, ip_dns2)) {
                        ModuleCore::log("WIFI: Failed to configure Static IP");
                    }
                }
                WiFi.begin(ssid, password);
                wifiStep = WIFI_STEP_STA_WAITING_CONNECT;
                beginWifiConnect = millis();
                break;
            case WIFI_STEP_STA_WAITING_CONNECT:
                ModuleCore::log("Wifi Step STA WAITING CONNECT: waiting connect (status=" + String(WiFi.status()) + ")");
                if (WiFi.status() == WL_CONNECTED) {
                    wifiStep = WIFI_STEP_STA_CONNECT;
                } else if (millis() - beginWifiConnect > 15000) {
                    ModuleCore::log("Wifi Step STA WAITING CONNECT: timeout");
                    wifiStep = WIFI_STEP_STA_CONNECT_TIMEOUT;
                }
                ModuleHardware::Gestion_LEDs();
                break;
            case WIFI_STEP_STA_CONNECT:
                ModuleCore::log("Wifi Step STA CONNECT: connected");
                ModuleDebug::stockMessage("WIFI: Connected IP address: " + WiFi.localIP().toString());
                wifiStep = WIFI_STEP_STA_FINAL;
                ::wifiUp();
                break;
            case WIFI_STEP_STA_CONNECT_TIMEOUT:
                ModuleCore::log("Wifi Step STA CONNECT TIMEOUT: going into AP mode");
                wifiStep = WIFI_STEP_AP;
                break;
            case WIFI_STEP_AP:
                ModuleCore::log("Wifi Step AP: waiting AP STA mode");
                if (WiFi.getMode() != WIFI_AP_STA) {
                    ModuleCore::log("Switching to AP STA mode");
                    // Go into software AP and STA modes.
                    WiFi.mode(WIFI_AP_STA);
                }
                wifiStep = WIFI_STEP_AP_WAITING_AP_STA;
                break;
            case WIFI_STEP_AP_WAITING_AP_STA:
                ModuleCore::log("Wifi Step AP WAITING AP STA: waiting AP STA mode");
                if (WiFi.getMode() == WIFI_AP_STA) {
                    wifiStep = WIFI_STEP_AP_BEGIN;
                }
                break;
            case WIFI_STEP_AP_BEGIN:
            {
                ModuleCore::log("Wifi Step AP BEGIN: begin AP");
                const char *ap_default_ssid = ModuleCore::getHostname(); // Mode Access point  IP: 192.168.4.1
                const char *ap_default_psk = NULL; // Pas de mot de passe en AP,
                ModuleCore::log("SSID: " + String(ap_default_ssid));
                if (WiFi.softAP(ap_default_ssid, ap_default_psk)) {
                    ModuleCore::log("Access Point Mode. IP address: " + WiFi.softAPIP().toString());
                    wifiStep = WIFI_STEP_AP_FINAL;
                    beginWifiAP = millis();
                    ModuleCore::log("WIFI: auto scan WiFi networks");
                    doWifiScan = millis();
                    ::wifiUp();
                } else {
                    ModuleCore::log("WIFI: Failed to configure AP");
                    wifiStep = WIFI_STEP_BOOT;
                }
                break;
            }
        }
    }
    
    void resetApTimout() {
        beginWifiAP = millis();
    }

    IPAddress ip2ip(unsigned long ip) {
        byte arr[4];
        ip_explode(ip, arr);
        return IPAddress(arr[3], arr[2], arr[1], arr[0]);
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
        staticIp = ip;
    }
    unsigned long getStaticIp() {
        return staticIp;
    }
    void setGateway(unsigned long gw) {
        gateway = gw;
    }
    unsigned long getGateway() {
        return gateway;
    }
    void setNetmask(unsigned long nm) {
        netmask = nm;
    }
    unsigned long getNetmask() {
        return netmask;
    }
    void setDns(unsigned long d) {
        dns = d;
    }
    unsigned long getDns() {
        return dns;
    }

    // web handlers
    void httpAjaxScanWifi(AsyncWebServerRequest* request, String& S) {
        String RS = RMS_RS;
        String GS = RMS_GS;
        resetWifiBug();

        // WiFi.scanComplete will return the number of networks found.
        int n = WiFi.scanComplete();
        Serial.println("Scan done");
        S = "";
        if (n < 0)
        {
            ModuleCore::log("WIFI: scan not available");
            return;
        }
        for (int i = 0; i < n; ++i)
        {
            S += WiFi.SSID(i).c_str() + RS + WiFi.RSSI(i) + GS;
        }
    }

    void httpUpdateWifi(AsyncWebServerRequest* request, String& S) {
        String RS = RMS_RS;
        String GS = RMS_GS;
        ModuleWifi::resetWifiBug();
        Serial.println("Set Wifi");
        String NewSsid = request->arg("ssid");
        NewSsid.trim();
        String NewPassword = request->arg("passe");
        NewPassword.trim();
        Serial.println(NewSsid);
        Serial.println(NewPassword);
        setWifiSsid(NewSsid.c_str());
        setWifiPassword(NewPassword.c_str());
        ModuleDebug::stockMessage("Wifi Begin : " + NewSsid);
        WiFi.begin(ssid, password);
        unsigned long newstartMillis = millis();
        while (WiFi.status() != WL_CONNECTED && (millis() - newstartMillis < 15000))
        {
            // Attente connexion au Wifi
            Serial.write('.');
            ModuleHardware::Gestion_LEDs();
            Serial.print(WiFi.status());
            delay(300);
        }
        S = "";
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
            String IP = WiFi.localIP().toString();
            S = "Ok" + RS;
            S += "ESP 32 connecté avec succès au wifi : " + NewSsid + " avec l'adresse IP : " + IP;
            S += "<br><br> Connectez vous au wifi : " + NewSsid;
            S += "<br><br> Cliquez sur l'adresse : <a href='http://" + IP + "' >http://" + IP + "</a>";
            ModuleEeprom::writeEeprom();
        }
        else
        {
            S = "No" + RS + "ESP32 non connecté à :" + ssid + "<br>";
        }
    }
} // namespace ModuleWifi