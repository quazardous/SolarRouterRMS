#include "ModuleConfig.h"
#include "ModuleCore.h"
#include <ArrayList.h>
// #include "HelperJson.h"
// #include "ModuleElem.h"
// #include <Hashtable.h>

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

    void loop(unsigned long msLoop) {
        // Your code here
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
        switch (e->type) {
            case ModuleElem::TYPE_BYTE:
                obj["value"] = e->getter.getByte(context);
                if (e->dirty) {
                    obj["backup"] = e->backup.Byte;
                }
                if (e->choices.chooseByte != NULL) {
                    JsonArray choices = obj.createNestedArray("choices");
                    size_t len;
                    const byte* values = e->choices.chooseByte(&len, context);
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
                    size_t len;
                    const unsigned short* values = e->choices.chooseUShort(&len, context);
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
                    size_t len;
                    const short* values = e->choices.chooseShort(&len, context);
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
                    size_t len;
                    const unsigned long* values = e->choices.chooseULong(&len, context);
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
                    size_t len;
                    const long* values = e->choices.chooseLong(&len, context);
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
                    size_t len;
                    const float* values = e->choices.chooseFloat(&len, context);
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
                    size_t len;
                    const char** values = e->choices.chooseCString(&len, context);
                    for (size_t i = 0; i < len; i++) {
                        choices.add(values[i]);
                    }
                }
                break;
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
                    case ModuleElem::TYPE_BYTE:
                    {
                        byte existingValue = e->getter.getByte(NULL);
                        byte newValue = update[name];
                        e->setter.setByte(newValue, NULL);
                        newValue = e->getter.getByte(NULL);
                        if (e->dirty) {
                            if (e->backup.Byte == newValue) {
                                e->dirty = false;
                            }
                        } else {
                            if (existingValue != newValue) {
                                e->backup.Byte = existingValue;
                                e->dirty = true;
                            }
                        }
                        break;
                    }
                    case ModuleElem::TYPE_USHORT:
                    {
                        unsigned short existingValue = e->getter.getUShort(NULL);
                        unsigned short newValue;
                        if (update[name].is<unsigned short>()) {
                            newValue = update[name].as<unsigned short>();
                        } else {
                            newValue = String(update[name].as<const char*>()).toInt();
                        }
                        e->setter.setUShort(newValue, NULL);
                        newValue = e->getter.getUShort(NULL);
                        if (e->dirty) {
                            if (e->backup.UShort == newValue) {
                                e->dirty = false;
                            }
                        } else {
                            if (existingValue != newValue) {
                                e->backup.UShort = existingValue;
                                e->dirty = true;
                            }
                        }
                        break;
                    }
                    case ModuleElem::TYPE_SHORT:
                    {
                        short existingValue = e->getter.getShort(NULL);
                        short newValue;
                        if (update[name].is<short>()) {
                            newValue = update[name].as<short>();
                        } else {
                            newValue = String(update[name].as<const char*>()).toInt();
                        }
                        e->setter.setShort(newValue, NULL);
                        newValue = e->getter.getShort(NULL);
                        if (e->dirty) {
                            if (e->backup.Short == newValue) {
                                e->dirty = false;
                            }
                        } else {
                            if (existingValue != newValue) {
                                e->backup.Short = existingValue;
                                e->dirty = true;
                            }
                        }
                        break;
                    }
                    case ModuleElem::TYPE_ULONG:
                    case ModuleElem::TYPE_IP:
                    {
                        unsigned long existingValue = e->getter.getULong(NULL);
                        unsigned long newValue;
                        if (update[name].is<unsigned long>()) {
                            newValue = update[name].as<unsigned long>();
                        } else {
                            newValue = String(update[name].as<const char*>()).toInt();
                        }
                        e->setter.setULong(newValue, NULL);
                        newValue = e->getter.getULong(NULL);
                        if (e->dirty) {
                            if (e->backup.ULong == newValue) {
                                e->dirty = false;
                            }
                        } else {
                            if (existingValue != newValue) {
                                e->backup.ULong = existingValue;
                                e->dirty = true;
                            }
                        }
                        break;
                    }
                    case ModuleElem::TYPE_LONG:
                    {
                        long existingValue = e->getter.getLong(NULL);
                        long newValue;
                        if (update[name].is<long>()) {
                            newValue = update[name].as<long>();
                        } else {
                            newValue = String(update[name].as<const char*>()).toInt();
                        }
                        e->setter.setLong(newValue, NULL);
                        if (e->dirty) {
                            if (e->backup.Long == newValue) {
                                e->dirty = false;
                            }
                        } else {
                            if (existingValue != newValue) {
                                e->backup.Long = existingValue;
                                e->dirty = true;
                            }
                        }
                        break;
                    }
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
                        break;
                    }
                    case ModuleElem::TYPE_FLOAT:
                    {
                        float existingValue = e->getter.getFloat(NULL);
                        float newValue;
                        if (update[name].is<float>()) {
                            newValue = update[name].as<float>();
                        } else {
                            newValue = String(update[name].as<const char*>()).toFloat();
                        }
                        e->setter.setFloat(newValue, NULL);
                        newValue = e->getter.getFloat(NULL);
                        if (e->dirty) {
                            if (e->backup.Float == newValue) {
                                e->dirty = false;
                            }
                        } else {
                            if (existingValue != newValue) {
                                e->backup.Float = existingValue;
                                e->dirty = true;
                            }
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
                        break;
                    }
                }
            }
        }
        apiGetConfig(request, out);
    }
} // namespace ModuleConfig
