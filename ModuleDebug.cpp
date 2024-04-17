#include "ModuleDebug.h"
#include "helpers.h"

namespace ModuleDebug
{
    RemoteDebug Debug;
    String Message[4];
    int idxMessage = 0;
    
    void setup()
    {
        Debug.begin("ESP32");
        Debug.println("Ready");
        Debug.print("IP address: ");
        Debug.println(WiFi.localIP());
    }

    void loop(unsigned long msLoop)
    {
        // start remote debug
        Debug.handle();
    }

    void stockMessage(const String &m)
    {
        String DATE = String(ts2str(time(NULL), "%Y-%m-%d %H:%M:%S"));
        String message = DATE + " : " + m;
        Serial.println(message);
        Message[idxMessage] = message;
        idxMessage = (idxMessage + 1) % 4;
    }

    void stockMessage(const char *m)
    {
        String DATE = String(ts2str(time(NULL), "%Y-%m-%d %H:%M:%S"));
        String message = DATE + " : " + String(m);
        Serial.println(message);
        Message[idxMessage] = message;
        idxMessage = (idxMessage + 1) % 4;
    }

    String* getMessages()
    {
        return Message;
    }

    RemoteDebug &getDebug()
    {
        return Debug;
    }

    void comboLog(const String &m)
    {
        stockMessage(m);
        Debug.println(m);
    }

    void comboLog(const char *m)
    {
        stockMessage(m);
        Debug.println(m);
    }
} // namespace ModuleDebug
