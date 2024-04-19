// ***************************
// Stockage des données en ROM
// ***************************
#include "ModuleStockage.h"
#include <EEPROM.h> //Librairie pour le stockage en EEPROM historique quotidien
#include "ModuleDebug.h"
#include "ModulePowerMeter.h"
#include "ModuleTriggers.h"
#include "ModuleSensor.h"
#include "ModuleCore.h"
#include "ModuleTime.h"
#include "ModuleMQTT.h"
#include "ModuleRomMap.h"
#include "ModuleElemMap.h"
#include "helpers.h"
#include "rms.h"

namespace ModuleStockage
{
    void INIT_EEPROM();
    unsigned long LectureCle();
    void readHisto();
    void LectureConsoMatinJour();
    void EnergieQuotidienne();
    void RAZ_Histo_Conso();

    String GS = RMS_GS;  //Group Separator
    String RS = RMS_RS;  //Record Separator

    long histoEnergy[RMS_EEPROM_HISTO_RANGE];
    unsigned long Cle_ROM;
    unsigned long previousHistoryTock;
    unsigned long previousTimer2sMillis;

    byte currentEepromUsage; // percentage of EEPROM used

    int idxPromDuJour = 0;
    // int adr_debut_para = 0;  //Adresses Para après le Wifi
    long EAS_T_J0 = 0;
    long EAI_T_J0 = 0;
    long EAS_M_J0 = 0;  //Debut du jour energie active
    long EAI_M_J0 = 0;

    int IdxStock2s = 0;
    int IdxStockPW = 0;

    int tabPw_Maison_5mn[RMS_POWER_HISTORY_SIZE_5MIN];  //Puissance Active:Soutiré-Injecté toutes les 5mn
    int tabPw_Triac_5mn[RMS_POWER_HISTORY_SIZE_5MIN];
    int tabTemperature_5mn[RMS_POWER_HISTORY_SIZE_5MIN];
    int tabPw_Maison_2s[RMS_POWER_HISTORY_SIZE_2SEC];   //Puissance Active: toutes les 2s
    int tabPw_Triac_2s[RMS_POWER_HISTORY_SIZE_2SEC];    //Puissance Triac: toutes les 2s
    int tabPva_Maison_2s[RMS_POWER_HISTORY_SIZE_2SEC];  //Puissance Active: toutes les 2s
    int tabPva_Triac_2s[RMS_POWER_HISTORY_SIZE_2SEC];

    void setup()
    {
        INIT_EEPROM();
        // Lecture Clé pour identifier si la ROM a déjà été initialisée
        Cle_ROM = RMS_EEPROM_KEY;
        unsigned long Rcle = LectureCle();
        Serial.println("cle : " + String(Rcle));
        if (Rcle == Cle_ROM)
        {
            // Programme déjà executé
            readHisto();
            LectureEnROM();
            LectureConsoMatinJour();
            ModuleTriggers::resetGpioActions();
        }
        else
        {
            RAZ_Histo_Conso();
        }
    }

    void loopTimer(unsigned long msNow)
    {
        // we will trigger the first history save in 10 seconds
        previousHistoryTock = msNow - 290000;
        previousTimer2sMillis = msNow;
    }

    void loop(unsigned long msLoop)
    {
        // Archivage et envois des mesures périodiquement
        //**********************************************
        if (!ModulePowerMeter::sourceIsValid())
            return;

        unsigned long msNow = millis();

        
        // Historique consommation par pas de 5mn
        if (TICKTOCK(msNow, previousHistoryTock, 300000))
        {
            tabPw_Maison_5mn[IdxStockPW] = ModulePowerMeter::getPower(ModulePowerMeter::DOMAIN_HOUSE);
            tabPw_Triac_5mn[IdxStockPW] = ModulePowerMeter::getPower(ModulePowerMeter::DOMAIN_TRIAC);
            tabTemperature_5mn[IdxStockPW] = int(ModuleSensor::getTemperature());
            IdxStockPW = (IdxStockPW + 1) % RMS_POWER_HISTORY_SIZE_5MIN;
        }

        // Historique consommation par pas de 2s
        if (TICKTOCK(msNow, previousTimer2sMillis, 2000))
        {
            tabPw_Maison_2s[IdxStock2s] = ModulePowerMeter::getPower(ModulePowerMeter::DOMAIN_HOUSE);
            tabPw_Triac_2s[IdxStock2s] = ModulePowerMeter::getPower(ModulePowerMeter::DOMAIN_TRIAC);
            tabPva_Maison_2s[IdxStock2s] = ModulePowerMeter::getVAPower(ModulePowerMeter::DOMAIN_HOUSE);
            tabPva_Triac_2s[IdxStock2s] = ModulePowerMeter::getVAPower(ModulePowerMeter::DOMAIN_TRIAC);
            IdxStock2s = (IdxStock2s + 1) % RMS_POWER_HISTORY_SIZE_2SEC;
            ModuleMQTT::envoiAuMQTT();
            ModuleTime::JourHeureChange();
            EnergieQuotidienne();
        }
    }

