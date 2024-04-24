#pragma once

#include <Arduino.h>
#include "ModuleServer.h"
#include "helpers.h"

#define RMS_POWER_METER_SOURCE_NONE "*NONE*"
#define RMS_POWER_METER_SOURCE_ENPHASE "Enphase"
#define RMS_POWER_METER_SOURCE_LINKY "Linky"
#define RMS_POWER_METER_SOURCE_PROXY "Proxy"
#define RMS_POWER_METER_SOURCE_SHELLYEM "ShellyEm"
#define RMS_POWER_METER_SOURCE_SMARTG "SmartG"
#define RMS_POWER_METER_SOURCE_UXI "UxI"
#define RMS_POWER_METER_SOURCE_UXIX2 "UxIx2"
#define RMS_POWER_METER_SOURCE_ERROR "*ERROR*"

namespace ModulePowerMeter
{
    enum source_t {
        SOURCE_NONE = 0,
        SOURCE_ENPHASE,
        SOURCE_LINKY,
        SOURCE_PROXY,
        SOURCE_SHELLYEM,
        SOURCE_SMARTG,
        SOURCE_UXI,
        SOURCE_UXIX2,
        SOURCE_ERROR,
    };
    enum domain_t {
        DOMAIN_TRIAC,
        DOMAIN_HOUSE,
    };

    struct electric_data_t {
        float voltage; // Volt
        float current; // Amp
        float powerFactor;
        float avgPower;
        float avgVaPower;
        float frequency; // Used for Triac only ?

        int powerIn, powerOut;  // Puissance en Watt 
        int vaPowerIn, vaPowerOut;  // Puissance en VA
        float instPowerIn, instPowerOut;  // Puissance instantanée en Watt
        float instVaPowerIn, instVaPowerOut;  // Puissance instantanée en VA

        float setInstPower(float power);
        float setInstVaPower(float power);

        // historized data
        unsigned long energyIn, energyOut;
        unsigned long energyDayIn, energyDayOut;
    };

    // events
    void boot();
    void startPowerMeterLoop();
    void resetCpuLoad0();
    void loop(unsigned long msLoop);
    void dayIsGone();
    // Allow loop to breathe
    void ping();
    void throttle(unsigned long throttle, bool yield = false);
    void signalSourceValid(); // signal that the active source is valid

    // getters / setters
    const source_t getSource();
    const char *getSourceName();
    const source_t getDataSource();
    const char *getDataSourceName();
    void setSourceByName(const char *name);
    void setExtIp(unsigned long ip);
    unsigned long getExtIp();
    void setCalibU(unsigned short calibU);
    unsigned short getCalibU();
    void setCalibI(unsigned short calibI);
    unsigned short getCalibI();
    void setSlowSmoothing(bool smoothing);
    bool getSlowSmoothing();
    electric_data_t *getElectricData(domain_t domain = DOMAIN_HOUSE);
    float get_kV();
    float get_kI();

    // states
    const cpu_load_t *getCpuLoad0();
    bool sourceIsValid();
    
    // power in Watt, < 0 if power is produced/injected, > 0 if power is consumed
    float getPower(domain_t domain = DOMAIN_HOUSE);
    // apparent power in VA
    float getVAPower(domain_t domain = DOMAIN_HOUSE);
    float getEnergy(domain_t domain = DOMAIN_HOUSE);

    float inPower(domain_t domain = DOMAIN_HOUSE);
    float outPower(domain_t domain = DOMAIN_HOUSE);
    float inVAPower(domain_t domain = DOMAIN_HOUSE);
    float outVAPower(domain_t domain = DOMAIN_HOUSE);

    // helpers
    const source_t getSourceFromName(const char *name);
    float PMax(float Pin);
    int PMax(int Pin);
    long PMax(long Pin);
    void powerFilter();

    // handlers
    void httpAjaxRMS(AsyncWebServerRequest* request, String& S);
} // namespace ModulePowerMeter