#pragma once

#include <Arduino.h>

// helps with list of elements
namespace ModuleElem
{

    enum group_t
    {
        GROUP_MAIN = 0,
        GROUP_WIFI,
        GROUP_POWERMETER,
        GROUP_EDF,
    };

    // list of kown elements
    // don't forget to update elem_names
    enum elem_t
    {
        ELEM_VERSION = 0,
        ELEM_EEPROM_KEY,
        ELEM_WIFI_SSID,
        ELEM_WIFI_PASSWORD,
        ELEM_DHCP_ON,
        ELEM_STATIC_IP,
        ELEM_GATEWAY,
        ELEM_NETMASK,
        ELEM_DNS,
        ELEM_SOURCE,
        ELEM_DATA_SOURCE, // 10
        ELEM_EXT_IP,
        ELEM_ENPHASE_USER,
        ELEM_ENPHASE_PWD,
        ELEM_ENPHASE_SERIAL,
        ELEM_SHELLYEM_PHASES,
        ELEM_MQTT_REPEAT,
        ELEM_MQTT_IP,
        ELEM_MQTT_PORT,
        ELEM_MQTT_USER,
        ELEM_MQTT_PWD, // 20
        ELEM_MQTT_PREFIX,
        ELEM_MQTT_DEVICE_NAME,
        ELEM_ROUTER_NAME,
        ELEM_FIX_PROBE_NAME,
        ELEM_MOBILE_PROBE_NAME,
        ELEM_TEMPERATURE_NAME,
        ELEM_TEMPERATURE,
        ELEM_CALIB_U,
        ELEM_CALIB_I,
        ELEM_TEMPO_EDF_ON, // 30
        // Triggers
        ELEM_TRIGGERS_COUNT,
        // Trigger sub elements
        ELEM_TRIGGER_TITLE,
        ELEM_TRIGGER_ACTIVE,
        ELEM_TRIGGER_HOST,
        ELEM_TRIGGER_PORT,
        ELEM_TRIGGER_ORDRE_ON,
        ELEM_TRIGGER_ORDRE_OFF,
        ELEM_TRIGGER_REPEAT,
        ELEM_TRIGGER_TEMPO,
        ELEM_TRIGGER_REACT,
        // Trigger Periods
        ELEM_TRIGGER_PERIODS_COUNT,
        // Trigger Periods sub sub elements
        ELEM_TRIGGER_PERIOD_TYPE,
        ELEM_TRIGGER_PERIOD_HFIN,
        ELEM_TRIGGER_PERIOD_HDEB,
        ELEM_TRIGGER_PERIOD_VMIN,
        ELEM_TRIGGER_PERIOD_VMAX,
        ELEM_TRIGGER_PERIOD_TINF,
        ELEM_TRIGGER_PERIOD_TSUP,
        ELEM_TRIGGER_PERIOD_TARIF
    };

    // data types
    enum elem_type_t
    {
        TYPE_BOOL,
        TYPE_BYTE,
        TYPE_SHORT,
        TYPE_USHORT,
        TYPE_LONG,
        TYPE_ULONG,
        TYPE_IP,
        TYPE_FLOAT,
        TYPE_CSTRING
    };

    // data types setters
    typedef union {
        void (*setByte)(byte, void *context);
        void (*setUShort)(unsigned short, void *context);
        void (*setShort)(short, void *context);
        void (*setULong)(unsigned long, void *context);
        void (*setLong)(long, void *context);
        void (*setBool)(bool, void *context);
        void (*setCString)(const char*, void *context);
        void (*setFloat)(float, void *context);
    } setter_t;

    // data types getters
    typedef union {
        byte (*getByte)(void *context);
        unsigned short (*getUShort)(void *context);
        short (*getShort)(void *context);
        unsigned long (*getULong)(void *context);
        long (*getLong)(void *context);
        bool (*getBool)(void *context);
        const char* (*getCString)(void *context);
        float (*getFloat)(void *context);
    } getter_t;