    void INIT_EEPROM()
    {
        if (!EEPROM.begin(RMS_EEPROM_MAX_SIZE))
        {
            ModuleCore::reboot("Failed to initialise EEPROM", 10000);
        }
    }

    // called if the day changes (end of the day)
    void onNewDay()
    {
        if (!ModulePowerMeter::sourceIsValid())
            return;

        ModulePowerMeter::electric_data_t *elecDataTriac = ModulePowerMeter::getElectricData(ModulePowerMeter::DOMAIN_TRIAC);
        ModulePowerMeter::electric_data_t *elecDataHouse = ModulePowerMeter::getElectricData(ModulePowerMeter::DOMAIN_HOUSE);

        // Données recues
        idxPromDuJour = (idxPromDuJour + 1 + RMS_EEPROM_HISTO_RANGE) % RMS_EEPROM_HISTO_RANGE;
        // On enregistre les conso en début de journée pour l'historique de l'année
        long energie = ModulePowerMeter::getEnergy(); // Bilan energie du jour

        const char *JourCourant = ModuleTime::getJourCourant();
        EEPROM.writeLong(RMS_EEPROM_OFFSET_HISTO + idxPromDuJour * sizeof(long), energie);
        histoEnergy[idxPromDuJour] = energie;
        EEPROM.writeULong(RMS_EEPROM_OFFSET_TRIAC_ENERGY_IN, long(elecDataTriac->energyIn));
        EEPROM.writeULong(RMS_EEPROM_OFFSET_TRIAC_ENERGY_OUT, long(elecDataTriac->energyOut));
        EEPROM.writeULong(RMS_EEPROM_OFFSET_HOUSE_ENERGY_IN, long(elecDataHouse->energyIn));
        EEPROM.writeULong(RMS_EEPROM_OFFSET_HOUSE_ENERGY_OUT, long(elecDataHouse->energyOut));
        EEPROM.writeString(RMS_EEPROM_OFFSET_TODAY, JourCourant);
        EEPROM.writeUShort(RMS_EEPROM_OFFSET_HISTO_IDX, idxPromDuJour);
        EEPROM.commit();
        LectureConsoMatinJour();
    }

    void RAZ_Histo_Conso()
    {
        // Mise a zero Zone stockage
        int address = RMS_EEPROM_OFFSET_HISTO;
        for (int i = 0; i < RMS_EEPROM_HISTO_RANGE; i++)
        {
            EEPROM.writeLong(address, 0);
            histoEnergy[i] = 0;
            address += sizeof(long);
        }
        EEPROM.writeULong(RMS_EEPROM_OFFSET_TRIAC_ENERGY_IN, 0);
        EEPROM.writeULong(RMS_EEPROM_OFFSET_TRIAC_ENERGY_OUT, 0);
        EEPROM.writeULong(RMS_EEPROM_OFFSET_HOUSE_ENERGY_IN, 0);
        EEPROM.writeULong(RMS_EEPROM_OFFSET_HOUSE_ENERGY_OUT, 0);
        EEPROM.writeString(RMS_EEPROM_OFFSET_TODAY, "");
        EEPROM.writeUShort(RMS_EEPROM_OFFSET_HISTO_IDX, 0);
        EEPROM.commit();
    }

