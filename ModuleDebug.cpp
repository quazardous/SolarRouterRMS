#include "ModuleDebug.h"

namespace ModuleDebug
{
    RemoteDebug Debug;
    
    void setup()
    {
        Debug.begin("ESP32");
        Debug.println("Ready");
        Debug.print("IP address: ");
        Debug.println(WiFi.localIP());
    }

    void loop()
    {
        Debug.handle();
    }
} // namespace ModuleDebug