    // data types choices
    typedef union {
        const byte* (*chooseByte)(size_t* len, bool* exhaustive, void *context);
        const unsigned short* (*chooseUShort)(size_t* len, bool* exhaustive, void *context);
        const short* (*chooseShort)(size_t* len, bool* exhaustive, void *context);
        const unsigned long* (*chooseULong)(size_t* len, bool* exhaustive, void *context);
        const long* (*chooseLong)(size_t* len, bool* exhaustive, void *context);
        const bool* (*chooseBool)(size_t* len, bool* exhaustive, void *context);
        const char** (*chooseCString)(size_t* len, bool* exhaustive, void *context);
        const float* (*chooseFloat)(size_t* len, bool* exhaustive, void *context);
    } choose_t;

    // backup data
    typedef struct {
        byte Byte;
        unsigned short UShort;
        short Short;
        unsigned long ULong;
        long Long;
        bool Bool;
        const char* CString;
        float Float;
    } backup_t;

    // callback type
    typedef void (*persist_t)(bool read_or_write, void* context);

    // managed element
    struct elem_map_t {
        elem_t element;
        elem_type_t type;
        setter_t setter;
        getter_t getter;
        bool readonly;
        group_t group;
        persist_t persist;
        bool dirty;
        const char* label;
        const char* help;
        choose_t choices;
        backup_t backup;
    };

    // getter to string
    String e2s(elem_map_t* elem, void* context = NULL);
    // string to setter
    void s2e(elem_map_t* elem, const String& str, void* context = NULL);
    const char* elemName(elem_t elem);
    const char* typeName(elem_type_t type);
    const char* groupName(group_t group);

    #undef RMS_ELEM_MAP_MAIN_ACCESSORS
    #define RMS_ELEM_MAP_MAIN_ACCESSORS(elem, type) \
        type elemGet ## elem(void* context); \
        void elemSet ## elem(type value, void* context);

    #undef RMS_ELEM_MAP_TRIGGER_ACCESSORS
    #define RMS_ELEM_MAP_TRIGGER_ACCESSORS(attr, type) \
        type elemGetTrigger##attr(void* context); \
        void elemSetTrigger##attr(type value, void* context);
    
    #undef RMS_ELEM_MAP_TRIGGER_PERIOD_ACCESSORS
    #define RMS_ELEM_MAP_TRIGGER_PERIOD_ACCESSORS(attr, type) \
        type elemGetTriggerPeriod##attr(void* context); \
        void elemSetTriggerPeriod##attr(type value, void* context);

