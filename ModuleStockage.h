#pragma once

#include <Arduino.h>
#include <WebServer.h>

#define RMS_EEPROM_KEY 812567808 // Valeur pour tester si ROM vierge ou pas. Un changement de valeur remet à zéro toutes les données. / Value to test whether blank ROM or not.

// Plan stockage
#define RMS_EEPROM_MAX_SIZE 4090
#define RMS_EEPROM_HISTO_RANGE 370               // Nb jour historique stocké (x 4 bytes)
#define RMS_EEPROM_OFFSET_HISTO 0                // taille 370*4 = 1480
#define RMS_EEPROM_OFFSET_TRIAC_ENERGY_IN  1480  // 1 long. Taille 4 Triac
#define RMS_EEPROM_OFFSET_TRIAC_ENERGY_OUT 1484  // 1 long. Taille 4.
#define RMS_EEPROM_OFFSET_HOUSE_ENERGY_IN  1488  // 1 long. Taille 4 Maison
#define RMS_EEPROM_OFFSET_HOUSE_ENERGY_OUT 1492  // 1 long. Taille 4
#define RMS_EEPROM_OFFSET_TODAY 1496             // String 8+1 DDMMYYYY
#define RMS_EEPROM_OFFSET_HISTO_IDX 1505         // Short taille 2
#define RMS_EEPROM_OFFSET_PARAMS 1507            // Clé + ensemble parametres peu souvent modifiés

#define RMS_POWER_HISTORY_SIZE_5MIN 600
#define RMS_POWER_HISTORY_SIZE_2SEC 300

/**
 * Stockage / EEPROM management
 */
namespace ModuleStockage
{
    void boot();
    void loopTimer(unsigned long mtsNow);
    void loop(unsigned long msLoop);
    // events
    void onNewDay();

    // helpers
    int LectureEnROM();
    int EcritureEnROM();

    // states
    byte getEepromUsage();

    // setters / getters
    void setEepromKey(unsigned long key);
    unsigned long getEepromKey();
    long *getHistoEnergy();
    int getHistoEnergyIdx();

    // web handlers
    void httpAjaxHisto48h(WebServer& server, String& S);
    void httpAjaxHisto10mn(WebServer& server, String& S);
    void httpAjaxHistoriqueEnergie1An(WebServer& server, String& S);
    void httpAjaxPara(WebServer& server, String& S);
    void httpUpdatePara(WebServer& server, String& S);
} // namespace ModuleStockage
