#include "ModuleSensor.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "hardware.h"
#include "ModuleDebug.h"
#include "helpers.h"

namespace ModuleSensor
{
    unsigned long mtsLastTempTock;

    // Température Capteur DS18B20
    OneWire oneWire(RMS_PIN_TEMP);
    DallasTemperature ds18b20(&oneWire);
    float temperature = -127;  // La valeur vaut -127 quand la sonde DS18B20 n'est pas présente

    void setup() {
        //Temperature
        ds18b20.begin();
        gaugeTemperature();
    }

    void loopTimer(unsigned long msNow) {
        mtsLastTempTock = msNow;
    }

    void loop(unsigned long msLoop) {
        unsigned long msNow = millis();
        if (TICKTOCK(msNow, mtsLastTempTock, 30000)) {
            //Temperature
            gaugeTemperature();
        }
    }

    // ***************
    // * Temperature *
    // ***************
    void gaugeTemperature() {
        float temperature_brute = -127;
        ds18b20.requestTemperatures();
        temperature_brute = ds18b20.getTempCByIndex(0);
        if (temperature_brute < -20 || temperature_brute > 130) {  //Invalide. Pas de capteur ou parfois mauvaise réponse
            Serial.print("Mesure Température invalide ");

        } else {
            temperature = temperature_brute;
            Serial.print("Température : ");
            Serial.print(temperature);
            Serial.println("°C");
            ModuleDebug::getDebug().print("Température : ");
            ModuleDebug::getDebug().print(temperature);
            ModuleDebug::getDebug().println("°C");
        }
    }

    float getTemperature() {
        return temperature;
    }
} // namespace ModuleSensor