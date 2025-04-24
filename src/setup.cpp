/* Dealing with variables initialization

*/

#include "setup.h"

String IpAddress2String(const IPAddress& ipAddress) {
  /******************************************************
   Function IpAddress2String
   Target: Convert IPAddress into String object
   Parameters:
    ipAddress: IPAdress object
   Return: IP Adress in String format
  *****************************************************/
  //Return IPAddress class in String format
  return String(ipAddress[0]) + String(".") +\
    String(ipAddress[1]) + String(".") +\
    String(ipAddress[2]) + String(".") +\
    String(ipAddress[3])  ; 
}


IPAddress stringToIPAddress(String stringIPAddress) {
  /******************************************************
   Function stringToIPAddress
   Target: Convert string into IPAdress class
   Parameters:
    stringIPAddress: IP adress in String format
   Return: IP Adress in IPAddress format
  *****************************************************/
  
  char charToTest;
  uint lastBegin=0,indexArray=0;
  int IPAddressOctectArray[4];
  for (uint i=0; i<=stringIPAddress.length(); i++) {
    charToTest=stringIPAddress.charAt(i);
    if (charToTest=='.') {    
      IPAddressOctectArray[indexArray]=stringIPAddress.substring(lastBegin,i).toInt();
      lastBegin=i+1;
      if (indexArray==2) {
        indexArray++;
        IPAddressOctectArray[indexArray]=stringIPAddress.substring(lastBegin,stringIPAddress.length()).toInt();
      }
      else indexArray++;
    }
  }
  
  return IPAddress(IPAddressOctectArray[0],IPAddressOctectArray[1],IPAddressOctectArray[2],IPAddressOctectArray[3]);
}

