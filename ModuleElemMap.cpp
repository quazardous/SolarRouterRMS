#include "ModuleElemMap.h"
#include "ModuleCore.h"
#include "ModuleTriggers.h"
#include "ModuleStockage.h"
#include "ModulePowerMeter.h"
#include "ModulePowerMeterEnphase.h"
#include "ModulePowerMeterShellyEm.h"
#include "ModuleEDF.h"
#include "ModuleSensor.h"
#include "ModuleWifi.h"
#include "ModuleMQTT.h"

namespace ModuleElemMap {
    #define RMS_ELEM_MAP_MAIN_GETTER_ONLY(elem, type, get) \
        type elemGet ## elem(void* context) \
        { \
            return get(); \
        } \
        void elemSet ## elem(type value, void* context) {} // dummy setter
    
    #define RMS_ELEM_MAP_MAIN_SETTER_ONLY(elem, type, set) \
        void elemSet ## elem(type value, void* context) \
        { \
            set(value); \
        } \
        type elemGet ## elem(void* context) {} // dummy getter

    #undef RMS_ELEM_MAP_MAIN_ACCESSORS
    #define RMS_ELEM_MAP_MAIN_ACCESSORS(elem, type, set, get) \
        type elemGet ## elem(void* context) \
        { \
            return get(); \
        } \
        void elemSet ## elem(type value, void* context) \
        { \
            set(value); \
        }

    #define RMS_ELEM_MAP_CONTEXT_TRIGGER_ATTR(context, attr) \
        (((Action *)context)->attr)

    #undef RMS_ELEM_MAP_TRIGGER_ACCESSORS
    #define RMS_ELEM_MAP_TRIGGER_ACCESSORS(attr, type, set, get) \
        type elemGetTrigger##attr(void* context) \
        { \
            return RMS_ELEM_MAP_CONTEXT_TRIGGER_ATTR(context, attr)get; \
        } \
        void elemSetTrigger##attr(type value, void* context) \
        { \
            RMS_ELEM_MAP_CONTEXT_TRIGGER_ATTR(context, attr) = set(value); \
        }

    #define RMS_ELEM_MAP_CONTEXT_TRIGGER_PERIOD_ARRAY_ATTR_ELEM_TUPLE(context, attr) \
        ((((Action *)(((void**)context))[0]))->attr[*((byte *)((((void**)context))[1]))])

    #undef RMS_ELEM_MAP_TRIGGER_PERIOD_ACCESSORS
    #define RMS_ELEM_MAP_TRIGGER_PERIOD_ACCESSORS(attr, type) \
        type elemGetTriggerPeriod##attr(void* context) \
        { \
            return RMS_ELEM_MAP_CONTEXT_TRIGGER_PERIOD_ARRAY_ATTR_ELEM_TUPLE(context, attr); \
        } \
        void elemSetTriggerPeriod##attr(type value, void* context) \
        { \
            RMS_ELEM_MAP_CONTEXT_TRIGGER_PERIOD_ARRAY_ATTR_ELEM_TUPLE(context, attr) = value; \
        }

