#pragma once

#include <Arduino.h>
#include <WebServer.h>

namespace ModuleWifi
{
    enum wifi_step_t {
        WIFI_STEP_BOOT = 0,
        WIFI_STEP_STA,
        WIFI_STEP_STA_WAITING_STA,
        WIFI_STEP_STA_BEGIN, // begin wifi
        WIFI_STEP_STA_WAITING_CONNECT,
        WIFI_STEP_STA_CONNECT_TIMEOUT, // timeout -> AP
        WIFI_STEP_STA_CONNECT,
        WIFI_STEP_STA_FINAL, // connected in station mode
        WIFI_STEP_AP,
        WIFI_STEP_AP_WAITING_AP_STA,
        WIFI_STEP_AP_BEGIN, // begin AP
        WIFI_STEP_AP_FINAL, // connected in AP mode (setup)
    };

    // events
    void boot();
    void loopTimer(unsigned long mtsNow);
    void loop(unsigned long msLoop);

    // helpers
    void resetWifiBug();
    unsigned int incrWifiBug();
    bool canConnectWifi(unsigned long msTimeout);
    void resetApTimout(); // prevent AP mode to go away if SSID is set

    // getters / setters
    void setWifiSsid(const char *ssid);
    const char *getWifiSsid();
    void setWifiPassword(const char *password);
    const char *getWifiPassword();
    void setDhcpOn(bool dhcp);
    bool getDhcpOn();
    void setStaticIp(unsigned long ip);
    unsigned long getStaticIp();
    void setGateway(unsigned long gw);
    unsigned long getGateway();
    void setNetmask(unsigned long nm);
    unsigned long getNetmask();
    void setDns(unsigned long dns);
    unsigned long getDns();

    // states
    bool isWifiConnected();
    bool isStationMode();
    bool hasInternet();
    unsigned int getWifiBug();

    // web handlers
    void httpAjaxScanWifi(WebServer& server, String& S);
    void httpUpdateWifi(WebServer& server, String& S);
} // namespace ModuleWifi