bool wifiVariablesInit() {
  /******************************************************
   Function wifiVariablesInit
   Target: Get the WiFi Credential-related variables from EEPROM or global_setup.h
            If varialbes exist in global_setup.h and doesn't exit in EEPPROM, then update EEPROM 
   Parameters: None
   Returns:
    updateEEPROM: True if there are changes to be stored in the EEPROM
  *****************************************************/

  char auxSSID[WIFI_MAX_SSID_LENGTH],auxPSSW[WIFI_MAX_PSSW_LENGTH],auxSITE[WIFI_MAX_SITE_LENGTH],
         auxNTP[NTP_SERVER_NAME_MAX_LENGTH],auxTZEnvVar[TZ_ENV_VARIABLE_MAX_LENGTH],auxTZName[TZ_ENV_NAME_MAX_LENGTH];
  bool updateEEPROM=false;

  memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);EEPROM.get(0x0D,auxSSID);
  if (String(auxSSID).compareTo("")==0) {
    #ifdef WIFI_SSID_CREDENTIALS
      wifiCred.wifiSSIDs[0]=WIFI_SSID_CREDENTIALS;  
      //Check if SSID must be updated in EEPROM
      if (wifiCred.wifiSSIDs[0].compareTo(String(auxSSID))!=0) {
        uint8_t auxLength=wifiCred.wifiSSIDs[0].length()+1;
        if (auxLength>WIFI_MAX_SSID_LENGTH-1) { //Substring if greater that max length
          auxLength=WIFI_MAX_SSID_LENGTH-1;
          wifiCred.wifiSSIDs[0]=wifiCred.wifiSSIDs[0].substring(0,auxLength);
        }
        memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);
        memcpy(auxSSID,wifiCred.wifiSSIDs[0].c_str(),auxLength);
        EEPROM.put(0x0D,auxSSID);
        updateEEPROM=true;
      }
    #else
      wifiCred.wifiSSIDs[0]=auxSSID;
    #endif
  } else wifiCred.wifiSSIDs[0]=auxSSID;
  memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);EEPROM.get(0x2E,auxPSSW);
  if (String(auxPSSW).compareTo("")==0) {
    #ifdef WIFI_PW_CREDENTIALS
      wifiCred.wifiPSSWs[0]=WIFI_PW_CREDENTIALS;
      //Check if PSSW must be updated in EEPROM
      if (wifiCred.wifiPSSWs[0].compareTo(String(auxPSSW))!=0) {
        uint8_t auxLength=wifiCred.wifiPSSWs[0].length()+1;
        if (auxLength>WIFI_MAX_PSSW_LENGTH-1) { //Substring if greater that max length
          auxLength=WIFI_MAX_PSSW_LENGTH-1;
          wifiCred.wifiPSSWs[0]=wifiCred.wifiPSSWs[0].substring(0,auxLength);
        }
        memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);
        memcpy(auxPSSW,wifiCred.wifiPSSWs[0].c_str(),auxLength);
        EEPROM.put(0x2E,auxPSSW);
        updateEEPROM=true;
      }
    #else
      wifiCred.wifiPSSWs[0]=auxPSSW;
    #endif
  } else wifiCred.wifiPSSWs[0]=auxPSSW;
  memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);EEPROM.get(0x6E,auxSITE);
  if (String(auxSITE).compareTo("")==0) { 
    #ifdef WIFI_SITE
      wifiCred.wifiSITEs[0]=WIFI_SITE;
      //Check if SITE must be updated in EEPROM
      if (wifiCred.wifiSITEs[0].compareTo(String(auxSITE))!=0) { 
        uint8_t auxLength=wifiCred.wifiSITEs[0].length()+1;
        if (auxLength>WIFI_MAX_SITE_LENGTH-1) { //Substring if greater that max length
          auxLength=WIFI_MAX_SITE_LENGTH-1;
          wifiCred.wifiSITEs[0]=wifiCred.wifiSITEs[0].substring(0,auxLength);
        }
        memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);
        memcpy(auxSITE,wifiCred.wifiSITEs[0].c_str(),auxLength);
        EEPROM.put(0x6E,auxSITE);
        updateEEPROM=true;
      }
    #else
      wifiCred.wifiSITEs[0]=auxSITE;
    #endif
  } else wifiCred.wifiSITEs[0]=auxSITE;
  memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);EEPROM.get(0x79,auxSSID);
  if (String(auxSSID).compareTo("")==0) { 
    #ifdef WIFI_SSID_CREDENTIALS_BK1
      wifiCred.wifiSSIDs[1]=WIFI_SSID_CREDENTIALS_BK1;
      //Check if SSID_BK1 must be updated in EEPROM
      if (wifiCred.wifiSSIDs[1].compareTo(String(auxSSID))!=0) {
        uint8_t auxLength=wifiCred.wifiSSIDs[1].length()+1;
        if (auxLength>WIFI_MAX_SSID_LENGTH-1) { //Substring if greater that max length
          auxLength=WIFI_MAX_SSID_LENGTH-1;
          wifiCred.wifiSSIDs[1]=wifiCred.wifiSSIDs[1].substring(0,auxLength);
        }
        memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);
        memcpy(auxSSID,wifiCred.wifiSSIDs[1].c_str(),auxLength);
        EEPROM.put(0x79,auxSSID);
        updateEEPROM=true;
      }
    #else
      wifiCred.wifiSSIDs[1]=auxSSID;
    #endif
  } else wifiCred.wifiSSIDs[1]=auxSSID;
  memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);EEPROM.get(0x9A,auxPSSW);
  if (String(auxPSSW).compareTo("")==0) { 
    #ifdef WIFI_PW_CREDENTIALS_BK1
      wifiCred.wifiPSSWs[1]=WIFI_PW_CREDENTIALS_BK1;
      //Check if PSSW_BK1 must be updated in EEPROM
      if (wifiCred.wifiPSSWs[1].compareTo(String(auxPSSW))!=0) {
        uint8_t auxLength=wifiCred.wifiPSSWs[1].length()+1;
        if (auxLength>WIFI_MAX_PSSW_LENGTH-1) { //Substring if greater that max length
          auxLength=WIFI_MAX_PSSW_LENGTH-1;
          wifiCred.wifiPSSWs[1]=wifiCred.wifiPSSWs[1].substring(0,auxLength);
        }
        memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);
        memcpy(auxPSSW,wifiCred.wifiPSSWs[1].c_str(),auxLength);
        EEPROM.put(0x9A,auxPSSW);
        updateEEPROM=true;
      }
    #else
      wifiCred.wifiPSSWs[1]=auxPSSW;
    #endif
  } else wifiCred.wifiPSSWs[1]=auxPSSW;
  memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);EEPROM.get(0xDA,auxSITE);
  if (String(auxSITE).compareTo("")==0) { 
    #ifdef WIFI_SITE_BK1
      wifiCred.wifiSITEs[1]=WIFI_SITE_BK1;
      //Check if SITE_BK1 must be updated in EEPROM
      if (wifiCred.wifiSITEs[1].compareTo(String(auxSITE))!=0) { //Substring if greater that max length
        uint8_t auxLength=wifiCred.wifiSITEs[1].length()+1;
        if (auxLength>WIFI_MAX_SITE_LENGTH-1) { //Substring if greater that max length
          auxLength=WIFI_MAX_SITE_LENGTH-1;
          wifiCred.wifiSITEs[1]=wifiCred.wifiSITEs[1].substring(0,auxLength);
        }
        memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);
        memcpy(auxSITE,wifiCred.wifiSITEs[1].c_str(),auxLength);
        EEPROM.put(0xDA,auxSITE);
        updateEEPROM=true;
      }
    #else
      wifiCred.wifiSITEs[1]=auxSITE;
    #endif
  } else wifiCred.wifiSITEs[1]=auxSITE;
  memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);EEPROM.get(0xE5,auxSSID);
  if (String(auxSSID).compareTo("")==0) { 
    #ifdef WIFI_SSID_CREDENTIALS_BK2
      wifiCred.wifiSSIDs[2]=WIFI_SSID_CREDENTIALS_BK2;
      //Check if SSID_BK2 must be updated in EEPROM
      if (wifiCred.wifiSSIDs[2].compareTo(String(auxSSID))!=0) {
        uint8_t auxLength=wifiCred.wifiSSIDs[2].length()+1;
        if (auxLength>WIFI_MAX_SSID_LENGTH-1) { //Substring if greater that max length
          auxLength=WIFI_MAX_SSID_LENGTH-1;
          wifiCred.wifiSSIDs[2]=wifiCred.wifiSSIDs[2].substring(0,auxLength);
        }
        memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);
        memcpy(auxSSID,wifiCred.wifiSSIDs[2].c_str(),auxLength);
        EEPROM.put(0xE5,auxSSID);
        updateEEPROM=true;
      }
    #else
      wifiCred.wifiSSIDs[2]=auxSSID;
    #endif
  } else wifiCred.wifiSSIDs[2]=auxSSID;
  memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);EEPROM.get(0x106,auxPSSW);
  if (String(auxPSSW).compareTo("")==0) { 
    #ifdef WIFI_PW_CREDENTIALS_BK2
      wifiCred.wifiPSSWs[2]=WIFI_PW_CREDENTIALS_BK2;
      //Check if PSSW_BK2 must be updated in EEPROM
      if (wifiCred.wifiPSSWs[2].compareTo(String(auxPSSW))!=0) {
        uint8_t auxLength=wifiCred.wifiPSSWs[2].length()+1;
        if (auxLength>WIFI_MAX_PSSW_LENGTH-1) { //Substring if greater that max length
          auxLength=WIFI_MAX_PSSW_LENGTH-1;
          wifiCred.wifiPSSWs[2]=wifiCred.wifiPSSWs[2].substring(0,auxLength);
        }
        memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);
        memcpy(auxPSSW,wifiCred.wifiPSSWs[2].c_str(),auxLength);
        EEPROM.put(0x106,auxPSSW);
        updateEEPROM=true;
      }
    #else
      wifiCred.wifiPSSWs[2]=auxPSSW;
    #endif
  } else wifiCred.wifiPSSWs[2]=auxPSSW;
  memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);EEPROM.get(0x146,auxSITE);
  if (String(auxSITE).compareTo("")==0) { 
    #ifdef WIFI_SITE_BK2
      wifiCred.wifiSITEs[2]=WIFI_SITE_BK2;
      //Check if SITE_BK2 must be updated in EEPROM
      if (wifiCred.wifiSITEs[2].compareTo(String(auxSITE))!=0) {
        uint8_t auxLength=wifiCred.wifiSITEs[2].length()+1;
        if (auxLength>WIFI_MAX_SITE_LENGTH-1) { //Substring if greater that max length
          auxLength=WIFI_MAX_SITE_LENGTH-1;
          wifiCred.wifiSITEs[2]=wifiCred.wifiSITEs[2].substring(0,auxLength);
        }
        memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);
        memcpy(auxSITE,wifiCred.wifiSITEs[2].c_str(),auxLength);
        EEPROM.put(0x146,auxSITE);
        updateEEPROM=true;
      }
    #else
      wifiCred.wifiSITEs[2]=auxSITE;
    #endif
  } else wifiCred.wifiSITEs[2]=auxSITE;

  //Get the rest of wifiCred.SiteAllow variables from EEPROM
  configVariables=EEPROM.read(0x2BE);
  wifiCred.SiteAllow[0]=configVariables & 0x01;
  wifiCred.SiteAllow[1]=configVariables & 0x02;
  wifiCred.SiteAllow[2]=configVariables & 0x04;

  return updateEEPROM;
}

