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
                break;
            case ModuleElem::TYPE_USHORT:
                obj["value"] = e->getter.getUShort(context);
                break;
            case ModuleElem::TYPE_SHORT:
                obj["value"] = e->getter.getShort(context);
                break;
            case ModuleElem::TYPE_ULONG:
            case ModuleElem::TYPE_IP:
                obj["value"] = e->getter.getULong(context);
                break;
            case ModuleElem::TYPE_LONG:
                obj["value"] = e->getter.getLong(context);
                break;
            case ModuleElem::TYPE_BOOL:
                obj["value"] = e->getter.getBool(context);
                break;
            case ModuleElem::TYPE_FLOAT:
                obj["value"] = e->getter.getFloat(context);
                break;
            case ModuleElem::TYPE_CSTRING:
                obj["value"] = String(e->getter.getCString(context));
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
} // namespace ModuleConfig
