// ********************
// Gestion des Actions
// ********************
#pragma once

#include <Arduino.h>

class Action
{
private:
    int Idx; // Index
    void CallExterne(String host, String url, int port);
    int T_LastAction = 0;
    int tempoTimer = 0;

public:
    enum trigger_type_t : byte {
        TRIGGER_TYPE_NONE = 0,
        TRIGGER_TYPE_OFF, // must be first after none
        TRIGGER_TYPE_ON,
        TRIGGER_TYPE_PW,
        TRIGGER_TYPE_TRIAC,
        TRIGGER_TYPE_ERROR // must be last
    };

    enum cutting_mode_t : byte {
        CUTTING_MODE_NONE = 0,
        CUTTING_MODE_SINUS_OR_RELAY, // must be first after none
        CUTTING_MODE_MULTI_SINUS,
        CUTTING_MODE_SINUS_TRAIN,
        CUTTING_MODE_ERROR // must be last
    };

    Action(); // Constructeur par defaut
    Action(int aIdx);

    void Definir(String ligne);
    String Lire();
    void Activer(float Pw, int Heure, float Temperature, int Ltarfbin);
    void Arreter();
    void RelaisOn();

    trigger_type_t TypeEnCours(int Heure, float Temperature, int Ltarfbin);
    int Valmin(int Heure);
    int Valmax(int Heure);
    void InitGpio();
    Action::cutting_mode_t Active;
    int Port;
    int Repeat;
    int Tempo;
    String Title;
    String Host;
    String OrdreOn;
    String OrdreOff;
    int Gpio;
    int OutOn;
    int OutOff;
    byte React;
    byte PeriodsCount;
    bool On;
    trigger_type_t Type[8];
    int Hdeb[8];
    int Hfin[8];
    int Vmin[8];
    int Vmax[8];
    int Tinf[8];
    int Tsup[8];
    byte Tarif[8];
};