    void LectureConsoMatinJour()
    {
        EAS_T_J0 = EEPROM.readULong(RMS_EEPROM_OFFSET_TRIAC_ENERGY_IN); // Triac
        EAI_T_J0 = EEPROM.readULong(RMS_EEPROM_OFFSET_TRIAC_ENERGY_OUT);
        EAS_M_J0 = EEPROM.readULong(RMS_EEPROM_OFFSET_HOUSE_ENERGY_IN); // Maison
        EAI_M_J0 = EEPROM.readULong(RMS_EEPROM_OFFSET_HOUSE_ENERGY_OUT);
        String DateCeJour = EEPROM.readString(RMS_EEPROM_OFFSET_TODAY);
        ModuleTime::setDateCeJour(DateCeJour.c_str());
        idxPromDuJour = EEPROM.readUShort(RMS_EEPROM_OFFSET_HISTO_IDX);

        ModulePowerMeter::electric_data_t *elecDataTriac = ModulePowerMeter::getElectricData(ModulePowerMeter::DOMAIN_TRIAC);
        ModulePowerMeter::electric_data_t *elecDataHouse = ModulePowerMeter::getElectricData(ModulePowerMeter::DOMAIN_HOUSE);

        // Check if data are OK (reboot ESP, etc)
        if (elecDataTriac->energyIn < EAS_T_J0)
        {
            elecDataTriac->energyIn = EAS_T_J0;
        }
        if (elecDataTriac->energyOut < EAI_T_J0)
        {
            elecDataTriac->energyOut = EAI_T_J0;
        }
        if (elecDataHouse->energyIn < EAS_M_J0)
        {
            elecDataHouse->energyIn = EAS_M_J0;
        }
        if (elecDataHouse->energyOut < EAI_M_J0)
        {
            elecDataHouse->energyOut = EAI_M_J0;
        }
    }

    void readHisto()
    {
        // put histo in RAM
        int address = RMS_EEPROM_OFFSET_HISTO;
        for (int i = 0; i < RMS_EEPROM_HISTO_RANGE; i++)
        {
            histoEnergy[i] = EEPROM.readLong(address);
            address += sizeof(int32_t);
        }
    }

    unsigned long LectureCle()
    {
        return EEPROM.readULong(RMS_EEPROM_OFFSET_PARAMS);
    }

    void eepromUsage(int address)
    {
        // done in ModulePowerMeter setters
        // kV = KV * CalibU / 1000; // Calibration coefficient to be applied
        // kI = KI * CalibI / 1000;
        int size = address - RMS_EEPROM_OFFSET_HISTO;
        currentEepromUsage = int(100.0 * size / RMS_EEPROM_MAX_SIZE);
        String m = "EEPROM usage : " + String(currentEepromUsage) + "%" + " (" + String(size) + "/" + String(RMS_EEPROM_MAX_SIZE) + " bytes)";
        Serial.println(m);
        ModuleDebug::getDebug().println(m);
    }

    int LectureEnROM()
    {
        int address = RMS_EEPROM_OFFSET_PARAMS;
        address = ModuleRomMap::readParameters(address);
        eepromUsage(address);
        return address;
    }
    int EcritureEnROM()
    {
        int address = RMS_EEPROM_OFFSET_PARAMS;
        address = ModuleRomMap::writeParameters(address, false);
        eepromUsage(address);
        EEPROM.commit();
        return address;
    }

    // ***********************************
    // * Calage Zéro Energie quotidienne * -
    // ***********************************

    void EnergieQuotidienne()
    {
        if (!ModuleTime::timeIsValid())
            return;

        if (ModulePowerMeter::getSource() == ModulePowerMeter::SOURCE_PROXY)
            return;

        ModulePowerMeter::electric_data_t *elecDataTriac = ModulePowerMeter::getElectricData(ModulePowerMeter::DOMAIN_TRIAC);
        ModulePowerMeter::electric_data_t *elecDataHouse = ModulePowerMeter::getElectricData(ModulePowerMeter::DOMAIN_HOUSE);
        
        if (elecDataTriac->energyIn < EAS_T_J0 || EAS_T_J0 == 0)
        {
            EAS_T_J0 = elecDataTriac->energyIn;
        }
        elecDataTriac->energyDayIn = elecDataTriac->energyIn - EAS_T_J0;
        if (elecDataTriac->energyOut < EAI_T_J0 || EAI_T_J0 == 0)
        {
            EAI_T_J0 = elecDataTriac->energyOut;
        }
        elecDataTriac->energyDayOut = elecDataTriac->energyOut - EAI_T_J0;

        if (elecDataHouse->energyIn < EAS_M_J0 || EAS_M_J0 == 0)
        {
            EAS_M_J0 = elecDataHouse->energyIn;
        }
        elecDataHouse->energyDayIn = elecDataHouse->energyIn - EAS_M_J0;
        if (elecDataHouse->energyOut < EAI_M_J0 || EAI_M_J0 == 0)
        {
            EAI_M_J0 = elecDataHouse->energyOut;
        }
        elecDataHouse->energyDayOut = elecDataHouse->energyOut - EAI_M_J0;

    }

