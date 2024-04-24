// ***************************
// Stockage des données en ROM
// ***************************
#include "ModuleEeprom.h"
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
#include "ModuleHistory.h"
#include "helpers.h"
#include "rms.h"

namespace ModuleEeprom
{
    void init();
    unsigned long readEepromKey();
    unsigned long readEepromLegacy806Key();
    void destroy();

    // key read from EEPROM
    unsigned long eepromKey = 0;
    unsigned long legacyKey = 0;

    bool readOnly = true;

    String GS = RMS_GS;  //Group Separator
    String RS = RMS_RS;  //Record Separator

    // fix EEPROM headers
    enum address_map_t
    {
        ADDRESS_MAP_HISTORY = 0,
        ADDRESS_MAP_TRIAC_ENERGY_IN,
        ADDRESS_MAP_TRIAC_ENERGY_OUT,
        ADDRESS_MAP_HOUSE_ENERGY_IN,
        ADDRESS_MAP_HOUSE_ENERGY_OUT,
        ADDRESS_MAP_TODAY,
        ADDRESS_MAP_HISTORY_DAY_IDX,
        ADDRESS_MAP_HISTORY_RANGE, // for future use, should be before HISTORY
        ADDRESS_MAP_PARAMS,
        ADDRESS_MAP__SIZE_, // keep last
    };
    int addressMap[ADDRESS_MAP__SIZE_];

    byte currentEepromUsage; // percentage of EEPROM used

    void boot()
    {
        init();
        unsigned long legacyKey = readEepromLegacy806Key();
        if (legacyKey == RMS_EEPROM_LEGACY806_KEY)
        {
            readOnly = true;
            // legacy data found, migrate to new format
            ModuleCore::log("Found EEPROM data from v8.06rms");
            ModuleCore::log("EEPROM will be read only until migration is done");
            return;
        }
        readOnly = false;
        // Lecture Clé pour identifier si la ROM a déjà été initialisée
        eepromKey = readEepromKey();
        ModuleCore::log("EEPROM KEY: " + String(eepromKey));
        if (eepromKey == RMS_EEPROM_KEY)
        {
            // Programme déjà executé
            // read all stored params
            readEeprom();
        } else {
            ModuleCore::log("EEPROM is empty");
            destroy();
        }
    }

    void loopTimer(unsigned long msNow)
    {
        // nothing yet
    }

    void loop(unsigned long msLoop)
    {
        // nothing yet
    }

    // ***********************************
    // * Calage Zéro Energie quotidienne * -
    // ***********************************

    // does nothing?
    // void EnergieQuotidienne() {
    //     if (DATEvalid && Source != "Ext") {
    //         if (Energie_M_Soutiree < EAS_M_J0 || EAS_M_J0 == 0) {
    //             EAS_M_J0 = Energie_M_Soutiree;
    //         }
    //         EnergieJour_M_Soutiree = Energie_M_Soutiree - EAS_M_J0;
    //         if (Energie_M_Injectee < EAI_M_J0 || EAI_M_J0 == 0) {
    //             EAI_M_J0 = Energie_M_Injectee;
    //         }
    //         EnergieJour_M_Injectee = Energie_M_Injectee - EAI_M_J0;
    //         if (Energie_T_Soutiree < EAS_T_J0 || EAS_T_J0 == 0) {
    //             EAS_T_J0 = Energie_T_Soutiree;
    //         }
    //         EnergieJour_T_Soutiree = Energie_T_Soutiree - EAS_T_J0;
    //         if (Energie_T_Injectee < EAI_T_J0 || EAI_T_J0 == 0) {
    //             EAI_T_J0 = Energie_T_Injectee;
    //         }
    //         EnergieJour_T_Injectee = Energie_T_Injectee - EAI_T_J0;
    //     }
    // }

