#include "ModuleRomMap.h"
#include <EEPROM.h>
#include "ModuleCore.h"
#include "ModuleTriggers.h"
#include "ModuleStockage.h"
#include "ModulePowerMeter.h"
#include "ModulePowerMeterEnphase.h"
#include "ModulePowerMeterShellyEm.h"
#include "ModuleEDF.h"
#include "ModuleWifi.h"
#include "ModuleMQTT.h"
#include "Actions.h"

// version of the EEPROM map
// increment if map changes
// TODO: invalidate EEPROM if version changes
#define EEPROM_MAP_VERSION 1

namespace ModuleRomMap
{
    // list of konw elements
    enum eeprom_elem_t
    {
        ELEM_ROM_EEPROM_KEY,
        ELEM_ROM_WIFI_SSID,
        ELEM_ROM_WIFI_PASSWORD,
        ELEM_ROM_DHCP_ON,
        ELEM_ROM_STATIC_IP,
        ELEM_ROM_GATEWAY,
        ELEM_ROM_NETMASK,
        ELEM_ROM_DNS,
        ELEM_ROM_SOURCE,
        ELEM_ROM_EXT_IP,
        ELEM_ROM_ENPHASE_USER,
        ELEM_ROM_ENPHASE_PWD,
        ELEM_ROM_ENPHASE_SERIAL,
        ELEM_ROM_SHELLYEM_PHASES,
        ELEM_ROM_MQTT_REPEAT,
        ELEM_ROM_MQTT_IP,
        ELEM_ROM_MQTT_PORT,
        ELEM_ROM_MQTT_USER,
        ELEM_ROM_MQTT_PWD,
        ELEM_ROM_MQTT_PREFIX,
        ELEM_ROM_MQTT_DEVICE_NAME,
        ELEM_ROM_ROUTER_NAM,
        ELEM_ROM_FIX_PROBE_NAME,
        ELEM_ROM_MOBILE_PROBE_NAME,
        ELEM_ROM_TEMPERATURE_NAME,
        ELEM_ROM_CALIB_U,
        ELEM_ROM_CALIB_I,
        ELEM_ROM_TEMPO_EDF_ON,
        // Triggers
        ELEM_ROM_TRIGGERS_COUNT,
        // Trigger sub elements
        ELEM_ROM_TRIGGER_TITLE,
        ELEM_ROM_TRIGGER_ACTIVE,
        ELEM_ROM_TRIGGER_HOST,
        ELEM_ROM_TRIGGER_PORT,
        ELEM_ROM_TRIGGER_ORDRE_ON,
        ELEM_ROM_TRIGGER_ORDRE_OFF,
        ELEM_ROM_TRIGGER_REPEAT,
        ELEM_ROM_TRIGGER_TEMPO,
        ELEM_ROM_TRIGGER_REACT,
        // Trigger Periods
        ELEM_ROM_TRIGGER_PERIODS_COUNT,
        // Trigger Periods sub sub elements
        ELEM_ROM_TRIGGER_PERIOD_TYPE,
        ELEM_ROM_TRIGGER_PERIOD_HFIN,
        ELEM_ROM_TRIGGER_PERIOD_HDEB,
        ELEM_ROM_TRIGGER_PERIOD_VMIN,
        ELEM_ROM_TRIGGER_PERIOD_VMAX,
        ELEM_ROM_TRIGGER_PERIOD_TINF,
        ELEM_ROM_TRIGGER_PERIOD_TSUP,
        ELEM_ROM_TRIGGER_PERIOD_TARIF
    };

    // data types
    enum eeprom_type_t
    {
        EEPROM_TYPE_BYTE,
        EEPROM_TYPE_USHORT,
        EEPROM_TYPE_ULONG,
        EEPROM_TYPE_BOOL,
        EEPROM_TYPE_CSTRING
    };

    // data types setters
    typedef union {
        void (*setByte)(byte, void *context);
        void (*setUShort)(unsigned short, void *context);
        void (*setULong)(unsigned long, void *context);
        void (*setBool)(bool, void *context);
        void (*setCString)(const char*, void *context);
    } eeprom_setter_t;

    // data types getters
    typedef union {
        byte (*getByte)(void *context);
        unsigned short (*getUShort)(void *context);
        unsigned long (*getULong)(void *context);
        bool (*getBool)(void *context);
        const char* (*getCString)(void *context);
    } eeprom_getter_t;

    // managed element
    struct rom_map_t {
        eeprom_elem_t element;
        eeprom_type_t type;
        eeprom_setter_t setter;
        eeprom_getter_t getter;
    };

