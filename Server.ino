// ***************
// *  WEB SERVER *
// ***************
void Init_Server() {
  //Init Web Server on port 80
  server.on("/", handleRoot);
  server.on("/MainJS", handleMainJS);
  server.on("/Para", handlePara);
  server.on("/ParaJS", handleParaJS);
  server.on("/ParaRouteurJS", handleParaRouteurJS);
  server.on("/ParaAjax", handleParaAjax);
  server.on("/ParaRouteurAjax", handleParaRouteurAjax);
  server.on("/ParaUpdate", handleParaUpdate);
  server.on("/Actions", handleActions);
  server.on("/ActionsJS", handleActionsJS);
  server.on("/ActionsUpdate", handleActionsUpdate);
  server.on("/ActionsAjax", handleActionsAjax);
  server.on("/Brute", handleBrute);
  server.on("/BruteJS", handleBruteJS);
  server.on("/ajax_histo48h", handleAjaxHisto48h);
  server.on("/ajax_histo1an", handleAjaxHisto1an);
  server.on("/ajax_dataRMS", handleAjaxRMS);
  server.on("/ajax_dataESP32", handleAjaxESP32);
  server.on("/ajax_data", handleAjaxData);
  server.on("/ajax_data10mn", handleAjaxData10mn);
  server.on("/ajax_etatActions", handleAjax_etatActions);
  server.on("/SetGPIO", handleSetGpio);
  server.on("/restart", handleRestart);
  server.on("/AP_ScanWifi", handleAP_ScanWifi);
  server.on("/AP_SetWifi", handleAP_SetWifi);
  server.onNotFound(handleNotFound);
  server.begin();
  Debug.println("HTTP server started");
}


void handleRoot() {                  //Pages principales
  if (WiFi.getMode() != WIFI_STA) {  // en AP et STA mode
    server.send(200, "text/html", String(pages[PAGE_CONNECT_HTML]));
  } else {  //Station Mode seul
    server.send(200, "text/html", String(pages[PAGE_MAIN_HTML]));
  }
}
void handleMainJS() {                             //Code Javascript
  server.send(200, "text/html", String(pages[PAGE_MAIN_JS]));  // Javascript code
}
void handleBrute() {  //Page données brutes
  server.send(200, "text/html", String(pages[PAGE_BRUTE_HTML]));
}
void handleBruteJS() {                                 //Code Javascript
  server.send(200, "text/html", String(pages[PAGE_BRUTE_JS]));  // Javascript code
}

