#include "ModuleCommon.h"

namespace ModuleCommon
{

    //Nombre Actions Max
    #define LesActionsLength 20
    //VARIABLES
    const char *ap_default_ssid;        // Mode Access point  IP: 192.168.4.1
    const char *ap_default_psk = NULL;  // Pas de mot de passe en AP,

    //Paramètres dans l'odre de stockage en ROM apres les données du RMS

    String ssid = "";
    String password = "";
    String Source = "UxI";
    String Source_data = "UxI";
    byte dhcpOn = 1;
    unsigned long IP_Fixe = 0;
    unsigned long Gateway = 0;
    unsigned long masque = 4294967040;
    unsigned long dns = 0;
    unsigned long RMSextIP = 0;
    unsigned int MQTTRepet = 0;
    unsigned long MQTTIP = 0;
    unsigned int MQTTPort = 1883;
    String MQTTUser = "User";
    String MQTTPwd = "password";
    String MQTTPrefix = "homeassistant";  // prefix obligatoire pour l'auto-discovery entre HA et Core-Mosquitto (par défaut c'est homeassistant)
    String MQTTdeviceName = "routeur_rms";
    String nomRouteur = "Routeur - RMS";
    String nomSondeFixe = "Données seconde sonde";
    String nomSondeMobile = "Données Maison";
    String nomTemperature = "Température";
    String GS = String((char)29);  //Group Separator
    String RS = String((char)30);  //Record Separator
    String Message[4];
    int idxMessage = 0;
    int P_cent_EEPROM;
    int cptLEDyellow = 0;
    int cptLEDgreen = 0;

    unsigned int CalibU = 1000;  //Calibration Routeur UxI
    unsigned int CalibI = 1000;
    int value0;
    int volt[100];
    int amp[100];
    float KV = 0.2083;  //Calibration coefficient for the voltage. Value for CalibU=1000 at startup
    float KI = 0.0642;  //Calibration coefficient for the current. Value for CalibI=1000 at startup
    float kV = 0.2083;  //Calibration coefficient for the voltage. Corrected value
    float kI = 0.0642;  //Calibration coefficient for the current. Corrected value
    float voltM[100];   //Voltage Mean value
    float ampM[100];

    long EAS_T_J0 = 0;
    long EAI_T_J0 = 0;
    long EAS_M_J0 = 0;  //Debut du jour energie active
    long EAI_M_J0 = 0;


    int adr_debut_para = 0;  //Adresses Para après le Wifi


    //Paramètres électriques
    float Tension_T, Intensite_T, PowerFactor_T, Frequence;
    float Tension_M, Intensite_M, PowerFactor_M;
    long Energie_T_Soutiree = 0;
    long Energie_T_Injectee = 0;
    long Energie_M_Soutiree = 0;
    long Energie_M_Injectee = 0;
    long EnergieJour_T_Injectee = 0;
    long EnergieJour_M_Injectee = 0;
    long EnergieJour_T_Soutiree = 0;
    long EnergieJour_M_Soutiree = 0;
    int PuissanceS_T, PuissanceS_M, PuissanceI_T, PuissanceI_M;
    int PVAS_T, PVAS_M, PVAI_T, PVAI_M;
    float PuissanceS_T_inst, PuissanceS_M_inst, PuissanceI_T_inst, PuissanceI_M_inst;
    float PVAS_T_inst, PVAS_M_inst, PVAI_T_inst, PVAI_M_inst;
    float Puissance_T_moy, Puissance_M_moy;
    float PVA_T_moy, PVA_M_moy;
    int PactConso_M, PactProd;
    int tabPw_Maison_5mn[600];  //Puissance Active:Soutiré-Injecté toutes les 5mn
    int tabPw_Triac_5mn[600];
    int tabTemperature_5mn[600];
    int tabPw_Maison_2s[300];   //Puissance Active: toutes les 2s
    int tabPw_Triac_2s[300];    //Puissance Triac: toutes les 2s
    int tabPva_Maison_2s[300];  //Puissance Active: toutes les 2s
    int tabPva_Triac_2s[300];
    int tabPulseSinusOn[101];
    int tabPulseSinusTotal[101];
    int IdxStock2s = 0;
    int IdxStockPW = 0;
    float PmaxReseau = 36000;  //Puissance Max pour eviter des débordements
    bool LissageLong = false;

