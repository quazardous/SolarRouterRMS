#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "ModuleElem.h"

namespace HelperJson
{
    String StringJson(String nom, String Json);
    String PrefiltreJson(String F1, String F2, String Json);
    float ValJson(String nom, String Json);
    long LongJson(String nom, String Json);
    long shiftedLongJson(String nom, String Json, char sep = ',', int shift = 999);
    float ValJsonSG(String nom, String Json);
    void e2json(JsonDocument &doc, ModuleElem::elem_map_t* e, void* context = NULL);
} // namespace HelperJson
