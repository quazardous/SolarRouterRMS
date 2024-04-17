#pragma once

#include <Arduino.h>

namespace ModuleMQTT
{
    void setup();
    void loopTimer(unsigned long mtsNow);
    void loop(unsigned long msLoop);

    // setters / getters
    void setRepeat(unsigned short repeat);
    unsigned short getRepeat();
    void setIp(unsigned long ip);
    unsigned long getIp();
    void setPort(unsigned short port);
    unsigned short getPort();
    void setUser(const char *user);
    const char *getUser();
    void setPwd(const char *pwd);
    const char *getPwd();
    void setPrefix(const char *prefix);
    const char *getPrefix();
    void setDeviceName(const char *device);
    const char *getDeviceName();
} // namespace ModuleMQTT