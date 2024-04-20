#include "ModuleHistory.h"
#include "ModulePowerMeter.h"
#include "ModuleEeprom.h"
#include "ModuleSensor.h"
#include "rms.h"

namespace ModuleHistory
{
    long histoEnergy[RMS_HISTORY_RANGE];

    short idxDayHisto = 0;

    unsigned long previous5minTock;
    unsigned long previous2secTock;

    // int adr_debut_para = 0;  //Adresses Para après le Wifi
    int IdxStock2s = 0;
    int IdxStockPW = 0;

    int tabPw_Maison_5mn[RMS_HISTORY_5MIN_SIZE];  //Puissance Active:Soutiré-Injecté toutes les 5mn
    int tabPw_Triac_5mn[RMS_HISTORY_5MIN_SIZE];
    int histoTemperature5min[RMS_HISTORY_5MIN_SIZE];
    int tabPw_Maison_2s[RMS_HISTORY_2SEC_SIZE];   //Puissance Active: toutes les 2s
    int tabPw_Triac_2s[RMS_HISTORY_2SEC_SIZE];    //Puissance Triac: toutes les 2s
    int tabPva_Maison_2s[RMS_HISTORY_2SEC_SIZE];  //Puissance Active: toutes les 2s
    int tabPva_Triac_2s[RMS_HISTORY_2SEC_SIZE];

    void boot()
    {
        for (int i = 0; i < RMS_HISTORY_RANGE; i++)
        {
            histoEnergy[i] = 0; // FIXME: use some special value to indicate invalid data
        }
        if (ModuleEeprom::hasData())
        {
            idxDayHisto = ModuleEeprom::readHisto(histoEnergy);
        }
    }

    void loopTimer(unsigned long msNow)
    {
        // we will trigger the first history save in 10 seconds
        previous5minTock = msNow - 290000;
        previous2secTock = msNow;
    }

    void loop(unsigned long msLoop)
    {

        // Archivage et envois des mesures périodiquement
        //**********************************************
        if (!ModulePowerMeter::sourceIsValid())
            return;

        unsigned long msNow = millis();
        // Historique consommation par pas de 5mn
        if (TICKTOCK(msNow, previous5minTock, 300000))
        {
            tabPw_Maison_5mn[IdxStockPW] = ModulePowerMeter::getPower(ModulePowerMeter::DOMAIN_HOUSE);
            tabPw_Triac_5mn[IdxStockPW] = ModulePowerMeter::getPower(ModulePowerMeter::DOMAIN_TRIAC);
            histoTemperature5min[IdxStockPW] = int(ModuleSensor::getTemperature());
            IdxStockPW = (IdxStockPW + 1) % RMS_HISTORY_5MIN_SIZE;
        }

        // Historique consommation par pas de 2s
        if (TICKTOCK(msNow, previous2secTock, 2000))
        {
            tabPw_Maison_2s[IdxStock2s] = ModulePowerMeter::getPower(ModulePowerMeter::DOMAIN_HOUSE);
            tabPw_Triac_2s[IdxStock2s] = ModulePowerMeter::getPower(ModulePowerMeter::DOMAIN_TRIAC);
            tabPva_Maison_2s[IdxStock2s] = ModulePowerMeter::getVAPower(ModulePowerMeter::DOMAIN_HOUSE);
            tabPva_Triac_2s[IdxStock2s] = ModulePowerMeter::getVAPower(ModulePowerMeter::DOMAIN_TRIAC);
            IdxStock2s = (IdxStock2s + 1) % RMS_HISTORY_2SEC_SIZE;
            // ModuleMQTT::envoiAuMQTT(); // keep that in ModuleMQTT
            // EnergieQuotidienne();
        }
    }

    // called if the day changes (end of the day)
    void dayIsGone()
    {
        if (!ModulePowerMeter::sourceIsValid())
            return;

        // Données recues
        idxDayHisto = (idxDayHisto + 1 + RMS_HISTORY_RANGE) % RMS_HISTORY_RANGE;
        long energie = ModulePowerMeter::getEnergy(); // Bilan energie du jour
        histoEnergy[idxDayHisto] = energie;
        // On enregistre les conso en début de journée pour l'historique de l'année
        ModuleEeprom::writeHistoDay(idxDayHisto, energie);
    }

    // setters / getters

    // web handlers
    void httpAjaxHistoriqueEnergie1An(WebServer& server, String& S)
    {
        S = "";
        int Adr_SoutInjec = 0;
        long EnergieJour = 0;
        long DeltaEnergieJour = 0;
        int iS = 0;
        long lastDay = 0;

        for (int i = 0; i < RMS_HISTORY_RANGE; i++)
        {
            // from today
            iS = (idxDayHisto + i + 1) % RMS_HISTORY_RANGE;
            EnergieJour = histoEnergy[iS];
            if (lastDay == 0)
            {
                lastDay = EnergieJour;
            }
            DeltaEnergieJour = EnergieJour - lastDay;
            lastDay = EnergieJour;
            S += String(DeltaEnergieJour) + ",";
        }
    }

    // Envoi Historique de 50h (600points) toutes les 5mn
    void httpAjaxHisto48h(WebServer& server, String& S) {
        String GS = RMS_GS;
        String RS = RMS_RS;
        String HouseH = "";
        String TriacH = "";
        String TempH = "";
        int iS = IdxStockPW;
        for (int i = 0; i < RMS_HISTORY_5MIN_SIZE; i++)
        {
            HouseH += String(tabPw_Maison_5mn[iS]) + ",";
            TriacH += String(tabPw_Triac_5mn[iS]) + ",";
            TempH += String(histoTemperature5min[iS]) + ",";
            iS = (1 + iS) % RMS_HISTORY_5MIN_SIZE;
        }
        // String Ouverture = "";
        //   for (int i = 0; i < NbActions; i++) {
        //     if ((LesActions[i].Actif > 0) && (ITmode > 0 || i > 0)) {
        //     iS = IdxStockPW;
        //     if (LesActions[i].Actif > 0) {
        //         Ouverture += GS;
        //         for (int j = 0; j < 600; j++) {
        //         Ouverture += String(tab_histo_ouverture[i][iS]) + RS;
        //         iS = (1 + iS) % 600;
        //         }
        //         Ouverture += LesActions[i].Titre;
        //     }
        //     }
        // }
        S = String(ModulePowerMeter::getDataSourceName()) 
            + GS + HouseH
            + GS + TriacH
            + GS + String(ModuleSensor::getTemperature())
            + GS + TempH;
    }

    void httpAjaxHisto10mn(WebServer& server, String& S) {
        // Envoi Historique de 10mn (300points)Energie Active Soutiré - Injecté
        String GS = RMS_GS;
        String RS = RMS_RS;
        String H = "";
        String T = "";
        int iS = IdxStock2s;
        for (int i = 0; i < RMS_HISTORY_2SEC_SIZE; i++)
        {
            H += String(tabPw_Maison_2s[iS]) + ",";
            H += String(tabPva_Maison_2s[iS]) + ",";
            T += String(tabPw_Triac_2s[iS]) + ",";
            T += String(tabPva_Triac_2s[iS]) + ",";
            iS = (1 + iS) % RMS_HISTORY_2SEC_SIZE;
        }
        const char *dataSource = ModulePowerMeter::getDataSourceName();
        S = String(dataSource) + GS + H + GS + T;
    }

} // namespace ModuleHistory