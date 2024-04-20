#include "ModuleTime.h"
#include <esp_sntp.h>
#include "helpers.h"
#include "ModuleEeprom.h"
#include "ModuleCore.h"
#include "ModuleDebug.h"

#define RMS_NTP_SERVER1 "fr.pool.ntp.org"
#define RMS_NTP_SERVER2 "time.nist.gov"
// cf Time-Zone: https://sites.google.com/a/usapiens.com/opnode/time-zones
#define RMS_NTP_TIMEZONE "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00"
#define RMS_NTP_SYNC_INTERVAL 3600

void dayIsGone();
void onTime();

namespace ModuleTime
{
    void time_sync_notification(struct timeval *tv);

    bool ntpSync = false;
    bool onTimeTriggered = false;
    const char *ntpServers[] = {
        RMS_NTP_SERVER1,
        RMS_NTP_SERVER2
    };
    unsigned long lastTock;
    // reference date computed from time()
    String JourCourant = "";
    // reference date read from EEPROM
    String DateCeJour = "";
    time_t now = 0;
    float decimalHour = 0;

    void boot() {

        // setting NTP
        //Heure / Hour . A Mettre en priorité avant WIFI (exemple ESP32 Simple Time)
        //External timer to obtain the Hour and reset Watt Hour every day at 0h
        sntp_set_time_sync_notification_cb(time_sync_notification);
        // Option
        sntp_servermode_dhcp(1);
        sntp_set_sync_interval(RMS_NTP_SYNC_INTERVAL * 1 * 1000); // 1h
        ModuleCore::log("NTP running every " + String(RMS_NTP_SYNC_INTERVAL) + "s");
        configTzTime(RMS_NTP_TIMEZONE, ntpServers[0], ntpServers[1]);

        if (ModuleEeprom::hasData()) {
            DateCeJour = ModuleEeprom::readToday();
            ModuleCore::log("Reading today from EEPROM: " + DateCeJour);
        }
    }

    void loopTimer(unsigned long mtsNow) {
        lastTock = mtsNow;
    }

    void loop(unsigned long msLoop) {
        unsigned long msNow = millis();
        if (TICKTOCK(msNow, lastTock, 30000)) {
            checkDayHourChange();
        }
    }

    time_t checkDayHourChange()
    {
        if (!timeIsValid()) return 0;
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
            // It's a new full day !!
            if (DateCeJour != "") {
                ModuleEeprom::writeToday(JourCourant);
                ModuleCore::log("Event dayIsGone()");
                ::dayIsGone();
            }
            DateCeJour = JourCourant;
        }
        return now;
    }

    // **************
    // * Heure DATE * -
    // **************
    void time_sync_notification(struct timeval *tv) {
        ModuleCore::log("NTP Time Sync");
        ntpSync = true;
        String message = String("Sync time in ms:");
        message += sntp_get_sync_interval();
        ModuleCore::log(message);
        if (!onTimeTriggered) {
            onTimeTriggered = true;
            checkDayHourChange();
            ModuleDebug::stockMessage("Time events done");
            ModuleCore::log("Event onTime()");
            ::onTime();
        }
    }

    float getDecimalHour() {
        return decimalHour;
    }

    bool timeIsValid() {
        return ntpSync;
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