    // states
    byte getEepromUsage()
    {
        return currentEepromUsage;
    }

    // setters / getters
    long *getHistoEnergy()
    {
        return histoEnergy;
    }
    int getHistoEnergyIdx()
    {
        return idxPromDuJour;
    }

    void setEepromKey(unsigned long key)
    {
        Cle_ROM = key;
    }
    unsigned long getEepromKey()
    {
        return Cle_ROM;
    }

    // web handlers
    // Envoi Historique de 50h (600points) toutes les 5mn
    void httpAjaxHisto48h(WebServer& server, String& S) {
        String GS = RMS_GS;
        String RS = RMS_RS;
        String HouseH = "";
        String TriacH = "";
        String TempH = "";
        int iS = IdxStockPW;
        for (int i = 0; i < RMS_POWER_HISTORY_SIZE_5MIN; i++)
        {
            HouseH += String(tabPw_Maison_5mn[iS]) + ",";
            TriacH += String(tabPw_Triac_5mn[iS]) + ",";
            TempH += String(tabTemperature_5mn[iS]) + ",";
            iS = (1 + iS) % RMS_POWER_HISTORY_SIZE_5MIN;
        }
        // String Ouverture = "";
        //   for (int i = 0; i < NbActions; i++) {
        //     if ((LesActions[i].Actif > 0) && (ITmode > 0 || i > 0)) {
        //     iS = IdxStockPW;
        //     if (LesActions[i].Actif > 0) {
        //         Ouverture += GS;
        //         for (int j = 0; j < 600; j++) {
        //         Ouverture += String(tab_histo_ouverture[i][iS]) + RS;
        //         iS = (1 + iS) % 600;
        //         }
        //         Ouverture += LesActions[i].Titre;
        //     }
        //     }
        // }
        S = String(ModulePowerMeter::getDataSourceName()) 
            + GS + HouseH
            + GS + TriacH
            + GS + String(ModuleSensor::getTemperature())
            + GS + TempH;
    }

    void httpAjaxHisto10mn(WebServer& server, String& S) {
        // Envoi Historique de 10mn (300points)Energie Active Soutiré - Injecté
        String H = "";
        String T = "";
        int iS = IdxStock2s;
        for (int i = 0; i < RMS_POWER_HISTORY_SIZE_2SEC; i++)
        {
            H += String(tabPw_Maison_2s[iS]) + ",";
            H += String(tabPva_Maison_2s[iS]) + ",";
            T += String(tabPw_Triac_2s[iS]) + ",";
            T += String(tabPva_Triac_2s[iS]) + ",";
            iS = (1 + iS) % RMS_POWER_HISTORY_SIZE_2SEC;
        }
        const char *dataSource = ModulePowerMeter::getDataSourceName();
        S = String(dataSource) + GS + H + GS + T;
    }

    void httpAjaxHistoriqueEnergie1An(WebServer& server, String& S)
    {
        S = "";
        int Adr_SoutInjec = 0;
        long EnergieJour = 0;
        long DeltaEnergieJour = 0;
        int iS = 0;
        long lastDay = 0;

        for (int i = 0; i < RMS_EEPROM_HISTO_RANGE; i++)
        {
            // from today
            iS = (idxPromDuJour + i + 1) % RMS_EEPROM_HISTO_RANGE;
            EnergieJour = histoEnergy[iS];
            if (lastDay == 0)
            {
                lastDay = EnergieJour;
            }
            DeltaEnergieJour = EnergieJour - lastDay;
            lastDay = EnergieJour;
            S += String(DeltaEnergieJour) + ",";
        }
    }

