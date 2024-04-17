// ***************************
// Stockage des données en ROM
// ***************************
#include "ModuleStockage.h"
#include <EEPROM.h> //Librairie pour le stockage en EEPROM historique quotidien
#include "ModuleDebug.h"
#include "ModulePowerMeter.h"
#include "ModuleCore.h"
#include "helpers.h"

#define CLE_Rom_Init 812567808 // Valeur pour tester si ROM vierge ou pas. Un changement de valeur remet à zéro toutes les données. / Value to test whether blank ROM or not.

// Plan stockage
#define EEPROM_SIZE 4090
#define NbJour 370              // Nb jour historique stocké
#define adr_HistoAn 0           // taille 2* 370*4=1480
#define adr_E_T_soutire0 1480   // 1 long. Taille 4 Triac
#define adr_E_T_injecte0 1484   // 1 long. Taille 4.
#define adr_E_M_soutire0 1488   // 1 long. Taille 4 Maison
#define adr_E_M_injecte0 1492   // 1 long. Taille 4
#define adr_DateCeJour 1496     // String 8+1
#define adr_lastStockConso 1505 // Short taille 2
#define adr_ParaActions 1507    // Clé + ensemble parametres peu souvent modifiés

namespace ModuleStockage
{
    unsigned long Cle_ROM;
    unsigned long previousHistoryTock;
    unsigned long previousTimer2sMillis;
    String nomSondeMobile = "Données Maison";
    String nomSondeFixe = "Données seconde sonde";
    String nomTemperature = "Température";

    String GS = String((char)29);  //Group Separator
    String RS = String((char)30);  //Record Separator
    int P_cent_EEPROM;

    int idxPromDuJour = 0;
    // int adr_debut_para = 0;  //Adresses Para après le Wifi
    long EAS_T_J0 = 0;
    long EAI_T_J0 = 0;
    long EAS_M_J0 = 0;  //Debut du jour energie active
    long EAI_M_J0 = 0;

    int IdxStock2s = 0;
    int IdxStockPW = 0;

    long EnergieJour_T_Injectee = 0;
    long EnergieJour_M_Injectee = 0;
    long EnergieJour_T_Soutiree = 0;
    long EnergieJour_M_Soutiree = 0;

    int tabPw_Maison_5mn[600];  //Puissance Active:Soutiré-Injecté toutes les 5mn
    int tabPw_Triac_5mn[600];
    int tabTemperature_5mn[600];
    int tabPw_Maison_2s[300];   //Puissance Active: toutes les 2s
    int tabPw_Triac_2s[300];    //Puissance Triac: toutes les 2s
    int tabPva_Maison_2s[300];  //Puissance Active: toutes les 2s
    int tabPva_Triac_2s[300];

    void setup()
    {
        INIT_EEPROM();
        // Lecture Clé pour identifier si la ROM a déjà été initialisée
        Cle_ROM = CLE_Rom_Init;
        unsigned long Rcle = LectureCle();
        Serial.println("cle : " + String(Rcle));
        if (Rcle == Cle_ROM)
        { // Programme déjà executé
            LectureEnROM();
            LectureConsoMatinJour();
            resetGpioActions();
        }
        else
        {
            RAZ_Histo_Conso();
        }
    }

    void loopTimer(unsigned long msNow)
    {
        // we will trigger the first history save in 10 seconds
        previousHistoryTock = msNow - 290000;
        previousTimer2sMillis = msNow;
    }

    void loop(unsigned long msLoop)
    {
        // Archivage et envois des mesures périodiquement
        //**********************************************
        if (!ModulePowerMeter::sourceIsValid())
            return;

        unsigned long msNow = millis();

        // Historique consommation par pas de 5mn
        if (TICKTOCK(msNow, previousHistoryTock, 300000))
        {
            tabPw_Maison_5mn[IdxStockPW] = PuissanceS_M - PuissanceI_M;
            tabPw_Triac_5mn[IdxStockPW] = PuissanceS_T - PuissanceI_T;
            tabTemperature_5mn[IdxStockPW] = int(temperature);
            IdxStockPW = (IdxStockPW + 1) % 600;
        }

        if (TICKTOCK(msNow, previousTimer2sMillis, 2000))
        {
            tabPw_Maison_2s[IdxStock2s] = PuissanceS_M - PuissanceI_M;
            tabPw_Triac_2s[IdxStock2s] = PuissanceS_T - PuissanceI_T;
            tabPva_Maison_2s[IdxStock2s] = PVAS_M - PVAI_M;
            tabPva_Triac_2s[IdxStock2s] = PVAS_T - PVAI_T;
            IdxStock2s = (IdxStock2s + 1) % 300;
            envoiAuMQTT();
            JourHeureChange();
            EnergieQuotidienne();
        }
    }

