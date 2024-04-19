#include "ModuleCore.h"
#include "ModuleWifi.h"
#include "ModuleDebug.h"
#include "ModuleTime.h"
#include "ModulePowerMeter.h"
#include "ModulePowerMeterShellyEm.h"
#include "ModuleStockage.h"
#include "ModuleTriggers.h"
#include "ModuleEDF.h"
#include "ModuleSensor.h"
#include "ModuleElemMap.h"
#include "config.h"
#include "version.h"
#include "rms.h"

#define RMS_MAX_NAME_LENGTH 128

void up();
void down();

namespace ModuleCore {
    // Not ready if EEPROM is not initialized aka first boot
    // RMS will be in setup mode (WIFI in AP)
    bool up = false;
    // RMS will trigger the UP event
    bool triggerUp = false;
    // RMS will trigger the DOWN event
    bool triggerDown = false;

    unsigned long getStartupSince(unsigned long mtsNow = 0);

    cpu_load_t cpuLoad1;

    char hostname[255] = RMS_HOSTNAME_PREFIX;
    char routerName[RMS_MAX_NAME_LENGTH] = RMS_ROUTER_NAME;
    char mobileProbeName[RMS_MAX_NAME_LENGTH] = RMS_MOBILE_PROBE_NAME;
    char fixProbeName[RMS_MAX_NAME_LENGTH] = RMS_FIX_PROBE_NAME;
    char temperatureName[RMS_MAX_NAME_LENGTH] = RMS_TEMPERATURE_NAME;

    unsigned long previousCheckTock;

    unsigned long startMillis; // Start time

    void boot() {
        // Generate hostname
        uint32_t chipId = 0;
        for (int i = 0; i < 17; i = i + 8) {
            chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
        }
        strcat(hostname, String(chipId).c_str()); //Add chip ID to hostname

        // Ports Série ESP
        Serial.begin(115200);
        log("Booting");
    }

    void upAndReady(bool ready) {
        if (ready == up) {
            // noop
            return;
        }
        up = ready;
        if (ready) {
            triggerUp = true;
            triggerDown = false;
        } else {
            triggerDown = true;
            triggerUp = false;
        }

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

        if (triggerUp) {
            log("RMS going UP...");
            triggerUp = false;
            ::up();
        }

        if (triggerDown) {
            triggerDown = false;
            log("RMS going DOWN...");
            ::down();
        }

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

            // Check IT / triggers
            ModuleTriggers::checkItStatus();

            log("RMS is " + String(up ? "UP" : "DOWN"));
        }

