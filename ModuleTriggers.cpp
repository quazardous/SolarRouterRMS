#include "Actions.h"
#include "ModuleTriggers.h"
#include "ModuleTime.h"
#include "ModulePowerMeter.h"
#include "ModuleSensor.h"
#include "ModuleEeprom.h"
#include "ModuleEDF.h"
#include "hardware.h"
#include "helpers.h"
#include "rms.h"

/**
 * Handle the actions / triggers
 */
namespace ModuleTriggers
{
    void handleOverProduction() ;

    struct it_counters_s {
        volatile int it_10ms = 0; // Interruption avant deglitch
        volatile int it_10ms_in = 0; // Interruption apres deglitch
        volatile int it_mode = 0; // IT externe Triac ou interne [-5,+5]
    };

    //Actions
    unsigned long previousOverProdMillis;

    Action LesActions[RMS_TRIGGERS_MAX];  //Liste des actions

    // using volatile to ensure consistency between loops and interrupts
    volatile byte NbActions = 0;
    volatile int Retard[RMS_TRIGGERS_MAX];
    volatile Action::cutting_mode_t Actif[RMS_TRIGGERS_MAX];
    volatile int PulseOn[RMS_TRIGGERS_MAX];
    volatile int PulseTotal[RMS_TRIGGERS_MAX];
    volatile int PulseComptage[RMS_TRIGGERS_MAX];
    volatile int Gpio[RMS_TRIGGERS_MAX];
    volatile int OutOn[RMS_TRIGGERS_MAX];
    volatile int OutOff[RMS_TRIGGERS_MAX];
    // TODO: use ATOMIC? https://www.arduino.cc/reference/en/language/variables/variable-scope-qualifiers/volatile/
     //Actions et Triac(action 0)
    float RetardF[RMS_TRIGGERS_MAX];  //Floating value of retard

    // Variables in RAM for interruptions
    volatile unsigned long ms_last_IT = 0; // millis()
    // internal IT counters used for process coupling
    it_counters_s it_counters;
    // flags and freezed values
    it_counters_infos_s it_counters_infos;

    hw_timer_t *timer = NULL;
    hw_timer_t *timer10ms = NULL;

    int tabPulseSinusOn[101];
    int tabPulseSinusTotal[101];

