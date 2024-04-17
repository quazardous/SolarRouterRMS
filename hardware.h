#pragma once

// PINS - GPIO
#define RMS_PIN_RXD2 26            //Pour Routeur Linky ou UxIx2 (sur carte ESP32 simple): Couple RXD2=26 et RMS_PIN_TXD2=27 . Pour carte ESP32 4 relais : Couple RXD2=17 et TXD2=27
#define RMS_PIN_TXD2 27
// WIFI aka YELLOW
#define RMS_PIN_LED_WIFI 18
// ACTIVITY aka GREEN
#define RMS_PIN_LED_ACTIVITY 19
#define RMS_PIN_PULSE_TRIAC 22
#define RMS_PIN_ZERO_CROSS 23
#define RMS_PIN_TEMP 13  //Capteur température


#define RMS_SER_BUF_SIZE 4096