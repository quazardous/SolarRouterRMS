#include "ModuleRomMap.h"
#include "ModuleElem.h"
#include "ModuleTriggers.h"
#include <EEPROM.h>
#include "Actions.h"


// version of the EEPROM map
// increment if map changes
// TODO: invalidate EEPROM if version changes
#define EEPROM_MAP_VERSION 1

namespace ModuleRomMap
{
    using ModuleElem::elem_map_t;
    using ModuleElem::elem_type_t;

    int readRomMap(elem_map_t *romMap, int romMapSize, int address, void *context = NULL);
    int writeRomMap(elem_map_t *romMap, int romMapSize, int address, void *context = NULL);

    #define RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(ELEM, TYPE, Elem, Type) \
        {ModuleElem::ELEM_ ## ELEM, ModuleElem::TYPE_ ## TYPE, {.set ## Type = ModuleElem::elemSet ## Elem}, {.get ## Type = ModuleElem::elemGet  ## Elem}}

    // mapping for Main elements
    // each element will be sequentially read/write from/to EEPROM
    elem_map_t mainRomMap[] = {
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(EEPROM_KEY, ULONG, EepromKey, ULong),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(WIFI_SSID, CSTRING, WifiSsid, CString),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(WIFI_PASSWORD, CSTRING, WifiPassword, CString),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(DHCP_ON, BOOL, DhcpOn, Bool),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(STATIC_IP, ULONG, StaticIp, ULong),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(GATEWAY, ULONG, Gateway, ULong),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(NETMASK, ULONG, Netmask, ULong),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(DNS, ULONG, Dns, ULong),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(SOURCE, CSTRING, Source, CString),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(EXT_IP, ULONG, ExtIp, ULong),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(ENPHASE_USER, CSTRING, EnphaseUser, CString),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(ENPHASE_PWD, CSTRING, EnphasePwd, CString),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(ENPHASE_SERIAL, ULONG, EnphaseSerial, ULong),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(SHELLYEM_PHASES, USHORT, ShellyEmPhases, UShort),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(MQTT_REPEAT, USHORT, MqttRepeat, UShort),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(MQTT_IP, ULONG, MqttIp, ULong),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(MQTT_PORT, USHORT, MqttPort, UShort),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(MQTT_USER, CSTRING, MqttUser, CString),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(MQTT_PWD, CSTRING, MqttPwd, CString),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(MQTT_PREFIX, CSTRING, MqttPrefix, CString),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(MQTT_DEVICE_NAME, CSTRING, MqttDeviceName, CString),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(ROUTER_NAME, CSTRING, RouterName, CString),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(FIX_PROBE_NAME, CSTRING, FixProbeName, CString),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(MOBILE_PROBE_NAME, CSTRING, MobileProbeName, CString),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(TEMPERATURE_NAME, CSTRING, TemperatureName, CString),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(CALIB_U, USHORT, CalibU, UShort),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(CALIB_I, USHORT, CalibI, UShort),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(TEMPO_EDF_ON, BOOL, TempoEdfOn, Bool),
        // Triggers
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(TRIGGERS_COUNT, BYTE, TriggersCount, Byte),
    };
    const int mainRomMapSize = sizeof(mainRomMap) / sizeof(mainRomMap[0]);

    #define RMS_ROM_MAP_TRIGGER_ROM_MAP_ELEM(ELEM, TYPE, Elem, Type) \
        {ModuleElem::ELEM_TRIGGER_ ## ELEM, ModuleElem::TYPE_ ## TYPE, {.set ## Type = ModuleElem::elemSetTrigger ## Elem}, {.get ## Type = ModuleElem::elemGetTrigger  ## Elem}}

    // mapping for Trigger elements
    // each element will be sequentially read/write from/to EEPROM
    elem_map_t triggerRomMap[] = {
        // Trigger sub elements
        RMS_ROM_MAP_TRIGGER_ROM_MAP_ELEM(TITLE, CSTRING, Title, CString),
        RMS_ROM_MAP_TRIGGER_ROM_MAP_ELEM(ACTIVE, BYTE, Active, Byte),
        RMS_ROM_MAP_TRIGGER_ROM_MAP_ELEM(HOST, CSTRING, Host, CString),
        RMS_ROM_MAP_TRIGGER_ROM_MAP_ELEM(PORT, USHORT, Port, UShort),
        RMS_ROM_MAP_TRIGGER_ROM_MAP_ELEM(ORDRE_ON, CSTRING, OrdreOn, CString),
        RMS_ROM_MAP_TRIGGER_ROM_MAP_ELEM(ORDRE_OFF, CSTRING, OrdreOff, CString),
        RMS_ROM_MAP_TRIGGER_ROM_MAP_ELEM(REPEAT, USHORT, Repeat, UShort),
        RMS_ROM_MAP_TRIGGER_ROM_MAP_ELEM(TEMPO, USHORT, Tempo, UShort),
        RMS_ROM_MAP_TRIGGER_ROM_MAP_ELEM(REACT, BYTE, React, Byte),
        // Trigger Periods
        RMS_ROM_MAP_TRIGGER_ROM_MAP_ELEM(PERIODS_COUNT, USHORT, PeriodsCount, UShort),
    };
    const int triggerRomMapSize = sizeof(triggerRomMap) / sizeof(triggerRomMap[0]);

    #define RMS_ROM_MAP_TRIGGER_PERIOD_ROM_MAP_ELEM(ELEM, TYPE, Elem, Type) \
        {ModuleElem::ELEM_TRIGGER_PERIOD_ ## ELEM, ModuleElem::TYPE_ ## TYPE, {.set ## Type = ModuleElem::elemSetTriggerPeriod ## Elem}, {.get ## Type = ModuleElem::elemGetTriggerPeriod  ## Elem}}

    // mapping for Trigger Periods elements
    // each element will be sequentially read/write from/to EEPROM
    elem_map_t triggerPeriodRomMap[] = {
        // Trigger Periods sub sub elements
        RMS_ROM_MAP_TRIGGER_PERIOD_ROM_MAP_ELEM(TYPE, BYTE, Type, Byte),
        RMS_ROM_MAP_TRIGGER_PERIOD_ROM_MAP_ELEM(HFIN, USHORT, Hfin, UShort),
        RMS_ROM_MAP_TRIGGER_PERIOD_ROM_MAP_ELEM(HDEB, USHORT, Hdeb, UShort),
        RMS_ROM_MAP_TRIGGER_PERIOD_ROM_MAP_ELEM(VMIN, USHORT, Vmin, UShort),
        RMS_ROM_MAP_TRIGGER_PERIOD_ROM_MAP_ELEM(VMAX, USHORT, Vmax, UShort),
        RMS_ROM_MAP_TRIGGER_PERIOD_ROM_MAP_ELEM(TINF, USHORT, Tinf, UShort),
        RMS_ROM_MAP_TRIGGER_PERIOD_ROM_MAP_ELEM(TSUP, USHORT, Tsup, UShort),
        RMS_ROM_MAP_TRIGGER_PERIOD_ROM_MAP_ELEM(TARIF, BYTE, Tarif, Byte)
    };
    const int triggerPeriodRomMapSize = sizeof(triggerPeriodRomMap) / sizeof(triggerPeriodRomMap[0]);

    int readParameters(int address)
    {
        byte triggersCount = ModuleTriggers::getTriggersCount();
        address = readRomMap(mainRomMap, mainRomMapSize, address);
        for(byte i = 0; i < triggersCount; i++)
        {
            Action *t = ModuleTriggers::getTrigger(i);
            address = readRomMap(triggerRomMap, triggerRomMapSize, address, t);
            for(int j = 0; j < t->PeriodsCount; j++)
            {
                // we pack a context tuple to pass the action and the period index
                // this will be unpack by RMS_ROM_MAP_CONTEXT_TRIGGER_PERIOD_ARRAY_ATTR_ELEM_TUPLE
                void *tuple[2];
                tuple[0] = t;
                tuple[0] = &j;
                address = readRomMap(triggerPeriodRomMap, triggerPeriodRomMapSize, address, tuple);
            }
        }
        return address;
    }

    int writeParameters(int address, bool commit)
    {
        byte triggersCount = ModuleTriggers::getTriggersCount();
        address = writeRomMap(mainRomMap, mainRomMapSize, address);
        for(byte i = 0; i < triggersCount; i++)
        {
            Action *t = ModuleTriggers::getTrigger(i);
            address = writeRomMap(triggerRomMap, triggerRomMapSize, address, t);
            // Periods Count should be initialized by the line above
            for(byte j = 0; j < t->PeriodsCount; j++)
            {
                // we pack a context tuple to pass the action and the period index
                // this will be unpack by RMS_ROM_MAP_CONTEXT_TRIGGER_PERIOD_ARRAY_ATTR_ELEM_TUPLE
                void *tuple[2];
                tuple[0] = t;
                tuple[0] = &j;
                address = writeRomMap(triggerPeriodRomMap, triggerPeriodRomMapSize, address, tuple);
            }
        }
        if (commit) EEPROM.commit();
        return address;
    }

    int readRomElem(const elem_map_t *romElem, int address, void *context = NULL)
    {
        switch (romElem->type)
        {
        case ModuleElem::TYPE_BYTE:
        {
            uint8_t vByte = EEPROM.readByte(address);
            romElem->setter.setByte(vByte, context);
            address += sizeof(uint8_t);
            break;
        }
        case ModuleElem::TYPE_USHORT:
        {
            uint16_t vUShort = EEPROM.readUShort(address);
            romElem->setter.setUShort(vUShort, context);
            address += sizeof(uint16_t);
            break;
        }
        case ModuleElem::TYPE_SHORT:
        {
            int16_t vShort = EEPROM.readShort(address);
            romElem->setter.setShort(vShort, context);
            address += sizeof(int16_t);
            break;
        }
        case ModuleElem::TYPE_ULONG:
        {
            uint32_t vULong = EEPROM.readULong(address);
            romElem->setter.setULong(vULong, context);
            address += sizeof(uint32_t);
            break;
        }
        case ModuleElem::TYPE_LONG:
        {
            int32_t vLong = EEPROM.readLong(address);
            romElem->setter.setLong(vLong, context);
            address += sizeof(int32_t);
            break;
        }
        case ModuleElem::TYPE_BOOL:
        {
            int8_t vByte = EEPROM.readBool(address);
            romElem->setter.setByte(vByte, context);
            address += sizeof(int8_t);
            break;
        }
        case ModuleElem::TYPE_CSTRING:
        {
            const String vCString = EEPROM.readString(address);
            romElem->setter.setCString(vCString.c_str(), context);
            address += vCString.length() + 1;
            break;
        }
        }
        return address;
    }

    int readRomMap(elem_map_t *romMap, int romMapSize, int address, void *context)
    {
        int i = 0;
        for (i = 0; i < romMapSize; i++)
        {
            address = readRomElem(&romMap[i], address, context);
        }
        return address;
    }

    int writeRomElem(const elem_map_t *romElem, int address, void *context = NULL)
    {
        switch (romElem->type)
        {
        case ModuleElem::TYPE_BYTE:
            EEPROM.writeByte(address, romElem->getter.getByte(context));
            address += sizeof(uint8_t);
            break;
        case ModuleElem::TYPE_USHORT:
            EEPROM.writeUShort(address, romElem->getter.getUShort(context));
            address += sizeof(uint16_t);
            break;
        case ModuleElem::TYPE_SHORT:
            EEPROM.writeShort(address, romElem->getter.getShort(context));
            address += sizeof(int16_t);
            break;
        case ModuleElem::TYPE_ULONG:
            EEPROM.writeULong(address, romElem->getter.getULong(context));
            address += sizeof(uint32_t);
            break;
        case ModuleElem::TYPE_LONG:
            EEPROM.writeLong(address, romElem->getter.getLong(context));
            address += sizeof(int32_t);
            break;
        case ModuleElem::TYPE_BOOL:
            EEPROM.writeBool(address, romElem->getter.getByte(context));
            address += sizeof(int8_t);
            break;
        case ModuleElem::TYPE_CSTRING:
            const char *vCString = romElem->getter.getCString(context);
            EEPROM.writeString(address, vCString);
            address += strlen(vCString) + 1;
            break;
        }
        return address;
    }

    int writeRomMap(elem_map_t *romMap, int romMapSize, int address, void *context)
    {
        int i = 0;
        for (i = 0; i < romMapSize; i++)
        {
            address = writeRomElem(&romMap[i], address, context);
        }
        return address;
    }
} // namespace ModuleRomMap
