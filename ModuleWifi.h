#pragma once

#include <Arduino.h>
#include <WebServer.h>

namespace ModuleWifi
{
    // events
    void boot();
    void loopTimer(unsigned long mtsNow);
    void loop(unsigned long msLoop);

    // helpers
    void resetWifiBug();
    unsigned int incrWifiBug();
    bool canConnectWifi(unsigned long msTimeout);

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