    void INIT_EEPROM(void)
    {
        if (!EEPROM.begin(EEPROM_SIZE))
        {
            Serial.println("Failed to initialise EEPROM");
            Serial.println("Restarting...");
            ModuleDebug::getDebug().println("Failed to initialise EEPROM");
            ModuleDebug::getDebug().println("Restarting...");
            delay(10000);
            ESP.restart();
        }
    }

    // called if the day changes
    void onNewDay()
    {
        if (!ModulePowerMeter::sourceIsValid())
            return;

        // Données recues
        idxPromDuJour = (idxPromDuJour + 1 + NbJour) % NbJour;
        // On enregistre les conso en début de journée pour l'historique de l'année
        long energie = Energie_M_Soutiree - Energie_M_Injectee; // Bilan energie du jour
        EEPROM.writeLong(idxPromDuJour * 4, energie);
        EEPROM.writeULong(adr_E_T_soutire0, long(Energie_T_Soutiree));
        EEPROM.writeULong(adr_E_T_injecte0, long(Energie_T_Injectee));
        EEPROM.writeULong(adr_E_M_soutire0, long(Energie_M_Soutiree));
        EEPROM.writeULong(adr_E_M_injecte0, long(Energie_M_Injectee));
        EEPROM.writeString(adr_DateCeJour, JourCourant);
        EEPROM.writeUShort(adr_lastStockConso, idxPromDuJour);
        EEPROM.commit();
        LectureConsoMatinJour();
    }

    void RAZ_Histo_Conso()
    {
        // Mise a zero Zone stockage
        int Adr_SoutInjec = adr_HistoAn;
        for (int i = 0; i < NbJour; i++)
        {
            EEPROM.writeLong(Adr_SoutInjec, 0);
            Adr_SoutInjec = Adr_SoutInjec + 4;
        }
        EEPROM.writeULong(adr_E_T_soutire0, 0);
        EEPROM.writeULong(adr_E_T_injecte0, 0);
        EEPROM.writeULong(adr_E_M_soutire0, 0);
        EEPROM.writeULong(adr_E_M_injecte0, 0);
        EEPROM.writeString(adr_DateCeJour, "");
        EEPROM.writeUShort(adr_lastStockConso, 0);
        EEPROM.commit();
    }

    void LectureConsoMatinJour(void)
    {
        EAS_T_J0 = EEPROM.readULong(adr_E_T_soutire0); // Triac
        EAI_T_J0 = EEPROM.readULong(adr_E_T_injecte0);
        EAS_M_J0 = EEPROM.readULong(adr_E_M_soutire0); // Maison
        EAI_M_J0 = EEPROM.readULong(adr_E_M_injecte0);
        DateCeJour = EEPROM.readString(adr_DateCeJour);
        idxPromDuJour = EEPROM.readUShort(adr_lastStockConso);
        if (Energie_T_Soutiree < EAS_T_J0)
        {
            Energie_T_Soutiree = EAS_T_J0;
        }
        if (Energie_T_Injectee < EAI_T_J0)
        {
            Energie_T_Injectee = EAI_T_J0;
        }
        if (Energie_M_Soutiree < EAS_M_J0)
        {
            Energie_M_Soutiree = EAS_M_J0;
        }
        if (Energie_M_Injectee < EAI_M_J0)
        {
            Energie_M_Injectee = EAI_M_J0;
        }
    }

