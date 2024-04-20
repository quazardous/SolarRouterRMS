/*
  PV Router / Routeur Solaire
  ****************************************
  Version V_8.07ng

  RMS=Routeur Multi Sources

  Choix de 5 sources différentes pour lire la consommation électrique en entrée de maison
  - lecture de la tension avec un transformateur et du courant avec une sonde ampèremétrique (UxI)
  - lecture des données du Linky (Linky)
  - module (JSY-MK-194T) intégrant une mesure de tension secteur et 2 sondes ampèmétriques (UxIx2)
  - Lecture passerelle Enphase - Envoy-S metered (firmware V5 et V7)
  - Lecture depuis un autre ESP qui comprend l'une des 4 sources citées plus haut
  - Lecture compteur SmartG (en test)

  En option une mesure de température (DS18B20) est possible.

  Historique des versions
  - V6.00_RMS Corrige un bug sur le nom du capteur de température et retire les mauvaises mesure de température
              Introduit la source Smart Gateways des compteurs belge, holladais etc. (en test)
              Introduit la source Shelly Em
              Introduit un code tarifaire numérique en plus de LTARF dans les messages MQTT si Linky
              Encode les URL vers Enphase pour passer les mots de passe
  - V6.01_RMS Recadre la courbe des températures sur 48h
              Affiche les valeurs des courbes suivant la postion du curseur en X
              Corrige un bug d'ocillation d'un relais externe pendant 1mn au début de l'action
  - V6.02_RMS Corrige un bug avec un Shelly en triphasé et en injection
  - V7.00_RMS Donne la couleur Tempo du jour et du lendemain donnée par EDF(pour ceux disposant de cette tarfication)
              Facilite la connexion au Wifi local lors de la première connexion
              Pour le Shelly, force Pva=0 quand Pw=0;
  - V7.01_RMS Corrige un bug sur l'affichage de l'énergie quotidienne dans le cas ESP externe
  - V7.02_RMS Prend en compe les caratères non alphabétiques dans le mot de passe réseau
  - V7.03_RMS Corrige un bug d'accès wifi vers Envoy et EDF
  - V8.00_RMS Crée les modes multi-sinus et train de sinus
              Change la bibliothèque pour la liaison série vers Linky ou UxIx2
  - V8.01_RMS Corrige un bug sur l'envoi MQTT du Triac
  - V8.02_RMS Modifie les libellés des relais On/Off dans la page d'accueil
  - V8.03_RMS Lissage voie secondaire pour le capteur Shelly
  - V8.04_RMS Rajoute un lissage sur les puissances uniquement si multi-sinus ou train de sinus avec une mesure de puissance courte
  - V8.05_RMS Rajoute un lissage sur les puissances voie secondaire Shelly
  - V8.06_RMS Rajout device["name"] = nomRouteur; dans DeviceTextToDiscover
              Début couleur Tempo du jour à 6h00 du matin et non Oh
              Modification loi de lissage pour les mesures courtes. Pour les multi-sinus et train-sinus.
              Correction bug sur régulation On/Off
  - V8.07ng   Refactor: split code in modules


  Les détails sont disponibles sur / Details are available here:
  https://f1atb.fr  Section Domotique / Home Automation

  Authors:
  - F1ATB André
  - David Berlioz<berliozdavid@gmail.com>


  AGPL v3.0

*/

// Modules
#include "ModuleCore.h"
#include "ModuleDebug.h"
#include "ModuleServer.h"
#include "ModuleHistory.h"
#include "ModuleEeprom.h"
#include "ModuleSensor.h"
#include "ModuleTime.h"
#include "ModuleWifi.h"
#include "ModuleOTAUpdate.h"
#include "ModuleTriggers.h"
#include "ModuleHardware.h"
#include "ModulePowerMeter.h"
#include "ModuleEDF.h"
#include "ModuleMQTT.h"

// SETUP
//*******
void setup()
{
    // boot
    ModuleCore::boot();
    ModuleHardware::boot();
    ModuleSensor::boot();
    ModuleEeprom::boot();
    ModuleHistory::boot();
    ModuleTime::boot();
    ModuleMQTT::boot();
    ModuleWifi::boot();
    ModuleEDF::boot();
    ModuleDebug::boot(); // init remote debug
    ModuleServer::boot();
    ModuleOTAUpdate::boot();
    ModulePowerMeter::boot();
    ModuleTriggers::boot();

    // Core 0 Task Loop
    // The Power Meter Loop is near real time
    ModulePowerMeter::startPowerMeterLoop();
    // Triac Interrupt and Phase Cutting
    ModuleTriggers::startIntTimers();

    // "fast" setup (timers)
    unsigned long msNow = millis();
    ModuleCore::loopTimer(msNow);
    ModuleHardware::loopTimer(msNow);
    ModuleSensor::loopTimer(msNow);
    ModuleEeprom::loopTimer(msNow);
    ModuleMQTT::loopTimer(msNow);
    ModuleTime::loopTimer(msNow);
    ModuleHistory::loopTimer(msNow);
    ModuleWifi::loopTimer(msNow);
    ModuleEDF::setupTimer(msNow);
    ModuleTriggers::loopTimer(msNow);
}

// Core 1 Default Loop
// The default loop handles non "real time" tasks
void loop()
{
    unsigned long msLoop = millis();
    ModuleCore::loop(msLoop);
    ModuleHardware::loop(msLoop);
    ModuleOTAUpdate::loop(msLoop);
    ModuleTime::loop(msLoop);
    ModuleSensor::loop(msLoop);
    ModuleDebug::loop(msLoop);
    ModuleServer::loop(msLoop); // TODO: use async server
    ModuleEeprom::loop(msLoop);
    ModuleHistory::loop(msLoop);
    ModuleMQTT::loop(msLoop);
    ModuleWifi::loop(msLoop);
    ModuleEDF::loop(msLoop);
    ModuleTriggers::loop(msLoop);
}

void onTime()
{
  // Called once when time is synced with ntp
}

void dayIsGone()
{
  // Called once a day at the end of the day
  // store daily history
  ModuleHistory::dayIsGone();
  // store power states
  ModulePowerMeter::dayIsGone();
}

// void up()
// {
//   // Called once when RMS is (re)up and ready
//   // ModuleCore will check
// }

// void down()
// {
//   // Called once when RMS goes down..
// }

void wifiUp()
{
  // Called once when wifi is up
}

void reboot()
{
  // Called before RMS reboots
}
