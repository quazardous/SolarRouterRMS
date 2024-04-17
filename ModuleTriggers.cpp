#include "Actions.h"
#include "ModuleTriggers.h"
#include "ModuleTime.h"
#include "ModulePowerMeter.h"
#include "ModuleSensor.h"
#include "ModuleEDF.h"
#include "hardware.h"
#include "helpers.h"

/**
 * Handle the actions / triggers
 */
namespace ModuleTriggers
{
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

    //Variables in RAM for interruptions
    volatile unsigned long mtsLastIT = 0;
    volatile int IT10ms = 0;     //Interruption avant deglitch
    volatile int IT10ms_in = 0;  //Interruption apres deglitch
    volatile int ITmode = 0;     //IT externe Triac ou interne

    hw_timer_t *timer = NULL;
    hw_timer_t *timer10ms = NULL;

    int tabPulseSinusOn[101];
    int tabPulseSinusTotal[101];

    void setup()
    {
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

    void startIntTimers() {
        // Interruptions du Triac
        attachInterrupt(RMS_PIN_ZERO_CROSS, onZeroCrossPowerRising, RISING);

        // Hardware timer 100uS
        timer = timerBegin(0, 80, true);  //Clock Divider, 1 micro second Tick
        timerAttachInterrupt(timer, &onTimer, true);
        timerAlarmWrite(timer, 100, true);  //Interrupt every 100 Ticks or microsecond
        timerAlarmEnable(timer);

        // Hardware timer 10ms
        timer10ms = timerBegin(1, 80, true);  //Clock Divider, 1 micro second Tick
        timerAttachInterrupt(timer10ms, &onTimer10ms, true);
        timerAlarmWrite(timer10ms, 10000, true);  //Interrupt every 10000 Ticks or microsecond
        timerAlarmEnable(timer10ms);
    }

    // Interruption du Triac Signal Zc, toutes les 10ms
    void IRAM_ATTR onZeroCrossPowerRising() {
        IT10ms++;

        if ((millis() - mtsLastIT) < 2) return; // to avoid glitch detection during 2ms
        ITmode += 2;

        if (ITmode > 5) ITmode = 5;
        IT10ms_in++;
        mtsLastIT = millis();
        if (ITmode > 0) GestionIT_10ms();  // IT synchrone avec le secteur signal Zc
    }

    //Interruptions, Current Zero Crossing from Triac device and Internal Timer
    //*************************************************************************
    void IRAM_ATTR onTimer10ms() {  //Interruption interne toutes 10ms
        ITmode--;
        if (ITmode < -5) ITmode = -5;
        if (ITmode < 0) GestionIT_10ms();  // IT non synchrone avec le secteur . Horloge interne
    }

    // Interruption Timer interne toutes les 100 micro secondes
    void IRAM_ATTR onTimer() {  //Interruption every 100 micro second
        if (Actif[0] == 1) {      // Découpe Sinus
            PulseComptage[0] += 1;
            if (PulseComptage[0] > Retard[0] && Retard[0] < 98 && ITmode > 0) {  //100 steps in 10 ms
                digitalWrite(RMS_PIN_PULSE_TRIAC, HIGH);                                    //Activate Triac
            } else {
                digitalWrite(RMS_PIN_PULSE_TRIAC, LOW);  //Stop Triac
            }
        }
    }

    void GestionIT_10ms() {
        for (int i = 0; i < NbActions; i++) {
            switch (Actif[i]) {  //valeur en RAM
            case 0:            //Inactif

                break;
            case 1:  //Decoupe Sinus uniquement pour Triac
                if (i == 0) {
                    PulseComptage[0] = 0;
                    digitalWrite(RMS_PIN_PULSE_TRIAC, LOW);  //Stop Découpe Triac
                }
                break;
            default:              // Multi Sinus ou Train de sinus
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
                        if (ITmode < 0 && i == 0 ) RetardF[i] = 100;
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

} // namespace ModuleTriggers