bool initTZVariables() {
  /******************************************************
   Function wifiVariablesInit
   Target: Init TZVariables
            If varialbes exist in global_setup.h and doesn't exit in EEPPROM, then update EEPROM 
   Parameters: None
   Returns:
    updateEEPROM: True if there are changes to be stored in the EEPROM
  *****************************************************/
  
  char auxTZEnvVar[TZ_ENV_VARIABLE_MAX_LENGTH],auxTZName[TZ_ENV_NAME_MAX_LENGTH];
  bool updateEEPROM=false;

  memset(auxTZEnvVar,'\0',TZ_ENV_VARIABLE_MAX_LENGTH);EEPROM.get(0x251,auxTZEnvVar);
  memset(auxTZName,'\0',TZ_ENV_NAME_MAX_LENGTH);EEPROM.get(0x28A,auxTZName);
  if (String(auxTZEnvVar).compareTo("")==0) {
    //Take the value from global_setup.h
    #ifdef NTP_TZ_ENV_VARIABLE
      TZEnvVariable=String(NTP_TZ_ENV_VARIABLE);
      //Check if TZEnvVariable must be updated in EEPROM
      if (TZEnvVariable.compareTo(String(auxTZEnvVar))!=0) {
        uint8_t auxLength=TZEnvVariable.length()+1;
        if (auxLength>TZ_ENV_VARIABLE_MAX_LENGTH-1) { //Substring if greater that max length
          auxLength=TZ_ENV_VARIABLE_MAX_LENGTH-1;
          TZEnvVariable=TZEnvVariable.substring(0,auxLength);
        }
        memset(auxTZEnvVar,'\0',TZ_ENV_VARIABLE_MAX_LENGTH);
        memcpy(auxTZEnvVar,TZEnvVariable.c_str(),auxLength);
        EEPROM.put(0x251,auxTZEnvVar);
        updateEEPROM=true;
      }

      TZName=String(NTP_TZ_NAME);
      //Check if TZName must be updated in EEPROM
      if (TZName.compareTo(String(auxTZName))!=0) {
        uint8_t auxLength=TZName.length()+1;
        if (auxLength>TZ_ENV_NAME_MAX_LENGTH-1) { //Substring if greater that max length
          auxLength=TZ_ENV_NAME_MAX_LENGTH-1;
          TZName=TZName.substring(0,auxLength);
        }
        memset(auxTZName,'\0',TZ_ENV_NAME_MAX_LENGTH);
        memcpy(auxTZName,TZName.c_str(),auxLength);
        EEPROM.put(0x28A,auxTZEnvVar);
        updateEEPROM=true;
      }
    #else
      //TZEnvVariable=String(auxTZEnvVar);
      TZEnvVariable=String("CET-1CEST,M3.5.0,M10.5.0/3");
      TZName=String("Europe/Madrid");
    #endif
  }
  else {
    TZEnvVariable=String(auxTZEnvVar);
    TZName=String(auxTZName);
  }

  return updateEEPROM;
}

bool ntpVariablesInit() {
  /******************************************************
   Function ntpVariablesInit
   Target: Get the NTP Server variables from EEPROM or global_setup.h
            If variables exist in global_setup.h and doesn't exist in EEPPROM, then update EEPROM 
   Parameters: None
   Returns:
    updateEEPROM: True if there are changes to be stored in the EEPROM
  *****************************************************/

  char auxNTP[NTP_SERVER_NAME_MAX_LENGTH],auxTZEnvVar[TZ_ENV_VARIABLE_MAX_LENGTH],auxTZName[TZ_ENV_NAME_MAX_LENGTH];
  bool updateEEPROM=false;

  memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);EEPROM.get(0x151,auxNTP);
  if (String(auxNTP).compareTo("")==0) { 
    #ifdef NTP_SERVER
      ntpServers[0]=NTP_SERVER;  
      //Check if NTP must be updated in EEPROM
      if (ntpServers[0].compareTo(String(auxNTP))!=0) {
        uint8_t auxLength=ntpServers[0].length()+1;
        if (auxLength>NTP_SERVER_NAME_MAX_LENGTH-1) { //Substring if greater that max length
          auxLength=NTP_SERVER_NAME_MAX_LENGTH-1;
          ntpServers[0]=ntpServers[0].substring(0,auxLength);
        }
        memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
        memcpy(auxNTP,ntpServers[0].c_str(),auxLength);
        EEPROM.put(0x151,auxNTP);
        updateEEPROM=true;
      }
    #else
      ntpServers[0]="time.apple.com"; //Always should be one NTP server
    #endif
  } else ntpServers[0]=auxNTP;

  memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);EEPROM.get(0x191,auxNTP);
  if (String(auxNTP).compareTo("")==0) { 
    #ifdef NTP_SERVER2
      ntpServers[0]=NTP_SERVER2;  
      //Check if NTP must be updated in EEPROM
      if (ntpServers[1].compareTo(String(auxNTP))!=0) {
        uint8_t auxLength=ntpServers[1].length()+1;
        if (auxLength>NTP_SERVER_NAME_MAX_LENGTH-1) { //Substring if greater that max length
          auxLength=NTP_SERVER_NAME_MAX_LENGTH-1;
          ntpServers[1]=ntpServers[1].substring(0,auxLength);
        }
        memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
        memcpy(auxNTP,ntpServers[1].c_str(),auxLength);
        EEPROM.put(0x191,auxNTP);
        updateEEPROM=true;
      }
    #else
      ntpServers[1]=auxNTP;
    #endif
  } else ntpServers[1]=auxNTP;

  memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);EEPROM.get(0x1D1,auxNTP);
  if (String(auxNTP).compareTo("")==0) { 
    #ifdef NTP_SERVER3
      ntpServers[0]=NTP_SERVER3;  
      //Check if NTP must be updated in EEPROM
      if (ntpServers[2].compareTo(String(auxNTP))!=0) {
        uint8_t auxLength=ntpServers[2].length()+1;
        if (auxLength>NTP_SERVER_NAME_MAX_LENGTH-1) { //Substring if greater that max length
          auxLength=NTP_SERVER_NAME_MAX_LENGTH-1;
          ntpServers[2]=ntpServers[2].substring(0,auxLength);
        }
        memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
        memcpy(auxNTP,ntpServers[2].c_str(),auxLength);
        EEPROM.put(0x1D1,auxNTP);
        updateEEPROM=true;
      }
    #else
      ntpServers[2]=auxNTP;
    #endif
  } else ntpServers[2]=auxNTP;

  memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);EEPROM.get(0x211,auxNTP);
  if (String(auxNTP).compareTo("")==0) {
    #ifdef NTP_SERVER4
      ntpServers[0]=NTP_SERVER4;  
      //Check if NTP must be updated in EEPROM
      if (ntpServers[3].compareTo(String(auxNTP))!=0) {
        uint8_t auxLength=ntpServers[3].length()+1;
        if (auxLength>NTP_SERVER_NAME_MAX_LENGTH-1) { //Substring if greater that max length
          auxLength=NTP_SERVER_NAME_MAX_LENGTH-1;
          ntpServers[3]=ntpServers[3].substring(0,auxLength);
        }
        memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
        memcpy(auxNTP,ntpServers[3].c_str(),auxLength);
        EEPROM.put(0x211,auxNTP);
        updateEEPROM=true;
      }
    #else
      ntpServers[3]=auxNTP;
    #endif
  }
  else ntpServers[3]=auxNTP;

  //Update volatile TZEnvVariable & TZName. Same function is called after waking up from sleep if pushed button1
  updateEEPROM|=initTZVariables();

  ntpServerIndex=0;

  return updateEEPROM;
}

