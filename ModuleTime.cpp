#include "ModuleTime.h"
#include <esp_sntp.h>
#include "helpers.h"
#include "ModuleStockage.h"
#include "ModuleDebug.h"

// Heure et Date
#define MAX_SIZE_T 80

#define NTP_SERVER1 "fr.pool.ntp.org"
#define NTP_SERVER2 "time.nist.gov"

namespace ModuleTime
{
    bool DATEvalid = false;
    const char *ntpServer1 = NTP_SERVER1;
    const char *ntpServer2 = NTP_SERVER2;
    unsigned long lastTock;
    // reference date computed from time()
    String JourCourant = "";
    // reference date read from EEPROM
    String DateCeJour = "";
    time_t now = 0;
    float decimalHour = 0;

    void setup() {
        //Heure / Hour . A Mettre en priorité avant WIFI (exemple ESP32 Simple Time)
        //External timer to obtain the Hour and reset Watt Hour every day at 0h
        sntp_set_time_sync_notification_cb(time_sync_notification);
        sntp_servermode_dhcp(1);                                                               // Option
        configTzTime("CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", ntpServer1, ntpServer2);  // Voir Time-Zone: https://sites.google.com/a/usapiens.com/opnode/time-zones
    }

    void loopTimer(unsigned long mtsNow) {
        lastTock = mtsNow;
    }

    void loop(unsigned long msLoop) {
        unsigned long msNow = millis();
        if (TICKTOCK(msNow, lastTock, 30000)) {
            JourHeureChange();
        }
    }

    time_t JourHeureChange()
    {
        if (!DATEvalid) return 0;
        // Time Update / de l'heure
        now = time(NULL);
        JourCourant = String(ts2str(now, "%d%m%Y"));

        int hour;
        int minute;
        int second;

        ts2YmdHis(now, NULL, NULL, NULL, &hour, &minute, &second);

        decimalHour = hour + (minute + second / 60.0) / 60.0;
        // compare with the last date read from EEPROM and/or from 24h ago
        if (DateCeJour != JourCourant)
        {
            // It's a new day !!
            if (DateCeJour != "")
                onNewDay();
            DateCeJour = JourCourant;
        }
        return now;
    }

    // Event once a day at the end of the day
    void onNewDay()
    {
        ModuleStockage::onNewDay();
    }

    // **************
    // * Heure DATE * -
    // **************
    void time_sync_notification(struct timeval *tv) {
        Serial.println("Notification de l'heure ( time synchronization event ) ");
        DATEvalid = true;
        Serial.print("Sync time in ms : ");
        Serial.println(sntp_get_sync_interval());
        JourHeureChange();
        ModuleDebug::stockMessage("Réception de l'heure");
    }

    float getDecimalHour() {
        return decimalHour;
    }

    bool timeIsValid() {
        return DATEvalid;
    }

    // setters / getters
    const char *getJourCourant() {
        return JourCourant.c_str();
    }
    void setDateCeJour(const char *date) {
        DateCeJour = date;
    }
    const char *getDateCeJour() {
        return DateCeJour.c_str();
    }
}