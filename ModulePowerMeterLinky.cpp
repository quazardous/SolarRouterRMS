// ****************************
// * Source de Mesures LINKY  *
// ****************************
#include "ModulePowerMeter.h"
#include "ModulePowerMeterLinky.h"
#include "ModuleHardware.h"
#include "ModuleDebug.h"
#include "ModuleEDF.h"
#include "hardware.h"

namespace ModulePowerMeterLinky
{

    float deltaWS = 0;
    float deltaWI = 0;
    int boucle_appel_Linky = 0;

    //Parameters for Linky
    bool LFon = false;
    bool EASTvalid = false;
    bool EAITvalid = false;
    volatile int IdxDataRawLinky = 0;
    volatile int IdxBufDecodLinky = 0;
    volatile char DataRawLinky[10000];  //Buffer entrée données Linky
    float moyPWS = 0;
    float moyPWI = 0;
    float moyPVAS = 0;
    float moyPVAI = 0;
    float COSphiS = 1;
    float COSphiI = 1;
    long TlastEASTvalide = 0;
    long TlastEAITvalide = 0;

    //Port Serie 2 - Remplace Serial2 qui bug
    HardwareSerial MySerial(2);

    void setup()
    {
        delay(20);
        MySerial.setRxBufferSize(RMS_SER_BUF_SIZE);
        MySerial.begin(9600, SERIAL_7E1, RMS_PIN_RXD2, RMS_PIN_TXD2); //  7-bit Even parity 1 stop bit pour le Linky
        delay(100);
    }