bool mqttVariablesInit() {
  /******************************************************
   Function wifiVariablesInit
   Target: Get the MQTT Server variables from EEPROM or global_setup.h
            If variables exist in global_setup.h and doesn't exist in EEPPROM, then update EEPROM 
   Parameters: None
   Returns:
    updateEEPROM: True if there are changes to be stored in the EEPROM
  *****************************************************/

  bool updateEEPROM=false;
  char auxUserName[WEB_USER_CREDENTIAL_LENGTH],auxUserPssw[WEB_PW_CREDENTIAL_LENGTH],
        auxMQTT[MQTT_SERVER_NAME_MAX_LENGTH];
  
  memset(auxMQTT,'\0',MQTT_SERVER_NAME_MAX_LENGTH);EEPROM.get(0x2BF,auxMQTT);
  if (String(auxMQTT).compareTo("")==0) {
    #ifdef MQTT_SERVER
      mqttServer=MQTT_SERVER;  
      //Check if MQTT server must be updated in EEPROM
      if (mqttServer.compareTo(String(auxMQTT))!=0) {
        uint8_t auxLength=mqttServer.length()+1;
        if (auxLength>MQTT_SERVER_NAME_MAX_LENGTH-1) { //Substring if greater that max length
          auxLength=MQTT_SERVER_NAME_MAX_LENGTH-1;
          mqttServer=mqttServer.substring(0,auxLength);
        }
        memset(auxMQTT,'\0',MQTT_SERVER_NAME_MAX_LENGTH);
        memcpy(auxMQTT,mqttServer.c_str(),auxLength);
        EEPROM.put(0x2BF,auxMQTT);
        updateEEPROM=true;
      }
    #else
      mqttServer=auxMQTT;
    #endif
  } else mqttServer=auxMQTT;

  //Get the MQTT User Credential-related variables from EEPROM or global_setup.h
  //If variables exist in global_setup.h and doesn't exist in EEPPROM, then update EEPROM 
  memset(auxUserName,'\0',MQTT_USER_CREDENTIAL_LENGTH);EEPROM.get(0x2FF,auxUserName);
  if (String(auxUserName).compareTo("")==0) {
    mqttUserName=MQTT_USER_CREDENTIAL;  
    EEPROM.put(0x2FF,auxUserName);
    updateEEPROM|=true;
  }
  else {
    mqttUserName=String(auxUserName);
  }
  memset(auxUserPssw,'\0',MQTT_PW_CREDENTIAL_LENGTH);EEPROM.get(0x30A,auxUserPssw);
  if (String(auxUserPssw).compareTo("")==0) {
    mqttUserPssw=MQTT_PW_CREDENTIAL;  
    EEPROM.put(0x30A,auxUserPssw);
    updateEEPROM|=true;
  }
  else {
    mqttUserPssw=String(auxUserPssw);
  }

  //Get the MQTT Topic Name variables from EEPROM or global_setup.h
  //If variables exist in global_setup.h and doesn't exist in EEPPROM, then update EEPROM
  char auxMqttTopicPrefix[MQTT_TOPIC_NAME_MAX_LENGTH];
  memset(auxMqttTopicPrefix,'\0',MQTT_TOPIC_NAME_MAX_LENGTH);EEPROM.get(0x315,auxMqttTopicPrefix);
  if (String(auxMqttTopicPrefix).compareTo("")==0) {
    #ifdef MQTT_TOPIC_PREFIX
      mqttTopicPrefix=MQTT_SERVER;
      if (mqttTopicPrefix.charAt(mqttTopicPrefix.length()-1)!='/') mqttTopicPrefix+="/"; //Adding slash at the end if needed
      //Check if MQTT topic name must be updated in EEPROM
      if (mqttTopicPrefix.compareTo(String(auxMqttTopicPrefix))!=0) {
        uint8_t auxLength=mqttTopicPrefix.length()+1;
        if (auxLength>MQTT_TOPIC_NAME_MAX_LENGTH-1) { //Substring if greater that max length
          auxLength=MQTT_TOPIC_NAME_MAX_LENGTH-1;
          mqttTopicPrefix=mqttTopicPrefix.substring(0,auxLength);
        }
        memset(auxMqttTopicPrefix,'\0',MQTT_TOPIC_NAME_MAX_LENGTH);
        memcpy(auxMqttTopicPrefix,mqttTopicPrefix.c_str(),auxLength);
        EEPROM.put(0x315,auxMqttTopicPrefix);
        updateEEPROM=true;
      }
    #else
      mqttTopicPrefix=auxMqttTopicPrefix;
    #endif
  } else mqttTopicPrefix=auxMqttTopicPrefix;
  mqttTopicName=mqttTopicPrefix+device; //Adding the device name to the MQTT Topic name

  return updateEEPROM;
}

