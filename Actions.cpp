// ********************
// Gestion des Actions
// ********************
#include "Actions.h"
#include <EEPROM.h>
#include <WiFiClient.h>
#include "ModuleDebug.h"
#include "rms.h"

// Class Action
Action::Action()
{
    Gpio = -1;
}

Action::Action(int aIdx)
{
    Gpio = -1; // si le n° de pin n'est pas valid, on ne fait rien
    Idx = aIdx;
    T_LastAction = int(millis() / 1000);
    On = false;
    Active = CUTTING_MODE_NONE;
    React = 50;
    OutOn = 1;
    OutOff = 0;
    Tempo = 0;
    Repeat = 0;
}

void Action::Arreter()
{
    int Tseconde = int(millis() / 1000);
    if ((Tseconde - T_LastAction) >= Tempo || Idx == 0 || Active != 1)
    {
        if (Gpio > 0 || Idx == 0)
        {
            digitalWrite(Gpio, OutOff);
            T_LastAction = Tseconde;
        }
        else
        {
            if (On || ((Tseconde - T_LastAction) > Repeat && Repeat != 0))
            {
                CallExterne(Host, OrdreOff, Port);
                T_LastAction = Tseconde;
            }
        }
        On = false;
    }
}

void Action::RelaisOn()
{
    int Tseconde = int(millis() / 1000);
    if ((Tseconde - T_LastAction) >= Tempo)
    {
        if (Gpio > 0)
        {
            digitalWrite(Gpio, OutOn);
            T_LastAction = Tseconde;
            On = true;
        }
        else
        {
            if (Active == 1)
            {
                if (!On || ((Tseconde - T_LastAction) > Repeat && Repeat != 0))
                {
                    CallExterne(Host, OrdreOn, Port);
                    T_LastAction = Tseconde;
                }
                On = true;
            }
        }
    }
}

void Action::Definir(String ligne)
{
    String RS = RMS_RS; // Record Separator
    int active = ligne.substring(0, ligne.indexOf(RS)).toInt();
    if (active < 0 || active >= CUTTING_MODE_ERROR)
        active = CUTTING_MODE_NONE;
    Active = cutting_mode_t(active);
    ligne = ligne.substring(ligne.indexOf(RS) + 1);
    Title = ligne.substring(0, ligne.indexOf(RS));
    ligne = ligne.substring(ligne.indexOf(RS) + 1);
    Host = ligne.substring(0, ligne.indexOf(RS));
    ligne = ligne.substring(ligne.indexOf(RS) + 1);
    Port = ligne.substring(0, ligne.indexOf(RS)).toInt();
    ligne = ligne.substring(ligne.indexOf(RS) + 1);
    OrdreOn = ligne.substring(0, ligne.indexOf(RS));
    ligne = ligne.substring(ligne.indexOf(RS) + 1);
    OrdreOff = ligne.substring(0, ligne.indexOf(RS));
    ligne = ligne.substring(ligne.indexOf(RS) + 1);
    Repeat = ligne.substring(0, ligne.indexOf(RS)).toInt();
    Repeat = min(Repeat, 32000);
    Repeat = max(0, Repeat);
    ligne = ligne.substring(ligne.indexOf(RS) + 1);
    Tempo = ligne.substring(0, ligne.indexOf(RS)).toInt();
    Tempo = min(Tempo, 32000);
    Tempo = max(0, Tempo);
    if (Repeat > 0)
    {
        Repeat = max(Tempo + 4, Repeat); // Pour eviter conflit
    }
    ligne = ligne.substring(ligne.indexOf(RS) + 1);
    React = byte(ligne.substring(0, ligne.indexOf(RS)).toInt());
    ligne = ligne.substring(ligne.indexOf(RS) + 1);
    PeriodsCount = byte(ligne.substring(0, ligne.indexOf(RS)).toInt());
    ligne = ligne.substring(ligne.indexOf(RS) + 1);
    int Hdeb_ = 0;
    for (byte i = 0; i < PeriodsCount; i++)
    {
        int type = ligne.substring(0, ligne.indexOf(RS)).toInt();
        if (type < 0 || type >= TRIGGER_TYPE_ERROR)
            type = TRIGGER_TYPE_NONE;
        Type[i] = trigger_type_t(type); // NO,OFF,ON,PW,Triac
        ligne = ligne.substring(ligne.indexOf(RS) + 1);
        Hfin[i] = ligne.substring(0, ligne.indexOf(RS)).toInt();
        Hdeb[i] = Hdeb_;
        Hdeb_ = Hfin[i];
        ligne = ligne.substring(ligne.indexOf(RS) + 1);
        Vmin[i] = ligne.substring(0, ligne.indexOf(RS)).toInt();
        ligne = ligne.substring(ligne.indexOf(RS) + 1);
        Vmax[i] = ligne.substring(0, ligne.indexOf(RS)).toInt();
        ligne = ligne.substring(ligne.indexOf(RS) + 1);
        Tinf[i] = ligne.substring(0, ligne.indexOf(RS)).toInt();
        ligne = ligne.substring(ligne.indexOf(RS) + 1);
        Tsup[i] = ligne.substring(0, ligne.indexOf(RS)).toInt();
        ligne = ligne.substring(ligne.indexOf(RS) + 1);
        Tarif[i] = ligne.substring(0, ligne.indexOf(RS)).toInt();
        ligne = ligne.substring(ligne.indexOf(RS) + 1);
    }
}

