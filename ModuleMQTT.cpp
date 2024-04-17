// **********************************************************************************************
// *                        MQTT AUTO-DISCOVERY POUR HOME ASSISTANT ou DOMOTICZ                            *
// **********************************************************************************************
#include "ModuleMQTT.h"
#include <PubSubClient.h> //Librairie pour la gestion Mqtt
#include "ModuleDebug.h"
#include "ModuleSensor.h"
#include "ModuleCore.h"
#include "ModuleEDF.h"
#include "ModuleTriggers.h"
#include "ModulePowerMeter.h"
#include <ArduinoJson.h>
#include "helpers.h"
#include "version.h"

#define RMS_MQTT_MAX_PACKET_SIZE 700
#define RMS_MQTT_MAX_STRING_SIZE 32

namespace ModuleMQTT
{
    bool Discovered = false;
    unsigned long previousMqttMillis;

    WiFiClient MqttClient;
    PubSubClient clientMQTT(MqttClient);

    // Types de composants reconnus par HA et obligatoires pour l'Auto-Discovery.
    const char *SSR = "sensor";
    const char *SLCT = "select";
    const char *NB = "number";
    const char *BINS = "binary_sensor";
    const char *SWTC = "switch";
    const char *TXT = "text";

    // Période d'envoie des données au serveur MQTT (en secondes)
    // 0 is disabled
    unsigned int MQTTRepet = 0;
    unsigned long MQTTIP = 0;
    unsigned int MQTTPort = 1883;

    char MQTTUser[RMS_MQTT_MAX_STRING_SIZE] = "User";
    char MQTTPwd[RMS_MQTT_MAX_STRING_SIZE] = "password";
    char MQTTPrefix[RMS_MQTT_MAX_STRING_SIZE] = "homeassistant";  // prefix obligatoire pour l'auto-discovery entre HA et Core-Mosquitto (par défaut c'est homeassistant)
    char MQTTdeviceName[RMS_MQTT_MAX_STRING_SIZE] = "routeur_rms";

    void setup()
    {
        // noop
    }

    void loopTimer(unsigned long msNow) {
        previousMqttMillis = msNow - 5000;
    }

    void loop(unsigned long msLoop) {
        if (MQTTRepet == 0)
            return;
        unsigned long msNow = millis();
        if (TICKTOCK(msNow, previousMqttMillis, MQTTRepet * 1000))
            envoiAuMQTT();
    }

    void envoiAuMQTT()
    { // Cette Fonction d'origine a été modifiée
        unsigned long tps = millis();
        int etat = 0; // utilisé pour l'envoie de l'état On/Off des actions.
        
        // vérifie si le client est connecté au serveur mqtt
        assertConnected();
        clientMQTT.loop();

        if (!Discovered)
        { //(uniquement au démarrage discovery = 0)
            sendMQTTDiscoveryMsg_global();
        }
        SendDataToHomeAssistant(); // envoie du Payload au State topic
        clientMQTT.loop();
    }

    void assertConnected()
    {
        if (!clientMQTT.connected())
        { // si le mqtt n'est pas connecté (utile aussi lors de la 1ere connexion)
            Serial.println("Connection au serveur MQTT ...");
            byte arr[4];
            ip_explode(MQTTIP, arr);
            String host = String(arr[3]) + "." + String(arr[2]) + "." + String(arr[1]) + "." + String(arr[0]);
            clientMQTT.setServer(host.c_str(), MQTTPort);
            clientMQTT.setCallback(callback); // Déclaration de la fonction de souscription
            if (clientMQTT.connect(MQTTdeviceName, MQTTUser, MQTTPwd))
            { // si l'utilisateur est connecté au mqtt
                String m = String(MQTTdeviceName) + " connecté";
                ModuleDebug::comboLog(m);
            }
            else
            { // si utilisateur pas connecté au mqtt
                String m = String("echec connexion au MQTT, code erreur= ") + String(clientMQTT.state());
                ModuleDebug::comboLog(m);
                ModuleDebug::stockMessage("Echec connexion MQTT : " + host);
                return;
            }
        } // ici l'utilisateur est normalement connecté au mqtt
    }

    // Callback  pour souscrire a un topic et  prévoir une action
    void callback(char *topic, byte *payload, unsigned int length)
    {
        String m = String("-------Nouveau message du broker mqtt. Non utilisé-----");
        Serial.println(m);
        ModuleDebug::getDebug().println(m);
    }
    //*************************************************************************
    //*          CONFIG OF DISCOVERY MESSAGE FOR HOME ASSISTANT  / DOMOTICZ             *
    //*************************************************************************