    String HistoriqueEnergie1An(void)
    {
        String S = "";
        int Adr_SoutInjec = 0;
        long EnergieJour = 0;
        long DeltaEnergieJour = 0;
        int iS = 0;
        long lastDay = 0;

        for (int i = 0; i < NbJour; i++)
        {
            iS = (idxPromDuJour + i + 1) % NbJour;
            Adr_SoutInjec = adr_HistoAn + iS * 4;
            EnergieJour = EEPROM.readLong(Adr_SoutInjec);
            if (lastDay == 0)
            {
                lastDay = EnergieJour;
            }
            DeltaEnergieJour = EnergieJour - lastDay;
            lastDay = EnergieJour;
            S += String(DeltaEnergieJour) + ",";
        }
        return S;
    }

    unsigned long LectureCle()
    {
        return EEPROM.readULong(adr_ParaActions);
    }

    void LectureEnROM()
    {
        int Hdeb;
        int address = adr_ParaActions;
        Cle_ROM = EEPROM.readULong(address);
        address += sizeof(unsigned long);
        ssid = EEPROM.readString(address);
        address += ssid.length() + 1;
        password = EEPROM.readString(address);
        address += password.length() + 1;
        dhcpOn = EEPROM.readByte(address);
        address += sizeof(byte);
        IP_Fixe = EEPROM.readULong(address);
        address += sizeof(unsigned long);
        Gateway = EEPROM.readULong(address);
        address += sizeof(unsigned long);
        masque = EEPROM.readULong(address);
        address += sizeof(unsigned long);
        dns = EEPROM.readULong(address);
        address += sizeof(unsigned long);
        Source = EEPROM.readString(address);
        address += Source.length() + 1;
        RMSextIP = EEPROM.readULong(address);
        address += sizeof(unsigned long);
        EnphaseUser = EEPROM.readString(address);
        address += EnphaseUser.length() + 1;
        EnphasePwd = EEPROM.readString(address);
        address += EnphasePwd.length() + 1;
        EnphaseSerial = EEPROM.readString(address);
        address += EnphaseSerial.length() + 1;
        MQTTRepet = EEPROM.readUShort(address);
        address += sizeof(unsigned short);
        MQTTIP = EEPROM.readULong(address);
        address += sizeof(unsigned long);
        MQTTPort = EEPROM.readUShort(address);
        address += sizeof(unsigned short);
        MQTTUser = EEPROM.readString(address);
        address += MQTTUser.length() + 1;
        MQTTPwd = EEPROM.readString(address);
        address += MQTTPwd.length() + 1;
        MQTTPrefix = EEPROM.readString(address);
        address += MQTTPrefix.length() + 1;
        MQTTdeviceName = EEPROM.readString(address);
        address += MQTTdeviceName.length() + 1;
        String nomRouteur = EEPROM.readString(address);
        ModuleCore::setName(nomRouteur.c_str());
        address += nomRouteur.length() + 1;
        nomSondeFixe = EEPROM.readString(address);
        address += nomSondeFixe.length() + 1;
        nomSondeMobile = EEPROM.readString(address);
        address += nomSondeMobile.length() + 1;
        nomTemperature = EEPROM.readString(address);
        address += nomTemperature.length() + 1;
        CalibU = EEPROM.readUShort(address);
        address += sizeof(unsigned short);
        CalibI = EEPROM.readUShort(address);
        address += sizeof(unsigned short);
        TempoEDFon = EEPROM.readByte(address);
        address += sizeof(byte);
        // Zone des actions
        NbActions = EEPROM.readUShort(address);
        address += sizeof(unsigned short);
        for (int iAct = 0; iAct < NbActions; iAct++)
        {
            LesActions[iAct].Active = EEPROM.readByte(address);
            address += sizeof(byte);
            LesActions[iAct].Titre = EEPROM.readString(address);
            address += LesActions[iAct].Titre.length() + 1;
            LesActions[iAct].Host = EEPROM.readString(address);
            address += LesActions[iAct].Host.length() + 1;
            LesActions[iAct].Port = EEPROM.readUShort(address);
            address += sizeof(unsigned short);
            LesActions[iAct].OrdreOn = EEPROM.readString(address);
            address += LesActions[iAct].OrdreOn.length() + 1;
            LesActions[iAct].OrdreOff = EEPROM.readString(address);
            address += LesActions[iAct].OrdreOff.length() + 1;
            LesActions[iAct].Repet = EEPROM.readUShort(address);
            address += sizeof(unsigned short);
            LesActions[iAct].Tempo = EEPROM.readUShort(address);
            address += sizeof(unsigned short);
            LesActions[iAct].React = EEPROM.readByte(address);
            address += sizeof(byte);
            LesActions[iAct].NbPeriode = EEPROM.readByte(address);
            address += sizeof(byte);
            Hdeb = 0;
            for (byte i = 0; i < LesActions[iAct].NbPeriode; i++)
            {
                LesActions[iAct].Type[i] = EEPROM.readByte(address);
                address += sizeof(byte);
                LesActions[iAct].Hfin[i] = EEPROM.readUShort(address);
                LesActions[iAct].Hdeb[i] = Hdeb;
                Hdeb = LesActions[iAct].Hfin[i];
                address += sizeof(unsigned short);
                LesActions[iAct].Vmin[i] = EEPROM.readShort(address);
                address += sizeof(unsigned short);
                LesActions[iAct].Vmax[i] = EEPROM.readShort(address);
                address += sizeof(unsigned short);
                LesActions[iAct].Tinf[i] = EEPROM.readShort(address);
                address += sizeof(unsigned short);
                LesActions[iAct].Tsup[i] = EEPROM.readShort(address);
                address += sizeof(unsigned short);
                LesActions[iAct].Tarif[i] = EEPROM.readByte(address);
                address += sizeof(byte);
            }
        }
        Calibration(address);
    }
    int EcritureEnROM()
    {
        int address = adr_ParaActions;
        EEPROM.writeULong(address, Cle_ROM);
        address += sizeof(unsigned long);
        EEPROM.writeString(address, ssid);
        address += ssid.length() + 1;
        EEPROM.writeString(address, password);
        address += password.length() + 1;
        EEPROM.writeByte(address, dhcpOn);
        address += sizeof(byte);
        EEPROM.writeULong(address, IP_Fixe);
        address += sizeof(unsigned long);
        EEPROM.writeULong(address, Gateway);
        address += sizeof(unsigned long);
        EEPROM.writeULong(address, masque);
        address += sizeof(unsigned long);
        EEPROM.writeULong(address, dns);
        address += sizeof(unsigned long);
        EEPROM.writeString(address, Source);
        address += Source.length() + 1;
        EEPROM.writeULong(address, RMSextIP);
        address += sizeof(unsigned long);
        EEPROM.writeString(address, EnphaseUser);
        address += EnphaseUser.length() + 1;
        EEPROM.writeString(address, EnphasePwd);
        address += EnphasePwd.length() + 1;
        EEPROM.writeString(address, EnphaseSerial);
        address += EnphaseSerial.length() + 1;
        EEPROM.writeUShort(address, MQTTRepet);
        address += sizeof(unsigned short);
        EEPROM.writeULong(address, MQTTIP);
        address += sizeof(unsigned long);
        EEPROM.writeUShort(address, MQTTPort);
        address += sizeof(unsigned short);
        EEPROM.writeString(address, MQTTUser);
        address += MQTTUser.length() + 1;
        EEPROM.writeString(address, MQTTPwd);
        address += MQTTPwd.length() + 1;
        EEPROM.writeString(address, MQTTPrefix);
        address += MQTTPrefix.length() + 1;
        EEPROM.writeString(address, MQTTdeviceName);
        address += MQTTdeviceName.length() + 1;
        EEPROM.writeString(address, nomRouteur);
        address += nomRouteur.length() + 1;
        EEPROM.writeString(address, nomSondeFixe);
        address += nomSondeFixe.length() + 1;
        EEPROM.writeString(address, nomSondeMobile);
        address += nomSondeMobile.length() + 1;
        EEPROM.writeString(address, nomTemperature);
        address += nomTemperature.length() + 1;
        EEPROM.writeUShort(address, CalibU);
        address += sizeof(unsigned short);
        EEPROM.writeUShort(address, CalibI);
        address += sizeof(unsigned short);
        EEPROM.writeByte(address, TempoEDFon);
        address += sizeof(byte);
        // Enregistrement des Actions
        EEPROM.writeUShort(address, NbActions);
        address += sizeof(unsigned short);
        for (int iAct = 0; iAct < NbActions; iAct++)
        {
            EEPROM.writeByte(address, LesActions[iAct].Active);
            address += sizeof(byte);
            EEPROM.writeString(address, LesActions[iAct].Titre);
            address += LesActions[iAct].Titre.length() + 1;
            EEPROM.writeString(address, LesActions[iAct].Host);
            address += LesActions[iAct].Host.length() + 1;
            EEPROM.writeUShort(address, LesActions[iAct].Port);
            address += sizeof(unsigned short);
            EEPROM.writeString(address, LesActions[iAct].OrdreOn);
            address += LesActions[iAct].OrdreOn.length() + 1;
            EEPROM.writeString(address, LesActions[iAct].OrdreOff);
            address += LesActions[iAct].OrdreOff.length() + 1;
            EEPROM.writeUShort(address, LesActions[iAct].Repet);
            address += sizeof(unsigned short);
            EEPROM.writeUShort(address, LesActions[iAct].Tempo);
            address += sizeof(unsigned short);
            EEPROM.writeByte(address, LesActions[iAct].React);
            address += sizeof(byte);
            EEPROM.writeByte(address, LesActions[iAct].NbPeriode);
            address += sizeof(byte);
            for (byte i = 0; i < LesActions[iAct].NbPeriode; i++)
            {
                EEPROM.writeByte(address, LesActions[iAct].Type[i]);
                address += sizeof(byte);
                EEPROM.writeUShort(address, LesActions[iAct].Hfin[i]);
                address += sizeof(unsigned short);
                EEPROM.writeShort(address, LesActions[iAct].Vmin[i]);
                address += sizeof(unsigned short);
                EEPROM.writeShort(address, LesActions[iAct].Vmax[i]);
                address += sizeof(unsigned short);
                EEPROM.writeShort(address, LesActions[iAct].Tinf[i]);
                address += sizeof(unsigned short);
                EEPROM.writeShort(address, LesActions[iAct].Tsup[i]);
                address += sizeof(unsigned short);
                EEPROM.writeByte(address, LesActions[iAct].Tarif[i]);
                address += sizeof(byte);
            }
        }
        Calibration(address);
        EEPROM.commit();
        return address;
    }
    void Calibration(int address)
    {
        // done in setters
        // kV = KV * CalibU / 1000; // Calibration coefficient to be applied
        // kI = KI * CalibI / 1000;
        P_cent_EEPROM = int(100 * address / EEPROM_SIZE);
        Serial.println("Mémoire EEPROM utilisée : " + String(P_cent_EEPROM) + "%");
        ModuleDebug::getDebug().println("Mémoire EEPROM utilisée : " + String(P_cent_EEPROM) + "%");
    }

