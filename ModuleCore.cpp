#include "ModuleCore.h"
#include "ModuleWifi.h"
#include "ModuleDebug.h"
#include "ModuleTime.h"
#include "ModulePowerMeter.h"
#include "ModulePowerMeterShellyEm.h"
#include "ModuleEeprom.h"
#include "ModuleTriggers.h"
#include "ModuleEDF.h"
#include "ModuleSensor.h"
#include "ModuleConfig.h"
#include "config.h"
#include "version.h"
#include "rms.h"
#include "HelperJson.h"

#define RMS_MAX_NAME_LENGTH 128

// void up();
// void down();
void reboot();

namespace ModuleCore {
    // // Not ready if EEPROM is not initialized aka first boot
    // // RMS will be in setup mode (WIFI in AP)
    // bool up = false;
    // // RMS will trigger the UP event
    // bool triggerUp = false;
    // // RMS will trigger the DOWN event
    // bool triggerDown = false;

    unsigned long getStartupSince(unsigned long mtsNow = 0);
    void config();

    cpu_load_t cpuLoad1;

    char hostname[255] = RMS_HOSTNAME_PREFIX;
    char routerName[RMS_MAX_NAME_LENGTH] = RMS_ROUTER_NAME;
    char mobileProbeName[RMS_MAX_NAME_LENGTH] = RMS_MOBILE_PROBE_NAME;
    char fixProbeName[RMS_MAX_NAME_LENGTH] = RMS_FIX_PROBE_NAME;
    char temperatureName[RMS_MAX_NAME_LENGTH] = RMS_TEMPERATURE_NAME;

    unsigned long previousCheckTock;

    unsigned long startMillis; // Start time
    unsigned long gracefulReboot = 0;
    unsigned long rebootEventDispatched = 0;

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
        log("Tick: " + String(portTICK_PERIOD_MS) + " ms");