    void sendMQTTDiscoveryMsg_global()
    {
        String ActType;
        ModulePowerMeter::source_t source = ModulePowerMeter::getSource();
        // augmente la taille du buffer wifi Mqtt (voir PubSubClient.h)
        clientMQTT.setBufferSize(RMS_MQTT_MAX_PACKET_SIZE); // voir -->#define MQTT_MAX_PACKET_SIZE 256 is the default value in PubSubClient.h
        if (source == ModulePowerMeter::SOURCE_UXIX2 || source == ModulePowerMeter::SOURCE_SHELLYEM)
        {
            DeviceToDiscover("PuissanceS_T", "W", "power", "0");
            DeviceToDiscover("PuissanceI_T", "W", "power", "0");
            DeviceToDiscover("Tension_T", "V", "voltage", "2");
            DeviceToDiscover("Intensite_T", "A", "current", "2");
            DeviceToDiscover("PowerFactor_T", "", "power_factor", "2");
            DeviceToDiscover("Energie_T_Soutiree", "Wh", "energy", "0");
            DeviceToDiscover("Energie_T_Injectee", "Wh", "energy", "0");
            DeviceToDiscover("EnergieJour_T_Soutiree", "Wh", "energy", "0");
            DeviceToDiscover("EnergieJour_T_Injectee", "Wh", "energy", "0");
            DeviceToDiscover("Frequence", "Hz", "frequency", "2");
        }

        if (ModuleSensor::getTemperature() > -100)
        {
            DeviceToDiscover("Temperature", "°C", "temperature", "1");
        }

        if (source == ModulePowerMeter::SOURCE_LINKY)
        {
            DeviceTextToDiscover("LTARF", "Option Tarifaire");
            DeviceToDiscover("Code_Tarifaire", "", "", "0");
        }

        DeviceToDiscover("PuissanceS_M", "W", "power", "0");
        DeviceToDiscover("PuissanceI_M", "W", "power", "0");
        DeviceToDiscover("Tension_M", "V", "voltage", "2");
        DeviceToDiscover("Intensite_M", "A", "current", "2");
        DeviceToDiscover("PowerFactor_M", "", "power_factor", "2");
        DeviceToDiscover("Energie_M_Soutiree", "Wh", "energy", "0");
        DeviceToDiscover("Energie_M_Injectee", "Wh", "energy", "0");
        DeviceToDiscover("EnergieJour_M_Soutiree", "Wh", "energy", "0");
        DeviceToDiscover("EnergieJour_M_Injectee", "Wh", "energy", "0");

        byte count = ModuleTriggers::getTriggersCount();
        for (int i = 0; i < count; i++)
        {
            ActType = "Ouverture_Relais_" + String(i);
            if (i == 0)
                ActType = "OuvertureTriac";
            DeviceToDiscover(ActType, "%", "power_factor", "0"); // Type power factor pour etre accepté par HA
        }

        Serial.println("Paramètres Auto-Discovery publiés !");
        ModuleDebug::getDebug().println("Paramètres Auto-Discovery publiés !");

        // clientMQTT.setBufferSize(512);  // go to initial value wifi/mqtt buffer
        Discovered = true;

    } // END OF sendMQTTDiscoveryMsg_global

