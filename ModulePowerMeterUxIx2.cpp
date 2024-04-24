// *******************************
// * Source de Mesures UI Double *
// *      Capteur JSY-MK-194     *
// *******************************
#include "ModulePowerMeter.h"
#include "ModulePowerMeterUxIx2.h"
#include "ModuleHardware.h"
#include "hardware.h"
#include "rms.h"

namespace ModulePowerMeterUxIx2
{

    //Parameters for JSY-MK-194T module
    byte ByteArray[130];
    long LesDatas[14];
    int Sens_1, Sens_2;

    //Port Serie 2 - Remplace Serial2 qui bug
    HardwareSerial MySerial(2);

    void boot()
    {
        MySerial.setRxBufferSize(RMS_SER_BUF_SIZE);
        MySerial.begin(4800, SERIAL_8N1, RMS_PIN_RXD2, RMS_PIN_TXD2); // PORT DE CONNEXION AVEC LE CAPTEUR JSY-MK-194
    }

    // Ecriture et Lecture port série du JSY-MK-194  .
    void gauge(unsigned long msLoop)
    {

        int i, j;
        byte msg_send[] = {0x01, 0x03, 0x00, 0x48, 0x00, 0x0E, 0x44, 0x18};
        // Demande Info sur le Serial port 2 (Modbus RTU)
        for (i = 0; i < 8; i++)
        {
            MySerial.write(msg_send[i]);
        }

        // Réponse en général à l'appel précédent (seulement 4800bauds)
        int a = 0;
        while (MySerial.available())
        {
            ByteArray[a] = MySerial.read();
            a++;
        }

        ModulePowerMeter::electric_data_t *elecDataHouse = ModulePowerMeter::getElectricData();
        ModulePowerMeter::electric_data_t *elecDataTriac = ModulePowerMeter::getElectricData(ModulePowerMeter::DOMAIN_TRIAC);
        if (a == 61)
        { // Message complet reçu
            j = 3;
            for (i = 0; i < 14; i++)
            { // conversion séries de 4 octets en long
                LesDatas[i] = 0;
                LesDatas[i] += ByteArray[j] << 24;
                j += 1;
                LesDatas[i] += ByteArray[j] << 16;
                j += 1;
                LesDatas[i] += ByteArray[j] << 8;
                j += 1;
                LesDatas[i] += ByteArray[j];
                j += 1;
            }
            Sens_1 = ByteArray[27]; // Sens 1
            Sens_2 = ByteArray[28];

            // Données du Triac
            elecDataTriac->voltage = LesDatas[0] * .0001;
            elecDataTriac->current = LesDatas[1] * .0001;
            float Puiss_1 = ModulePowerMeter::PMax(float(LesDatas[2] * .0001));
            elecDataTriac->energyIn = int(LesDatas[3] * .1);
            elecDataTriac->powerFactor = LesDatas[4] * .001;
            elecDataTriac->energyOut = int(LesDatas[5] * .1);
            elecDataTriac->frequency = LesDatas[7] * .01;
            float PVA1 = 0;
            if (elecDataTriac->powerFactor > 0)
            {
                PVA1 = Puiss_1 / elecDataTriac->powerFactor;
            }
            if (Sens_1 > 0)
            {
                // Injection sur Triac. Ne devrait pas arriver
                elecDataTriac->instPowerIn = 0;
                elecDataTriac->instPowerOut = Puiss_1;
                elecDataTriac->instVaPowerIn = 0;
                elecDataTriac->instVaPowerOut = PVA1;
            }
            else
            {
                elecDataTriac->instPowerIn = Puiss_1;
                elecDataTriac->instPowerOut = 0;
                elecDataTriac->instVaPowerIn = PVA1;
                elecDataTriac->instVaPowerOut = 0;
            }

            // Données générale de la Maison
            elecDataHouse->voltage = LesDatas[8] * .0001;
            elecDataHouse->current = LesDatas[9] * .0001;
            float Puiss_2 = ModulePowerMeter::PMax(float(LesDatas[10] * .0001));
            elecDataHouse->energyIn = int(LesDatas[11] * .1);
            elecDataHouse->powerFactor = LesDatas[12] * .001;
            elecDataHouse->energyOut = int(LesDatas[13] * .1);
            float PVA2 = 0;
            if (elecDataHouse->powerFactor > 0)
            {
                PVA2 = Puiss_2 / elecDataHouse->powerFactor;
            }
            if (Sens_2 > 0)
            {
                // Injection en entrée de Maison
                elecDataHouse->instPowerIn = 0;
                elecDataHouse->instPowerOut = Puiss_2;
                elecDataHouse->instVaPowerIn = 0;
                elecDataHouse->instVaPowerOut = PVA2;
            }
            else
            {
                elecDataHouse->instPowerIn = Puiss_2;
                elecDataHouse->instPowerOut = 0;
                elecDataHouse->instVaPowerIn = PVA2;
                elecDataHouse->instVaPowerOut = 0;
            }
            ModulePowerMeter::powerFilter();
            ModulePowerMeter::signalSourceValid();
            // Reset du Watchdog à chaque trame du module JSY-MK-194 reçue
            ModulePowerMeter::ping();
            ModuleHardware::resetConnectivityLed();
        }
    }

    // web handlers
    void httpAjaxRMS(AsyncWebServerRequest* request, String& S) {
        String RS = RMS_RS;
        String GS = RMS_GS;
        ModulePowerMeter::electric_data_t *elecDataHouse = ModulePowerMeter::getElectricData();
        ModulePowerMeter::electric_data_t *elecDataTriac = ModulePowerMeter::getElectricData(ModulePowerMeter::DOMAIN_TRIAC);
        S += GS + String(elecDataHouse->voltage) 
            + RS + String(elecDataHouse->current) 
            + RS + String(elecDataHouse->powerIn - elecDataHouse->powerOut) 
            + RS + String(elecDataHouse->powerFactor) 
            + RS + String(elecDataHouse->energyIn) 
            + RS + String(elecDataHouse->energyOut);
        S += RS + String(elecDataTriac->voltage) 
            + RS + String(elecDataTriac->current) 
            + RS + String(elecDataTriac->powerIn - elecDataTriac->powerOut) 
            + RS + String(elecDataTriac->powerFactor) 
            + RS + String(elecDataTriac->energyIn) 
            + RS + String(elecDataTriac->energyOut);
        S += RS + String(elecDataTriac->frequency);     
    }
} // namespace ModulePowerMeterUxIx2