String Action::Lire()
{
    String GS = RMS_GS; // Group Separator
    String RS = RMS_RS; // Record Separator
    String S;
    S += String(Active) + RS;
    S += Title + RS;
    S += Host + RS;
    S += String(Port) + RS;
    S += OrdreOn + RS;
    S += OrdreOff + RS;
    S += String(Repeat) + RS;
    S += String(Tempo) + RS;
    S += String(React) + RS;
    S += String(PeriodsCount) + RS;
    for (byte i = 0; i < PeriodsCount; i++)
    {
        S += String(Type[i]) + RS;
        S += String(Hfin[i]) + RS;
        S += String(Vmin[i]) + RS;
        S += String(Vmax[i]) + RS;
        S += String(Tinf[i]) + RS;
        S += String(Tsup[i]) + RS;
        S += String(Tarif[i]) + RS;
    }
    return S + GS;
}

Action::trigger_type_t Action::TypeEnCours(int Heure, float Temperature, int Ltarfbin)
{
    // Retourne type d'action active à cette heure et test temperature OK
    trigger_type_t S = TRIGGER_TYPE_OFF;
    bool TemperatureOk;
    bool TarifOk;
    for (int i = 0; i < PeriodsCount; i++)
    {
        TemperatureOk = true;
        if (Temperature > -100)
        {
            if (Tinf[i] <= 100 && Temperature > Tinf[i])
            {
                TemperatureOk = false;
            }
            if (Tsup[i] <= 100 && Temperature < Tsup[i])
            {
                TemperatureOk = false;
            }
        }
        TarifOk = true;
        if (Ltarfbin > 0 && (Ltarfbin & Tarif[i]) == 0)
            TarifOk = false;
        if (Heure >= Hdeb[i] && Heure <= Hfin[i] && TemperatureOk && TarifOk)
            S = Type[i];
    }
    return S; // 0=NO,1=OFF,2=ON,3=PW,4=Triac
}

int Action::Valmin(int Heure)
{
    // Retourne la valeur Vmin (ex seuil Triac) à cette heure
    int S = 0;
    for (int i = 0; i < PeriodsCount; i++)
    {
        if (Heure >= Hdeb[i] && Heure <= Hfin[i])
        {
            S = Vmin[i];
        }
    }
    return S;
}

int Action::Valmax(int Heure)
{
    // Retourne la valeur Vmax (ex ouverture du Triac) à cette heure
    int S = 0;
    for (int i = 0; i < PeriodsCount; i++)
    {
        if (Heure >= Hdeb[i] && Heure <= Hfin[i])
        {
            S = Vmax[i];
        }
    }
    return S;
}

void Action::InitGpio()
{ 
    // Initialise les sorties GPIO pour des relais
    int p;
    int q;
    String S;

    if (Idx > 0)
    {
        T_LastAction = 0;
        Gpio = -1;
        p = OrdreOn.indexOf("gpio=");
        if (p >= 0)
        {
            S = OrdreOn.substring(p + 5);
            q = S.indexOf("&");
            if (q == -1)
                q = 2;
            Gpio = S.substring(0, q).toInt();
            OutOn = 1 + OrdreOn.indexOf("out=1");
            OutOn = min(OutOn, 1);
            OutOff = (OutOn + 1) % 2;

            if (Gpio <= 0 || Gpio > 33)
                Gpio = -1; // GPIO non valide
            if (Host != "" && Host != "localhost")
                Gpio = -1;
            if (Gpio > 0)
            {
                pinMode(Gpio, OUTPUT);
                digitalWrite(Gpio, OutOff);
            }
        }
    }
}

void Action::CallExterne(String host, String url, int port)
{
    // Use WiFiClient class to create TCP connections
    WiFiClient clientExt;
    char hostbuf[host.length() + 1];
    host.toCharArray(hostbuf, host.length() + 1);

    if (!clientExt.connect(hostbuf, port))
    {
        ModuleDebug::stockMessage("connection to clientExt failed :" + host);
        return;
    }
    clientExt.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (clientExt.available() == 0)
    {
        if (millis() - timeout > 5000)
        {
            ModuleDebug::stockMessage(">>> clientESP_Ext Timeout ! : " + host);
            clientExt.stop();
            return;
        }
    }

    // Read all the lines of the reply from server
    while (clientExt.available())
    {
        String line = clientExt.readStringUntil('\r');
    }
}