    //Parameters for JSY-MK-194T module
    byte ByteArray[130];
    long LesDatas[14];
    int Sens_1, Sens_2;

    //Parameters for Linky
    bool LFon = false;
    bool EASTvalid = false;
    bool EAITvalid = false;
    volatile int IdxDataRawLinky = 0;
    volatile int IdxBufDecodLinky = 0;
    volatile char DataRawLinky[10000];  //Buffer entrée données Linky
    float moyPWS = 0;
    float moyPWI = 0;
    float moyPVAS = 0;
    float moyPVAI = 0;
    float COSphiS = 1;
    float COSphiI = 1;
    long TlastEASTvalide = 0;
    long TlastEAITvalide = 0;
    String LTARF = "";  //Option tarifaire EDF
    String STGE = "";   //Status Tempo uniquement EDF

    //Paramètres for Enphase-Envoye-Smetered
    String TokenEnphase = "";
    String EnphaseUser = "";
    String EnphasePwd = "";
    String EnphaseSerial = "0";  //Sert égalemnet au Shelly comme numéro de voie
    String JsonToken = "";
    String Session_id = "";
    long LastwhDlvdCum = 0;  //Dernière valeur cumul Wh Soutire-injecté.
    float EMI_Wh = 0;        //Energie entrée Maison Injecté Wh
    float EMS_Wh = 0;        //Energie entrée Maison Injecté Wh

    //Paramètres for SmartGateways
    String SG_dataBrute = "";

    //Paramètres for Shelly Em
    String ShEm_dataBrute = "";
    int ShEm_comptage_appels = 0;
    float PwMoy2 = 0;  //Moyenne voie secondsaire
    float pfMoy2 = 1;  //pf Voie secondaire

    //Paramètres pour EDF
    String DateEDF = "";  //an-mois-jour
    byte TempoEDFon = 0;
    int LastHeureEDF = -1;
    int LTARFbin = 0;  //Code binaire  des tarifs







    //Internal Timers
    unsigned long previousWifiMillis;
    unsigned long previousHistoryMillis;
    unsigned long previousWsMillis;
    unsigned long previousWiMillis;
    unsigned long LastRMS_Millis;
    unsigned long previousTimer2sMillis;
    unsigned long previousOverProdMillis;
    unsigned long previousLEDsMillis;
    unsigned long previousETX;
    unsigned long PeriodeProgMillis = 1000;
    unsigned long T0_seconde = 0;
    unsigned long T_On_seconde = 0;
    unsigned long previousTimeRMS;
    float previousTimeRMSMin = 1000;
    float previousTimeRMSMax = 0;
    float previousTimeRMSMoy = 0;
    unsigned long previousMqttMillis;

    //Actions et Triac(action 0)
    float RetardF[LesActionsLength];  //Floating value of retard
    //Variables in RAM for interruptions
    volatile unsigned long lastIT = 0;
    volatile int IT10ms = 0;     //Interruption avant deglitch
    volatile int IT10ms_in = 0;  //Interruption apres deglitch
    volatile int ITmode = 0;     //IT externe Triac ou interne
    hw_timer_t *timer = NULL;
    hw_timer_t *timer10ms = NULL;


    volatile int Retard[LesActionsLength];
    volatile int Actif[LesActionsLength];
    volatile int PulseOn[LesActionsLength];
    volatile int PulseTotal[LesActionsLength];
    volatile int PulseComptage[LesActionsLength];
    volatile int Gpio[LesActionsLength];
    volatile int OutOn[LesActionsLength];
    volatile int OutOff[LesActionsLength];

    //Port Serie 2 - Remplace Serial2 qui bug
    HardwareSerial MySerial(2);

    // Heure et Date
    #define MAX_SIZE_T 80
    const char *ntpServer1 = "fr.pool.ntp.org";
    const char *ntpServer2 = "time.nist.gov";
    String DATE = "";
    String DateCeJour = "";
    bool DATEvalid = false;
    int HeureCouranteDeci = 0;
    int idxPromDuJour = 0;

    //Debug via WIFI instead of Serial
    //Connect a Telnet terminal on port 23
    WiFiClient MqttClient;
    PubSubClient clientMQTT(MqttClient);
    int WIFIbug = 0;
    WiFiClientSecure clientSecu;
    WiFiClientSecure clientSecuEDF;

} // namespace ModuleCommon