    using ModuleElemMap::elem_map_t;
    using ModuleElemMap::elem_type_t;
    #define RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM, TYPE, Elem, Type) \
        {ModuleElemMap::ELEM, ModuleElemMap:: TYPE, {.set ## Type = ModuleElemMap::elemSet ## Elem}, {.get ## Type = ModuleElemMap::elemGet ## Elem}, 0}
    #define RMS_STOCKAGE_AJAX_PARAM_READ(ELEM, TYPE, Elem, Type) \
        {ModuleElemMap::ELEM, ModuleElemMap:: TYPE, {.set ## Type = NULL}, {.get ## Type = ModuleElemMap::elemGet ## Elem}, 1}

    elem_map_t ajax_params_map[] = {
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_DHCP_ON, TYPE_BOOL, DhcpOn, Bool),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_STATIC_IP, TYPE_ULONG, StaticIp, ULong),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_GATEWAY, TYPE_ULONG, Gateway, ULong),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_NETMASK, TYPE_ULONG, Netmask, ULong),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_DNS, TYPE_ULONG, Dns, ULong),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_SOURCE, TYPE_CSTRING, Source, CString),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_EXT_IP, TYPE_ULONG, ExtIp, ULong),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_ENPHASE_USER, TYPE_CSTRING, EnphaseUser, CString),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_ENPHASE_PWD, TYPE_CSTRING, EnphasePwd, CString),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_ENPHASE_SERIAL, TYPE_ULONG, EnphaseSerial, ULong),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_SHELLYEM_PHASES, TYPE_USHORT, ShellyEmPhases, UShort),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_MQTT_REPEAT, TYPE_USHORT, MqttRepeat, UShort),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_MQTT_IP, TYPE_ULONG, MqttIp, ULong),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_MQTT_PORT, TYPE_USHORT, MqttPort, UShort),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_MQTT_USER, TYPE_CSTRING, MqttUser, CString),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_MQTT_PWD, TYPE_CSTRING, MqttPwd, CString),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_MQTT_PREFIX, TYPE_CSTRING, MqttPrefix, CString),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_MQTT_DEVICE_NAME, TYPE_CSTRING, MqttDeviceName, CString),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_ROUTER_NAME, TYPE_CSTRING, RouterName, CString),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_FIX_PROBE_NAME, TYPE_CSTRING, FixProbeName, CString),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_MOBILE_PROBE_NAME, TYPE_CSTRING, MobileProbeName, CString),
        RMS_STOCKAGE_AJAX_PARAM_READ(ELEM_TEMPERATURE, TYPE_FLOAT, Temperature, Float),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_TEMPERATURE_NAME, TYPE_CSTRING, TemperatureName, CString),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_CALIB_U, TYPE_USHORT, CalibU, UShort),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_CALIB_I, TYPE_USHORT, CalibI, UShort),
        RMS_STOCKAGE_AJAX_PARAM_ELEM(ELEM_TEMPO_EDF_ON, TYPE_BOOL, TempoEdfOn, Bool),
    };
    const int ajax_params_map_size = sizeof(ajax_params_map) / sizeof(elem_map_t);

    void httpAjaxPara(WebServer& server, String& S) {
        
        S = "";
        for (int i = 0; i < ajax_params_map_size; i++)
        {
            if (i > 0) S += RS;
            S += ModuleElemMap::e2s(&ajax_params_map[i]);
        }
    }

    void httpUpdatePara(WebServer& server, String& S) {
        String lesparas = server.arg("lesparas") + RS;
        if (count_chars(lesparas.c_str(), RS[0]) != (ajax_params_map_size - 1))
        {
            S = "ERR";
            return;
        }

        for (int i = 0; i < ajax_params_map_size; i++)
        {
            elem_map_t* e = &ajax_params_map[i];
            if (e->readonly)
                continue;
            String val = lesparas.substring(0, lesparas.indexOf(RS));
            val.trim();
            ModuleElemMap::s2e(e, val);
        }
        int adresse_max = EcritureEnROM();
        S = "OK" + String(adresse_max);
    }
} // namespace ModuleStockage
