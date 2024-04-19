#include <esp_task_wdt.h> //Pour un Watchdog
#include "ModulePowerMeter.h"
#include "ModulePowerMeterEnphase.h"
#include "ModulePowerMeterUxI.h"
#include "ModulePowerMeterUxIx2.h"
#include "ModulePowerMeterLinky.h"
#include "ModulePowerMeterSmartG.h"
#include "ModulePowerMeterShellyEm.h"
#include "ModulePowerMeterProxy.h"
#include "ModuleTime.h"
#include "helpers.h"
#include "rms.h"
// for variable params ?
// #include <stdarg.h>

// Watchdog de 180 secondes. Le systeme se Reset si pas de dialoque avec le LINKY ou JSY-MK-194T ou Enphase-Envoye pendant 180s
// Watchdog for 180 seconds. The system resets if no dialogue with the Linky or JSY-MK-194T or Enphase-Envoye for 180s
#define RMS_POWER_METER_WDT_TIMEOUT 180

#define RMS_POWER_METER_KV 0.2083
#define RMS_POWER_METER_KI 0.0642

namespace ModulePowerMeter
{
    // external IP for some sources
    unsigned long RMSextIP = 0;

    TaskHandle_t powerMeterLoopTask; // Multicoeur - Processeur 0 - Collecte données RMS local ou distant

    bool EnergieActiveValide = false;
    source_t activeSource = SOURCE_UXI;
    // Real source used after proxy
    source_t activeDataSource = SOURCE_UXI;

    cpu_load_t cpuLoad0;
    unsigned long variableThrottle = 1000;
    unsigned long lastTock;

    float PmaxReseau = 36000;  //Puissance Max pour eviter des débordements
    bool slowSmoothing = false;

    // float EMI_Wh = 0;        //Energie entrée Maison Injecté Wh
    // float EMS_Wh = 0;        //Energie entrée Maison Injecté Wh

    unsigned int CalibU = 1000;  // Calibration Routeur UxI
    unsigned int CalibI = 1000;

    // FIXME: remove?
    const float KV = RMS_POWER_METER_KV;  //Calibration coefficient for the voltage. Value for CalibU=1000 at startup
    const float KI = RMS_POWER_METER_KI;  //Calibration coefficient for the current. Value for CalibI=1000 at startup

    float kV = RMS_POWER_METER_KV;  //Calibration coefficient for the voltage. Corrected value
    float kI = RMS_POWER_METER_KI;  //Calibration coefficient for the current. Corrected value

    electric_data_t elecData[2];

    void setup() {

        init_puissance();
        //Adaptation à la Source
        Serial.println("Source : " + String(getSourceName()));

        activeDataSource = activeSource;
        switch (activeSource)
        {
        case SOURCE_UXI:
            ModulePowerMeterUxI::setup();
            break;
        case SOURCE_UXIX2:
            ModulePowerMeterUxIx2::setup();
            break;
        case SOURCE_LINKY:
            ModulePowerMeterLinky::setup();
            break;
        case SOURCE_ENPHASE:
            ModulePowerMeterEnphase::setup();
            break;
        case SOURCE_SMARTG:
            ModulePowerMeterSmartG::setup();
            break;
        case SOURCE_SHELLYEM:
            ModulePowerMeterShellyEm::setup();
            break;
        case SOURCE_PROXY:
            ModulePowerMeterProxy::setup();
            activeDataSource = ModulePowerMeterProxy::getProxySource();
            break;
        }

        // Watchdog initialisation
        // enable panic so ESP32 restarts
        esp_task_wdt_init(RMS_POWER_METER_WDT_TIMEOUT, true);

    }

    void startPowerMeterLoop() {
        unsigned long msNow = millis();
        // init timers
        lastTock = msNow;
        cpuLoad0.lastTock = msNow;

        xTaskCreatePinnedToCore(  //Préparation Tâche Multi Coeur
            powerMeterLoopCallback,        /* Task function. */
            "powerMeterLoopCallback",      /* name of task. */
            10000,                  /* Stack size of task */
            NULL,                   /* parameter of the task */
            10,                     /* priority of the task */
            &powerMeterLoopTask,    /* Task handle to keep track of created task */
            0);                     /* pin task to core 0 */
    }

    /* **********************
    * ****************** *
    * * Tâches Coeur 0 * *
    * ****************** *
    **********************
    */

    void powerMeterLoopCallback(void *pvParameters) {
        esp_task_wdt_add(NULL);  //add current thread to WDT watch
        ping();
        for (;;) {
            unsigned long msNow = millis();
            estimate_cpu_load(msNow, &cpuLoad0);
            powerMeterLoop(msNow);
        }
    }

    // Signal task loop that we are still alive and OK
    void ping() {
        esp_task_wdt_reset();
    }

    void signalSourceValid() {
        EnergieActiveValide = true;
    }