void EEPROMInit() {
  /******************************************************
   Function EEPROMInit
   Target: Take variables from EEPROM or saved them to EEPROM
   Parameters: None
   Returns: Nothing
  *****************************************************/

  bool updateEEPROM=false;

  //EEPROM init first
  EEPROM.begin(EEPROM_SIZE); // Initialize EEPROM with predefined size. Config variables ares stored there

  memset(activeCookie,'\0',COOKIE_SIZE); //init variable
  memset(currentSetCookie,'\0',COOKIE_SIZE); //init variable

  //Check if it is the first run after the very first firmware upload
  memset(firmwareVersion,'\0',VERSION_CHAR_LENGTH+1);EEPROM.get(0x00,firmwareVersion);
  uint16_t readChecksum,computedChecksum;
  readChecksum=EEPROM.read(7);readChecksum=readChecksum<<8;readChecksum|=EEPROM.read(6);
  computedChecksum=checkSum((byte*)firmwareVersion,VERSION_CHAR_LENGTH);
  
  if (readChecksum!=computedChecksum) {
    //It's the first run after the very first firmware upload
    //Variable inizialization to values configured in global_setup

    if (debugModeOn) {boardSerialPort.println("  [EEPROMInit] - Version checksums differ: firmwareVersion"+String(firmwareVersion)+", readChecksums="+String(readChecksum)+", computedChecksum="+String(computedChecksum));}

    //Save version and checksum
    byte auxBuf[]=VERSION;
    computedChecksum=checkSum(auxBuf,VERSION_CHAR_LENGTH);
    for (int i=0; i<VERSION_CHAR_LENGTH; i++) EEPROM.write(i,auxBuf[i]);EEPROM.write(VERSION_CHAR_LENGTH,'\0');
    EEPROM.write(6,(byte) computedChecksum);
    computedChecksum=computedChecksum>>8;
    EEPROM.write(7,(byte) computedChecksum);
    
    //Writing default values in EEPROM
    factoryConfReset();
    updateEEPROM=true;
  }
  else {
    //Not the very first run after firmware upgrade.

    //Update the firmware version in EEPROM if needed
    if (String(firmwareVersion).compareTo(String(VERSION))!=0) {
      updateEEPROM=true;
      byte auxBuf[]=VERSION;
      computedChecksum=checkSum(auxBuf,VERSION_CHAR_LENGTH);
      for (int i=0; i<VERSION_CHAR_LENGTH; i++) EEPROM.write(i,auxBuf[i]);EEPROM.write(VERSION_CHAR_LENGTH,'\0');
      EEPROM.write(6,(byte) computedChecksum);
      computedChecksum=computedChecksum>>8;
      EEPROM.write(7,(byte) computedChecksum);
      bootCount=0;
    }

    //Get the rest of variables from EEPROM
    configVariables=EEPROM.read(0x08);
    //configSavingEnergyMode=configVariables & 0x02?reducedEnergy:lowestEnergy;
    httpCloudEnabled=configVariables & 0x04;
    bluetoothEnabled=configVariables & 0x08;
    wifiEnabled=configVariables & 0x10;
    webServerEnabled=configVariables & 0x20;
    mqttServerEnabled=configVariables & 0x40;
    secureMqttEnabled=configVariables & 0x80;

    //Get the WiFi Credential-related variables from EEPROM or global_setup.h
    //If variables exist in global_setup.h and doesn't exit in EEPPROM, then update EEPROM 
    updateEEPROM|=wifiVariablesInit();

    //Get the NTP Server variables from EEPROM or global_setup.h
    //If variables exist in global_setup.h and doesn't exist in EEPPROM, then update EEPROM 
    updateEEPROM|=ntpVariablesInit();

    //Get the WEB User Credential-related variables from EEPROM or global_setup.h
    //If variables exist in global_setup.h and doesn't exist in EEPPROM, then update EEPROM 
    /*TO BE DONE*/

    //Get the MQTT Server variables from EEPROM or global_setup.h
    //If variables exist in global_setup.h and doesn't exist in EEPPROM, then update EEPROM 
    updateEEPROM|=mqttVariablesInit();

    //Set the bootCount from EEPROM
    bootCount=bootCount==255?EEPROM.read(0x3DE)+1:1; //bootCount = 255 if VERSION = version in EEPROM. Otherwhise bootCount=0 (need to update value in EEPROM)
    EEPROM.write(0x3DE,bootCount);

    //Set the resettCount from EEPROM and check if it needs to be updated
    resetCount=EEPROM.read(0x3DF);
    switch (esp_reset_reason()) { //v1.2.0 - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/misc_system_api.html#_CPPv418esp_reset_reason_t
      case ESP_RST_UNKNOWN: //Reset reason can not be determined
        //In all these cases, there was a wrong code that triggered the reset. Count it
        resetCount++;updateEEPROM=true;
        boardSerialPort.println("  [EEPROMInit] - Reset reason=ESP_RST_UNKNOWN. Resetting resetCount");break;
      case ESP_RST_PANIC: //Software reset due to exception/panic
        //In all these cases, there was a wrong code that triggered the reset. Count it
        resetCount++;updateEEPROM=true;
        boardSerialPort.println("  [EEPROMInit] - Reset reason=ESP_RST_PANIC. Resetting resetCount");break;
      case ESP_RST_INT_WDT: //Reset (software or hardware) due to interrupt watchdog.
        boardSerialPort.println("  [EEPROMInit] - Reset reason=ESP_RST_INT_WDT");break;
      case ESP_RST_TASK_WDT: //Reset due to task watchdog
        //In all these cases, there was a wrong code that triggered the reset. Count it
        resetCount++;updateEEPROM=true;
        boardSerialPort.println("  [EEPROMInit] - Reset reason=ESP_RST_TASK_WDT. Resetting resetCount");break;
      case ESP_RST_WDT: //Reset due to other watchdogs
        //In all these cases, there was a wrong code that triggered the reset. Count it
        resetCount++;updateEEPROM=true;
        boardSerialPort.println("  [EEPROMInit] - Reset reason=ESP_RST_WDT. Resetting resetCount");break;
      case ESP_RST_POWERON: //Power-on event
        boardSerialPort.println("  [EEPROMInit] - Reset reason=ESP_RST_POWERON");break;
      case ESP_RST_EXT: //External pin (not applicable for ESP32)
        boardSerialPort.println("  [EEPROMInit] - Reset reason=ESP_RST_EXT");break;
      case ESP_RST_SW: //Software reset via esp_restart()
        boardSerialPort.println("  [EEPROMInit] - Reset reason=ESP_RST_SW");break;
      case ESP_RST_DEEPSLEEP: //Reset after exiting deep sleep mode
        boardSerialPort.println("  [EEPROMInit] - Reset reason=ESP_RST_DEEPSLEEP");break;
      case ESP_RST_BROWNOUT: //Brownout reset (software or hardware) - Supply voltage goes below safe level
        boardSerialPort.println("  [EEPROMInit] - Reset reason=ESP_RST_BROWNOUT");break;
      case ESP_RST_SDIO: //Reset over SDIO
        boardSerialPort.println("  [EEPROMInit] - Reset reason=ESP_RST_SDIO");break;
      default:
        boardSerialPort.println("  [EEPROMInit] - Reset reason=default");break;
    }
  }

  if (updateEEPROM) 
    {if (debugModeOn) {boardSerialPort.println("  [EEPROMInit] - Update EEPROM variables with values taken from global_setup.h");}}
  else 
    {if (debugModeOn) {boardSerialPort.println("  [EEPROMInit] - Update EEPROM with bootCount variables: bootCount="+String(bootCount));}}
  EEPROM.commit();
}