    void init()
    {
        if (!EEPROM.begin(RMS_EEPROM_MAX_SIZE))
        {
            ModuleCore::reboot("Failed to initialise EEPROM", 10000);
        }
        // init address map
        int address = RMS_EEPROM_OFFSET_HEAD;
        addressMap[ADDRESS_MAP_HISTORY] = address;
        address += RMS_HISTORY_RANGE * sizeof(long);
        addressMap[ADDRESS_MAP_TRIAC_ENERGY_IN] = address;
        address += sizeof(long);
        addressMap[ADDRESS_MAP_TRIAC_ENERGY_OUT] = address;
        address += sizeof(long);
        addressMap[ADDRESS_MAP_HOUSE_ENERGY_IN] = address;
        address += sizeof(long);
        addressMap[ADDRESS_MAP_HOUSE_ENERGY_OUT] = address;
        address += sizeof(long);
        addressMap[ADDRESS_MAP_TODAY] = address;
        address += 9; // 8 chars + null // FIXME: use time_t instead of string
        addressMap[ADDRESS_MAP_HISTORY_DAY_IDX] = address;
        address += sizeof(short);
        addressMap[ADDRESS_MAP_HISTORY_RANGE] = address; // future use, should be before HISTORY
        address += sizeof(short);
        addressMap[ADDRESS_MAP_PARAMS] = address;
    }

    int getHistoryAddress(short idxDay)
    {
        return addressMap[ADDRESS_MAP_HISTORY] + idxDay * sizeof(long);
    }

    void writeHistoDay(short idxDay, long energy) {
        if (readOnly)
        {
            ModuleCore::log("EEPROM: Cannot write history data of day (read-only)");
            return;
        }
        // On enregistre les conso en début de journée pour l'historique de l'année
        EEPROM.writeLong(getHistoryAddress(idxDay), energy);
        EEPROM.writeString(addressMap[ADDRESS_MAP_TODAY], ModuleTime::getJourCourant());
        EEPROM.writeUShort(addressMap[ADDRESS_MAP_HISTORY_RANGE], RMS_HISTORY_RANGE);
        EEPROM.writeUShort(addressMap[ADDRESS_MAP_HISTORY_DAY_IDX], idxDay);
        
        EEPROM.commit();
    }

    void readEnergyData(unsigned long &energyInTriac, unsigned long &energyOutTriac, unsigned long &energyInHouse, unsigned long &energyOutHouse) {
        energyInTriac = EEPROM.readULong(addressMap[ADDRESS_MAP_TRIAC_ENERGY_IN]);
        energyOutTriac = EEPROM.readULong(addressMap[ADDRESS_MAP_TRIAC_ENERGY_OUT]);
        energyInHouse = EEPROM.readULong(addressMap[ADDRESS_MAP_HOUSE_ENERGY_IN]);
        energyOutHouse = EEPROM.readULong(addressMap[ADDRESS_MAP_HOUSE_ENERGY_OUT]);
    }

    void writeEnergyData(unsigned long energyInTriac, unsigned long energyOutTriac, unsigned long energyInHouse, unsigned long energyOutHouse) {
        if (readOnly)
        {
            ModuleCore::log("EEPROM: Cannot write energy data (read-only)");
            return;
        }
        EEPROM.writeULong(addressMap[ADDRESS_MAP_TRIAC_ENERGY_IN], energyInTriac);
        EEPROM.writeULong(addressMap[ADDRESS_MAP_TRIAC_ENERGY_OUT], energyOutTriac);
        EEPROM.writeULong(addressMap[ADDRESS_MAP_HOUSE_ENERGY_IN], energyInHouse);
        EEPROM.writeULong(addressMap[ADDRESS_MAP_HOUSE_ENERGY_OUT], energyOutHouse);
        EEPROM.commit();
    }

    String readToday()
    {
        return EEPROM.readString(addressMap[ADDRESS_MAP_TODAY]);
    }

    void writeToday(const String &today)
    {
        if (readOnly)
        {
            ModuleCore::log("EEPROM: Cannot write today (read-only)");
            return;
        }
        char buffer[9];
        strncpy(buffer, today.c_str(), 8);
        buffer[8] = '\0';
        EEPROM.writeString(addressMap[ADDRESS_MAP_TODAY], buffer);
        EEPROM.commit();
    }