    // setters / getters for mapped Elements
    // mainRomMap
    RMS_ELEM_MAP_MAIN_GETTER_ONLY(Version, const char *, ModuleCore::getVersion)
    RMS_ELEM_MAP_MAIN_ACCESSORS(EepromKey, unsigned long, ModuleStockage::setEepromKey, ModuleStockage::getEepromKey)
    RMS_ELEM_MAP_MAIN_ACCESSORS(RouterName, const char *, ModuleCore::setRouterName, ModuleCore::getRouterName)
    RMS_ELEM_MAP_MAIN_ACCESSORS(FixProbeName, const char *, ModuleCore::setFixProbeName, ModuleCore::getFixProbeName)
    RMS_ELEM_MAP_MAIN_ACCESSORS(MobileProbeName, const char *, ModuleCore::setMobileProbeName, ModuleCore::getMobileProbeName)
    RMS_ELEM_MAP_MAIN_ACCESSORS(TemperatureName, const char *, ModuleCore::setTemperatureName, ModuleCore::getTemperatureName)
    RMS_ELEM_MAP_MAIN_ACCESSORS(WifiSsid, const char *, ModuleWifi::setWifiSsid, ModuleWifi::getWifiSsid)
    RMS_ELEM_MAP_MAIN_ACCESSORS(WifiPassword, const char *, ModuleWifi::setWifiPassword, ModuleWifi::getWifiPassword)
    RMS_ELEM_MAP_MAIN_ACCESSORS(DhcpOn, bool, ModuleWifi::setDhcpOn, ModuleWifi::getDhcpOn)
    RMS_ELEM_MAP_MAIN_ACCESSORS(StaticIp, unsigned long, ModuleWifi::setStaticIp, ModuleWifi::getStaticIp)
    RMS_ELEM_MAP_MAIN_ACCESSORS(Gateway, unsigned long, ModuleWifi::setGateway, ModuleWifi::getGateway)
    RMS_ELEM_MAP_MAIN_ACCESSORS(Netmask, unsigned long, ModuleWifi::setNetmask, ModuleWifi::getNetmask)
    RMS_ELEM_MAP_MAIN_ACCESSORS(Dns, unsigned long, ModuleWifi::setDns, ModuleWifi::getDns)
    // use name instead of enum to allow better better compatibility
    RMS_ELEM_MAP_MAIN_ACCESSORS(Source, const char *, ModulePowerMeter::setSourceByName, ModulePowerMeter::getSourceName)
    RMS_ELEM_MAP_MAIN_GETTER_ONLY(DataSource, const char *, ModulePowerMeter::getDataSourceName)
    // External IP common to multiple power meter sources
    RMS_ELEM_MAP_MAIN_ACCESSORS(ExtIp, unsigned long, ModulePowerMeter::setExtIp, ModulePowerMeter::getExtIp)
    RMS_ELEM_MAP_MAIN_ACCESSORS(CalibU, unsigned short, ModulePowerMeter::setCalibU, ModulePowerMeter::getCalibU)
    RMS_ELEM_MAP_MAIN_ACCESSORS(CalibI, unsigned short, ModulePowerMeter::setCalibI, ModulePowerMeter::getCalibI)
    RMS_ELEM_MAP_MAIN_ACCESSORS(EnphaseUser, const char *, ModulePowerMeterEnphase::setUser, ModulePowerMeterEnphase::getUser)
    RMS_ELEM_MAP_MAIN_ACCESSORS(EnphasePwd, const char *, ModulePowerMeterEnphase::setPwd, ModulePowerMeterEnphase::getPwd)
    RMS_ELEM_MAP_MAIN_ACCESSORS(EnphaseSerial, unsigned long, ModulePowerMeterEnphase::setSerial, ModulePowerMeterEnphase::getSerial)
    RMS_ELEM_MAP_MAIN_ACCESSORS(ShellyEmPhases, unsigned short, ModulePowerMeterShellyEm::setPhasesNumber, ModulePowerMeterShellyEm::getPhasesNumber)
    RMS_ELEM_MAP_MAIN_ACCESSORS(MqttRepeat, unsigned short, ModuleMQTT::setRepeat, ModuleMQTT::getRepeat)
    RMS_ELEM_MAP_MAIN_ACCESSORS(MqttIp, unsigned long, ModuleMQTT::setIp, ModuleMQTT::getIp)
    RMS_ELEM_MAP_MAIN_ACCESSORS(MqttPort, unsigned short, ModuleMQTT::setPort, ModuleMQTT::getPort)
    RMS_ELEM_MAP_MAIN_ACCESSORS(MqttUser, const char *, ModuleMQTT::setUser, ModuleMQTT::getUser)
    RMS_ELEM_MAP_MAIN_ACCESSORS(MqttPwd, const char *, ModuleMQTT::setPwd, ModuleMQTT::getPwd)
    RMS_ELEM_MAP_MAIN_ACCESSORS(MqttPrefix, const char *, ModuleMQTT::setPrefix, ModuleMQTT::getPrefix)
    RMS_ELEM_MAP_MAIN_ACCESSORS(MqttDeviceName, const char *, ModuleMQTT::setDeviceName, ModuleMQTT::getDeviceName)
    RMS_ELEM_MAP_MAIN_GETTER_ONLY(Temperature, float, ModuleSensor::getTemperature)
    RMS_ELEM_MAP_MAIN_ACCESSORS(TempoEdfOn, bool, ModuleEDF::setTempo, ModuleEDF::getTempo)

    // Triggers
    // (not using macro is OK)
    void elemSetTriggersCount(byte value, void* context)
    {
        ModuleTriggers::setTriggersCount(value);
    }
    byte elemGetTriggersCount(void* context)
    {
        return ModuleTriggers::getTriggersCount();
    }

