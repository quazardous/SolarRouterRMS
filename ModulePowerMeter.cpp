#include "ModulePowerMeter.h"

namespace ModulePowerMeter
{
    bool EnergieActiveValide = false;
    
    void setup() {

        //Tableau Longueur Pulse et Longueur Trame pour Multi-Sinus de 0 à 100%
        float erreur;
        float vrai;
        float target;
        for (int I = 0; I < 101; I++) {
            tabPulseSinusTotal[I] = -1;
            tabPulseSinusOn[I] = -1;
            target = float(I) / 100.0;
            for (int T = 20; T < 101; T++) {
            for (int N = 0; N <= T; N++) {
                if (T % 2 == 1 || N % 2 == 0) {  // Valeurs impair du total ou pulses pairs pour éviter courant continu
                vrai = float(N) / float(T);
                erreur = abs(vrai - target);
                if (erreur < 0.004) {
                    tabPulseSinusTotal[I] = T;
                    tabPulseSinusOn[I] = N;
                    N = 101;
                    T = 101;
                }
                }
            }
            }
        }

        init_puissance();
        //Adaptation à la Source
        Serial.println("Source : " + Source);

        if (Source == "UxIx2") {
            Setup_UxIx2();
        }
        if (Source == "UxI") {
            Setup_UxI();
        }
        if (Source == "Linky") {
            Setup_Linky();
        }
        if (Source == "Enphase") {
            Setup_Enphase();
        }
        if (Source == "Ext") {
        } else {
            Source_data = Source;
        }
    }

    void realtimeLoop() {
        unsigned long tps = millis();
        float deltaT = float(tps - previousTimeRMS);
        previousTimeRMS = tps;
        previousTimeRMSMin = min(previousTimeRMSMin, deltaT);
        previousTimeRMSMin = previousTimeRMSMin + 0.001;
        previousTimeRMSMax = max(previousTimeRMSMax, deltaT);
        previousTimeRMSMax = previousTimeRMSMax * 0.9999;
        previousTimeRMSMoy = deltaT * 0.001 + previousTimeRMSMoy * 0.999;


        //Recupération des données RMS
        //******************************
        if (tps - LastRMS_Millis > PeriodeProgMillis) {  //Attention delicat pour eviter pb overflow
            LastRMS_Millis = tps;
            unsigned long ralenti = long(PuissanceS_M / 10);  // On peut ralentir échange sur Wifi si grosse puissance en cours
            if (Source == "UxI") {
                LectureUxI();
                PeriodeProgMillis = 40;
            }
            if (Source == "UxIx2") {
                LectureUxIx2();
                PeriodeProgMillis = 400;
            }
            if (Source == "Linky") {
                LectureLinky();
                PeriodeProgMillis = 2;
            }
            if (Source == "Enphase") {
                LectureEnphase();
                LastRMS_Millis = millis();
                PeriodeProgMillis = 200 + ralenti;  //On s'adapte à la vitesse réponse Envoye-S metered
            }
            if (Source == "SmartG") {
                LectureSmartG();
                LastRMS_Millis = millis();
                PeriodeProgMillis = 200 + ralenti;  //On s'adapte à la vitesse réponse SmartGateways
            }
            if (Source == "ShellyEm") {
                LectureShellyEm();
                LastRMS_Millis = millis();
                PeriodeProgMillis = 200 + ralenti;  //On s'adapte à la vitesse réponse ShellyEm
            }
            if (Source == "Ext") {
                CallESP32_Externe();
                LastRMS_Millis = millis();
                PeriodeProgMillis = 200 + ralenti;  //Après pour ne pas surchargé Wifi
            }
        }
        delay(2);
    }

    //*************
    //* Test Pmax *
    //*************
    float PfloatMax(float Pin) {
        float P = max(-PmaxReseau, Pin);
        P = min(PmaxReseau, P);
        return P;
    }

    int PintMax(int Pin) {
        int M = int(PmaxReseau);
        int P = max(-M, Pin);
        P = min(M, P);
        return P;
    }
    
    void filtre_puissance() {  //Filtre RC

        float A = 0.3;  //Coef pour un lissage en multi-sinus et train de sinus sur les mesures de puissance courte
        float B = 0.7;
        if (!LissageLong) {
            A = 1;
            B = 0;
        }

        Puissance_T_moy = A * (PuissanceS_T_inst - PuissanceI_T_inst) + B * Puissance_T_moy;
        if (Puissance_T_moy < 0) {
            PuissanceI_T = -int(Puissance_T_moy);  //Puissance Watt affichée en entier  Triac
            PuissanceS_T = 0;
        } else {
            PuissanceS_T = int(Puissance_T_moy);
            PuissanceI_T = 0;
        }


        Puissance_M_moy = A * (PuissanceS_M_inst - PuissanceI_M_inst) + B * Puissance_M_moy;
        if (Puissance_M_moy < 0) {
            PuissanceI_M = -int(Puissance_M_moy);  //Puissance Watt affichée en entier Maison
            PuissanceS_M = 0;
        } else {
            PuissanceS_M = int(Puissance_M_moy);
            PuissanceI_M = 0;
        }


        PVA_T_moy = A * (PVAS_T_inst - PVAI_T_inst) + B * PVA_T_moy;  //Puissance VA affichée en entiers
        if (PVA_T_moy < 0) {
            PVAI_T = -int(PVA_T_moy);
            PVAS_T = 0;
        } else {
            PVAS_T = int(PVA_T_moy);
            PVAI_T = 0;
        }

        PVA_M_moy = A * (PVAS_M_inst - PVAI_M_inst) + B * PVA_M_moy;
        if (PVA_M_moy < 0) {
            PVAI_M = -int(PVA_M_moy);
            PVAS_M = 0;
        } else {
            PVAS_M = int(PVA_M_moy);
            PVAI_M = 0;
        }
    }

    void init_puissance() {
        PuissanceS_T = 0;
        PuissanceS_M = 0;
        PuissanceI_T = 0;
        PuissanceI_M = 0;  //Puissance Watt affichée en entiers Maison et Triac
        PVAS_T = 0;
        PVAS_M = 0;
        PVAI_T = 0;
        PVAI_M = 0;  //Puissance VA affichée en entiers Maison et Triac
        PuissanceS_T_inst = 0.0;
        PuissanceS_M_inst = 0.0;
        PuissanceI_T_inst = 0.0;
        PuissanceI_M_inst = 0.0;
        PVAS_T_inst = 0.0;
        PVAS_M_inst = 0.0;
        PVAI_T_inst = 0.0;
        PVAI_M_inst = 0.0;
        Puissance_T_moy = 0.0;
        Puissance_M_moy = 0.0;
        PVA_T_moy = 0.0;
        PVA_M_moy = 0.0;
    }
} // namespace ModulePowerMeter