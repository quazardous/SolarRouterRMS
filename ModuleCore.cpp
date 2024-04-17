#include "ModuleCore.h"
#include "ModuleWifi.h"
#include "ModuleDebug.h"
#include "ModulePowerMeter.h"
#include "config.h"

#define RMS_MAX_NAME_LENGTH 128

namespace ModuleCore {

    cpu_load_t cpuLoad1;

    char hostname[255] = RMS_HOSTNAME_PREFIX;
    char routerName[RMS_MAX_NAME_LENGTH] = RMS_ROUTER_NAME;
    char mobileProbeName[RMS_MAX_NAME_LENGTH] = RMS_MOBILE_PROBE_NAME;
    char fixProbeName[RMS_MAX_NAME_LENGTH] = RMS_FIX_PROBE_NAME;
    char temperatureName[RMS_MAX_NAME_LENGTH] = RMS_TEMPERATURE_NAME;

    unsigned long previousCheckTock;

    unsigned long startMillis; // Start time

    void setup() {
        // Generate hostname
        uint32_t chipId = 0;
        for (int i = 0; i < 17; i = i + 8) {
            chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
        }
        strcat(hostname, String(chipId).c_str()); //Add chip ID to hostname

        // Ports Série ESP
        Serial.begin(115200);
        Serial.println("Booting");

        // esp_task_wdt_reset();

        //Timers
        // previousETX = millis();
    }

    void loopTimer(unsigned long mtsNow) {
        cpuLoad1.lastTock = mtsNow;
        // we will trigger the first check in 5s
        previousCheckTock = mtsNow - 25000;

        startMillis = mtsNow;
    }

    void loop(unsigned long msLoop) {
        unsigned long msNow = millis();
        // Estimation charge coeur
        estimate_cpu_load(msNow, &cpuLoad1);

        // Check health
        if (TICKTOCK(msNow, previousCheckTock, 30000))
        {
            // Check WIFI
            int wifiBug = ModuleWifi::getWifiBug();
            //Test présence WIFI toutes les 30s et autres
            if (!ModuleWifi::canConnectWifi(10000)) {
                if (wifiBug > 2) {
                    reboot("No WIFI !!!", 1000);
                    return;
                }
            }

            String m;
            if (!ModuleWifi::isStationMode()) {
                Serial.println("Access Point Mode. IP address: " + String(WiFi.softAPIP()));
            } else {

                // Wifi status
                m = "IP address: " + String(WiFi.localIP());
                Serial.println(m);
                ModuleDebug::getDebug().println(m);
                m = "Niveau Signal WIFI:" + String(WiFi.RSSI());
                Serial.println(m);
                ModuleDebug::getDebug().println(m);
                m = "WIFIbug:" + String(wifiBug);
                Serial.println(m);
                ModuleDebug::getDebug().println(m);

                // Display CPU load
                const cpu_load_t *cpuLoad0 = ModulePowerMeter::getCpuLoad0();
                m = "RMS loop (Core 0) in ms - Min : " + String(int(cpuLoad0->min)) 
                    + " Avg : " + String(int(cpuLoad0->avg)) 
                    + "  Max : " + String(int(cpuLoad0->max));
                Serial.println(m);
                ModuleDebug::getDebug().println(m);

                m = "Main Loop (Core 1) in ms - Min : " + String(int(cpuLoad1.min)) 
                    + " Avg : " + String(int(cpuLoad1.avg)) 
                    + "  Max : " + String(int(cpuLoad1.max));
                Serial.println(m);
                ModuleDebug::getDebug().println(m);
            }
            int T = int(millis() / 1000);
            float DureeOn = float(T) / 3600;
            m = "ESP32 ON: " + String(DureeOn) + " hours";
            Serial.println(m);
            ModuleDebug::getDebug().println(m);
            // call to EDF data put in dedicated module
        }

        // Connecté en  Access Point depuis 3mn. Pas normal
        if (getStartupSince() > 180000 && !ModuleWifi::isStationMode()) {
            reboot("Not in WiFi Station since 3 minutes...", 5000);
            return;
        }
    }

    void reboot(String $m = "", int $delay = 0) {
        if ($m.length() > 0) {
            Serial.println($m);
        }
        Serial.println("REBOOT !!!");
        if ($delay > 0) {
            delay($delay);
        }
        ESP.restart();
    }

    unsigned long getStartupSince(unsigned long mtsNow = 0) {
        if (mtsNow == 0) {
            mtsNow = millis();
        }
        return mtsNow - startMillis;
    }

    const char *getHostname() {
        return hostname;
    }

    // setters / getters
    void setRouterName(const char *n) {
        strncpy(routerName, n, sizeof(routerName) - 1);
        routerName[sizeof(routerName) - 1] = '\0';
    }
    const char *getRouterName() {
        return routerName;
    }
    void setMobileProbeName(const char *n) {
        strncpy(mobileProbeName, n, sizeof(mobileProbeName) - 1);
        mobileProbeName[sizeof(mobileProbeName) - 1] = '\0';
    }
    const char *getMobileProbeName() {
        return mobileProbeName;
    }
    void setFixProbeName(const char *n) {
        strncpy(fixProbeName, n, sizeof(fixProbeName) - 1);
        fixProbeName[sizeof(fixProbeName) - 1] = '\0';
    }
    const char *getFixProbeName() {
        return fixProbeName;
    }
    void setTemperatureName(const char *n) {
        strncpy(temperatureName, n, sizeof(temperatureName) - 1);
        temperatureName[sizeof(temperatureName) - 1] = '\0';
    }
    const char *getTemperatureName() {
        return temperatureName;
    }

    // states
    const cpu_load_t *getCpuLoad1() {
        return &cpuLoad1;
    }

} // namespace ModuleCore