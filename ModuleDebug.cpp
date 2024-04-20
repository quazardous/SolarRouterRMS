#include "ModuleDebug.h"
#include "ModuleCore.h"
#include "ModuleTime.h"
#include "helpers.h"

namespace ModuleDebug
{
    RemoteDebug Debug;
    String Message[RMS_DEBUG_STOCK_MESSAGES];
    int idxMessage = 0;
    
    void boot()
    {
        Debug.begin("ESP32");
        Debug.println("Ready");
        Debug.print("IP address: ");
        Debug.println(WiFi.localIP().toString());
    }

    void loop(unsigned long msLoop)
    {
        // start remote debug
        Debug.handle();
    }

    // getters / setters
    String* getMessages()
    {
        return Message;
    }

    RemoteDebug &getDebug()
    {
        return Debug;
    }

    // helpers
    void stockMessage(const String &m)
    {
        ModuleCore::log(m);
        String DATE;
        if (ModuleTime::timeIsValid()) {
            DATE = String(ts2str(time(NULL), "%Y-%m-%d %H:%M:%S"));
        } else {
            char d[32];
            sprintf(d, "%010.3fs", millis() / 1000.0);
            DATE = String(d);
        }
        String message = DATE + ": " + m;
        Message[idxMessage] = message;
        idxMessage = (idxMessage + 1) % RMS_DEBUG_STOCK_MESSAGES;
    }

    void stockMessage(const char *m)
    {
        stockMessage(String(m));
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

    int getMessageIdx()
    {
        return idxMessage;
    }
} // namespace ModuleDebug