void variablesInit() {
  /******************************************************
   Function variablesInit
   Target: Init all the variables
   Parameters: None
   Returns: Nothing
  *****************************************************/
  
  //Adding the 3 latest mac bytes to the device name (in Hex format)
  WiFi.macAddress(mac);
  device=device+"-"+String((char)hex_digits[mac[3]>>4])+String((char)hex_digits[mac[3]&15])+
    String((char)hex_digits[mac[4]>>4])+String((char)hex_digits[mac[4]&15])+
    String((char)hex_digits[mac[5]>>4])+String((char)hex_digits[mac[5]&15]);

  //EEPROM init
  EEPROMInit(); //Wifi, ntp, web server and mqtt variables updated from that function
  
 //Variable init
  lastMQTTChangeCheck=0,lastCloudClockChangeCheck=0;

  //MQTT callbacks definition
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onPublish(onMqttPublish);

  //Pre-setting up URL things to upload samples to an external server
  //Converting SERVER_UPLOAD_SAMPLES into IPAddress variable
  serverToUploadSamplesIPAddress=stringToIPAddress(serverToUploadSamplesString);
}

uint32_t tempSensorInit(boolean debugModeOn) {
  /******************************************************
   Function tempSensorInit
   Target: Init all the variables
   Parameters:
    debugModeOn: True to print out logs
   Return: error_type
  *****************************************************/

  uint32_t error_setup=0;
  pinMode(I2C_SDA,INPUT_PULLUP); pinMode(I2C_SCL,INPUT_PULLUP);
  tempHumSensor.begin(I2C_SDA,I2C_SCL);

  int errorSns = tempHumSensor.getError();
  uint8_t statSns = tempHumSensor.getStatus();

  if (!tempHumSensor.isConnected() || 0==tempHumSensorType.compareTo("UNKNOW"))
    error_setup|=ERROR_SENSOR_TEMP_HUM_SETUP;
  if ((error_setup & ERROR_SENSOR_TEMP_HUM_SETUP)==0) { 
    if (debugModeOn) {
      boardSerialPort.print("\n  [tempSensorInit] - Tp/Hm Sensor type: "); boardSerialPort.println(tempHumSensorType); 
      boardSerialPort.print("  [tempSensorInit] - Tp/Hm Sen. status: "); boardSerialPort.println(statSns,HEX);
      boardSerialPort.print("  [tempSensorInit] - Tp/Hm Sen.  error: "); boardSerialPort.println(errorSns,HEX);
      boardSerialPort.print("  [tempSensorInit] - Tp/Hm Sen. resolu.: "); boardSerialPort.println(tempHumSensor.getResolution());
      boardSerialPort.println("  [tempSensorInit] - OK");
    }
    else boardSerialPort.println(" ...... [OK]");
  }
  else {
    if (debugModeOn) {
      boardSerialPort.print(".\n  [tempSensorInit] - Tp/Hm Sensor type: "); boardSerialPort.print(tempHumSensorType);boardSerialPort.println(" - Shouldn't be UNKNOWN");
      boardSerialPort.print("  [tempSensorInit] - Tp/Hm Sen. status: "); boardSerialPort.println(statSns,HEX);
      boardSerialPort.print("  [tempSensorInit] - Tp/Hm Sen.  error: "); boardSerialPort.print(errorSns,HEX);boardSerialPort.println(" - Should be 0");
      boardSerialPort.print("  [tempSensorInit] - Tp/Hm Sen. resolu.: "); boardSerialPort.println(tempHumSensor.getResolution());
      boardSerialPort.println("  [tempSensorInit] - KO");
    }
    else boardSerialPort.println(" ...... [KO]");
  }
  return error_setup;
}

