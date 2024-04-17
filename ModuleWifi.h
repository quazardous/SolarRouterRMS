#pragma once

#include <Arduino.h>

namespace ModuleWifi
{
    // events
    void setup();
    void loopTimer(unsigned long mtsNow);
    void loop(unsigned long msLoop);

    // helpers
    void resetWifiBug();
    unsigned int incrWifiBug();

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
} // namespace ModuleWifi