    // setters / getters for mapped Elements
    // Root elements
    RMS_ELEM_MAP_MAIN_ACCESSORS(Version, const char *)
    RMS_ELEM_MAP_MAIN_ACCESSORS(EepromKey, unsigned long)
    RMS_ELEM_MAP_MAIN_ACCESSORS(RouterName, const char *)
    RMS_ELEM_MAP_MAIN_ACCESSORS(FixProbeName, const char *)
    RMS_ELEM_MAP_MAIN_ACCESSORS(MobileProbeName, const char *)
    RMS_ELEM_MAP_MAIN_ACCESSORS(TemperatureName, const char *)
    RMS_ELEM_MAP_MAIN_ACCESSORS(WifiSsid, const char *)
    RMS_ELEM_MAP_MAIN_ACCESSORS(WifiPassword, const char *)
    RMS_ELEM_MAP_MAIN_ACCESSORS(DhcpOn, bool)
    RMS_ELEM_MAP_MAIN_ACCESSORS(StaticIp, unsigned long)
    RMS_ELEM_MAP_MAIN_ACCESSORS(Gateway, unsigned long)
    RMS_ELEM_MAP_MAIN_ACCESSORS(Netmask, unsigned long)
    RMS_ELEM_MAP_MAIN_ACCESSORS(Dns, unsigned long)
    // use name instead of enum to allow better better compatibility
    RMS_ELEM_MAP_MAIN_ACCESSORS(Source, const char *)
    RMS_ELEM_MAP_MAIN_ACCESSORS(DataSource, const char *)
    // External IP common to multiple power meter sources
    RMS_ELEM_MAP_MAIN_ACCESSORS(ExtIp, unsigned long)
    RMS_ELEM_MAP_MAIN_ACCESSORS(CalibU, unsigned short)
    RMS_ELEM_MAP_MAIN_ACCESSORS(CalibI, unsigned short)
    RMS_ELEM_MAP_MAIN_ACCESSORS(EnphaseUser, const char *)
    RMS_ELEM_MAP_MAIN_ACCESSORS(EnphasePwd, const char *)
    RMS_ELEM_MAP_MAIN_ACCESSORS(EnphaseSerial, unsigned long)
    RMS_ELEM_MAP_MAIN_ACCESSORS(ShellyEmPhases, unsigned short)
    RMS_ELEM_MAP_MAIN_ACCESSORS(MqttRepeat, unsigned short)
    RMS_ELEM_MAP_MAIN_ACCESSORS(MqttIp, unsigned long)
    RMS_ELEM_MAP_MAIN_ACCESSORS(MqttPort, unsigned short)
    RMS_ELEM_MAP_MAIN_ACCESSORS(MqttUser, const char *)
    RMS_ELEM_MAP_MAIN_ACCESSORS(MqttPwd, const char *)
    RMS_ELEM_MAP_MAIN_ACCESSORS(MqttPrefix, const char *)
    RMS_ELEM_MAP_MAIN_ACCESSORS(MqttDeviceName, const char *)
    RMS_ELEM_MAP_MAIN_ACCESSORS(TemperatureName, const char *)
    RMS_ELEM_MAP_MAIN_ACCESSORS(Temperature, float)
    RMS_ELEM_MAP_MAIN_ACCESSORS(TempoEdfOn, bool)
    // Triggers
    RMS_ELEM_MAP_MAIN_ACCESSORS(TriggersCount, byte)

    // Trigger sub elements
    RMS_ELEM_MAP_TRIGGER_ACCESSORS(Title, const char *)
    RMS_ELEM_MAP_TRIGGER_ACCESSORS(Host, const char *)
    RMS_ELEM_MAP_TRIGGER_ACCESSORS(Port, unsigned short)
    RMS_ELEM_MAP_TRIGGER_ACCESSORS(OrdreOn, const char *)
    RMS_ELEM_MAP_TRIGGER_ACCESSORS(OrdreOff, const char *)
    RMS_ELEM_MAP_TRIGGER_ACCESSORS(Repeat, unsigned short)
    RMS_ELEM_MAP_TRIGGER_ACCESSORS(Tempo, unsigned short)
    RMS_ELEM_MAP_TRIGGER_ACCESSORS(React, byte)
    RMS_ELEM_MAP_TRIGGER_ACCESSORS(Active, byte)
    // Trigger periods
    RMS_ELEM_MAP_TRIGGER_ACCESSORS(PeriodsCount, unsigned short)

    // Trigger Periods sub sub elements
    RMS_ELEM_MAP_TRIGGER_PERIOD_ACCESSORS(Hfin, unsigned short)
    RMS_ELEM_MAP_TRIGGER_PERIOD_ACCESSORS(Hdeb, unsigned short)
    RMS_ELEM_MAP_TRIGGER_PERIOD_ACCESSORS(Vmin, unsigned short)
    RMS_ELEM_MAP_TRIGGER_PERIOD_ACCESSORS(Vmax, unsigned short)
    RMS_ELEM_MAP_TRIGGER_PERIOD_ACCESSORS(Tinf, unsigned short)
    RMS_ELEM_MAP_TRIGGER_PERIOD_ACCESSORS(Tsup, unsigned short)
    RMS_ELEM_MAP_TRIGGER_PERIOD_ACCESSORS(Tarif, byte)
    RMS_ELEM_MAP_TRIGGER_PERIOD_ACCESSORS(Type, byte)

} // namespace ModuleElem