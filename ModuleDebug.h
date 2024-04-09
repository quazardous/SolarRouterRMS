#pragma once

// Debug via Wifi
#include <RemoteDebug.h>

namespace ModuleDebug
{
    extern RemoteDebug Debug;
    void setup();
    void loop();
} // namespace ModuleDebug