uint32_t wifiInit(boolean wifiEnabled,boolean debugModeOn) {
  /******************************************************
   Function wifiInit
   Target: Init the wifi stuff
   Parameters:
    wifiEnabled: True if WiFi is enabled
    debugModeOn: True to print out logs
   Return: error_setup
  *****************************************************/
 
  uint32_t error_setup=0;
  
  if (wifiEnabled) {//Only if WiFi is enabled
    if (debugModeOn) {
      boardSerialPort.println("\n  [wifiInit] - WiFi enabled. Setting WiFi variables ....");
      boardSerialPort.println("  [wifiInit]    + wifiCred.wifiSSIDs[0]='"+wifiCred.wifiSSIDs[0]+"', wifiCred.wifiPSSWs[0]='"+wifiCred.wifiPSSWs[0]+"', wifiCred.wifiSITEs[0]='"+wifiCred.wifiSITEs[0]+"'");
      boardSerialPort.println("  [wifiInit]    + wifiCred.wifiSSIDs[1]='"+wifiCred.wifiSSIDs[1]+"', wifiCred.wifiPSSWs[1]='"+wifiCred.wifiPSSWs[1]+"', wifiCred.wifiSITEs[1]='"+wifiCred.wifiSITEs[1]+"'");
      boardSerialPort.println("  [wifiInit]    + wifiCred.wifiSSIDs[2]='"+wifiCred.wifiSSIDs[2]+"', wifiCred.wifiPSSWs[2]='"+wifiCred.wifiPSSWs[2]+"', wifiCred.wifiSITEs[2]='"+wifiCred.wifiSITEs[2]+"'");
      boardSerialPort.println("  [wifiInit]    + ntpServers[0]='"+ntpServers[0]+"', ntpServers[1]='"+ntpServers[1]+"', ntpServers[2]='"+ntpServers[2]+"', ntpServers[3]='"+ntpServers[3]+"'");
      boardSerialPort.println("  [wifiInit]    + TZEnvVariable='"+TZEnvVariable+"', TZName='"+TZName+"'");
    }
    error_setup|=wifiConnect(debugModeOn,&auxLoopCounter,&auxCounter);

    //print Logs
    if ((error_setup & ERROR_WIFI_SETUP)==0 ) { 
      if (debugModeOn) boardSerialPort.println("[setup] - WiFi connection .......... [OK]");
      else boardSerialPort.println(" ...... [OK]");

      //WifiNet is updated in printCurrentWiFi(), which is called by wifiConnect(true,X);
      //WiFi.RSSI() might be used instead, but doesn't hurt keeping wifiNet.RSSI instead, as printCurrentWiFi() is required
      // to print logs in here.
      if (wifiNet.RSSI>=WIFI_100_RSSI) wifiCurrentStatus=wifi100Status;
      else if (wifiNet.RSSI>=WIFI_075_RSSI) wifiCurrentStatus=wifi75Status;
      else if (wifiNet.RSSI>=WIFI_050_RSSI) wifiCurrentStatus=wifi50Status;
      else if (wifiNet.RSSI>=WIFI_025_RSSI) wifiCurrentStatus=wifi25Status;
      else if (wifiNet.RSSI<WIFI_000_RSSI) wifiCurrentStatus=wifi0Status;
    } else {
      if (debugModeOn) boardSerialPort.println("  [wifiInit] - WiFi connection .......... [KO]");
      else boardSerialPort.println(" ...... [KO]");
      wifiCurrentStatus=wifiOffStatus;
    }
  }
  else {//If WiFi is not enabled, then inform
    if (debugModeOn) boardSerialPort.println("\n  [wifiInit] - WiFi connection .......... [N/E]");
    else boardSerialPort.println(" ...... [N/E]");
    wifiCurrentStatus=wifiOffStatus;
  }

  return error_setup;
}

uint32_t httpCloudInit(boolean wifiEnabled,boolean httpCloudEnabled,enum wifiStatus wifiCurrentStatus,boolean debugModeOn,boolean fromSetup) {
  /******************************************************
   Function httpCloudInit
   Target: Init the HTTP web server cloud stuff
   Parameters:
    wifiEnabled: True if WiFi is enabled
    wifiCurrentStatus: enum wifiStatus {wifiOffStatus,wifi0Status,wifi25Status,wifi50Status,wifi75Status,wifi100Status};
    debugModeOn: True to print out logs
    fromSetup: True if called from setup
   Return: error_setup
  *****************************************************/
  uint32_t error_setup=0;
  
  //HTTP Cloud Server
  CloudSyncCurrentStatus=CloudSyncOffStatus;

  if (httpCloudEnabled) {//Only if HTTP Cloud is enabled
    if (wifiCurrentStatus!=wifiOffStatus && wifiEnabled) { 
      if (debugModeOn) boardSerialPort.println("\n  [httpCloudInit] - Setting HTTP Cloud server ....");
      error_setup|=sendHttpRequest(debugModeOn,serverToUploadSamplesIPAddress,SERVER_UPLOAD_PORT,"test HTTP/1.1",fromSetup);
      lastTimeHTTPClouCheck=millis();
      if ((error_setup & ERROR_CLOUD_SERVER)==0) {
        CloudSyncCurrentStatus=CloudSyncOnStatus;
        if (debugModeOn) boardSerialPort.println("  [httpCloudInit] - HTTP Cloud config .......... [OK]");
        else boardSerialPort.println(" ...... [OK]");
      }
      else {
        if (debugModeOn) boardSerialPort.println("  [httpCloudInit] - HTTP Cloud config .......... [KO]");
        else boardSerialPort.println(" ...... [KO]");
      }
    }
    else {
      if (debugModeOn) boardSerialPort.println("\n  [httpCloudInit] - HTTP Cloud server can't be setup: No WiFi connection or WiFi is disabled....");
      else boardSerialPort.println(" ...... [KO-N/E]");
    }
  }
  else {
    if (debugModeOn) boardSerialPort.println("\n  [httpCloudInit] - HTTP Cloud is disabled....");
    else boardSerialPort.println(" ...... [N/E]");
  }
  CloudSyncLastStatus=CloudSyncCurrentStatus;

  return error_setup;
}

uint32_t ntpInit(boolean wifiEnabled,boolean ntpEnabled,enum wifiStatus wifiCurrentStatus,boolean debugModeOn,boolean fromSetup) {
  /******************************************************
   Function ntpInit
   Target: Init the NTP stuff
   Parameters:
    wifiEnabled: True if WiFi is enabled
    wifiCurrentStatus: enum wifiStatus {wifiOffStatus,wifi0Status,wifi25Status,wifi50Status,wifi75Status,wifi100Status};
    debugModeOn: True to print out logs
    fromSetup: True if called from setup
   Return: error_setup
  *****************************************************/
  uint32_t error_setup=0;
  
  //NTP Server
  CloudClockCurrentStatus=CloudClockOffStatus;

  if (ntpEnabled) {//Only if NTP is enabled
    if (wifiCurrentStatus!=wifiOffStatus && wifiEnabled) { 
      if (debugModeOn) boardSerialPort.println("\n  [ntpInit] - Setting NTP server ....");
      error_setup|=setupNTPConfig(debugModeOn,fromSetup,&auxLoopCounter2,&whileLoopTimeLeft); //Control variables were init in initVariables()
      lastTimeNTPCheck=millis();
      if ((error_setup & ERROR_NTP_SERVER)==0) {
        CloudClockCurrentStatus=CloudClockOnStatus;
        if (debugModeOn) boardSerialPort.println("  [ntpInit] - NTP config .......... [OK]");
        else boardSerialPort.println(" ...... [OK]");
      }
      else {
        CloudClockCurrentStatus=CloudClockOffStatus;
        if (debugModeOn) boardSerialPort.println("  [ntpInit] - NTP config .......... [KO]");
        else boardSerialPort.println(" ...... [KO]");
      }
    }
    else {
      if (debugModeOn) boardSerialPort.println("\n  [ntpInit] - NTP server can't be setup: No WiFi connection or WiFi is disabled....");
      else boardSerialPort.println(" ...... [KO-N/E]");
    }
  }
  else {
    if (debugModeOn) boardSerialPort.println("\n  [ntpInit] - NTP is disabled....");
    else boardSerialPort.println(" ...... [N/E]");
  }
  CloudClockLastStatus=CloudClockCurrentStatus;

  return error_setup;
}

