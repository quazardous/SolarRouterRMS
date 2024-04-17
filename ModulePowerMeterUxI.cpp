// ****************************
// * Source de Mesures U et I *
// *          UXI             *
// ****************************
#include "ModulePowerMeter.h"
#include "ModulePowerMeterUxI.h"
#include "ModuleHardware.h"
#include "ModulePowerMeter.h"

namespace ModulePowerMeterUxI
{

    float EASfloat = 0;
    float EAIfloat = 0;

    const int AnalogIn0 = 35;  //Pour Routeur Uxi
    const int AnalogIn1 = 32;
    const int AnalogIn2 = 33;  //Note: si GPIO 33 non disponible sur la carte ESP32, utilisez la 34. If GPIO 33 not available on the board replace by GPIO 34

    float voltM[100];   // Voltage Mean value
    float ampM[100];
    int volt[100];
    int amp[100];

    void setup()
    {
        for (int i = 0; i < 100; i++)
        { // Reset table measurements
            voltM[i] = 0;
            ampM[i] = 0;
        }
    }

    void gauge(unsigned long msLoop)
    {
        MeasurePower();
        ComputePower();
    }

    void MeasurePower()
    { // Lecture Tension et courants pendant 20ms
        int iStore;
        int value0 = analogRead(AnalogIn0); // Mean value. Should be at 3.3v/2
        unsigned long MeasureMillis = millis();

        while (millis() - MeasureMillis < 21)
        {                                      // Read values in continuous during 20ms. One loop is around 150 micro seconds
            iStore = (micros() % 20000) / 200; // We have more results that we need during 20ms to fill the tables of 100 samples
            volt[iStore] = analogRead(AnalogIn1) - value0;
            amp[iStore] = analogRead(AnalogIn2) - value0;
        }
    }
    void ComputePower()
    {
        ModulePowerMeter::electric_data_t *elecDataHouse = ModulePowerMeter::getElectricData();
        float PWcal = 0; // Computation Power in Watt
        float V;
        float I;
        float Uef2 = 0;
        float Ief2 = 0;
        float kV = ModulePowerMeter::get_kV();
        float kI = ModulePowerMeter::get_kI();
        for (int i = 0; i < 100; i++)
        {
            voltM[i] = (19 * voltM[i] + float(volt[i])) / 20; // Mean value. First Order Filter. Short Integration
            V = kV * voltM[i];
            Uef2 += sq(V);
            ampM[i] = (19 * ampM[i] + float(amp[i])) / 20; // Mean value. First Order Filter
            I = kI * ampM[i];
            Ief2 += sq(I);
            PWcal += V * I;
        }
        Uef2 = Uef2 / 100;        // square of voltage
        elecDataHouse->voltage = sqrt(Uef2);   // RMS voltage
        Ief2 = Ief2 / 100;        // square of current
        elecDataHouse->current = sqrt(Ief2); // RMS current
        PWcal = ModulePowerMeter::PMax(PWcal / 100);
        float PVA = ModulePowerMeter::PMax(floor(elecDataHouse->voltage * elecDataHouse->current));
        float PowerFactor = 0;
        if (PVA > 0)
        {
            PowerFactor = floor(100 * PWcal / PVA) / 100;
        }
        elecDataHouse->powerFactor = PowerFactor;
        if (PWcal >= 0)
        {
            // Watt Hour,Every 40ms. Soutirée
            EASfloat += PWcal / 90000;
            // Watt Hour,Every 40ms. Soutirée
            elecDataHouse->energyIn = int(EASfloat);
            elecDataHouse->instPowerIn = PWcal;
            elecDataHouse->instPowerOut = 0;
            elecDataHouse->instVaPowerIn = PVA;
            elecDataHouse->instVaPowerOut = 0;
        }
        else
        {
            EAIfloat += -PWcal / 90000;
            elecDataHouse->energyOut = int(EAIfloat);
            elecDataHouse->instPowerIn = 0; 
            elecDataHouse->instPowerOut = -PWcal;
            elecDataHouse->instVaPowerIn = 0;
            elecDataHouse->instVaPowerOut = PVA;
        }
        ModulePowerMeter::powerFilter();
        ModulePowerMeter::signalSourceValid();
        ModulePowerMeter::ping();
        ModuleHardware::resetConnectivityLed();
    }

} // namespace ModulePowerMeterUxi