    void DeviceToDiscover(String Name, String Unit, String Class, String Round)
    {

        String StateTopic = String(MQTTPrefix) + "/" + MQTTdeviceName + "_state";
        DynamicJsonDocument doc(512); // this is the Payload json format
        JsonObject device;            // for device object  "device": {}
        JsonArray option;             // options (array) of this device
        char buffer[512];
        size_t n;
        bool published;

        String ESP_ID = WiFi.macAddress(); // The chip ID is essentially its MAC address.                                        // ID de l'entité pour HA
        String DiscoveryTopic;             // nom du topic pour ce capteur

        // DiscoveryTopic = ConfigTopic(Name, SSR, "config");
        DiscoveryTopic = String(MQTTPrefix) + "/" + String(SSR) + "/" + String(MQTTdeviceName) + "_" + String(Name) + "/" + String("config");
        doc["name"] = String(MQTTdeviceName) + " " + String(Name);    // concatenation des ID uniques des entités (capteurs ou commandes)
        doc["uniq_id"] = String(MQTTdeviceName) + "_" + String(Name); // concatenation des Noms uniques des entités
        doc["stat_t"] = StateTopic;
        doc["unit_of_meas"] = Unit;
        if (Unit == "Wh")
        {
            doc["state_class"] = "total_increasing";
        }
        doc["device_class"] = Class;
        doc["val_tpl"] = "{{ value_json." + Name + "|default(0)| round(" + Round + ") }}";
        device = doc.createNestedObject("device");
        device["ids"][0] = ESP_ID; // identification unique sur Home Assistant (obligatoire).
        device["cu"] = "http://" + WiFi.localIP().toString();
        device["hw"] = String(ESP.getChipModel()) + " rev." + String(ESP.getChipRevision());
        device["mf"] = "F1ATB - https://f1atb.fr";
        device["mdl"] = "ESP32 - " + ESP_ID;
        device["name"] = String(ModuleCore::getRouterName());
        device["sw"] = VERSION;

        n = serializeJson(doc, buffer);
        published = clientMQTT.publish(DiscoveryTopic.c_str(), buffer, n);
        doc.clear();
        buffer[0] = '\0';
    }
    void DeviceBinToDiscover(String Name, String Titre)
    {

        String StateTopic = String(MQTTPrefix) + "/" + MQTTdeviceName + "_state";
        DynamicJsonDocument doc(512); // this is the Payload json format
        JsonObject device;            // for device object  "device": {}
        JsonArray option;             // options (array) of this device
        char buffer[512];
        size_t n;
        bool published;

        String ESP_ID = WiFi.macAddress(); // The chip ID is essentially its MAC address.                                        // ID de l'entité pour HA
        String DiscoveryTopic;             // nom du topic pour ce capteur

        DiscoveryTopic = String(MQTTPrefix) + "/" + String(BINS) + "/" + String(MQTTdeviceName) + "_" + String(Name) + "/" + String("config");
        doc["name"] = String(MQTTdeviceName) + " " + String(Titre);   // concatenation des ID uniques des entités (capteurs ou commandes)
        doc["uniq_id"] = String(MQTTdeviceName) + "_" + String(Name); // concatenation des Noms uniques des entités
        doc["stat_t"] = StateTopic;
        doc["init"] = "OFF"; // default value
        doc["ic"] = "mdi:electric-switch";
        doc["val_tpl"] = "{{ value_json." + Name + " }}";
        device = doc.createNestedObject("device");
        device["ids"][0] = ESP_ID;
        device["name"] = String(ModuleCore::getRouterName());
        n = serializeJson(doc, buffer);
        published = clientMQTT.publish(DiscoveryTopic.c_str(), buffer, n);
        doc.clear();
        buffer[0] = '\0';
    }

    void DeviceTextToDiscover(String Name, String Titre)
    {
        String StateTopic = String(MQTTPrefix) + "/" + MQTTdeviceName + "_state";
        DynamicJsonDocument doc(512); // this is the Payload json format
        JsonObject device;            // for device object  "device": {}
        JsonArray option;             // options (array) of this device
        char buffer[512];
        size_t n;
        bool published;
        String ESP_ID = WiFi.macAddress(); // The chip ID is essentially its MAC address.                                        // ID de l'entité pour HA
        String DiscoveryTopic;             // nom du topic pour ce capteur
        DiscoveryTopic = String(MQTTPrefix) + "/" + String(SSR) + "/" + String(MQTTdeviceName) + "_" + String(Name) + "/" + String("config");
        doc["name"] = String(MQTTdeviceName) + " " + String(Titre);   // concatenation des ID uniques des entités (capteurs ou commandes)
        doc["uniq_id"] = String(MQTTdeviceName) + "_" + String(Name); // concatenation des Noms uniques des entités
        doc["stat_t"] = StateTopic;
        doc["device_class"] = "enum";
        doc["val_tpl"] = "{{ value_json." + Name + " }}";
        device = doc.createNestedObject("device");
        device["ids"][0] = ESP_ID;
        device["name"] = String(ModuleCore::getRouterName());
        n = serializeJson(doc, buffer);
        published = clientMQTT.publish(DiscoveryTopic.c_str(), buffer, n);
        doc.clear();
        buffer[0] = '\0';
    }
    //****************************************
    //* ENVOIE DES DATAS VERS HOME ASSISTANT *
    //****************************************

