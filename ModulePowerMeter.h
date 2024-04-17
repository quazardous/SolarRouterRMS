#pragma once

#include <Arduino.h>
#include "helpers.h"

#define RMS_POWER_METER_SOURCE_ENPHASE "Enphase"
#define RMS_POWER_METER_SOURCE_LINKY "Linky"
#define RMS_POWER_METER_SOURCE_PROXY "Proxy"
#define RMS_POWER_METER_SOURCE_SHELLYEM "ShellyEm"
#define RMS_POWER_METER_SOURCE_SMARTG "SmartG"
#define RMS_POWER_METER_SOURCE_UXI "UxI"
#define RMS_POWER_METER_SOURCE_UXIX2 "UxIx2"
#define RMS_POWER_METER_SOURCE_ERROR "ERROR"

namespace ModulePowerMeter
{
    enum source_t {
        SOURCE_ENPHASE,
        SOURCE_LINKY,
        SOURCE_PROXY,
        SOURCE_SHELLYEM,
        SOURCE_SMARTG,
        SOURCE_UXI,
        SOURCE_UXIX2,
        SOURCE_ERROR,
    };

    const char *sourceNames[] = {
        RMS_POWER_METER_SOURCE_ENPHASE,
        RMS_POWER_METER_SOURCE_LINKY,
        RMS_POWER_METER_SOURCE_PROXY,
        RMS_POWER_METER_SOURCE_SHELLYEM,
        RMS_POWER_METER_SOURCE_SMARTG,
        RMS_POWER_METER_SOURCE_UXI,
        RMS_POWER_METER_SOURCE_UXIX2,
        RMS_POWER_METER_SOURCE_ERROR
    };

    // events
    void setup();
    void startPowerMeterLoop();
    void resetCpuLoad0();
    void loop(unsigned long msLoop);
    // Allow loop to breathe
    void ping();
    void throttle(unsigned long throttle, bool yield = false);
    void signalSourceValid(); // signal that the active source is valid

    // getters / setters
    const source_t getSource();
    const char *getSourceName();
    void setSourceByName(const char *name);
    void setExtIp(unsigned long ip);
    unsigned long getExtIp();
    void setCalibU(unsigned short calibU);
    unsigned short getCalibU();
    void setCalibI(unsigned short calibI);
    unsigned short getCalibI();
    void setSlowSmoothing(bool smoothing);
    bool getSlowSmoothing();

    // states
    const cpu_load_t *getCpuLoad0();
    bool sourceIsValid();
    
    float getPower(bool house = true);
    
    float getVAPower(bool house = true);

    enum domain_t {
        DOMAIN_TRIAC,
        DOMAIN_HOUSE,
    };
    enum direction_t {
        SENS_IN, // power is consumed
        SENS_OUT, // power is produced
    };
    // power in Watt, < 0 if power is produced/injected, > 0 if power is consumed
    float getPower(domain_t domain = DOMAIN_HOUSE);
    // apparent power in VA
    float getVAPower(domain_t domain = DOMAIN_HOUSE);
    float inPower(domain_t domain = DOMAIN_HOUSE);
    float outPower(domain_t domain = DOMAIN_HOUSE);
    float inVAPower(domain_t domain = DOMAIN_HOUSE);
    float outVAPower(domain_t domain = DOMAIN_HOUSE);

    // helpers
    const source_t getSourceFromName(const char *name);
    float PMax(float Pin);
    int PMax(int Pin);
} // namespace ModulePowerMeter