    // Lecture port série du LINKY
    void gauge(unsigned long msLoop)
    {
        int V = 0;
        long OldWh = 0;
        float deltaWh = 0;
        float Pmax = 0;
        float Pmin = 0;
        unsigned long Tm = 0;
        float deltaT = 0;
        boucle_appel_Linky++;
        if (boucle_appel_Linky > 4000)
        {
            ModuleDebug::getDebug().println(boucle_appel_Linky);
            boucle_appel_Linky = 0;
            MySerial.flush();
            MySerial.write("Ok");
            ModuleDebug::stockMessage("Attente Linky 4000 boucles = 8s");
            Serial.println("Attente Linky 4000 boucles = 8s");
        }
        while (MySerial.available() > 0)
        {
            boucle_appel_Linky = 0;
            V = MySerial.read();
            DataRawLinky[IdxDataRawLinky] = char(V);
            IdxDataRawLinky = (IdxDataRawLinky + 1) % 10000;
            switch (V)
            {
            case 2:
                // STX (Start Text)
                break;
            case 3:
                // ETX (End Text)
                // previousETX = millis();
                ModuleHardware::resetConnectivityLed();
                LFon = false;
                break;
            case 10:
                // Line Feed. Debut Groupe
                LFon = true;
                IdxBufDecodLinky = IdxDataRawLinky;
                break;
            case 13:
                ModulePowerMeter::electric_data_t *elecDataHouse = ModulePowerMeter::getElectricData();
                // Line Feed. Debut Groupe
                if (LFon)
                {
                    // Debut groupe OK
                    LFon = false;
                    int nb_tab = 0;
                    String code = "";
                    String val = "";
                    int checksum = 0;
                    int checkLinky = -1;

                    while (IdxBufDecodLinky != IdxDataRawLinky)
                    {
                        if (DataRawLinky[IdxBufDecodLinky] == char(9))
                        {
                            // Tabulation
                            nb_tab++;
                        }
                        else
                        {
                            if (nb_tab == 0)
                            {
                                code += DataRawLinky[IdxBufDecodLinky];
                            }
                            if (nb_tab == 1)
                            {
                                val += DataRawLinky[IdxBufDecodLinky];
                            }
                            if (nb_tab <= 1)
                            {
                                checksum += (int)DataRawLinky[IdxBufDecodLinky];
                            }
                        }
                        IdxBufDecodLinky = (IdxBufDecodLinky + 1) % 10000;
                        if (checkLinky == -1 && nb_tab == 2)
                        {
                            checkLinky = (int)DataRawLinky[IdxBufDecodLinky];
                            checksum += 18;           // 2 tabulations
                            checksum = checksum & 63; // 0x3F
                            checksum = checksum + 32; // 0x20
                        }
                    }
                    if (code.indexOf("EAST") == 0 || code.indexOf("EAIT") == 0 || code == "SINSTS" || code.indexOf("SINSTI") == 0)
                    {
                        if (checksum != checkLinky)
                        {
                            ModuleDebug::getDebug().println("Erreur checksum code : " + code + " " + String(checksum) + "," + String(checkLinky));
                            ModuleDebug::stockMessage("Erreur checksum code : " + code + " " + String(checksum) + "," + String(checkLinky));
                        }
                        else
                        {
                            if (code.indexOf("EAST") == 0)
                            {

                                OldWh = elecDataHouse->energyIn;
                                if (OldWh == 0)
                                {
                                    OldWh = val.toInt();
                                }
                                elecDataHouse->energyIn = val.toInt();
                                Tm = millis();
                                deltaT = float(Tm - TlastEASTvalide);
                                deltaT = deltaT / float(3600000);
                                if (elecDataHouse->energyIn == OldWh)
                                {
                                    // Pas de resultat en Wh
                                    Pmax = 1.3 / deltaT;
                                    moyPWS = min(moyPWS, Pmax);
                                }
                                else
                                {
                                    TlastEASTvalide = Tm;
                                    deltaWh = float(elecDataHouse->energyIn - OldWh);
                                    deltaWS = deltaWh / deltaT;
                                    Pmin = (deltaWh - 1) / deltaT;
                                    moyPWS = max(moyPWS, Pmin); // saut à la montée en puissance
                                }
                                moyPWS = 0.05 * deltaWS + 0.95 * moyPWS;
                                EASTvalid = true;
                                if (!EAITvalid && Tm > 8000)
                                { 
                                    // Cas des CACSI ou EAIT n'est jamais positionné
                                    EAITvalid = true;
                                }
                            }
                            if (code.indexOf("EAIT") == 0)
                            {
                                OldWh = elecDataHouse->energyOut;
                                if (OldWh == 0)
                                {
                                    OldWh = val.toInt();
                                }
                                elecDataHouse->energyOut = val.toInt();
                                Tm = millis();
                                deltaT = float(Tm - TlastEAITvalide);
                                deltaT = deltaT / float(3600000);
                                if (elecDataHouse->energyOut == OldWh)
                                { 
                                    // Pas de resultat en Wh
                                    Pmax = 1.3 / deltaT;
                                    moyPWI = min(moyPWI, Pmax);
                                }
                                else
                                {
                                    TlastEAITvalide = Tm;
                                    deltaWh = float(elecDataHouse->energyOut - OldWh);
                                    deltaWI = deltaWh / deltaT;
                                    Pmin = (deltaWh - 1) / deltaT;
                                    moyPWI = max(moyPWI, Pmin); // saut à la montée en puissance
                                }
                                moyPWI = 0.05 * deltaWI + 0.95 * moyPWI;
                                EAITvalid = true;
                            }
                            if (EASTvalid && EAITvalid)
                            {
                                ModulePowerMeter::signalSourceValid();
                            }
                            if (code == "SINSTS")
                            {
                                // Puissance apparente soutirée. Egalité pour ne pas confondre avec SINSTS1 (triphasé)
                                elecDataHouse->vaPowerIn = ModulePowerMeter::PMax(int(val.toInt()));
                                moyPVAS = 0.05 * float(elecDataHouse->vaPowerIn) + 0.95 * moyPVAS;
                                moyPWS = min(moyPWS, moyPVAS);
                                if (moyPVAS > 0)
                                {
                                    COSphiS = moyPWS / moyPVAS;
                                    COSphiS = min(float(1.0), COSphiS);
                                    elecDataHouse->powerFactor = COSphiS;
                                }
                                elecDataHouse->powerIn = ModulePowerMeter::PMax(int(COSphiS * float(elecDataHouse->vaPowerIn)));
                            }
                            if (code.indexOf("SINSTI") == 0)
                            {
                                // Puissance apparente injectée
                                elecDataHouse->vaPowerOut = ModulePowerMeter::PMax(int(val.toInt()));
                                moyPVAI = 0.05 * float(elecDataHouse->vaPowerOut) + 0.95 * moyPVAI;
                                moyPWI = min(moyPWI, moyPVAI);
                                if (moyPVAI > 0)
                                {
                                    COSphiI = moyPWI / moyPVAI;
                                    COSphiI = min(float(1.0), COSphiI);
                                    elecDataHouse->powerFactor = COSphiI;
                                }
                                elecDataHouse->powerOut = ModulePowerMeter::PMax(int(COSphiI * float(elecDataHouse->vaPowerOut)));
                            }
                        }
                    }
                    if (code.indexOf("DATE") == 0)
                    {
                        // Reset du Watchdog à chaque trame du Linky reçue
                        ModulePowerMeter::ping();
                    }
                    if (code.indexOf("URMS1") == 0)
                    {
                        elecDataHouse->voltage = val.toFloat(); // phase 1 uniquement
                    }
                    if (code.indexOf("IRMS1") == 0)
                    {
                        elecDataHouse->current = val.toFloat(); // Phase 1 uniquement
                    }
                    if (!ModuleEDF::getTempo())
                    {
                        // On prend tarif sur Linky
                        if (code.indexOf("LTARF") == 0)
                        {
                            String LTARF = val; // Option Tarifaire
                            LTARF.trim();
                            ModuleEDF::setLTARF(LTARF.c_str());
                        }
                        if (code.indexOf("STGE") == 0)
                        {
                            String STGE = val; // Status
                            STGE.trim();
                            STGE = STGE.substring(1, 2); // Tempo lendemain et jour sur 1 octet
                            ModuleEDF::setSTGE(STGE.c_str());
                        }
                    }
                }
                break;
            default:
                break;
            }
        }
    }

} // namespace ModulePowerMeterLinky