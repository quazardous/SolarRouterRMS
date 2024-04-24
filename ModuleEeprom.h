#pragma once

#include <Arduino.h>
#include "ModuleServer.h"

// Valeur pour tester si ROM vierge ou pas.
// Un changement de valeur remet à zéro toutes les données. / Value to test whether blank ROM or not.
#define RMS_EEPROM_KEY 2024042301UL

#define RMS_EEPROM_MAX_SIZE 4090
#define RMS_EEPROM_OFFSET_HEAD 0 // Head of the EEPROM data

// handle migration for historical data from v8.06_rms
#define RMS_EEPROM_LEGACY806_KEY 812567808 // Key of v8.06_rms
#define RMS_EEPROM_LEGACY806_KEY_OFFSET 1507
#define RMS_EEPROM_LEGACY806_HISTO_RANGE 370
#define RMS_EEPROM_LEGACY806_HISTO_OFFSET 0

/**
 * EEPROM management (Data persistence)
 */
namespace ModuleEeprom
{
    void boot();
    void loopTimer(unsigned long mtsNow);
    void loop(unsigned long msLoop);
    // events
    void onNewDay();

    // helpers
    int readEeprom();
    int writeEeprom();
    short readHisto(long* histoEnergy);
    void writeHistoDay(short idxDay, long energy);
    String readToday();
    void writeToday(const String &today);
    void readEnergyData(unsigned long &energyInTriac, unsigned long &energyOutTriac, unsigned long &energyInHouse, unsigned long &energyOutHouse);
    void writeEnergyData(unsigned long energyInTriac, unsigned long energyOutTriac, unsigned long energyInHouse, unsigned long energyOutHouse);

    // states
    // EEPROM has stored data
    bool hasData();
    bool hasLegacy806Data();
    byte getEepromUsage();

    // setters / getters
    void setEepromKey(unsigned long key);
    unsigned long getEepromKey();
    unsigned long getEepromLegacyKey();
    long *getHistoEnergy();
    int getHistoEnergyIdx();

    // web handlers
    void httpAjaxPara(AsyncWebServerRequest* request, String& S);
    void httpUpdatePara(AsyncWebServerRequest* request, String& S);
} // namespace ModuleEeprom