    #define RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(ELEM, TYPE, Elem, Type) \
        {ELEM_ROM_ ## ELEM, EEPROM_TYPE_ ## TYPE, {.set ## Type = elemSetRom ## Elem}, {.get ## Type = elemGetRom  ## Elem}}

    // mapping for Main elements
    // each element will be sequentially read/write from/to EEPROM
    rom_map_t mainRomMap[] = {
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
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(SHELLYEM_PHASES, USHORT, ShellyEmChannel, UShort),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(MQTT_REPEAT, USHORT, MqttRepeat, UShort),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(MQTT_IP, ULONG, MqttIp, ULong),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(MQTT_PORT, USHORT, MqttPort, UShort),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(MQTT_USER, CSTRING, MqttUser, CString),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(MQTT_PWD, CSTRING, MqttPwd, CString),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(MQTT_PREFIX, CSTRING, MqttPrefix, CString),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(MQTT_DEVICE_NAME, CSTRING, MqttDeviceName, CString),
        RMS_ROM_MAP_MAIN_ROM_MAP_ELEM(ROUTER_NAM, CSTRING, RouterName, CString),
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
        {ELEM_ROM_TRIGGER_ ## ELEM, EEPROM_TYPE_ ## TYPE, {.set ## Type = elemSetRomTrigger ## Elem}, {.get ## Type = elemGetRomTrigger  ## Elem}}

    // mapping for Trigger elements
    // each element will be sequentially read/write from/to EEPROM
    rom_map_t triggerRomMap[] = {
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
        {ELEM_ROM_TRIGGER_PERIOD_ ## ELEM, EEPROM_TYPE_ ## TYPE, {.set ## Type = elemSetRomTriggerPeriod ## Elem}, {.get ## Type = elemGetRomTriggerPeriod  ## Elem}}

    // mapping for Trigger Periods elements
    // each element will be sequentially read/write from/to EEPROM
    rom_map_t triggerPeriodRomMap[] = {
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

    int readEeprom(int address)
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

    int writeEeprom(int address)
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
        return address;
    }

    int readRomMap(rom_map_t *romMap, int romMapSize, int address, void *context = NULL)
    {
        int i = 0;
        for (i = 0; i < romMapSize; i++)
        {
            address = readRomElem(&romMap[i], address, context);
        }
        return address;
    }

    int readRomElem(const rom_map_t *romElem, int address, void *context = NULL)
    {
        switch (romElem->type)
        {
        case EEPROM_TYPE_BYTE:
            uint8_t vByte = EEPROM.readByte(address);
            romElem->setter.setByte(vByte, context);
            address += sizeof(uint8_t);
            break;
        case EEPROM_TYPE_USHORT:
            uint16_t vUShort = EEPROM.readUShort(address);
            romElem->setter.setUShort(vUShort, context);
            address += sizeof(uint16_t);
            break;
        case EEPROM_TYPE_ULONG:
            uint32_t vULong = EEPROM.readULong(address);
            romElem->setter.setULong(vULong, context);
            address += sizeof(uint32_t);
            break;
        case EEPROM_TYPE_BOOL:
            int8_t vByte = EEPROM.readBool(address);
            romElem->setter.setByte(vByte, context);
            address += sizeof(int8_t);
            break;
        case EEPROM_TYPE_CSTRING:
            const String vCString = EEPROM.readString(address);
            romElem->setter.setCString(vCString.c_str(), context);
            address += vCString.length() + 1;
            break;
        }
        return address;
    }

    int writeRomMap(rom_map_t *romMap, int romMapSize, int address, void *context = NULL)
    {
        int i = 0;
        for (i = 0; i < romMapSize; i++)
        {
            address = writeRomElem(&romMap[i], address, context);
        }
        return address;
    }

    int writeRomElem(const rom_map_t *romElem, int address, void *context = NULL)
    {
        switch (romElem->type)
        {
        case EEPROM_TYPE_BYTE:
            EEPROM.writeByte(address, romElem->getter.getByte(context));
            address += sizeof(uint8_t);
            break;
        case EEPROM_TYPE_USHORT:
            EEPROM.writeUShort(address, romElem->getter.getUShort(context));
            address += sizeof(uint16_t);
            break;
        case EEPROM_TYPE_ULONG:
            EEPROM.writeULong(address, romElem->getter.getULong(context));
            address += sizeof(uint32_t);
            break;
        case EEPROM_TYPE_BOOL:
            EEPROM.writeBool(address, romElem->getter.getByte(context));
            address += sizeof(int8_t);
            break;
        case EEPROM_TYPE_CSTRING:
            const char *vCString = romElem->getter.getCString(context);
            EEPROM.writeString(address, vCString);
            address += strlen(vCString) + 1;
            break;
        }
        return address;
    }

    #define RMS_ROM_MAP_MAIN_VAR_ACCESSORS(elem, target, type, set, get) \
        type elemGetRom ## elem ## (void* context) \
        { \
            return (target) ## get; \
        } \
        void elemSetRom ## elem ## (type value, void* context) \
        { \
            target = set ## (value); \
        }

    #define RMS_ROM_MAP_MAIN_FN_ACCESSORS(elem, type, set, get) \
        type elemGetRom ## elem ## (void* context) \
        { \
            return get ## (); \
        } \
        void elemSetRom ## elem ## (type value, void* context) \
        { \
            set ## (value); \
        }

    // setters / getters for EEPROM Elements
    // mainRomMap
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(EepromKey, unsigned long, ModuleStockage::setEepromKey, ModuleStockage::getEepromKey)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(RouterName, const char *, ModuleCore::setRouterName, ModuleCore::getRouterName)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(FixProbeName, const char *, ModuleCore::setFixProbeName, ModuleCore::getFixProbeName)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(MobileProbeName, const char *, ModuleCore::setMobileProbeName, ModuleCore::getMobileProbeName)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(TemperatureName, const char *, ModuleCore::setTemperatureName, ModuleCore::getTemperatureName)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(WifiSsid, const char *, ModuleWifi::setWifiSsid, ModuleWifi::getWifiSsid)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(WifiPassword, const char *, ModuleWifi::setWifiPassword, ModuleWifi::getWifiPassword)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(DhcpOn, bool, ModuleWifi::setDhcpOn, ModuleWifi::getDhcpOn)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(StaticIp, unsigned long, ModuleWifi::setStaticIp, ModuleWifi::getStaticIp)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(Gateway, unsigned long, ModuleWifi::setGateway, ModuleWifi::getGateway)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(Netmask, unsigned long, ModuleWifi::setNetmask, ModuleWifi::getNetmask)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(Dns, unsigned long, ModuleWifi::setDns, ModuleWifi::getDns)
    // use name instead of enum to allow better better compatibility
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(Source, const char *, ModulePowerMeter::setSourceByName, ModulePowerMeter::getSourceName)
    // External IP common to multiple power meter sources
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(ExtIp, unsigned long, ModulePowerMeter::setExtIp, ModulePowerMeter::getExtIp)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(CalibU, unsigned short, ModulePowerMeter::setCalibU, ModulePowerMeter::getCalibU)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(CalibI, unsigned short, ModulePowerMeter::setCalibI, ModulePowerMeter::getCalibI)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(EnphaseUser, const char *, ModulePowerMeterEnphase::setUser, ModulePowerMeterEnphase::getUser)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(EnphasePwd, const char *, ModulePowerMeterEnphase::setPwd, ModulePowerMeterEnphase::getPwd)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(EnphaseSerial, unsigned long, ModulePowerMeterEnphase::setSerial, ModulePowerMeterEnphase::getSerial)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(ShellyEmChannel, unsigned short, ModulePowerMeterShellyEm::setPhasesNumber, ModulePowerMeterShellyEm::getPhasesNumber)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(MqttRepeat, unsigned short, ModuleMQTT::setRepeat, ModuleMQTT::getRepeat)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(MqttIp, unsigned long, ModuleMQTT::setIp, ModuleMQTT::getIp)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(MqttPort, unsigned short, ModuleMQTT::setPort, ModuleMQTT::getPort)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(MqttUser, const char *, ModuleMQTT::setUser, ModuleMQTT::getUser)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(MqttPwd, const char *, ModuleMQTT::setPwd, ModuleMQTT::getPwd)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(MqttPrefix, const char *, ModuleMQTT::setPrefix, ModuleMQTT::getPrefix)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(MqttDeviceName, const char *, ModuleMQTT::setDeviceName, ModuleMQTT::getDeviceName)
    RMS_ROM_MAP_MAIN_FN_ACCESSORS(TempoEdfOn, bool, ModuleEDF::setTempo, ModuleEDF::getTempo)

    // Triggers
    // (not using macro is OK)
    void elemSetRomTriggersCount(byte value, void* context)
    {
        ModuleTriggers::setTriggersCount(value);
    }
    byte elemGetRomTriggersCount(void* context)
    {
        return ModuleTriggers::getTriggersCount();
    }

    #define RMS_ROM_MAP_CONTEXT_TRIGGER_ATTR(context, attr) \
        (((Action *)context)->attr)

    #define RMS_ROM_MAP_TRIGGER_ACCESSORS(attr, type, set, get) \
        type elemGetRomTrigger##attr(void* context) \
        { \
            return RMS_ROM_MAP_CONTEXT_TRIGGER_ATTR(context, attr) ## get; \
        } \
        void elemSetRomTrigger##attr(type value, void* context) \
        { \
            RMS_ROM_MAP_CONTEXT_TRIGGER_ATTR(context, attr) = set ## (value); \
        }

    // Trigger sub elements
    RMS_ROM_MAP_TRIGGER_ACCESSORS(Title, const char *, String, .c_str())
    RMS_ROM_MAP_TRIGGER_ACCESSORS(Host, const char *, String, .c_str())
    RMS_ROM_MAP_TRIGGER_ACCESSORS(Port, unsigned short, , )
    RMS_ROM_MAP_TRIGGER_ACCESSORS(OrdreOn, const char *, String, .c_str())
    RMS_ROM_MAP_TRIGGER_ACCESSORS(OrdreOff, const char *, String, .c_str())
    RMS_ROM_MAP_TRIGGER_ACCESSORS(Repeat, unsigned short, , )
    RMS_ROM_MAP_TRIGGER_ACCESSORS(Tempo, unsigned short, , )
    RMS_ROM_MAP_TRIGGER_ACCESSORS(React, byte, , )

    // RMS_ROM_MAP_TRIGGER_ACCESSORS(Active, byte, , )
    byte elemGetRomTriggerActive(void *context) { return (((Action *)context)->Active); }
    void elemSetRomTriggerActive(byte value, void *context) {
        if (value >= Action::CUTTING_MODE_ERROR)
            value = Action::CUTTING_MODE_NONE;
        (((Action *)context)->Active) = (Action::cutting_mode_t)(value);
    }
    // Trigger periods
    RMS_ROM_MAP_TRIGGER_ACCESSORS(PeriodsCount, unsigned short, , )

    #define RMS_ROM_MAP_CONTEXT_TRIGGER_PERIOD_ARRAY_ATTR_ELEM_TUPLE(context, attr) \
        ((((Action *)(((void**)context))[0]))->attr[*((byte *)((((void**)context))[1]))])

    #define RMS_ROM_MAP_TRIGGER_PERIOD_ACCESSORS(attr, type) \
        type elemGetRomTriggerPeriod##attr(void* context) \
        { \
            return RMS_ROM_MAP_CONTEXT_TRIGGER_PERIOD_ARRAY_ATTR_ELEM_TUPLE(context, attr); \
        } \
        void elemSetRomTriggerPeriod##attr(type value, void* context) \
        { \
            RMS_ROM_MAP_CONTEXT_TRIGGER_PERIOD_ARRAY_ATTR_ELEM_TUPLE(context, attr) = value; \
        }

    // Trigger Periods sub sub elements
    RMS_ROM_MAP_TRIGGER_PERIOD_ACCESSORS(Hfin, unsigned short)
    RMS_ROM_MAP_TRIGGER_PERIOD_ACCESSORS(Hdeb, unsigned short)
    RMS_ROM_MAP_TRIGGER_PERIOD_ACCESSORS(Vmin, unsigned short)
    RMS_ROM_MAP_TRIGGER_PERIOD_ACCESSORS(Vmax, unsigned short)
    RMS_ROM_MAP_TRIGGER_PERIOD_ACCESSORS(Tinf, unsigned short)
    RMS_ROM_MAP_TRIGGER_PERIOD_ACCESSORS(Tsup, unsigned short)
    RMS_ROM_MAP_TRIGGER_PERIOD_ACCESSORS(Tarif, byte)
    // RMS_ROM_MAP_TRIGGER_PERIOD_ACCESSORS(Type, byte)
    byte elemGetRomTriggerPeriodType(void *context) { return RMS_ROM_MAP_CONTEXT_TRIGGER_PERIOD_ARRAY_ATTR_ELEM_TUPLE(context, Type); }
    void elemSetRomTriggerPeriodType(byte value, void *context) { 
        if (value >= Action::TRIGGER_TYPE_ERROR)
            value = Action::TRIGGER_TYPE_NONE;
        RMS_ROM_MAP_CONTEXT_TRIGGER_PERIOD_ARRAY_ATTR_ELEM_TUPLE(context, Type) = (Action::trigger_type_t) value;
    }

} // namespace ModuleRomMap
