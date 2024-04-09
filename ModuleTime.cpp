#include "ModuleTime.h"

namespace ModuleTime
{
    // **************
    // * Heure DATE * -
    // **************
    void time_sync_notification(struct timeval *tv) {
        Serial.println("Notification de l'heure ( time synchronization event ) ");
        DATEvalid = true;
        Serial.print("Sync time in ms : ");
        Serial.println(sntp_get_sync_interval());
        JourHeureChange();
        StockMessage("Réception de l'heure");
    }

    void setup() {
        //Heure / Hour . A Mettre en priorité avant WIFI (exemple ESP32 Simple Time)
        //External timer to obtain the Hour and reset Watt Hour every day at 0h
        sntp_set_time_sync_notification_cb(time_sync_notification);
        sntp_servermode_dhcp(1);                                                               //Option
        configTzTime("CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", ntpServer1, ntpServer2);  //Voir Time-Zone: https://sites.google.com/a/usapiens.com/opnode/time-zones
    }

    void loop() {
        JourHeureChange();
    }
}