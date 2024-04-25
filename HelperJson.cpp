#include "HelperJson.h"

namespace HelperJson
{
    String StringJson(String nom, String Json) {
        int p = Json.indexOf(nom);
        Json = Json.substring(p);
        p = Json.indexOf(":");
        Json = Json.substring(p + 1);
        p = Json.indexOf("\"");
        Json = Json.substring(p + 1);
        p = Json.indexOf("\"");
        Json = Json.substring(0, p);
        return Json;
    }

    String PrefiltreJson(String F1, String F2, String Json)
    {
        int p = Json.indexOf(F1);
        Json = Json.substring(p);
        p = Json.indexOf(F2);
        Json = Json.substring(p);
        return Json;
    }

    float ValJson(String nom, String Json)
    {
        int p = Json.indexOf(nom);
        Json = Json.substring(p);
        p = Json.indexOf(":");
        Json = Json.substring(p + 1);
        int q = Json.indexOf(",");
        p = Json.indexOf("}");
        p = min(p, q);
        float val = 0;
        if (p > 0)
        {
            Json = Json.substring(0, p);
            val = Json.toFloat();
        }
        return val;
    }

    long LongJson(String nom, String Json)
    {
        // Pour éviter des problèmes d'overflow
        int p = Json.indexOf(nom);
        Json = Json.substring(p);
        p = Json.indexOf(":");
        Json = Json.substring(p + 1);
        int q = Json.indexOf(".");
        p = Json.indexOf("}");
        p = min(p, q);
        long val = 0;
        if (p > 0)
        {
            Json = Json.substring(0, p);
            val = Json.toInt();
        }
        return val;
    }

    long shiftedLongJson(String nom, String Json, char sep, int shift)
    { 
        // Alternative a LongJson au dessus pour extraire chez EDF nb jour Tempo  https://particulier.edf.fr/services/rest/referentiel/getNbTempoDays?TypeAlerte=TEMPO
        int p = Json.indexOf(nom);
        Json = Json.substring(p);
        p = Json.indexOf(":");
        Json = Json.substring(p + 1);
        int q = Json.indexOf(sep); //<==== Recherche d'une virgule et non d'un point
        if (q == -1 && shift > 0)
            q = shift; //  /<==== Ajout de ces 2 lignes pour que la ligne p = min(p, q); ci dessous donne le bon résultat
        p = Json.indexOf("}");
        p = min(p, q);
        long val = 0;
        if (p > 0)
        {
            Json = Json.substring(0, p);
            val = Json.toInt();
        }
        return val;
    }

    float ValJsonSG(String nom, String Json)
    {
        int p = Json.indexOf(nom);
        Json = Json.substring(p);
        p = Json.indexOf(":");
        Json = Json.substring(p + 2);
        p = Json.indexOf(",");
        float val = 0;
        if (p > 0)
        {
            Json = Json.substring(0, p);
            val = Json.toFloat();
        }
        return val;
    }

    void e2json(JsonDocument &doc, ModuleElemMap::elem_map_t* e, void* context) {
        String name = ModuleElemMap::elemName(e->element);
        switch (e->type) {
            case ModuleElemMap::TYPE_BYTE:
                doc[name] = e->getter.getByte(context);
                break;
            case ModuleElemMap::TYPE_USHORT:
                doc[name] = e->getter.getUShort(context);
                break;
            case ModuleElemMap::TYPE_SHORT:
                doc[name] = e->getter.getShort(context);
                break;
            case ModuleElemMap::TYPE_ULONG:
                doc[name] = e->getter.getULong(context);
                break;
            case ModuleElemMap::TYPE_LONG:
                doc[name] = e->getter.getLong(context);
                break;
            case ModuleElemMap::TYPE_BOOL:
                doc[name] = e->getter.getBool(context);
                break;
            case ModuleElemMap::TYPE_FLOAT:
                doc[name] = e->getter.getFloat(context);
                break;
            case ModuleElemMap::TYPE_CSTRING:
                doc[name] = String(e->getter.getCString(context));
                break;
        }
    }
} // namespace HelperJson