        config();
    }

    // void upAndReady(bool ready) {
    //     if (ready == up) {
    //         // noop
    //         return;
    //     }
    //     up = ready;
    //     if (ready) {
    //         triggerUp = true;
    //         triggerDown = false;
    //     } else {
    //         triggerDown = true;
    //         triggerUp = false;
    //     }
    // }

    void loopTimer(unsigned long mtsNow) {
        cpuLoad1.lastTock = mtsNow;
        // we will trigger the first check in 5s
        previousCheckTock = mtsNow - 25000;

        startMillis = mtsNow;
    }

    void loop(unsigned long msLoop) {
        if (gracefulReboot > 0 && rebootEventDispatched == 0) {
            rebootEventDispatched = millis();
            // fire reboot event ASAP
            log("Dispatch reboot event");
            ::reboot();
        }
        unsigned long msNow = millis();

        if (gracefulReboot < msNow) {
            log("GRACEFUL REBOOT NOW");
            ESP.restart();
        }
        // Estimation charge coeur
        estimate_cpu_load(msNow, &cpuLoad1);

        // if (triggerUp) {
        //     log("RMS going UP...");
        //     triggerUp = false;
        //     ::up();
        // }

        // if (triggerDown) {
        //     triggerDown = false;
        //     log("RMS going DOWN...");
        //     ::down();
        // }

        // Check health
        if (TICKTOCK(msNow, previousCheckTock, 30000))
        {
            String m;
            // Display CPU load
            const cpu_load_t *cpuLoad0 = ModulePowerMeter::getCpuLoad0();
            m = "RMS loop (Core 0) in ms - Min : " + String(int(cpuLoad0->min)) 
                + " Avg : " + String(int(cpuLoad0->avg)) 
                + "  Max : " + String(int(cpuLoad0->max));
            log(m);

            m = "Main Loop (Core 1) in ms - Min : " + String(int(cpuLoad1.min)) 
                + " Avg : " + String(int(cpuLoad1.avg)) 
                + "  Max : " + String(int(cpuLoad1.max));
            log(m);

            float since = millis() / 1000.0;
            char s[32];
            sprintf(s, "%02d:%02d", int(since / 3600), int((int(since) % 3600) / 60));
            m = "ESP32 running since " + String(s) + " hours";
            log(m);

            log("EEPROM Key: " + String(ModuleEeprom::getEepromKey()));
            // call to EDF data put in dedicated module

            // Check IT / triggers
            ModuleTriggers::checkItStatus();

            // log("RMS is " + String(up ? "UP" : "DOWN"));
        }

        // // Connecté en  Access Point depuis 3mn. Pas normal
        // if (getStartupSince() > 180000 && !ModuleWifi::isStationMode()) {
        //     reboot("Not in WiFi Station since 3 minutes...", 5000);
        //     return;
        // }
    }

    void panic(String m, int msDelay) {
        if (m.length() > 0) {
            Serial.println(m);
        }
        ::reboot();
        if (msDelay > 0) {
            log("REBOOT in " + String(msDelay) + "s");
            delay(msDelay);
        }
        ESP.restart();
    }

    void reboot(String m, int msDelay) {
        unsigned int newRebootTime = millis() + msDelay;
        // allow adding more delay
        if (newRebootTime > gracefulReboot) {
            gracefulReboot = newRebootTime;
        }
        msDelay = gracefulReboot - millis();
        log(String("Graceful reboot in ") + String(msDelay) + " ms: " + m);
    }

    ModuleElem::elem_map_t config_map[] = {
        RMS_CONFIG_ELEM_MAP(GROUP_MAIN, ELEM_ROUTER_NAME, TYPE_CSTRING, CString, setRouterName, getRouterName, NULL, const char*, "Router Name", NULL, NULL),
        RMS_CONFIG_ELEM_MAP(GROUP_MAIN, ELEM_MOBILE_PROBE_NAME, TYPE_CSTRING, CString, setMobileProbeName, getMobileProbeName, NULL, const char*, NULL, NULL, NULL),
        RMS_CONFIG_ELEM_MAP(GROUP_MAIN, ELEM_FIX_PROBE_NAME, TYPE_CSTRING, CString, setFixProbeName, getFixProbeName, NULL, const char*, NULL, NULL, NULL),
        RMS_CONFIG_ELEM_MAP(GROUP_MAIN, ELEM_TEMPERATURE_NAME, TYPE_CSTRING, CString, setTemperatureName, getTemperatureName, NULL, const char*, NULL, NULL, NULL),
    };
    const int config_map_size = sizeof(config_map) / sizeof(ModuleElem::elem_map_t);

    void config() {
        ModuleConfig::registerConfig(config_map, config_map_size);
    }

    // helpers
    void checkup() {
        // check if everything is OK to be UP
        // UP means normal operation mode
    }

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
    // bool isUp() {
    //     return up;
    // }

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
    void httpAjaxESP32(AsyncWebServerRequest* request, String& S) {
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
        S += RS + coeur0 + RS + coeur1 + RS + String(ModuleEeprom::getEepromUsage()) + RS;
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

    void httpAjaxData(AsyncWebServerRequest* request, String& S) {
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

    using ModuleElem::elem_map_t;
    using ModuleElem::elem_type_t;
    #define RMS_CORE_AJAX_PARAM_ELEM(ELEM, TYPE, Elem, Type) \
        {ModuleElem::ELEM, ModuleElem::TYPE, {.set ## Type = ModuleElem::elemSet ## Elem}, {.get ## Type = ModuleElem::elemGet ## Elem}}

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

    void httpAjaxPara(AsyncWebServerRequest* request, String& S) {
        String RS = RMS_RS;
        S = "";
        for (int i = 0; i < ajax_params_map_size; i++)
        {
            if (i > 0) S += RS;
            S += ModuleElem::e2s(&ajax_params_map[i]);
        }
    }

    elem_map_t hello_params_map[] = {
        RMS_CORE_AJAX_PARAM_ELEM(ELEM_EEPROM_KEY, TYPE_ULONG, EepromKey, ULong),
        RMS_CORE_AJAX_PARAM_ELEM(ELEM_SOURCE, TYPE_CSTRING, Source, CString),
        RMS_CORE_AJAX_PARAM_ELEM(ELEM_DATA_SOURCE, TYPE_CSTRING, DataSource, CString),
        RMS_CORE_AJAX_PARAM_ELEM(ELEM_ROUTER_NAME, TYPE_CSTRING, RouterName, CString),
        RMS_CORE_AJAX_PARAM_ELEM(ELEM_FIX_PROBE_NAME, TYPE_CSTRING, FixProbeName, CString),
        RMS_CORE_AJAX_PARAM_ELEM(ELEM_MOBILE_PROBE_NAME, TYPE_CSTRING, MobileProbeName, CString),
        RMS_CORE_AJAX_PARAM_ELEM(ELEM_EXT_IP, TYPE_ULONG, ExtIp, ULong),
        RMS_CORE_AJAX_PARAM_ELEM(ELEM_TEMPERATURE_NAME, TYPE_CSTRING, TemperatureName, CString)
    };
    const int hello_params_map_size = sizeof(hello_params_map) / sizeof(elem_map_t);

    // API handlers
    void apiHello(AsyncWebServerRequest* request, JsonDocument& doc) {
        for (int i = 0; i < hello_params_map_size; i++)
        {
            HelperJson::e2json(doc, &hello_params_map[i]);
        }
    }

    void apiReboot(AsyncWebServerRequest* request, JsonDocument& doc) {
        reboot();
        doc["message"] = "Will reboot soon";
    }

} // namespace ModuleCore