    void powerMeterLoop(unsigned long msNow) {
        //Recupération des données RMS
        //******************************
        if (TICKTOCK(msNow, lastTock, variableThrottle)) {  //Attention delicat pour eviter pb overflow
            unsigned long ralenti = long(elecData[DOMAIN_HOUSE].powerIn / 10);  // On peut ralentir échange sur Wifi si grosse puissance en cours
            switch (activeSource) {
            case SOURCE_UXI:
                ModulePowerMeterUxI::gauge(msNow);
                throttle(40);
                break;
            case SOURCE_UXIX2:
                ModulePowerMeterUxIx2::gauge(msNow);
                throttle(400);
                break;
            case SOURCE_LINKY:
                ModulePowerMeterLinky::gauge(msNow);
                throttle(2);
                break;
            case SOURCE_ENPHASE:
                ModulePowerMeterEnphase::gauge(msNow);
                // On s'adapte à la vitesse réponse Envoye-S metered
                throttle(200 + ralenti, true);
                break;
            case SOURCE_SMARTG:
                ModulePowerMeterSmartG::gauge(msNow);
                // On s'adapte à la vitesse réponse SmartGateways
                throttle(200 + ralenti, true);
                break;
            case SOURCE_SHELLYEM:
                ModulePowerMeterShellyEm::gauge(msNow);
                // On s'adapte à la vitesse réponse ShellyEm
                throttle(200 + ralenti, true);
                break;
            case SOURCE_PROXY:
                ModulePowerMeterProxy::gauge(msNow);
                // Après pour ne pas surcharger Wifi
                throttle(200 + ralenti, true);
                break;
            }
        }
        delay(2);
    }

    // some modules have additional stuff to do in the loop
    void loop(unsigned long msLoop) {
        switch (activeSource)
        {
        case SOURCE_ENPHASE:
            ModulePowerMeterEnphase::loop(msLoop);
            break;
        }
    }

    //*************
    //* Test Pmax *
    //*************
    float PMax(float Pin) {
        float P = max(-PmaxReseau, Pin);
        P = min(PmaxReseau, P);
        return P;
    }

    int PMax(int Pin) {
        int M = int(PmaxReseau);
        int P = max(-M, Pin);
        P = min(M, P);
        return P;
    }

    long PMax(long Pin) {
        long M = long(PmaxReseau);
        long P = max(-M, Pin);
        P = min(M, P);
        return P;
    }
    
    void powerFilter()
    {
        // Filtre RC

        // Coef pour un lissage en multi-sinus et train de sinus sur les mesures de puissance courte
        float S = slowSmoothing ? 0.3 : 1.0;

        for (int i = DOMAIN_TRIAC; i <= DOMAIN_HOUSE; i++) {
            powerSmoothing(S, elecData[i].instPowerIn, elecData[i].instPowerOut, elecData[i].avgPower, elecData[i].powerIn, elecData[i].powerOut);
            powerSmoothing(S, elecData[i].instVaPowerIn, elecData[i].instVaPowerOut, elecData[i].avgVaPower, elecData[i].vaPowerIn, elecData[i].vaPowerOut);
        }
    }

    void powerSmoothing(float S, float instPowerIn, float instPowerOut, float &avgPower, int &powerIn, int &powerOut) {
        avgPower = S * (instPowerIn - instPowerOut) + (1.0 - S) * avgPower;
        if (avgPower < 0) {
            powerOut = -int(avgPower); // Puissance Watt affichée en entier
            powerIn = 0;
        } else {
            powerOut = 0;
            powerIn = int(avgPower);
        }
    }

    void init_puissance() {
        for (int i = DOMAIN_TRIAC; i <= DOMAIN_HOUSE; i++) {
            elecData[i].avgPower = 0.0;
            elecData[i].avgVaPower = 0.0;
            elecData[i].powerIn = 0;
            elecData[i].powerOut = 0; // Puissance Watt affichée en entiers Maison et Triac
            elecData[i].vaPowerIn = 0;
            elecData[i].vaPowerOut = 0; // Puissance VA affichée en entiers Maison et Triac
            elecData[i].instPowerIn = 0.0;
            elecData[i].instPowerOut = 0.0;
            elecData[i].instVaPowerIn = 0.0;
            elecData[i].instVaPowerOut = 0.0;
        }
    }

    const cpu_load_t *getCpuLoad0() {
        return &cpuLoad0;
    }

    void resetCpuLoad0() {
        cpuLoad0.lastTock = millis();
        cpuLoad0.min = 1000;
        // FIXME: why 1 not 0?
        cpuLoad0.max = 1;
        cpuLoad0.avg = 1;
    }

    void throttle(unsigned long throttle, bool yield = false) {
        if (yield) {
            lastTock = millis();
        }
        variableThrottle = throttle;
    }