uint32_t mqttClientInit(boolean wifiEnabled, boolean mqttServerEnabled,boolean secureMqttEnabled, uint32_t error_setup, bool debugModeOn, bool fromSetup, String mqttTopicName, String device) {
  /******************************************************
   Function mqttClientInit
   Target: Init the MQTT stuff
   Parameters:
    wifiEnabled: True if wifi is enabled
    mqttServerEnabled: True if mqtt is enabled
    secureMqttEnabled: True if secure mqtt is enabled
    debugModeOn: True to print out logs
    fromSetup: True if it is called from setup time
    mqttTopicName: MQTT topic name
    device; device name
   Return: error_setup
  *****************************************************/

  uint32_t errorMqtt=NO_ERROR;
  MqttSyncCurrentStatus=MqttSyncOffStatus;

  if (mqttServerEnabled) {//Only if MQTT is enabled
    if (wifiCurrentStatus!=wifiOffStatus && wifiEnabled) { 
      if (debugModeOn) {
        if (fromSetup) {boardSerialPort.print("\n");boardSerialPort.print("  [mqttClientInit] - MQTT init.");}
        else boardSerialPort.print(String(millis())+" - [mqttClientInit] - MQTT init.");
      }
      
      //Connect to MQTT Server
      uint8_t ip[4];
      if (sscanf(mqttServer.c_str(), "%u.%u.%u.%u", &ip[0], &ip[1], &ip[2], &ip[3]) ==4) {
        //FQDN
        mqttClient.setServer(mqttServer.c_str(), MQTTSERVER_PORT); //init variable 
      }
      else {
        //IP Address
        mqttClient.setServer(stringToIPAddress(mqttServer), MQTTSERVER_PORT); //init variable
      }

      // If secure MQTT is enabled
      if (secureMqttEnabled) {
        mqttClient.setCredentials(mqttUserName.c_str(),mqttUserPssw.c_str()); //init variable
      }

      if ((error_setup & ERROR_WIFI_SETUP)==0 && wifiEnabled) { 
        //Last Will Testament message (to be sent by the Broker if connection is lost), not retained in the broker - Home Assistant support - v1.9
        //Requires the Will Flag to be set
        //As mqttClient.setWill is Asynchronous, it requires the variables to be static (char []). String("....").c_str() into the mqttClient.setWill call doen't work.
        char topicLWT[String(mqttTopicName+"/LWT").length()+1];strcpy(topicLWT,String(mqttTopicName+"/LWT").c_str());
        char payloadLWT[8]; strcpy(payloadLWT,"Offline\0");
        mqttClient.setWill(topicLWT,0,false,payloadLWT);

        if (fromSetup && debugModeOn) {
          boardSerialPort.println("\n  [mqttClientInit] - Connecting to MQTT SRV: "+mqttServer+", secureMqttEnabled="+String(secureMqttEnabled));
          boardSerialPort.println("  [mqttClientInit] - Root Topic Name="+mqttTopicName);
          boardSerialPort.println("  [mqttClientInit] - userName="+userName+", userPssw=******");
        }

        //Connect to MQTT broker
        mqttClient.connect();
        
        //Wait for the MQTT client to get connected or timeout (5sg), whatever happens first
        ulong now=millis();
        while (!mqttClient.connected() && (millis()<=now+5000)) {;}

        if (mqttClient.connected()) {
          MqttSyncCurrentStatus=MqttSyncOnStatus;
          if (fromSetup && debugModeOn) {
            boardSerialPort.println("  [mqttClientInit] - Connection to MQTT SRV: "+mqttServer+" [OK]");
            boardSerialPort.println("  [mqttClientInit] - Sending Home Assistant Discovery Messages");
          }
          
          //Publish HA Discovery messages - v1.9
          mqttClientPublishHADiscovery(mqttTopicName,device,WiFi.localIP().toString());
        }

        if (MqttSyncCurrentStatus==MqttSyncOnStatus) {
          if (fromSetup) {
            if (debugModeOn) {boardSerialPort.println("  [mqttClientInit] - [OK]");}
            else boardSerialPort.println(" ...... [OK]");
          }
          else boardSerialPort.println(" ...... [OK]");
        }
        else
        {
          if (fromSetup) {
            if (debugModeOn) {boardSerialPort.println("  [mqttClientInit] - [KO]");}
            else boardSerialPort.println(" ...... [KO]");
          }
          else boardSerialPort.println(" ...... [KO]");
          errorMqtt=ERROR_MQTT_SERVER;
        }
      }
      else 
      {
        if (fromSetup) {
          if (debugModeOn)  {boardSerialPort.println("  [mqttClientInit]- No WiFi");}
          else boardSerialPort.println(" ...... [KO-N/E]");
        }
        else boardSerialPort.println(" ...... [KO-N/E]");
        errorMqtt=ERROR_MQTT_SERVER;
      }
    }
    else {
      if (fromSetup) {
        if (debugModeOn) boardSerialPort.println("\n  [mqttClientInit] - MQTT server can't be setup: No WiFi connection or WiFi is disabled....");
        else boardSerialPort.println(" ...... [KO-N/E]");
      }
      else boardSerialPort.println(" ...... [KO-N/E]"); //This case is supposed not to occur
      errorMqtt=ERROR_MQTT_SERVER;
    }
  }
  else {
    if (fromSetup) {
      if (debugModeOn) boardSerialPort.println("\n  [mqttClientInit] - MQTT is disabled....");
      else boardSerialPort.println(" ...... [N/E]");
    }
    else boardSerialPort.println(" ...... [N/E]"); //This case is supposed not to occur
    errorMqtt=ERROR_MQTT_SERVER;
  }

  return errorMqtt;
}