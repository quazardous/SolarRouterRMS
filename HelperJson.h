#pragma once

#include <Arduino.h>

namespace HelperJson
{
    String StringJson(String nom, String Json);
    String PrefiltreJson(String F1, String F2, String Json);
    float ValJson(String nom, String Json);
    long LongJson(String nom, String Json);
    long shiftedLongJson(String nom, String Json, char sep = ',', int shift = 999);
    float ValJsonSG(String nom, String Json);
} // namespace HelperJson
