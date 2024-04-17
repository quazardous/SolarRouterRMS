// *******************************************************
// * Recherche Info Tempo EDF pour les sources non Linky *
// *******************************************************
#pragma once

#include <Arduino.h>

namespace ModuleEDF
{
    // events
    void setup();
    void setupTimer(unsigned long msNow);
    void loop(unsigned long msLoop);

    // setters / getters
    void setTempo(bool tempo);
    bool getTempo();
    void setLTARF(const char *ltarf);
    const char *getLTARF();
    unsigned int getBinaryLTARF();
    void setSTGE(const char *stge);
    const char *getSTGE();
} // namespace ModuleEDF