    // Trigger sub elements
    RMS_ELEM_MAP_TRIGGER_ACCESSORS(Title, const char *, String, .c_str())
    RMS_ELEM_MAP_TRIGGER_ACCESSORS(Host, const char *, String, .c_str())
    RMS_ELEM_MAP_TRIGGER_ACCESSORS(Port, unsigned short, , )
    RMS_ELEM_MAP_TRIGGER_ACCESSORS(OrdreOn, const char *, String, .c_str())
    RMS_ELEM_MAP_TRIGGER_ACCESSORS(OrdreOff, const char *, String, .c_str())
    RMS_ELEM_MAP_TRIGGER_ACCESSORS(Repeat, unsigned short, , )
    RMS_ELEM_MAP_TRIGGER_ACCESSORS(Tempo, unsigned short, , )
    RMS_ELEM_MAP_TRIGGER_ACCESSORS(React, byte, , )

    // RMS_ELEM_MAP_TRIGGER_ACCESSORS(Active, byte, , )
    byte elemGetTriggerActive(void *context) { return (((Action *)context)->Active); }
    void elemSetTriggerActive(byte value, void *context) {
        if (value >= Action::CUTTING_MODE_ERROR)
            value = Action::CUTTING_MODE_NONE;
        (((Action *)context)->Active) = (Action::cutting_mode_t)(value);
    }
    // Trigger periods
    RMS_ELEM_MAP_TRIGGER_ACCESSORS(PeriodsCount, unsigned short, , )

    // Trigger Periods sub sub elements
    RMS_ELEM_MAP_TRIGGER_PERIOD_ACCESSORS(Hfin, unsigned short)
    RMS_ELEM_MAP_TRIGGER_PERIOD_ACCESSORS(Hdeb, unsigned short)
    RMS_ELEM_MAP_TRIGGER_PERIOD_ACCESSORS(Vmin, unsigned short)
    RMS_ELEM_MAP_TRIGGER_PERIOD_ACCESSORS(Vmax, unsigned short)
    RMS_ELEM_MAP_TRIGGER_PERIOD_ACCESSORS(Tinf, unsigned short)
    RMS_ELEM_MAP_TRIGGER_PERIOD_ACCESSORS(Tsup, unsigned short)
    RMS_ELEM_MAP_TRIGGER_PERIOD_ACCESSORS(Tarif, byte)
    // RMS_ELEM_MAP_TRIGGER_PERIOD_ACCESSORS(Type, byte)
    byte elemGetTriggerPeriodType(void *context) { return RMS_ELEM_MAP_CONTEXT_TRIGGER_PERIOD_ARRAY_ATTR_ELEM_TUPLE(context, Type); }
    void elemSetTriggerPeriodType(byte value, void *context) { 
        if (value >= Action::TRIGGER_TYPE_ERROR)
            value = Action::TRIGGER_TYPE_NONE;
        RMS_ELEM_MAP_CONTEXT_TRIGGER_PERIOD_ARRAY_ATTR_ELEM_TUPLE(context, Type) = (Action::trigger_type_t) value;
    }

    // helpers
    String e2s(elem_map_t* elem, void* context) {
        switch (elem->type) {
        case TYPE_BOOL:
            return String(elem->getter.getBool(context));
        case TYPE_BYTE:
            return String(elem->getter.getByte(context));
        case TYPE_SHORT:
            return String(elem->getter.getShort(context));
        case TYPE_USHORT:
            return String(elem->getter.getUShort(context));
        case TYPE_LONG:
            return String(elem->getter.getLong(context));
        case TYPE_ULONG:
            return String(elem->getter.getULong(context));
        case TYPE_FLOAT:
            return String(elem->getter.getFloat(context));
        case TYPE_CSTRING:
            return String(elem->getter.getCString(context));
        }
        return String("*error*");
    }
    void s2e(elem_map_t* elem, const String& str, void* context) {
        switch (elem->type) {
        case TYPE_BOOL:
            elem->setter.setBool(str.toInt(), context);
            break;
        case TYPE_BYTE:
            elem->setter.setByte(str.toInt(), context);
            break;
        case TYPE_SHORT:
            elem->setter.setShort(str.toInt(), context);
            break;
        case TYPE_USHORT:
            elem->setter.setUShort(str.toInt(), context);
            break;
        case TYPE_LONG:
            elem->setter.setLong(str.toInt(), context);
            break;
        case TYPE_ULONG:
            elem->setter.setULong(strtoul(str.c_str(), NULL, 10), context);
            break;
        case TYPE_FLOAT:
            elem->setter.setFloat(str.toFloat(), context);
            break;
        case TYPE_CSTRING:
            elem->setter.setCString(str.c_str(), context);
            break;
        }
    }

    
    
} // namespace ModuleElemMap