    // getters / setters
    const source_t getSource() {
        return activeSource;
    }
    void setSourceByName(const char *name) {
        source_t source = getSourceFromName(name);
        setSource(source);
    }
    void setSource(const source_t source) {
        if (source == activeSource) {
            return;
        }
        activeSource = source;
        if (activeSource == SOURCE_PROXY) {
            activeDataSource = SOURCE_NONE;
        } else {
            activeDataSource = source;
        }
    }
    const char *getSourceName() {
        return sourceNames[(int) activeSource];
    }
    const source_t getDataSource() {
        return activeDataSource;
    }
    const char *getDataSourceName() {
        return sourceNames[(int) activeDataSource];
    }
    void setExtIp(unsigned long ip) {
        RMSextIP = ip;
    }
    unsigned long getExtIp() {
        return RMSextIP;
    }
    void setCalibU(unsigned short calibU) {
        CalibU = calibU;
        kV = RMS_POWER_METER_KV * float(calibU) / 1000.0; // Calibration coefficient to be applied
    }
    unsigned short getCalibU() {
        return RMS_POWER_METER_KV * 1000;
    }
    void setCalibI(unsigned short calibI) {
        CalibI = calibI;
        kI = RMS_POWER_METER_KI * float(calibI) / 1000.0; // Calibration coefficient to be applied
    }
    unsigned short getCalibI() {
        return RMS_POWER_METER_KI * 1000;
    }
    void setSlowSmoothing(bool smoothing) {
        slowSmoothing = smoothing;
    }
    bool getSlowSmoothing() {
        return slowSmoothing;
    }
    electric_data_t *getElectricData(domain_t domain = DOMAIN_HOUSE) {
        return &elecData[domain];
    }
    float get_kV() {
        return kV;
    }
    float get_kI() {
        return kI;
    }

    // states
    bool sourceIsValid() {
        return EnergieActiveValide;
    }
    float getPower(domain_t domain = DOMAIN_HOUSE) {
        return float(elecData[domain].powerIn - elecData[domain].powerOut);
    }
    float getVAPower(domain_t domain = DOMAIN_HOUSE) {
        return float(elecData[domain].vaPowerIn - elecData[domain].vaPowerOut);
    }
    float getEnergy(domain_t domain = DOMAIN_HOUSE) {
        return float(elecData[domain].energyIn - elecData[domain].energyOut);
    }
    float inPower(domain_t domain = DOMAIN_HOUSE) {
        return float(elecData[domain].powerIn);
    }
    float outPower(domain_t domain = DOMAIN_HOUSE) {
        return float(elecData[domain].powerOut);
    }
    float inVAPower(domain_t domain = DOMAIN_HOUSE) {
        return float(elecData[domain].vaPowerIn);
    }
    float outVAPower(domain_t domain = DOMAIN_HOUSE) {
        return float(elecData[domain].vaPowerOut);
    }

    // helpers
    const source_t getSourceFromName(const char *name) {
        for (int i = 0; i < sizeof(sourceNames) / sizeof(char*); i++) {
            if (strcmp(name, sourceNames[i]) == 0) {
                return (source_t) i;
            }
        }
        return SOURCE_ERROR;
    }

    float electric_data_t::setInstPower(float power) {
        power = PMax(power);
        if (power < 0)
        {
            instPowerIn = 0;
            instPowerOut = int(-power);
        }
        else
        {
            instPowerIn = int(power);
            instPowerOut = 0;
        }
        return power;
    }

    float electric_data_t::setInstVaPower(float power) {
        power = PMax(power);
        if (power < 0)
        {
            instVaPowerIn = 0;
            instVaPowerOut = int(-power);
        }
        else
        {
            instVaPowerIn = int(power);
            instVaPowerOut = 0;
        }
        return power;
    }

    // handlers
    void httpAjaxRMS(WebServer& server, String& S)
    {
        // Envoi des dernières données  brutes reçues du RMS
        S = "";
        
        const char *sourceName = getSourceName();

        if (activeSource == ModulePowerMeter::SOURCE_PROXY)
        {
            ModulePowerMeterProxy::httpAjaxRMS(server, S);
            return;
        }

        String RS = RMS_RS;
        time_t now = time(NULL);
        String DATE = String(ts2str(now, "%Y-%m-%d %H:%M:%S"));
        S = DATE + RS + String(sourceName);
        switch (activeSource)
        {
        case ModulePowerMeter::SOURCE_UXI:
            ModulePowerMeterUxI::httpAjaxRMS(server, S);
            break;

        case ModulePowerMeter::SOURCE_UXIX2:
            ModulePowerMeterUxIx2::httpAjaxRMS(server, S);
            break;

        case ModulePowerMeter::SOURCE_LINKY:
            ModulePowerMeterLinky::httpAjaxRMS(server, S);
            break;

        case ModulePowerMeter::SOURCE_ENPHASE:
            ModulePowerMeterEnphase::httpAjaxRMS(server, S);
            break;

        case ModulePowerMeter::SOURCE_SMARTG:
            ModulePowerMeterSmartG::httpAjaxRMS(server, S);
            break;

        case ModulePowerMeter::SOURCE_SHELLYEM:
            ModulePowerMeterShellyEm::httpAjaxRMS(server, S);
            break;
        }
    }
} // namespace ModulePowerMeter