    void boot()
    {
        resetGpioActions();

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
                    // Valeurs impair du total ou pulses pairs pour éviter courant continu
                    if (T % 2 == 1 || N % 2 == 0) {
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

        for (int i = 0; i < RMS_TRIGGERS_MAX; i++) {
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
        Gpio[0] = RMS_PIN_PULSE_TRIAC;
        LesActions[0].Gpio = RMS_PIN_PULSE_TRIAC;
    }

    void loopTimer(unsigned long mtsNow) {
        previousOverProdMillis = mtsNow;
    }

    void loop(unsigned long msLoop) {
        if (!ModulePowerMeter::sourceIsValid()) return;
        unsigned long msNow = millis();
        if (TICKTOCK(msNow, previousOverProdMillis, 200)) {
            handleOverProduction();
        }
    }

    // Interruptions, Current Zero Crossing from Triac device and Internal Timer
    // *************************************************************************

    // main IT handler
    void handleIT10ms() {
        for (int i = 0; i < NbActions; i++) {
            switch (Actif[i]) {  //valeur en RAM
            case Action::CUTTING_MODE_NONE:
                // Inactif
                break;

            case Action::CUTTING_MODE_SINUS_OR_RELAY:
                // Decoupe Sinus uniquement pour Triac
                if (i == 0) {
                    PulseComptage[0] = 0;
                    digitalWrite(RMS_PIN_PULSE_TRIAC, LOW);  // Stop Découpe Triac
                }
                break;

            default:
                // Multi Sinus ou Train de sinus
                if (Gpio[i] > 0) {  //Gpio valide
                    if (PulseComptage[i] < PulseOn[i]) {
                        digitalWrite(Gpio[i], OutOn[i]);
                    } else {
                        digitalWrite(Gpio[i], OutOff[i]);  //Stop
                    }
                    PulseComptage[i]++;
                    if (PulseComptage[i] >= PulseTotal[i]) {
                        PulseComptage[i] = 0;
                    }
                }
                break;
            }
        }
    }

    // Interruption du Triac Signal Zc, toutes les 10ms (2 times in 50Hz = 100Hz)
    void IRAM_ATTR onZeroCrossPowerRising() {
        it_counters.it_10ms++;

        // to avoid glitch detection during 2ms
        if ((millis() - ms_last_IT) < 2) return;
        it_counters.it_mode += 2; // now we are 2 beats after

        if (it_counters.it_mode > 5) it_counters.it_mode = 5;
        it_counters.it_10ms_in++;
        ms_last_IT = millis();
        if (it_counters.it_mode > 0) handleIT10ms();  // IT synchrone avec le secteur signal Zc
    }

    // Interruption Timer interne toutes les 100 micro secondes
    // Interruption every 100 micro second
    void IRAM_ATTR onTimer100us() {
        // Découpe Sinus
        if (Actif[0] == Action::CUTTING_MODE_SINUS_OR_RELAY) {
            PulseComptage[0] += 1;
            // 100 steps in 10 ms
            if (PulseComptage[0] > Retard[0] && Retard[0] < 98 && it_counters.it_mode > 0) {
                // Activate Triac
                digitalWrite(RMS_PIN_PULSE_TRIAC, HIGH);                                    
            } else {
                //Stop Triac
                digitalWrite(RMS_PIN_PULSE_TRIAC, LOW);
            }
        }
    }

    // Interruption interne toutes 10ms
    void IRAM_ATTR onTimer10ms() {
        it_counters.it_mode--;
        if (it_counters.it_mode < -5) it_counters.it_mode = -5;
        if (it_counters.it_mode < 0) handleIT10ms();  // IT non synchrone avec le secteur. Horloge interne
    }

    // Entry point for the timers
    void startIntTimers() {
        // Interruptions du Triac
        attachInterrupt(RMS_PIN_ZERO_CROSS, onZeroCrossPowerRising, RISING);

        // Hardware timer 100uS
        timer = timerBegin(0, 80, true);  //Clock Divider, 1 micro second Tick
        timerAttachInterrupt(timer, &onTimer100us, true);
        timerAlarmWrite(timer, 100, true);  //Interrupt every 100 Ticks or microsecond
        timerAlarmEnable(timer);

        // Hardware timer 10ms
        timer10ms = timerBegin(1, 80, true);  //Clock Divider, 1 micro second Tick
        timerAttachInterrupt(timer10ms, &onTimer10ms, true);
        timerAlarmWrite(timer10ms, 10000, true);  //Interrupt every 10000 Ticks or microsecond
        timerAlarmEnable(timer10ms);
    }


    // ************
    // *  ACTIONS *
    // ************
    void handleOverProduction() {
        float SeuilPw;
        float MaxTriacPw;
        float GainBoucle;
        Action::trigger_type_t Type_En_Cours = Action::TRIGGER_TYPE_NONE;
        bool lissage = false;
        // Cas du Triac. Action 0
        if (NbActions == 0) {
            // Cas d'un capteur seul et actions déporté sur autre ESP
            ModulePowerMeter::setSlowSmoothing(true);
            return;
        }
        // Puissance est la puissance en entrée de maison. >0 si soutire. <0 si injecte
        float Puissance = ModulePowerMeter::getPower();
        bool timeIsValid = ModuleTime::timeIsValid();
        float temperature = ModuleSensor::getTemperature();
        float HeureCouranteDeci = decimal_hour() * 100;
        int binLTARF = ModuleEDF::getBinaryLTARF();
        for (byte i = 0; i < NbActions; i++) {
            Actif[i] = LesActions[i].Active;
            if (Actif[i] >= 2)
                lissage = true; // En RAM
            // 0=NO, 1=OFF, 2=ON, 3=PW, 4=Triac
            Type_En_Cours = LesActions[i].TypeEnCours(int(HeureCouranteDeci), temperature, binLTARF);
            // NB: On ne traite plus le NO
            if (Actif[i] > 0 && Type_En_Cours > Action::TRIGGER_TYPE_OFF && timeIsValid) {
                if (Type_En_Cours == Action::TRIGGER_TYPE_ON) {
                    RetardF[i] = 0;
                } else {  // 3 ou 4
                    SeuilPw = float(LesActions[i].Valmin(HeureCouranteDeci));
                    MaxTriacPw = float(LesActions[i].Valmax(HeureCouranteDeci));
                    GainBoucle = float(LesActions[i].React);  //Valeur stockée dans Port
                    if (Actif[i] == 1 && i > 0) {
                        // Les relais en On/Off
                        if (Puissance > MaxTriacPw) {
                            // OFF
                            RetardF[i] = 100;
                        }                   
                        if (Puissance < SeuilPw) {
                            // ON
                            RetardF[i] = 0;
                        }
                    } else {
                        // le Triac ou les relais en sinus                                                                 
                        // On ferme très légèrement si pas de message reçu. Sécurité
                        RetardF[i] = RetardF[i] + 0.0001;
                        // Gain de boucle de l'asservissement
                        RetardF[i] = RetardF[i] + (Puissance - SeuilPw) * GainBoucle / 10000;
                        if (RetardF[i] < 100 - MaxTriacPw) {
                            RetardF[i] = 100 - MaxTriacPw;
                        }
                        // Triac pas possible sur synchro interne
                        if (it_counters.it_mode < 0 && i == 0 ) RetardF[i] = 100;
                    }
                    if (RetardF[i] < 0) { RetardF[i] = 0; }
                    if (RetardF[i] > 100) { RetardF[i] = 100; }
                }
            } else {
                RetardF[i] = 100;
            }
            // Valeur entiere pour piloter le Triac et les relais
            Retard[i] = int(RetardF[i]);
            if (Retard[i] == 100) {
                // Force en cas d'arret des IT
                LesActions[i].Arreter();
                // Stop Triac ou relais
                PulseOn[i] = 0;
            } else {
                // valeur en RAM du Mode de regulation
                switch (Actif[i])
                {
                case Action::CUTTING_MODE_SINUS_OR_RELAY:
                    // Decoupe Sinus pour Triac ou On/Off pour relais
                    if (i > 0)
                        LesActions[i].RelaisOn();
                    break;
                case Action::CUTTING_MODE_MULTI_SINUS:
                    // Multi Sinus
                    PulseOn[i] = tabPulseSinusOn[100 - Retard[i]];
                    PulseTotal[i] = tabPulseSinusTotal[100 - Retard[i]];
                    break;
                case Action::CUTTING_MODE_SINUS_TRAIN:
                    // Train de Sinus
                    PulseOn[i] = 100 - Retard[i];
                    // Nombre impair pour éviter courant continu
                    PulseTotal[i] = 99;
                    break;
                }
            }
        }
        ModulePowerMeter::setSlowSmoothing(lissage);
    }

    // setters / getters
    void setTriggersCount(byte nbActions) {
        NbActions = nbActions;
    }
    byte getTriggersCount() {
        return NbActions;
    }
    Action *getTriggers() {
        return LesActions;
    }
    Action *getTrigger(byte i) {
        return &LesActions[i];
    }
    it_counters_infos_s *getItCountersInfos(bool $check) {
        if ($check)
            checkItStatus();
        return &it_counters_infos;
    }

    // helpers
    void resetGpioActions() {
        for (int i = 1; i < NbActions; i++) {
            LesActions[i].InitGpio();
            Gpio[i] = LesActions[i].Gpio;
            OutOn[i] = LesActions[i].OutOn;
            OutOff[i] = LesActions[i].OutOff;
        }
    }

    byte incrTriggersCount() {
        return ++NbActions;
    }

    int getDelay(byte i)
    {
        return Retard[i];
    }

    void checkItStatus() {
        // Check IT status
        it_counters.it_10ms = 0;
        it_counters.it_10ms_in = 0;
        // skip some beats..
        delay(15);

        it_counters_infos.it_10ms = it_counters.it_10ms;
        it_counters_infos.it_10ms_in = it_counters.it_10ms_in;
        it_counters_infos.it_mode = it_counters.it_mode;
        it_counters_infos.triac = (int) (it_counters.it_10ms_in > 0);
        it_counters_infos.synchronized = (int) (it_counters_infos.it_mode > 0);
    }

    void httpAjaxTriggersStates(WebServer& server, String& S) {
        String RS = RMS_RS;
        String GS = RMS_GS;
        int NbActifs = 0;
        S = "";
        for (int i = 0; i < NbActions; i++)
        {
            if (LesActions[i].Active > 0)
            {
                S += String(i) + RS + LesActions[i].Title + RS;
                if (LesActions[i].Active == 1 && i > 0)
                {
                    if (LesActions[i].On)
                    {
                        S += "On" + RS;
                    }
                    else
                    {
                        S += "Off" + RS;
                    }
                }
                else
                {
                    S += String(100 - Retard[i]) + RS;
                }
                S += GS;
                NbActifs++;
            }
        }
        S = String(ModuleSensor::getTemperature()) 
            + GS + String(ModulePowerMeter::getDataSourceName())
            + GS + String(ModulePowerMeter::getExtIp())
            + GS + NbActifs + GS + S;
    }

    void httpAjaxTriggers(WebServer& server, String& S) {
        String RS = RMS_RS;
        String GS = RMS_GS;
        S = String(ModuleSensor::getTemperature()) 
            + RS + String(ModuleEDF::getBinaryLTARF()) 
            + RS + String(it_counters.it_mode) + GS;
        for (int i = 0; i < NbActions; i++)
        {
            S += LesActions[i].Lire();
        }
    }

    void httpUpdateTriggers(WebServer& server, String& S) {
        String RS = RMS_RS;
        String GS = RMS_GS;
        int adresse_max = 0;
        String s = server.arg("actions");
        String ligne = "";
        resetGpioActions(); // RAZ anciennes actions
        NbActions = 0;
        while (s.indexOf(GS) > 3 && NbActions < RMS_TRIGGERS_MAX)
        {
            ligne = s.substring(0, s.indexOf(GS));
            s = s.substring(s.indexOf(GS) + 1);
            LesActions[NbActions].Definir(ligne);
            NbActions++;
        }
        adresse_max = ModuleEeprom::writeEeprom();
        resetGpioActions();
        S = "OK" + String(adresse_max);
    }
} // namespace ModuleTriggers