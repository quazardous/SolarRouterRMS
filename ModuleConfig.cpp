#include "ModuleConfig.h"
#include "ModuleCore.h"
#include <ArrayList.h>
// #include "HelperJson.h"
// #include "ModuleElem.h"
// #include <Hashtable.h>

void configUpdated(const ModuleConfig::elem_list_t* updated);

#define RMS_CONFIG_POST_CASE(Type, type, toType) \
{ \
    type existingValue = e->getter.get##Type(NULL); \
    type newValue; \
    if (update[name].is<type>()) { \
        newValue = update[name].as<type>(); \
    } else { \
        newValue = String(update[name].as<const char*>()).toType(); \
    } \
    e->setter.set##Type(newValue, NULL); \
    newValue = e->getter.get##Type(NULL); \
    if (e->dirty) { \
        if (e->backup.Type == newValue) { \
            e->dirty = false; \
        } \
    } else { \
        if (existingValue != newValue) { \
            e->backup.Type = existingValue; \
            e->dirty = true; \
        } \
    } \
    if (existingValue != newValue) { \
        updatedElems.add(e->element); \
    } \
    break; \
}

// Module focusing on configuration
// It's a bridge between the web interface and other modules
namespace ModuleConfig {

    elem_map_list_t config_maps;
    // Hashtable<int,elem_map_list_t*> config_groups_lists;
    // ArrayList<ModuleElem::group_t> config_groups;

    void boot() {
        // Sort config_maps by group
        for (size_t i = 0; i < config_maps.size(); i++) {
            ModuleElem::elem_map_t* elem_map = config_maps.get(i);
            ModuleCore::log("Config: " + String(ModuleElem::groupName(elem_map->group)) + " > " + String(ModuleElem::elemName(elem_map->element)) );
            // const int group = (int) elem_map->group;
            // if (!config_groups_lists.containsKey(group)) {
            //     config_groups_lists.put(group, new elem_map_list_t());
            // }
            // (*config_groups_lists.get(group))->add(elem_map);
        }
    }

    elem_list_t updatedElems;
    unsigned long lastUpdate = 0;
    unsigned long updateDispatched = 0;

    void loop(unsigned long msLoop) {
        if (lastUpdate > updateDispatched) {
            updateDispatched = millis();
            elem_list_t updatedElemsUnique;
            for (size_t i = 0; i < updatedElems.size(); i++) {
                ModuleElem::elem_t elem = updatedElems.get(i);
                if (!updatedElemsUnique.contains(elem)) {
                    updatedElemsUnique.add(elem);
                }
            }
            updatedElems.clear();
            // dispatch public event for other modules
            ::configUpdated(&updatedElemsUnique);
        }
    }

    // helpers
    void registerConfig(ModuleElem::elem_map_t* elem_map, size_t elem_map_size) {
        for (size_t i = 0; i < elem_map_size; i++) {
            config_maps.add(&(elem_map[i]));
        }
    }

    void e2json(JsonObject &parent, ModuleElem::elem_map_t* e, void* context) {
        JsonObject obj = parent.createNestedObject(ModuleElem::elemName(e->element));
        obj["group"] = ModuleElem::groupName(e->group);
        obj["type"] = ModuleElem::typeName(e->type);
        obj["readonly"] = e->readonly;
        obj["dirty"] = e->dirty;
        if (e->label != NULL) {
            obj["label"] = e->label;
        }
        if (e->help != NULL) {
            obj["help"] = e->help;
        }
        size_t len;
        bool exhaustive;
        switch (e->type) {
            case ModuleElem::TYPE_BYTE:
                obj["value"] = e->getter.getByte(context);
                if (e->dirty) {
                    obj["backup"] = e->backup.Byte;
                }
                if (e->choices.chooseByte != NULL) {
                    JsonArray choices = obj.createNestedArray("choices");
                    const byte* values = e->choices.chooseByte(&len, &exhaustive, context);
                    for (size_t i = 0; i < len; i++) {
                        choices.add(values[i]);
                    }
                }
                break;
            case ModuleElem::TYPE_USHORT:
                obj["value"] = e->getter.getUShort(context);
                if (e->dirty) {
                    obj["backup"] = e->backup.UShort;
                }
                if (e->choices.chooseUShort != NULL) {
                    JsonArray choices = obj.createNestedArray("choices");
                    const unsigned short* values = e->choices.chooseUShort(&len, &exhaustive, context);
                    for (size_t i = 0; i < len; i++) {
                        choices.add(values[i]);
                    }
                }
                break;
            case ModuleElem::TYPE_SHORT:
                obj["value"] = e->getter.getShort(context);
                if (e->dirty) {
                    obj["backup"] = e->backup.Short;
                }
                if (e->choices.chooseShort != NULL) {
                    JsonArray choices = obj.createNestedArray("choices");
                    const short* values = e->choices.chooseShort(&len, &exhaustive, context);
                    for (size_t i = 0; i < len; i++) {
                        choices.add(values[i]);
                    }
                }
                break;
            case ModuleElem::TYPE_ULONG:
            case ModuleElem::TYPE_IP:
                obj["value"] = e->getter.getULong(context);
                if (e->dirty) {
                    obj["backup"] = e->backup.ULong;
                }
                if (e->choices.chooseULong != NULL) {
                    JsonArray choices = obj.createNestedArray("choices");
                    const unsigned long* values = e->choices.chooseULong(&len, &exhaustive, context);
                    for (size_t i = 0; i < len; i++) {
                        choices.add(values[i]);
                    }
                }
                break;
            case ModuleElem::TYPE_LONG:
                obj["value"] = e->getter.getLong(context);
                if (e->dirty) {
                    obj["backup"] = e->backup.Long;
                }
                if (e->choices.chooseLong != NULL) {
                    JsonArray choices = obj.createNestedArray("choices");
                    const long* values = e->choices.chooseLong(&len, &exhaustive, context);
                    for (size_t i = 0; i < len; i++) {
                        choices.add(values[i]);
                    }
                }
                break;
            case ModuleElem::TYPE_BOOL:
                obj["value"] = e->getter.getBool(context);
                if (e->dirty) {
                    obj["backup"] = e->backup.Bool;
                }
                break;
            case ModuleElem::TYPE_FLOAT:
                obj["value"] = e->getter.getFloat(context);
                if (e->dirty) {
                    obj["backup"] = e->backup.Float;
                }
                if (e->choices.chooseFloat != NULL) {
                    JsonArray choices = obj.createNestedArray("choices");
                    const float* values = e->choices.chooseFloat(&len, &exhaustive, context);
                    for (size_t i = 0; i < len; i++) {
                        choices.add(values[i]);
                    }
                }
                break;
            case ModuleElem::TYPE_CSTRING:
                obj["value"] = String(e->getter.getCString(context));
                if (e->dirty) {
                    obj["backup"] = e->backup.CString;
                }
                if (e->choices.chooseCString != NULL) {
                    JsonArray choices = obj.createNestedArray("choices");
                    const char** values = e->choices.chooseCString(&len, &exhaustive, context);
                    for (size_t i = 0; i < len; i++) {
                        choices.add(values[i]);
                    }
                }
                break;
        }
        if (obj.containsKey("choices")) {
            obj["exhaustive"] = exhaustive;
        }
    }

