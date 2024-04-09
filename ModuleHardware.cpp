#include "ModuleHardware.h"

namespace ModuleHarware
{
    //****************
    //* Gestion LEDs *
    //****************
    void Gestion_LEDs() {
        int retard_min = 100;
        int retardI;
        cptLEDyellow++;

        if (WiFi.status() != WL_CONNECTED) {  // Attente connexion au Wifi
            if (WiFi.getMode() == WIFI_STA) {   // en  Station mode
            cptLEDyellow = (cptLEDyellow + 6) % 10;
            cptLEDgreen = cptLEDyellow;
            } else {  //AP Mode
            cptLEDyellow = cptLEDyellow % 10;
            cptLEDgreen = (cptLEDyellow + 5) % 10;
            }
        } else {
            for (int i = 0; i < NbActions; i++) {
            retardI = Retard[i];
            retard_min = min(retard_min, retardI);
            }
            if (retard_min < 100) {
            cptLEDgreen = int((cptLEDgreen + 1 + 8 / (1 + retard_min / 10))) % 10;
            } else {
            cptLEDgreen = 10;
            }
        }
        if (cptLEDyellow > 5) {
            digitalWrite(LedYellow, LOW);
        } else {
            digitalWrite(LedYellow, HIGH);
        }
        if (cptLEDgreen > 5) {
            digitalWrite(LedGreen, LOW);
        } else {
            digitalWrite(LedGreen, HIGH);
        }
    }
} // namespace ModuleHarware