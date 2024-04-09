#include "Actions.h"
#include "ModuleTriggers.h"

/**
 * Handle the actions / triggers
 */
namespace ModuleTriggers
{
    //Actions
    Action LesActions[LesActionsLength];  //Liste des actions
    volatile int NbActions = 0;

    void setup() {
        for (int i = 0; i < LesActionsLength; i++) {
            LesActions[i] = Action(i);  //Creation objets
            PulseOn[i] = 0;             //1/2 sinus
            PulseTotal[i] = 100;
            PulseComptage[i] = 0;
            Retard[i] = 100;
            RetardF[i] = 100;
            OutOn[i] = 1;
            OutOff[i] = 0;
            Gpio[i] = -1;
        }
        Gpio[0] = pulseTriac;
        LesActions[0].Gpio = pulseTriac;
    }
    void loop() {

    }

    // ************
    // *  ACTIONS *
    // ************
    void GestionOverproduction() {
        float SeuilPw;
        float MaxTriacPw;
        float GainBoucle;
        int Type_En_Cours = 0;
        bool lissage = false;
        //Puissance est la puissance en entrée de maison. >0 si soutire. <0 si injecte
        //Cas du Triac. Action 0
        float Puissance = float(PuissanceS_M - PuissanceI_M);
        if (NbActions == 0) LissageLong = true;  //Cas d'un capteur seul et actions déporté sur autre ESP
        for (int i = 0; i < NbActions; i++) {
            Actif[i] = LesActions[i].Actif;
            if (Actif[i] >= 2) lissage = true;                                                    //En RAM
            Type_En_Cours = LesActions[i].TypeEnCours(HeureCouranteDeci, temperature, LTARFbin);  //0=NO,1=OFF,2=ON,3=PW,4=Triac
            if (Actif[i] > 0 && Type_En_Cours > 1 && DATEvalid) {                                 // On ne traite plus le NO
            if (Type_En_Cours == 2) {
                RetardF[i] = 0;
            } else {  // 3 ou 4
                SeuilPw = float(LesActions[i].Valmin(HeureCouranteDeci));
                MaxTriacPw = float(LesActions[i].Valmax(HeureCouranteDeci));
                GainBoucle = float(LesActions[i].Reactivite);  //Valeur stockée dans Port
                if (Actif[i] == 1 && i > 0) {                                            //Les relais en On/Off
                if (Puissance > MaxTriacPw) { RetardF[i] = 100; }                      //OFF
                if (Puissance < SeuilPw) { RetardF[i] = 0; }                           //On
                } else {                                                                 // le Triac ou les relais en sinus
                RetardF[i] = RetardF[i] + 0.0001;                                      //On ferme très légèrement si pas de message reçu. Sécurité
                RetardF[i] = RetardF[i] + (Puissance - SeuilPw) * GainBoucle / 10000;  // Gain de boucle de l'asservissement
                if (RetardF[i] < 100 - MaxTriacPw) { RetardF[i] = 100 - MaxTriacPw; }
                if (ITmode < 0 && i==0 ) RetardF[i] = 100;  //Triac pas possible sur synchro interne
                }
                if (RetardF[i] < 0) { RetardF[i] = 0; }
                if (RetardF[i] > 100) { RetardF[i] = 100; }
            }
            } else {
            RetardF[i] = 100;
            }
            Retard[i] = int(RetardF[i]);  //Valeure entiere pour piloter le Triac et les relais
            if (Retard[i] == 100) {       // Force en cas d'arret des IT
            LesActions[i].Arreter();
            PulseOn[i] = 0;  //Stop Triac ou relais
            } else {

            switch (Actif[i]) {  //valeur en RAM du Mode de regulation
                case 1:            //Decoupe Sinus pour Triac ou On/Off pour relais
                if (i > 0) LesActions[i].RelaisOn();
                break;
                case 2:  // Multi Sinus
                PulseOn[i] = tabPulseSinusOn[100 - Retard[i]];
                PulseTotal[i] = tabPulseSinusTotal[100 - Retard[i]];
                break;
                case 3:  // Train de Sinus
                PulseOn[i] = 100 - Retard[i];
                PulseTotal[i] = 99;  //Nombre impair pour éviter courant continu
                break;
            }
            }
        }
        LissageLong = lissage;
    }

    void InitGpioActions() {
        for (int i = 1; i < NbActions; i++) {
            LesActions[i].InitGpio();
            Gpio[i] = LesActions[i].Gpio;
            OutOn[i] = LesActions[i].OutOn;
            OutOff[i] = LesActions[i].OutOff;
        }
    }
} // namespace ModuleTriggers