    void destroy()
    {
        // Mise a zero Zone stockage
        for (short idxDay = 0; idxDay < RMS_HISTORY_RANGE; idxDay++)
        {
            EEPROM.writeLong(getHistoryAddress(idxDay), 0);
        }
        EEPROM.writeULong(addressMap[ADDRESS_MAP_TRIAC_ENERGY_IN], 0);
        EEPROM.writeULong(addressMap[ADDRESS_MAP_TRIAC_ENERGY_OUT], 0);
        EEPROM.writeULong(addressMap[ADDRESS_MAP_HOUSE_ENERGY_IN], 0);
        EEPROM.writeULong(addressMap[ADDRESS_MAP_HOUSE_ENERGY_OUT], 0);
        EEPROM.writeString(addressMap[ADDRESS_MAP_TODAY], "");
        EEPROM.writeUShort(addressMap[ADDRESS_MAP_HISTORY_RANGE], RMS_HISTORY_RANGE);
        EEPROM.writeUShort(addressMap[ADDRESS_MAP_HISTORY_DAY_IDX], 0);
        EEPROM.commit();
    }

    short readHisto(long* histoEnergy)
    {
        // put histo in RAM
        for (short i = 0; i < RMS_HISTORY_RANGE; i++)
        {
            histoEnergy[i] = EEPROM.readLong(getHistoryAddress(i));
        }
        return EEPROM.readUShort(addressMap[ADDRESS_MAP_HISTORY_DAY_IDX]);
    }

    short readLegacy806Histo(long* histoEnergy)
    {
        // put legacy histo in RAM

        int address = RMS_EEPROM_LEGACY806_HISTO_OFFSET;
        for (short i = 0; i < RMS_EEPROM_LEGACY806_HISTO_RANGE; i++)
        {
            histoEnergy[i] = EEPROM.readLong(address);
            address += sizeof(long);
        }
        return EEPROM.readUShort(addressMap[ADDRESS_MAP_HISTORY_DAY_IDX]);
    }

    unsigned long readEepromKey()
    {
        return EEPROM.readULong(addressMap[ADDRESS_MAP_PARAMS]);
    }

    unsigned long readEepromLegacy806Key()
    {
        return EEPROM.readULong(RMS_EEPROM_LEGACY806_KEY_OFFSET);
    }

    void eepromUsage(int addressEnd)
    {
        // done in ModulePowerMeter setters
        // kV = KV * CalibU / 1000; // Calibration coefficient to be applied
        // kI = KI * CalibI / 1000;
        int size = addressEnd - addressMap[ADDRESS_MAP_HISTORY];
        currentEepromUsage = int(100.0 * size / RMS_EEPROM_MAX_SIZE);
        String m = "EEPROM usage : " + String(currentEepromUsage) + "%" + " (" + String(size) + "/" + String(RMS_EEPROM_MAX_SIZE) + " bytes)";
        ModuleCore::log(m);
        ModuleDebug::getDebug().println(m);
    }

    int readEeprom()
    {
        int address = addressMap[ADDRESS_MAP_PARAMS];
        address = ModuleRomMap::readParameters(address);
        eepromUsage(address);
        return address;
    }
    int writeEeprom()
    {
        if (readOnly)
        {
            ModuleCore::log("EEPROM is read-only");
            return 0;
        }
        int address = addressMap[ADDRESS_MAP_PARAMS];
        address = ModuleRomMap::writeParameters(address);
        eepromUsage(address);
        return address;
    }

    // states
    byte getEepromUsage()
    {
        return currentEepromUsage;
    }
    bool hasData() {
        return eepromKey == RMS_EEPROM_KEY;
    }
    bool hasLegacy806Data() {
        return legacyKey == RMS_EEPROM_LEGACY806_KEY;
    }

    // setters / getters
    void setEepromKey(unsigned long key)
    {
        eepromKey = key;
    }
    unsigned long getEepromKey()
    {
        return eepromKey;
    }

    // web handlers
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

    void httpAjaxPara(AsyncWebServerRequest* request, String& S) {
        
        S = "";
        for (int i = 0; i < ajax_params_map_size; i++)
        {
            if (i > 0) S += RS;
            S += ModuleElemMap::e2s(&ajax_params_map[i]);
        }
    }

    void httpUpdatePara(AsyncWebServerRequest* request, String& S) {
        String lesparas = request->arg("lesparas") + RS;
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
        int adresse_max = writeEeprom();
        S = "OK" + String(adresse_max);
    }
} // namespace ModuleEeprom