void handleAjaxRMS() {  // Envoi des dernières données  brutes reçues du RMS
  String S = "";
  String RMSExtDataB = "";
  int LastIdx = server.arg(0).toInt();
  if (Source == "Ext") {
    // Use WiFiClient class to create TCP connections
    WiFiClient clientESP_RMS;
    byte arr[4];
    arr[0] = RMSextIP & 0xFF;          // 0x78
    arr[1] = (RMSextIP >> 8) & 0xFF;   // 0x56
    arr[2] = (RMSextIP >> 16) & 0xFF;  // 0x34
    arr[3] = (RMSextIP >> 24) & 0xFF;  // 0x12

    String host = String(arr[3]) + "." + String(arr[2]) + "." + String(arr[1]) + "." + String(arr[0]);
    if (!clientESP_RMS.connect(host.c_str(), 80)) {
      StockMessage("connection to client ESP_RMS external failed (call from  handleAjaxRMS)");
      return;
    }
    String url = "/ajax_dataRMS?idx=" + String(LastIdx);
    clientESP_RMS.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (clientESP_RMS.available() == 0) {
      if (millis() - timeout > 5000) {
        StockMessage(">>> clientESP_RMS Timeout !");
        clientESP_RMS.stop();
        return;
      }
    }
    // Lecture des données brutes distantes
    while (clientESP_RMS.available()) {
      RMSExtDataB += clientESP_RMS.readStringUntil('\r');
    }
    S = RMSExtDataB.substring(RMSExtDataB.indexOf("\n\n") + 2);
  } else {
    S = DATE + RS + Source_data;
    if (Source_data == "UxI") {
      S += RS + String(Tension_M) + RS + String(Intensite_M) + RS + String(PowerFactor_M) + GS;
      int i0 = 0;
      int i1 = 0;
      for (int i = 0; i < 100; i++) {
        i1 = (i + 1) % 100;
        if (voltM[i] <= 0 && voltM[i1] > 0) {
          i0 = i1;  //Point de départ tableau . Phase positive
          i = 100;
        }
      }
      for (int i = 0; i < 100; i++) {
        i1 = (i + i0) % 100;
        S += String(int(10 * voltM[i1])) + RS;  //Voltages*10. Increase dynamic
      }
      S += "0" + GS;
      for (int i = 0; i < 100; i++) {
        i1 = (i + i0) % 100;
        S += String(int(10 * ampM[i1])) + RS;  //Currents*10
      }
      S += "0";
    }
    if (Source_data == "UxIx2") {

      S += GS + String(Tension_M) + RS + String(Intensite_M) + RS + String(PuissanceS_M - PuissanceI_M) + RS + String(PowerFactor_M) + RS + String(Energie_M_Soutiree) + RS + String(Energie_M_Injectee);
      S += RS + String(Tension_T) + RS + String(Intensite_T) + RS + String(PuissanceS_T - PuissanceI_T) + RS + String(PowerFactor_T) + RS + String(Energie_T_Soutiree) + RS + String(Energie_T_Injectee);
      S += RS + String(Frequence);
    }
    if (Source_data == "Linky") {
      S += GS;
      while (LastIdx != IdxDataRawLinky) {
        S += String(DataRawLinky[LastIdx]);
        LastIdx = (1 + LastIdx) % 10000;
      }
      S += GS + String(IdxDataRawLinky);
    }
    if (Source_data == "Enphase") {
      S += GS + String(Tension_M) + RS + String(Intensite_M) + RS + String(PuissanceS_M - PuissanceI_M) + RS + String(PowerFactor_M) + RS + String(Energie_M_Soutiree) + RS + String(Energie_M_Injectee);
      S += RS + String(PactProd) + RS + String(PactConso_M);
      String SessionId = "Not Received from Enphase";
      if (Session_id != "") {
        SessionId = "Ok Received from Enphase";
      }
      String Token_Enphase = "Not Received from Enphase";
      if (TokenEnphase.length() > 50) {
        Token_Enphase = "Ok Received from Enphase";
      }
      if (EnphaseUser == "") {
        SessionId = "Not Requested";
        Token_Enphase = "Not Requested";
      }
      S += RS + SessionId;

      S += RS + Token_Enphase;
    }
    if (Source_data == "SmartG") {
      S += GS + SG_dataBrute;
    }
    if (Source_data == "ShellyEm") {
      S += GS + ShEm_dataBrute;
    }
  }

  server.send(200, "text/html", S);
}
void handleAjaxHisto48h() {  // Envoi Historique de 50h (600points) toutes les 5mn
  String S = "";
  String T = "";
  String U = "";
  int iS = IdxStockPW;
  for (int i = 0; i < 600; i++) {
    S += String(tabPw_Maison_5mn[iS]) + ",";
    T += String(tabPw_Triac_5mn[iS]) + ",";
    U += String(tabTemperature_5mn[iS]) + ",";
    iS = (1 + iS) % 600;
  }
  server.send(200, "text/html", Source_data + GS + S + GS + T + GS + String(temperature) + GS + U);
}
void handleAjaxESP32() {  // Envoi des dernières infos sur l'ESP32
  IT10ms = 0;
  IT10ms_in = 0;
  String S = "";
  float H = float(T_On_seconde) / 3600;
  String coeur0 = String(int(previousTimeRMSMin)) + ", " + String(int(previousTimeRMSMoy)) + ", " + String(int(previousTimeRMSMax));
  String coeur1 = String(int(previousLoopMin)) + ", " + String(int(previousLoopMoy)) + ", " + String(int(previousLoopMax));
  S += String(H) + RS + WiFi.RSSI() + RS + WiFi.BSSIDstr() + RS + WiFi.macAddress() + RS + ssid + RS + WiFi.localIP().toString() + RS + WiFi.gatewayIP().toString() + RS + WiFi.subnetMask().toString();
  S += RS + coeur0 + RS + coeur1 + RS + String(P_cent_EEPROM) + RS;
  delay(15);  //Comptage interruptions
  if (IT10ms_in > 0) {
    S += String(IT10ms_in) + "/" + String(IT10ms);
  } else {
    S += "Pas de Triac";
  }
  if (ITmode > 0) {
    S += RS + "Secteur";
  } else {
    S += RS + "Horloge ESP";
  }
  int j = idxMessage;
  for (int i = 0; i < 4; i++) {
    S += RS + Message[j];
    j = (j + 1) % 4;
  }
  server.send(200, "text/html", S);
}
void handleAjaxHisto1an() {  // Envoi Historique Energie quotiiienne sur 1 an 370 points
  server.send(200, "text/html", HistoriqueEnergie1An());
}
void handleAjaxData() {  //Données page d'accueil
  String DateLast = "Attente de l'heure par Internet";
  if (DATEvalid) {
    DateLast = DATE;
  }
  String S = "Deb" + RS + DateLast + RS + Source_data + RS + LTARF + RS + STGE + RS + String(temperature);
  S += GS + String(PuissanceS_M) + RS + String(PuissanceI_M) + RS + String(PVAS_M) + RS + String(PVAI_M);
  S += RS + String(EnergieJour_M_Soutiree) + RS + String(EnergieJour_M_Injectee) + RS + String(Energie_M_Soutiree) + RS + String(Energie_M_Injectee);
  if (Source_data == "UxIx2" || (Source_data == "ShellyEm" && EnphaseSerial.toInt() < 3)) {  //UxIx2 ou Shelly monophasé avec 2 sondes
    S += GS + String(PuissanceS_T) + RS + String(PuissanceI_T) + RS + String(PVAS_T) + RS + String(PVAI_T);
    S += RS + String(EnergieJour_T_Soutiree) + RS + String(EnergieJour_T_Injectee) + RS + String(Energie_T_Soutiree) + RS + String(Energie_T_Injectee);
  }
  S += GS + "Fin";
  server.send(200, "text/html", S);
}
void handleAjax_etatActions() {
  int NbActifs = 0;
  String S = "";
  String On_;
  for (int i = 0; i < NbActions; i++) {
    if (LesActions[i].Actif > 0) {
      S += String(i) + RS + LesActions[i].Titre + RS;
      if (LesActions[i].Actif == 1 && i > 0) {
        if (LesActions[i].On) {
          S += "On" + RS;
        } else {
          S += "Off" + RS;
        }
      } else {
        S += String(100 - Retard[i]) + RS;
      }
      S += GS;
      NbActifs++;
    }
  }
  S = String(temperature) + GS + String(Source_data) + GS + String(RMSextIP) + GS + NbActifs + GS + S;
  server.send(200, "text/html", S);
}
void handleRestart() {  // Eventuellement Reseter l'ESP32 à distance
  server.send(200, "text/plain", "OK Reset. Attendez.");
  delay(1000);
  ESP.restart();
}
void handleAjaxData10mn() {  // Envoi Historique de 10mn (300points)Energie Active Soutiré - Injecté
  String S = "";
  String T = "";
  int iS = IdxStock2s;
  for (int i = 0; i < 300; i++) {
    S += String(tabPw_Maison_2s[iS]) + ",";
    S += String(tabPva_Maison_2s[iS]) + ",";
    T += String(tabPw_Triac_2s[iS]) + ",";
    T += String(tabPva_Triac_2s[iS]) + ",";
    iS = (1 + iS) % 300;
  }
  server.send(200, "text/html", Source_data + GS + S + GS + T);
}
void handleActions() {
  server.send(200, "text/html", String(pages[PAGE_ACTIONS_HTML]));
}
void handleActionsJS() {
  server.send(200, "text/html", String(pages[PAGE_ACTIONS_JS]));
}
void handleActionsUpdate() {
  int adresse_max = 0;
  String s = server.arg("actions");
  String ligne = "";
  InitGpioActions();  //RAZ anciennes actions
  NbActions = 0;
  while (s.indexOf(GS) > 3 && NbActions < LesActionsLength) {
    ligne = s.substring(0, s.indexOf(GS));
    s = s.substring(s.indexOf(GS) + 1);
    LesActions[NbActions].Definir(ligne);
    NbActions++;
  }
  adresse_max = EcritureEnROM();
  server.send(200, "text/plain", "OK" + String(adresse_max));
  InitGpioActions();
}
void handleActionsAjax() {
  String S = String(temperature) + RS + String(LTARFbin) + RS + String(ITmode) + GS;
  for (int i = 0; i < NbActions; i++) {
    S += LesActions[i].Lire();
  }
  server.send(200, "text/html", S);
}
void handlePara() {
  server.send(200, "text/html", String(pages[PAGE_PARA_HTML]));
}
void handleParaUpdate() {
  String Vp[24];
  String lesparas = server.arg("lesparas") + RS;
  int idx = 0;
  while (lesparas.length() > 0) {
    Vp[idx] = lesparas.substring(0, lesparas.indexOf(RS));
    lesparas = lesparas.substring(lesparas.indexOf(RS) + 1);
    idx++;
    Vp[idx].trim();
  }
  dhcpOn = byte(Vp[0].toInt());
  IP_Fixe = strtoul(Vp[1].c_str(), NULL, 10);
  Gateway = strtoul(Vp[2].c_str(), NULL, 10);
  masque = strtoul(Vp[3].c_str(), NULL, 10);
  dns = strtoul(Vp[4].c_str(), NULL, 10);
  Source = Vp[5];
  RMSextIP = strtoul(Vp[6].c_str(), NULL, 10);
  EnphaseUser = Vp[7];
  EnphasePwd = Vp[8];
  EnphaseSerial = Vp[9];
  MQTTRepet = Vp[10].toInt();
  MQTTIP = strtoul(Vp[11].c_str(), NULL, 10);
  MQTTPort = Vp[12].toInt();  //2 bytes
  MQTTUser = Vp[13];
  MQTTPwd = Vp[14];
  MQTTPrefix = Vp[15];
  MQTTdeviceName = Vp[16];
  nomRouteur = Vp[17];
  nomSondeFixe = Vp[18];
  nomSondeMobile = Vp[19];
  nomTemperature = Vp[20];
  CalibU = Vp[21].toInt();  //2 bytes
  CalibI = Vp[22].toInt();  //2 bytes
  TempoEDFon = byte(Vp[23].toInt());
  int adresse_max = EcritureEnROM();
  if (Source != "Ext") {
    Source_data = Source;
  }
  server.send(200, "text/plain", "OK" + String(adresse_max));
  LastHeureEDF = -1;
}
void handleParaJS() {
  server.send(200, "text/html", String(pages[PAGE_PARA_JS]));
}
void handleParaRouteurJS() {
  server.send(200, "text/html", String(pages[PAGE_GLOBAL_PARA_JS]));
}
void handleParaAjax() {
  String S = String(dhcpOn) + RS + String(IP_Fixe) + RS + String(Gateway) + RS + String(masque) + RS + String(dns) + RS + Source + RS + String(RMSextIP) + RS;
  S += EnphaseUser + RS + EnphasePwd + RS + EnphaseSerial + RS;
  S += String(MQTTRepet) + RS + String(MQTTIP) + RS + String(MQTTPort) + RS + MQTTUser + RS + MQTTPwd;
  S += RS + MQTTPrefix + RS + MQTTdeviceName + RS + nomRouteur + RS + nomSondeFixe + RS + nomSondeMobile;
  S += RS + String(temperature) + RS + nomTemperature;
  S += RS + String(CalibU) + RS + String(CalibI);
  S += RS + String(TempoEDFon);
  server.send(200, "text/html", S);
}
void handleParaRouteurAjax() {
  String S = Source + RS + Source_data + RS + nomRouteur + RS + Version + RS + nomSondeFixe + RS + nomSondeMobile + RS + String(RMSextIP);
  S += RS + nomTemperature;
  server.send(200, "text/html", S);
}
void handleSetGpio() {
  int gpio = server.arg("gpio").toInt();
  int out = server.arg("out").toInt();
  String S = "Refut : gpio =" + String(gpio) + " out =" + String(out);
  if (gpio >= 0 && gpio <= 33 && out >= 0 && out <= 1) {
    pinMode(gpio, OUTPUT);
    digitalWrite(gpio, out);
    S = "OK : gpio =" + String(gpio) + " out =" + String(out);
  }
  server.send(200, "text/html", S);
}
void handleAP_ScanWifi() {
  WIFIbug = 0;
  Serial.println("Scan start");

  // WiFi.scanNetworks will return the number of networks found.
  int n = WiFi.scanNetworks();
  Serial.println("Scan done");
  String S = "";
  if (n == 0) {
    Serial.println("Pas de réseau Wifi trouvé");
  } else {
    Serial.print(n);
    Serial.println(" réseaux trouvés");
    Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.printf("%2d", i + 1);
      Serial.print(" | ");
      Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
      Serial.print(" | ");
      Serial.printf("%4d", WiFi.RSSI(i));
      Serial.println();
      S += WiFi.SSID(i).c_str() + RS + WiFi.RSSI(i) + GS;
    }
  }
  WiFi.scanDelete();
  server.send(200, "text/html", S);
}
void handleAP_SetWifi() {
  WIFIbug = 0;
  Serial.println("Set Wifi");
  String NewSsid = server.arg("ssid");
  NewSsid.trim();
  String NewPassword = server.arg("passe");
  NewPassword.trim();
  Serial.println(NewSsid);
  Serial.println(NewPassword);
  ssid = NewSsid;
  password = NewPassword;
  StockMessage("Wifi Begin : " + ssid);
  WiFi.begin(ssid.c_str(), password.c_str());
  unsigned long newstartMillis = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - newstartMillis < 15000)) {  // Attente connexion au Wifi
    Serial.write('.');
    Gestion_LEDs();
    Serial.print(WiFi.status());
    delay(300);
  }
  String S = "";
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    String IP = WiFi.localIP().toString();
    S = "Ok" + RS;
    S += "ESP 32 connecté avec succès au wifi : " + ssid + " avec l'adresse IP : " + IP;
    S += "<br><br> Connectez vous au wifi : " + ssid;
    S += "<br><br> Cliquez sur l'adresse : <a href='http://" + IP + "' >http://" + IP + "</a>";
    EcritureEnROM();
  } else {
    S = "No" + RS + "ESP32 non connecté à :" + ssid + "<br>";
  }
  server.send(200, "text/html", S);
  delay(1000);
  ESP.restart();
}


void handleNotFound() {  //Page Web pas trouvé
  Debug.println(F("Fichier non trouvé"));
  String message = "Fichier non trouvé\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}