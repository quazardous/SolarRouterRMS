#include "ModuleCore.h"
#include "config.h"

#define RMS_MAX_NAME_LENGTH 128

namespace ModuleCore {

    float previousLoopMin = 1000;
    float previousLoopMax = 0;
    float previousLoopMoy = 0;

    cpu_load_t cpuLoad1;

    char hostname[255] = RMS_HOSTNAME_PREFIX;
    char routerName[RMS_MAX_NAME_LENGTH] = RMS_ROUTER_NAME;
    char mobileProbeName[RMS_MAX_NAME_LENGTH] = RMS_MOBILE_PROBE_NAME;
    char fixProbeName[RMS_MAX_NAME_LENGTH] = RMS_FIX_PROBE_NAME;
    char temperatureName[RMS_MAX_NAME_LENGTH] = RMS_TEMPERATURE_NAME;

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
    }

    void loop(unsigned long msLoop) {
        unsigned long msNow = millis();
        // Estimation charge coeur
        estimate_cpu_load(msNow, &cpuLoad1);
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