        // Connecté en  Access Point depuis 3mn. Pas normal
        if (getStartupSince() > 180000 && !ModuleWifi::isStationMode()) {
            reboot("Not in WiFi Station since 3 minutes...", 5000);
            return;
        }
    }

    void reboot(String $m, int $delay) {
        if ($m.length() > 0) {
            Serial.println($m);
        }
        Serial.println("REBOOT !!!");
        if ($delay > 0) {
            delay($delay);
        }
        ESP.restart();
    }

    // helpers
    void log(const String &m)
    {
        char d[32];
        sprintf(d, "[%010.3fs] ", millis() / 1000.0);
        String l = String(d);
            l += m;
        Serial.println(l);
    }
    void log(const char *m)
    {
        log(String(m));
    }

    // states
    unsigned long getStartupSince(unsigned long mtsNow) {
        if (mtsNow == 0) {
            mtsNow = millis();
        }
        return mtsNow - startMillis;
    }
    bool isUp() {
        return up;
    }

    // setters / getters
    const char *getHostname() {
        return hostname;
    }

    const char *getVersion() {
        return RMS_VERSION;
    }
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

    // helpers


    // web handlers
    void httpAjaxESP32(WebServer& server, String& S) {
        String GS = RMS_GS;
        String RS = RMS_RS;
        S = "";
        ModuleTriggers::it_counters_infos_s* it_infos = ModuleTriggers::getItCountersInfos(true);
        unsigned long msNow = millis();

        const cpu_load_t *cpuLoad0 = ModulePowerMeter::getCpuLoad0();
        const cpu_load_t *cpuLoad1 = ModuleCore::getCpuLoad1();
        float H = float(msNow) / 3600000;
        String coeur0 = String(int(cpuLoad0->min)) + ", " + String(int(cpuLoad0->avg)) + ", " + String(int(cpuLoad0->max));
        String coeur1 = String(int(cpuLoad1->min)) + ", " + String(int(cpuLoad1->avg)) + ", " + String(int(cpuLoad1->max));
        S += String(H) 
            + RS + WiFi.RSSI() 
            + RS + WiFi.BSSIDstr() 
            + RS + WiFi.macAddress() 
            + RS + String(ModuleWifi::getWifiSsid()) 
            + RS + WiFi.localIP().toString() 
            + RS + WiFi.gatewayIP().toString() 
            + RS + WiFi.subnetMask().toString();
        S += RS + coeur0 + RS + coeur1 + RS + String(ModuleStockage::getEepromUsage()) + RS;
        if (it_infos->triac)
        {
            S += String(it_infos->it_10ms_in) + "/" + String(it_infos->it_10ms);
        }
        else
        {
            S += "Pas de Triac";
        }
        if (it_infos->synchronized)
        {
            S += RS + "Secteur";
        }
        else
        {
            S += RS + "Horloge ESP";
        }
        String *Messages = ModuleDebug::getMessages();
        int j = ModuleDebug::getMessageIdx();
        for (int i = 0; i < RMS_DEBUG_STOCK_MESSAGES; i++)
        {
            S += RS + Messages[j];
            j = (j + 1) % RMS_DEBUG_STOCK_MESSAGES;
        }
    }

    void httpAjaxData(WebServer& server, String& S) {
        String GS = RMS_GS;
        String RS = RMS_RS;
        // Données page d'accueil
        String DateLast = "Attente de l'heure par Internet";
        if (ModuleTime::timeIsValid())
        {
            DateLast = String(ts2str(time(NULL), "%Y-%m-%d %H:%M:%S"));
        }
        ModulePowerMeter::electric_data_t *elecDataHouse = ModulePowerMeter::getElectricData();
        ModulePowerMeter::source_t dataSource = ModulePowerMeter::getDataSource();
        S = "Deb" + RS + DateLast 
            + RS + String(ModulePowerMeter::getDataSourceName()) 
            + RS + String(ModuleEDF::getLTARF()) 
            + RS + String(ModuleEDF::getSTGE())  
            + RS + String(ModuleSensor::getTemperature());
        S += GS + String(elecDataHouse->powerIn) 
            + RS + String(elecDataHouse->powerOut) 
            + RS + String(elecDataHouse->vaPowerIn) 
            + RS + String(elecDataHouse->vaPowerOut);
        S += RS + String(elecDataHouse->energyDayIn) 
            + RS + String(elecDataHouse->energyDayOut) 
            + RS + String(elecDataHouse->energyIn)
            + RS + String(elecDataHouse->energyOut);
        if (dataSource == ModulePowerMeter::SOURCE_UXIX2 || (dataSource == ModulePowerMeter::SOURCE_SHELLYEM && ModulePowerMeterShellyEm::getPhasesNumber() < 3))
        {
            // UxIx2 ou Shelly monophasé avec 2 sondes
            ModulePowerMeter::electric_data_t *elecDataTriac = ModulePowerMeter::getElectricData(ModulePowerMeter::DOMAIN_TRIAC);
            S += GS + String(elecDataTriac->powerIn)
                + RS + String(elecDataTriac->powerOut) 
                + RS + String(elecDataTriac->vaPowerIn) 
                + RS + String(elecDataTriac->vaPowerOut);
            S += RS + String(elecDataTriac->energyDayIn) 
                + RS + String(elecDataTriac->energyDayOut) 
                + RS + String(elecDataTriac->energyIn) 
                + RS + String(elecDataTriac->energyOut);
        }
        S += GS + "Fin";
    }

    using ModuleElemMap::elem_map_t;
    using ModuleElemMap::elem_type_t;
    #define RMS_CORE_AJAX_PARAM_ELEM(ELEM, TYPE, Elem, Type) \
        {ModuleElemMap::ELEM, ModuleElemMap::TYPE, {.set ## Type = ModuleElemMap::elemSet ## Elem}, {.get ## Type = ModuleElemMap::elemGet ## Elem}}

    elem_map_t ajax_params_map[] = {
        RMS_CORE_AJAX_PARAM_ELEM(ELEM_SOURCE, TYPE_CSTRING, Source, CString),
        RMS_CORE_AJAX_PARAM_ELEM(ELEM_DATA_SOURCE, TYPE_CSTRING, DataSource, CString),
        RMS_CORE_AJAX_PARAM_ELEM(ELEM_ROUTER_NAME, TYPE_CSTRING, RouterName, CString),
        RMS_CORE_AJAX_PARAM_ELEM(ELEM_VERSION, TYPE_CSTRING, Version, CString),
        RMS_CORE_AJAX_PARAM_ELEM(ELEM_FIX_PROBE_NAME, TYPE_CSTRING, FixProbeName, CString),
        RMS_CORE_AJAX_PARAM_ELEM(ELEM_MOBILE_PROBE_NAME, TYPE_CSTRING, MobileProbeName, CString),
        RMS_CORE_AJAX_PARAM_ELEM(ELEM_EXT_IP, TYPE_ULONG, ExtIp, ULong),
        RMS_CORE_AJAX_PARAM_ELEM(ELEM_TEMPERATURE_NAME, TYPE_CSTRING, TemperatureName, CString)
    };
    const int ajax_params_map_size = sizeof(ajax_params_map) / sizeof(elem_map_t);

    void httpAjaxPara(WebServer& server, String& S) {
        String RS = RMS_RS;
        S = "";
        for (int i = 0; i < ajax_params_map_size; i++)
        {
            if (i > 0) S += RS;
            S += ModuleElemMap::e2s(&ajax_params_map[i]);
        }
    }

} // namespace ModuleCore