    // API handlers
    void apiGetConfig(AsyncWebServerRequest* request, JsonDocument& doc) {
        JsonObject configs = doc.createNestedObject("configs");
        for (size_t i = 0; i < config_maps.size(); i++) {
            e2json(configs, config_maps.get(i));
        }
    }

    void apiPostConfig(AsyncWebServerRequest* request, JsonDocument& in, JsonDocument& out) {
        JsonObject update = in["update"];
        for (size_t i = 0; i < config_maps.size(); i++) {
            ModuleElem::elem_map_t* e = config_maps.get(i);
            const char *name = ModuleElem::elemName(e->element);
            if (update.containsKey(name)) {
                switch (e->type)
                {
                    case ModuleElem::TYPE_BYTE: RMS_CONFIG_POST_CASE(Byte, byte, toInt)
                    case ModuleElem::TYPE_USHORT: RMS_CONFIG_POST_CASE(UShort, unsigned short, toInt)
                    case ModuleElem::TYPE_SHORT: RMS_CONFIG_POST_CASE(Short, short, toInt)
                    case ModuleElem::TYPE_ULONG: 
                    case ModuleElem::TYPE_IP: RMS_CONFIG_POST_CASE(ULong, unsigned long, toInt)
                    case ModuleElem::TYPE_LONG: RMS_CONFIG_POST_CASE(Long, long, toInt)
                    case ModuleElem::TYPE_FLOAT: RMS_CONFIG_POST_CASE(Float, float, toFloat)
                    case ModuleElem::TYPE_BOOL:
                    {
                        bool existingValue = e->getter.getBool(NULL);
                        bool newValue;
                        if (update[name].as<bool>()) {
                            newValue = update[name].as<bool>();
                        } else {
                            String str = String(update[name].as<const char*>());
                            str.toLowerCase();
                            if (str == "true") {
                                newValue = true;
                            } else if (str == "false") {
                                newValue = false;
                            } else {
                                newValue = str.toInt();
                            }
                        }
                        e->setter.setBool(newValue, NULL);
                        newValue = e->getter.getBool(NULL);
                        if (e->dirty) {
                            if (e->backup.Bool == newValue) {
                                e->dirty = false;
                            }
                        } else {
                            if (existingValue != newValue) {
                                e->backup.Bool = existingValue;
                                e->dirty = true;
                            }
                        }
                        if (existingValue != newValue) {
                            updatedElems.add(e->element);
                        }
                        break;
                    }
                    case ModuleElem::TYPE_CSTRING:
                    {
                        String existingValue = String(e->getter.getCString(NULL));
                        String newValue = String(update[name].as<const char*>());
                        e->setter.setCString(newValue.c_str(), NULL);
                        newValue = String(e->getter.getCString(NULL));
                        if (e->dirty) {
                            if (strcmp(e->backup.CString, newValue.c_str()) == 0) {
                                e->dirty = false;
                            }
                        } else {
                            if (strcmp(existingValue.c_str(), newValue.c_str()) != 0) {
                                if (e->backup.CString != NULL) {
                                    free((void*)e->backup.CString); // free previous backup (if any)
                                    e->backup.CString = NULL;
                                }
                                e->backup.CString = strdup(existingValue.c_str());
                                e->dirty = true;
                            }
                        }
                        if (strcmp(existingValue.c_str(), newValue.c_str()) != 0) {
                            updatedElems.add(e->element);
                        }
                        break;
                    }
                }
            }
        }
        // tell loop to dispatch updates
        lastUpdate = millis();
        apiGetConfig(request, out);
    }
} // namespace ModuleConfig