    // ***********************************
    // * Calage Zéro Energie quotidienne * -
    // ***********************************

    void EnergieQuotidienne()
    {
        if (!DATEvalid)
            return;

        if (Source == "Ext")
            return;

        if (Energie_M_Soutiree < EAS_M_J0 || EAS_M_J0 == 0)
        {
            EAS_M_J0 = Energie_M_Soutiree;
        }
        EnergieJour_M_Soutiree = Energie_M_Soutiree - EAS_M_J0;
        if (Energie_M_Injectee < EAI_M_J0 || EAI_M_J0 == 0)
        {
            EAI_M_J0 = Energie_M_Injectee;
        }
        EnergieJour_M_Injectee = Energie_M_Injectee - EAI_M_J0;
        if (Energie_T_Soutiree < EAS_T_J0 || EAS_T_J0 == 0)
        {
            EAS_T_J0 = Energie_T_Soutiree;
        }
        EnergieJour_T_Soutiree = Energie_T_Soutiree - EAS_T_J0;
        if (Energie_T_Injectee < EAI_T_J0 || EAI_T_J0 == 0)
        {
            EAI_T_J0 = Energie_T_Injectee;
        }
        EnergieJour_T_Injectee = Energie_T_Injectee - EAI_T_J0;
    }

    void setEepromKey(unsigned long key)
    {
        Cle_ROM = key;
    }

    unsigned long getEepromKey()
    {
        return Cle_ROM;
    }
} // namespace ModuleStockage
