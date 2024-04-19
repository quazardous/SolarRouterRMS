#include "ModuleHardware.h"
#include "ModuleWifi.h"
#include "ModuleTriggers.h"
#include "helpers.h"
#include "hardware.h"

namespace ModuleHardware
{
    unsigned long mtsLastLEDTock;

    int ConnectivityLedCounter = 0; // Wifi Led Counter aka Yellow Led
    int ActivityLedCounter = 0; // Activity Led Counter aka Green Led

    void boot() {
        // Pin initialisation
        pinMode(RMS_PIN_LED_CONNECTIVITY, OUTPUT);
        pinMode(RMS_PIN_LED_ACTIVITY, OUTPUT);
        pinMode(RMS_PIN_ZERO_CROSS, INPUT_PULLDOWN);
        pinMode(RMS_PIN_PULSE_TRIAC, OUTPUT);
        digitalWrite(RMS_PIN_LED_CONNECTIVITY, LOW);
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
        ConnectivityLedCounter++;

        if (!ModuleWifi::isWifiConnected()) {
            // Attente connexion au Wifi
            if (ModuleWifi::isStationMode()) {
                // en  Station mode
                ConnectivityLedCounter = (ConnectivityLedCounter + 6) % 10;
                ActivityLedCounter = ConnectivityLedCounter;
            } else {
                // AP Mode
                ConnectivityLedCounter = ConnectivityLedCounter % 10;
                ActivityLedCounter = (ConnectivityLedCounter + 5) % 10;
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
        if (ConnectivityLedCounter > 5) {
            digitalWrite(RMS_PIN_LED_CONNECTIVITY, LOW);
        } else {
            digitalWrite(RMS_PIN_LED_CONNECTIVITY, HIGH);
        }
        if (ActivityLedCounter > 5) {
            digitalWrite(RMS_PIN_LED_ACTIVITY, LOW);
        } else {
            digitalWrite(RMS_PIN_LED_ACTIVITY, HIGH);
        }
    }

    // setters / getters
    void setConnectivityLedCounter(int counter) {
        ConnectivityLedCounter = counter;
    }
    int getConnectivityLedCounter() {
        return ConnectivityLedCounter;
    }

    void setActivityLedCounter(int counter) {
        ActivityLedCounter = counter;
    }
    int getActivityLedCounter() {
        return ActivityLedCounter;
    }

    // helpers
    void resetConnectivityLed() {
        if (ConnectivityLedCounter > 30) {
            ConnectivityLedCounter = 4;
        }
    }
} // namespace ModuleHardware