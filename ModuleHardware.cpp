#include "ModuleHardware.h"
#include "ModuleWifi.h"
#include "ModuleTriggers.h"
#include "helpers.h"
#include "hardware.h"

namespace ModuleHardware
{
    unsigned long mtsLastLEDTock;

    int WifiLedCounter = 0; // Wifi Led Counter aka Yellow Led
    int ActivityLedCounter = 0; // Activity Led Counter aka Green Led

    void setup() {
        // Pin initialisation
        pinMode(RMS_PIN_LED_WIFI, OUTPUT);
        pinMode(RMS_PIN_LED_ACTIVITY, OUTPUT);
        pinMode(RMS_PIN_ZERO_CROSS, INPUT_PULLDOWN);
        pinMode(RMS_PIN_PULSE_TRIAC, OUTPUT);
        digitalWrite(RMS_PIN_LED_WIFI, LOW);
        digitalWrite(RMS_PIN_LED_ACTIVITY, LOW);
        digitalWrite(RMS_PIN_PULSE_TRIAC, LOW);  //Stop Triac
    }

    void loopTimer(unsigned long msNow) {
        mtsLastLEDTock = msNow;
    }
    
    void loop(unsigned long msLoop) {
        unsigned long msNow = millis();
        if (TICKTOCK(msNow, mtsLastLEDTock, 50)) {
            Gestion_LEDs();
        }
    }
    //****************
    //* Gestion LEDs *
    //****************
    void Gestion_LEDs() {
        int retard_min = 100;
        int retardI;
        WifiLedCounter++;

        if (!ModuleWifi::isWifiConnected()) {
            // Attente connexion au Wifi
            if (ModuleWifi::isStationMode()) {
                // en  Station mode
                WifiLedCounter = (WifiLedCounter + 6) % 10;
                ActivityLedCounter = WifiLedCounter;
            } else {
                // AP Mode
                WifiLedCounter = WifiLedCounter % 10;
                ActivityLedCounter = (WifiLedCounter + 5) % 10;
            }
        } else {
            byte count = ModuleTriggers::getTriggersCount();
            for (byte i = 0; i < count; i++) {
                retardI = ModuleTriggers::getDelay(i);
                retard_min = min(retard_min, retardI);
            }
            if (retard_min < 100) {
                ActivityLedCounter = int((ActivityLedCounter + 1 + 8 / (1 + retard_min / 10))) % 10;
            } else {
                ActivityLedCounter = 10;
            }
        }
        if (WifiLedCounter > 5) {
            digitalWrite(RMS_PIN_LED_WIFI, LOW);
        } else {
            digitalWrite(RMS_PIN_LED_WIFI, HIGH);
        }
        if (ActivityLedCounter > 5) {
            digitalWrite(RMS_PIN_LED_ACTIVITY, LOW);
        } else {
            digitalWrite(RMS_PIN_LED_ACTIVITY, HIGH);
        }
    }
} // namespace ModuleHardware