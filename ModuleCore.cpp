#include <esp_task_wdt.h> //Pour un Watchdog
#include "ModuleCore.h"
// #include "hardware.h"

//Watchdog de 180 secondes. Le systeme se Reset si pas de dialoque avec le LINKY ou JSY-MK-194T ou Enphase-Envoye pendant 180s
//Watchdog for 180 seconds. The system resets if no dialogue with the Linky or  JSY-MK-194T or Enphase-Envoye for 180s
#define WDT_TIMEOUT 180

#define SER_BUF_SIZE 4096

namespace ModuleCore
{
    const int AnalogIn0 = 35;  //Pour Routeur Uxi
    const int AnalogIn1 = 32;
    const int AnalogIn2 = 33;  //Note: si GPIO 33 non disponible sur la carte ESP32, utilisez la 34. If GPIO 33 not available on the board replace by GPIO 34

    TaskHandle_t Task1; // Multicoeur - Processeur 0 - Collecte données RMS local ou distant
    
    unsigned long startMillis; // Start time
    unsigned long previousLoop; // Timer for CPU load
    float previousLoopMin = 1000;
    float previousLoopMax = 0;
    float previousLoopMoy = 0;

    FunctionPtr realtimeLoop;

    void setup() {
        startMillis = millis();
        previousLEDsMillis = startMillis;

        // Pin initialisation
        pinMode(LedYellow, OUTPUT);
        pinMode(LedGreen, OUTPUT);
        pinMode(zeroCross, INPUT_PULLDOWN);
        pinMode(pulseTriac, OUTPUT);
        digitalWrite(LedYellow, LOW);
        digitalWrite(LedGreen, LOW);
        digitalWrite(pulseTriac, LOW);  //Stop Triac

        // Watchdog initialisation
        esp_task_wdt_init(WDT_TIMEOUT, true);  //enable panic so ESP32 restarts

        // Ports Série ESP
        Serial.begin(115200);
        Serial.println("Booting");

        esp_task_wdt_reset();

        previousLoop = millis();
    }

    void setupRealtimeLoop(FunctionPtr pRealtimeLoop) {
        realtimeLoop = pRealtimeLoop;
        xTaskCreatePinnedToCore(  //Préparation Tâche Multi Coeur
            realtimeLoopTask,        /* Task function. */
            "realtimeLoopTask",      /* name of task. */
            10000,                  /* Stack size of task */
            NULL,                   /* parameter of the task */
            10,                     /* priority of the task */
            &Task1,                 /* Task handle to keep track of created task */
            0);                     /* pin task to core 0 */

        //Interruptions du Triac et Timer interne
        attachInterrupt(zeroCross, currentNull, RISING);

        //Hardware timer 100uS
        timer = timerBegin(0, 80, true);  //Clock Divider, 1 micro second Tick
        timerAttachInterrupt(timer, &onTimer, true);
        timerAlarmWrite(timer, 100, true);  //Interrupt every 100 Ticks or microsecond
        timerAlarmEnable(timer);

        //Hardware timer 10ms
        timer10ms = timerBegin(1, 80, true);  //Clock Divider, 1 micro second Tick
        timerAttachInterrupt(timer10ms, &onTimer10ms, true);
        timerAlarmWrite(timer10ms, 10000, true);  //Interrupt every 10000 Ticks or microsecond
        timerAlarmEnable(timer10ms);

        //Timers
        previousWifiMillis = millis() - 25000;
        previousHistoryMillis = millis() - 290000;
        previousTimer2sMillis = millis();
        previousTimeRMS = millis();
        previousMqttMillis = millis() - 5000;
        previousETX = millis();
        previousOverProdMillis = millis();
        LastRMS_Millis = millis();
    }

    void loop() {
        // Estimation charge coeur
        unsigned long tps = millis();
        float deltaT = float(tps - previousLoop);
        previousLoop = tps;
        previousLoopMin = min(previousLoopMin, deltaT);
        previousLoopMin = previousLoopMin + 0.001;
        previousLoopMax = max(previousLoopMax, deltaT);
        previousLoopMax = previousLoopMax * 0.9999;
        previousLoopMoy = deltaT * 0.001 + previousLoopMoy * 0.999;
    }

    /* **********************
    * ****************** *
    * * Tâches Coeur 0 * *
    * ****************** *
    **********************
    */

    void realtimeLoopTask(void *pvParameters) {
        esp_task_wdt_add(NULL);  //add current thread to WDT watch
        esp_task_wdt_reset();
        for (;;) {
            realtimeLoop();
        }
    }

    //Interruptions, Current Zero Crossing from Triac device and Internal Timer
    //*************************************************************************
    void IRAM_ATTR onTimer10ms() {  //Interruption interne toutes 10ms
        ITmode--;
        if (ITmode < -5) ITmode = -5;
        if (ITmode < 0) GestionIT_10ms();  //IT non synchrone avec le secteur . Horloge interne
    }

    // Interruption du Triac Signal Zc, toutes les 10ms
    void IRAM_ATTR currentNull() {
        IT10ms += 1;
        if ((millis() - lastIT) > 2) {  // to avoid glitch detection during 2ms
            ITmode++;
            ITmode++;
            if (ITmode > 5) ITmode = 5;
            IT10ms_in++;
            lastIT = millis();
            if (ITmode > 0) GestionIT_10ms();  // IT synchrone avec le secteur signal Zc
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
                digitalWrite(pulseTriac, LOW);  //Stop Découpe Triac
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

    // Interruption Timer interne toutes les 100 micro secondes
    void IRAM_ATTR onTimer() {  //Interruption every 100 micro second
        if (Actif[0] == 1) {      // Découpe Sinus
            PulseComptage[0] += 1;
            if (PulseComptage[0] > Retard[0] && Retard[0] < 98 && ITmode > 0) {  //100 steps in 10 ms
                digitalWrite(pulseTriac, HIGH);                                    //Activate Triac
            } else {
                digitalWrite(pulseTriac, LOW);  //Stop Triac
            }
        }
    }
} // namespace ModuleCore
