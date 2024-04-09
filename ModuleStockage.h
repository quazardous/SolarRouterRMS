#pragma once

//Plan stockage
#define EEPROM_SIZE 4090
#define NbJour 370             //Nb jour historique stocké
#define adr_HistoAn 0          //taille 2* 370*4=1480
#define adr_E_T_soutire0 1480  // 1 long. Taille 4 Triac
#define adr_E_T_injecte0 1484
#define adr_E_M_soutire0 1488    // 1 long. Taille 4 Maison
#define adr_E_M_injecte0 1492    // 1 long. Taille 4
#define adr_DateCeJour 1496      // String 8+1
#define adr_lastStockConso 1505  // Short taille 2
#define adr_ParaActions 1507     //Clé + ensemble parametres peu souvent modifiés

#include <EEPROM.h>        //Librairie pour le stockage en EEPROM historique quotidien

namespace ModuleStockage
{
    void setup();
    void loop();
} // namespace ModuleStockage