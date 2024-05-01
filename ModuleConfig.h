#pragma once

#include <Arduino.h>
#include <ArrayList.h>
#include "config.h"
#include "ModuleElem.h"
#include "ModuleServer.h"

// Macro for non contextual element
// persit = NULL for default persist (EEPROM)
#define RMS_CONFIG_ELEM_MAP(GROUP, ELEM, TYPE, Type, setter, getter, persist, ctype, help, label, choices) \
    {ModuleElem::ELEM, ModuleElem::TYPE, {.set ## Type = [](ctype v, void* context) -> void { \
        setter(v); \
    }}, {.get ## Type = [](void* context) -> ctype { \
        return getter(); \
    }}, false, ModuleElem::GROUP, persist, false, label, help, {.choose ## Type = choices }}

namespace ModuleConfig {

    typedef ArrayList<ModuleElem::elem_map_t*> elem_map_list_t;

    // events
    // must be ran after all modules have registered their config
    void boot();
    void loop(unsigned long msLoop);

    // helpers
    void registerConfig(ModuleElem::elem_map_t* elem_map, size_t elem_map_size);
    void e2json(JsonObject &parent, ModuleElem::elem_map_t* e, void* context = NULL);

    // API handlers
    void apiGetConfig(AsyncWebServerRequest* request, JsonDocument& doc);
    void apiPostConfig(AsyncWebServerRequest* request, JsonDocument& in, JsonDocument& out);

} // namespace ModuleConfig
