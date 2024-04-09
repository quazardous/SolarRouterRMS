#include <OneWire.h>
#include <DallasTemperature.h>
#include "hardware.h"
#include "ModuleSensor.h"
#include "ModuleDebug.h"

namespace ModuleSensor
{
    //Température Capteur DS18B20
    OneWire oneWire(PIN_TEMP);
    DallasTemperature ds18b20(&oneWire);
    float temperature = -127;  // La valeur vaut -127 quand la sonde DS18B20 n'est pas présente

    // ***************
    // * Temperature *
    // ***************
    void LectureTemperature() {
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
            ModuleDebug::Debug.print("Température : ");
            ModuleDebug::Debug.print(temperature);
            ModuleDebug::Debug.println("°C");
        }
    }

    void setup() {
        //Temperature
        ds18b20.begin();
        LectureTemperature();
    }

    void loop() {
        //Temperature
        LectureTemperature();
    }
} // namespace ModuleSensor