    void SendDataToHomeAssistant()
    {
        ModulePowerMeter::source_t source = ModulePowerMeter::getSource();
        ModulePowerMeter::electric_data_t *elecDataTriac = ModulePowerMeter::getElectricData(ModulePowerMeter::DOMAIN_TRIAC);
        // common state topic for all entities of this ESP32 Device
        String StateTopic = String(MQTTPrefix) + "/" + MQTTdeviceName + "_state";
        String ActType;
        DynamicJsonDocument doc(1024); // 1024 octets = capacité  largement suffisante
        char buffer[1024];
        if (source == ModulePowerMeter::SOURCE_UXIX2 || source == ModulePowerMeter::SOURCE_SHELLYEM)
        {
            doc["PuissanceS_T"] = elecDataTriac->powerIn; // Triac
            doc["PuissanceI_T"] = elecDataTriac->powerOut; // Triac
            doc["Tension_T"] = elecDataTriac->voltage;
            doc["Intensite_T"] = elecDataTriac->current;
            doc["PowerFactor_T"] = elecDataTriac->powerFactor;
            doc["Energie_T_Soutiree"] = elecDataTriac->energyIn;
            doc["Energie_T_Injectee"] = elecDataTriac->energyOut;
            doc["EnergieJour_T_Soutiree"] = elecDataTriac->energyDayIn;
            doc["EnergieJour_T_Injectee"] = elecDataTriac->energyDayOut;
            doc["Frequence"] = elecDataTriac->frequency;
        }
        
        if (ModuleSensor::getTemperature() > -100)
        {
            doc["Temperature"] = ModuleSensor::getTemperature();
        }

        if (source == ModulePowerMeter::SOURCE_LINKY)
        {

            String LTARF = String(ModuleEDF::getLTARF());
            doc["LTARF"] = LTARF;
            int code = 0;
            if (LTARF.indexOf("HEURE  CREUSE") >= 0)
                code = 1; // Code Linky
            if (LTARF.indexOf("HEURE  PLEINE") >= 0)
                code = 2;
            if (LTARF.indexOf("HC BLEU") >= 0)
                code = 11;
            if (LTARF.indexOf("HP BLEU") >= 0)
                code = 12;
            if (LTARF.indexOf("HC BLANC") >= 0)
                code = 13;
            if (LTARF.indexOf("HP BLANC") >= 0)
                code = 14;
            if (LTARF.indexOf("HC ROUGE") >= 0)
                code = 15;
            if (LTARF.indexOf("HP ROUGE") >= 0)
                code = 16;
            if (LTARF.indexOf("TEMPO_BLEU") >= 0)
                code = 17; // Code EDF
            if (LTARF.indexOf("TEMPO_BLANC") >= 0)
                code = 18;
            if (LTARF.indexOf("TEMPO_ROUGE") >= 0)
                code = 19;
            doc["Code_Tarifaire"] = code;
        }
        ModulePowerMeter::electric_data_t *elecDataHouse = ModulePowerMeter::getElectricData(ModulePowerMeter::DOMAIN_HOUSE);
        doc["PuissanceS_M"] = elecDataHouse->energyIn; // Maison
        doc["PuissanceI_M"] = elecDataHouse->energyOut; // Maison
        doc["Tension_M"] = elecDataHouse->voltage;
        doc["Intensite_M"] = elecDataHouse->current;
        doc["PowerFactor_M"] = elecDataHouse->powerFactor;
        doc["Energie_M_Soutiree"] = elecDataHouse->energyIn;
        doc["Energie_M_Injectee"] = elecDataHouse->energyOut;
        doc["EnergieJour_M_Soutiree"] = elecDataHouse->energyDayIn;
        doc["EnergieJour_M_Injectee"] = elecDataHouse->energyDayOut;

        byte count = ModuleTriggers::getTriggersCount();
        for (byte i = 0; i < count; i++)
        {
            ActType = "Ouverture_Relais_" + String(i);
            if (i == 0)
                ActType = "OuvertureTriac";
            doc[ActType] = 100 - ModuleTriggers::getDelay(i);
        }

        size_t n = serializeJson(doc, buffer);
        bool published = clientMQTT.publish(StateTopic.c_str(), buffer, n);
        doc.clear();
        buffer[0] = '\0';

    } // END SendDataToHomeAssistant

    // setters / getters
    void setRepeat(unsigned short repeat)
    {
        MQTTRepet = repeat;
    }
    unsigned short getRepeat()
    {
        return MQTTRepet;
    }

    void setIp(unsigned long ip)
    {
        MQTTIP = ip;
    }
    unsigned long getIp()
    {
        return MQTTIP;
    }

    void setPort(unsigned short port)
    {
        MQTTPort = port;
    }
    unsigned short getPort()
    {
        return MQTTPort;
    }

    void setUser(const char *user)
    {
        strncpy(MQTTUser, user, sizeof(MQTTUser) - 1);
        MQTTUser[sizeof(MQTTUser) - 1] = '\0';
    }

    const char *getUser()
    {
        return MQTTUser;
    }

    void setPwd(const char *pwd)
    {
        strncpy(MQTTPwd, pwd, sizeof(MQTTPwd) - 1);
        MQTTPwd[sizeof(MQTTPwd) - 1] = '\0';
    }

    const char *getPwd()
    {
        return MQTTPwd;
    }

    void setPrefix(const char *prefix)
    {
        strncpy(MQTTPrefix, prefix, sizeof(MQTTPrefix) - 1);
        MQTTPrefix[sizeof(MQTTPrefix) - 1] = '\0';
    }

    const char *getPrefix()
    {
        return MQTTPrefix;
    }

    void setDeviceName(const char *device)
    {
        strncpy(MQTTdeviceName, device, sizeof(MQTTdeviceName) - 1);
        MQTTdeviceName[sizeof(MQTTdeviceName) - 1] = '\0';
    }

    const char *getDeviceName()
    {
        return MQTTdeviceName;
    }
} // namespace ModuleMQTT
