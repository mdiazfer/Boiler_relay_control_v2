#include "eeprom_utils.h"

//EEPROM MAP - Max size for ESP32 is 4kB = 0x000 - 0xFFF - https://esp32.com/viewtopic.php?t=9136#p38187
//Address 0-5: Stores the firmware version char []*
//Address 6-7: Stores checksum
//Address 08: Stores Config Variable Valued (configVariables)
//  - Bit 0: notFirstRun - 1=true, 0=false
//  - Bit 1: configSavingEnergyMode variable - 1=reduced, 0=lowest
//  - Bit 2: uploadSamplesEnabled - 1=true, 0=false
//  - Bit 3: bluetoothEnabled - 1=true, 0=false
//  - Bit 4: wifiEnabled - 1=true, 0=false
//  - Bit 5: webServerEnable - 1=true, 0=false
//  - Bit 6: mqttServerEnable - 1=true, 0=false
//  - Bit 7: secureMmqttEnable - 1=true, 0=false
//Address 09-0C: Stores float_t batCharge (4 B)
//Address 0D-2D:   SSID1 char []* (32 B+null=33 B)
//Address 2E-6D:   PSSW1 char []* (63 B+null=64 B)
//Address 6E-78:   SITE1 char []* (10 B+null=11 B)
//Address 79-99:   SSID2 char []* (32 B+null=33 B)
//Address 9A-D9:   PSSW2 char []* (63 B+null=64 B)
//Address DA-E4:   SITE2 char []* (10 B+null=11 B)
//Address E5-105:  SSID3 char []* (32 B+null=33 B)
//Address 106-145: PSSW3 char []* (63 B+null=64 B)
//Address 146-150: SITE3 char []* (10 B+null=11 B)
//Address 151-190: NTP1 char []* (63 B+null=64 B)
//Address 191-1D0: NTP2 char []* (63 B+null=64 B)
//Address 1D1-210: NTP3 char []* (63 B+null=64 B)
//Address 211-250: NTP4 char []* (63 B+null=64 B)
//Address 251-289: TZEnvVar char []* (56 B+null=57 B)
//Address 28A-2A7: TZName char []* (29 B+null=30 B)
//Address 2A8-2B2: User name char []* (10 B+null=11 B)
//Address 2B3-2BD: User passw char []* (10 B+null=11 B)
//Address 2BE: Stores Misc Variable Values (1 B)
//  - Bit 0: siteAllowToUploadSamples - 1=true, 0=false
//  - Bit 1: siteBk1AllowToUploadSamples - 1=true, 0=false
//  - Bit 2: siteBk2AllowToUploadSamples - 1=true, 0=false
//Address 2BF-2FE: MQTT char []* (63 B+null=64 B)
//Address 2FF-309: MQTT User name char []* (10 B+null=11 B)
//Address 30A-314: MQTT User passw char []* (10 B+null=11 B)
//Address 315-3DD: MQTT Topic name char []* (200 B+null=201 B)
//Address 3DE: bootCount - Number of restarts since last upgrade
//Address 3DF: resetCount - Number of non controlled resets since last upgrade
//Address 3E0-416: iBeacon Proximity char []* (36 B+null=37 B)
//Address 417-418: iBeacon Major uint16_t 2 B
//Address 419-41A: iBeacon Minor uint16_t  2 B
//Address 41B: resetPreventiveCount - Number of controlled preventive resets since last upgrade
//Address 41C: resetSwCount - Number of software (restart button) resets since last upgrade
//Address 41D-420: minHeapSinceUpgrade uint32_t  4 B
//Address 421-464: heaterTimeOnYear struct  68 B
//Address 465-4A8: heaterTimeOnPreviousYear struct  68 B
//Address 4A9-4EC: boilerTimeOnYear struct  68 B
//Address 4ED-530: boilerTimeOnPreviousYear struct  68 B
//Address 531: resetPreventiveWebServerCount - Number of controlled preventive web server resets since last upgrade
//Address 532: resetSWWebCount - resets done from the web maintenance page
//Address 533: resetSWMqttCount - resets done from HA (mqqtt)
//Address 534: resetSWUpgradeCount - resets done due to firmware upgrade from maintenance web page
//Address 535: errorsWiFiCnt - Counter for WiFi errors
//Address 536: errorsNTPCnt -  Counter for NTP sync errors
//Address 537: errorsHTTPUptsCnt -  Counter for HTTP Cloud uploads errors
//Address 538: errorsMQTTCnt -  Counter for MQTT errors
//Address 539: SPIFFSErrors -  Counter for SPIFFS errors
//Address 53A: errorsConnectivityCnt -  Counter for Connectivity errors (being WiFi connected)
//Address 53B: errorsWebServerCnt -  Counter for Web Server errors (being WiFi connected but not serving web pages)
//Address 53C: resetWebServerCnt - resets done due to  Web Server errors (being WiFi connected but not serving web pages)
//Address 53D-605: powerMqttTopic - MQTT Power Topic name char []* (200 B+null=201 B)
//Address 606: Stores Config variable flags
//  - Bit 0: powerMeasureEnabled - 1=true, 0=false
//  - Bit 1: powerMeasureSubscribed - 1=true, 0=false
//Address 607-608: powerOnFlameThreshold - Power Threshold to decide if the boiler is burning GAS or not
//Address 609-60C: minMaxHeapBlockSizeSinceUpgrade uint32_t  4 B

uint16_t checkSum(byte *addr, uint32_t count) {
  /******************************************************
   Function checkSum
   Target: Compute Internet Checksum for count bytes beginning at location addr
            https://cse.usf.edu/~kchriste/tools/checksum.c
   Parameters:
    *addr: pointer to the address to start computing the checksum
    count: number of bytes to take for checksum calculation
  Returns:
    checksum
   *****************************************************/

  register uint32_t sum = 0;
  byte auxBuff[count];
  memcpy(auxBuff,addr,count);

  // Main summing loop
  while(count > 1)
  {
    //sum = sum + *((uint16_t *) addr)++;
    sum=sum+auxBuff[count-1];
    count = count - 2;
  }

  // Add left-over byte, if any
  if (count > 0)
    sum = sum + *((byte *) addr);

  // Fold 32-bit sum to 16 bits
  while (sum>>16)
    sum = (sum & 0xFFFF) + (sum >> 16);

  return(~sum);
} //checkSum

void factoryConfReset() {
  /******************************************************
   Function factoryConfReset
   Target: Write default values in EEPROM
   Parameters: None
   Returns: Nothing
  *****************************************************/
  uint8_t configVariables;

  wifiEnabled=WIFI_ENABLED;
  bluetoothEnabled=BLE_ENABLED;
  httpCloudEnabled=UPLOAD_SAMPLES_ENABLED;
  //configSavingEnergyMode=reducedEnergy; //Default value
  webServerEnabled=WEBSERVER_ENABLED;
  mqttServerEnabled=MQTTSERVER_ENABLED;
  secureMqttEnabled=SECURE_MQTT_ENABLED;
  powerMeasureEnabled=MQTT_POWER_MEASURE_ENABLED;
  
  //Now initialize configVariables
  configVariables=0x01; //Bit 0, notFirstRun=true
  //if (reducedEnergy==configSavingEnergyMode) configVariables|=0x02; //Bit 1: configSavingEnergyMode
  if (httpCloudEnabled) configVariables|=0x04; //Bit 2: httpCloudEnabled
  if (bluetoothEnabled) configVariables|=0x08; //Bit 3: bluetoothEnabled
  if (wifiEnabled) configVariables|=0x10; //Bit 4: wifiEnabled
  if (webServerEnabled) configVariables|=0x20; //Bit 5: webServerEnabled
  if (mqttServerEnabled) configVariables|=0x40; //Bit 6: mqttServerEnabled
  if (secureMqttEnabled) configVariables|=0x80; //Bit 7: secureMqttEnabled
  
  //Write variables in EEPROM to be available the next boots up
  EEPROM.write(0x08,configVariables);

  configVariables=0x0; 
  if (powerMeasureEnabled) configVariables|=0x01; //Bit 0, powerMeasureEnabled=false
  
  //Write variables in EEPROM to be available the next boots up
  EEPROM.write(0x606,configVariables);

  
  //Write WiFi Credential-related variables
  char auxSSID[WIFI_MAX_SSID_LENGTH],auxPSSW[WIFI_MAX_PSSW_LENGTH],auxSITE[WIFI_MAX_SITE_LENGTH],
        auxNTP[NTP_SERVER_NAME_MAX_LENGTH],auxTZEnvVar[TZ_ENV_VARIABLE_MAX_LENGTH],auxTZName[TZ_ENV_NAME_MAX_LENGTH];
  //Set variables for SSID or null if no config in global_setup.h file
  memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);
  memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);
  memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);
  #ifdef WIFI_SSID_CREDENTIALS
    String(WIFI_SSID_CREDENTIALS).toCharArray(auxSSID,String(WIFI_SSID_CREDENTIALS).length()+1);
  #endif
  #ifdef WIFI_PW_CREDENTIALS
    String(WIFI_PW_CREDENTIALS).toCharArray(auxPSSW,String(WIFI_PW_CREDENTIALS).length()+1);
  #endif
  #ifdef WIFI_SITE
    String(WIFI_SITE).toCharArray(auxSITE,String(WIFI_SITE).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x0D,auxSSID);wifiCred.wifiSSIDs[0]=auxSSID;
  EEPROM.put(0x2E,auxPSSW);wifiCred.wifiPSSWs[0]=auxPSSW;
  EEPROM.put(0x6E,auxSITE);wifiCred.wifiSITEs[0]=auxSITE;
  
  if (debugModeOn) {printLogln("  [factoryConfReset] - Wrote auxSSID='"+String(auxSSID)+"', auxPSSW='"+String(auxPSSW)+"', auxSITE='"+String(auxSITE)+"'");}
  
  //Set variables for SSID_BK1 or null if no config in global_setup.h file
  memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);
  memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);
  memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);
  #ifdef WIFI_SSID_CREDENTIALS_BK1
    String(WIFI_SSID_CREDENTIALS_BK1).toCharArray(auxSSID,String(WIFI_SSID_CREDENTIALS_BK1).length()+1);
  #endif
  #ifdef WIFI_PW_CREDENTIALS_BK1
    String(WIFI_PW_CREDENTIALS_BK1).toCharArray(auxPSSW,String(WIFI_PW_CREDENTIALS_BK1).length()+1);
  #endif
  #ifdef WIFI_SITE_BK1
    String(WIFI_SITE_BK1).toCharArray(auxSITE,String(WIFI_SITE_BK1).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x79,auxSSID);wifiCred.wifiSSIDs[1]=auxSSID;
  EEPROM.put(0x9A,auxPSSW);wifiCred.wifiPSSWs[1]=auxPSSW;
  EEPROM.put(0xDA,auxSITE);wifiCred.wifiSITEs[1]=auxSITE;
  
  if (debugModeOn) {printLogln("  [factoryConfReset] - Wrote auxSSID='"+String(auxSSID)+"', auxPSSW='"+String(auxPSSW)+"', auxSITE='"+String(auxSITE)+"'");}
  
  //Set variables for SSID_BK2 or null if no config in global_setup.h file
  memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);
  memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);
  memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);
  #ifdef WIFI_SSID_CREDENTIALS_BK2
    String(WIFI_SSID_CREDENTIALS_BK2).toCharArray(auxSSID,String(WIFI_SSID_CREDENTIALS_BK2).length()+1);
  #endif
  #ifdef WIFI_PW_CREDENTIALS_BK2
    String(WIFI_PW_CREDENTIALS_BK2).toCharArray(auxPSSW,String(WIFI_PW_CREDENTIALS_BK2).length()+1);
  #endif
  #ifdef WIFI_SITE_BK2
    String(WIFI_SITE_BK2).toCharArray(auxSITE,String(WIFI_SITE_BK2).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0xE5,auxSSID);wifiCred.wifiSSIDs[2]=auxSSID;
  EEPROM.put(0x106,auxPSSW);wifiCred.wifiPSSWs[2]=auxPSSW;
  EEPROM.put(0x146,auxSITE);wifiCred.wifiSITEs[2]=auxSITE;

  if (debugModeOn) {printLogln("  [factoryConfReset] - Wrote auxSSID='"+String(auxSSID)+"', auxPSSW='"+String(auxPSSW)+"', auxSITE='"+String(auxSITE)+"'");}

  memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
  #ifdef NTP_SERVER
    String(NTP_SERVER).toCharArray(auxNTP,String(NTP_SERVER).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x151,auxNTP);ntpServers[0]=auxNTP;

  if (debugModeOn) {printLogln("  [factoryConfReset] - Wrote auxNTP='"+String(auxNTP)+"'");}

  memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
  #ifdef NTP_SERVER2
    String(NTP_SERVER2).toCharArray(auxNTP,String(NTP_SERVER2).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x191,auxNTP);ntpServers[1]=auxNTP;

  if (debugModeOn) {printLogln("  [factoryConfReset] - Wrote auxNTP='"+String(auxNTP)+"'");}

  memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
  #ifdef NTP_SERVER3
    String(NTP_SERVER3).toCharArray(auxNTP,String(NTP_SERVER3).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x1D1,auxNTP);ntpServers[2]=auxNTP;

  if (debugModeOn) {printLogln("  [factoryConfReset] - Wrote auxNTP='"+String(auxNTP)+"'");}

  memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
  #ifdef NTP_SERVER4
    String(NTP_SERVER4).toCharArray(auxNTP,String(NTP_SERVER4).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x211,auxNTP);ntpServers[3]=auxNTP;

  if (debugModeOn) {printLogln("  [factoryConfReset] - Wrote auxNTP='"+String(auxNTP)+"'");}

  memset(auxTZEnvVar,'\0',TZ_ENV_VARIABLE_MAX_LENGTH);
  memset(auxTZName,'\0',TZ_ENV_NAME_MAX_LENGTH);
  #ifdef NTP_TZ_ENV_VARIABLE
    String(NTP_TZ_ENV_VARIABLE).toCharArray(auxTZEnvVar,String(NTP_TZ_ENV_VARIABLE).length()+1);
    String(NTP_TZ_NAME).toCharArray(auxTZName,String(NTP_TZ_NAME).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x251,auxTZEnvVar);TZEnvVariable=String(auxTZEnvVar);
  EEPROM.put(0x28A,auxTZName);TZName=String(auxTZName);

  if (debugModeOn) {printLogln("  [factoryConfReset] - Wrote auxTZEnvVar='"+String(auxTZEnvVar)+"', auxTZName='"+String(auxTZName)+"'");}

  //Write Web User Credential-related variables
  char auxUserName[WEB_USER_CREDENTIAL_LENGTH],auxUserPssw[WEB_PW_CREDENTIAL_LENGTH];
  //Set variables for Web User Credential or null if no config in global_setup.h file
  memset(auxUserName,'\0',WEB_USER_CREDENTIAL_LENGTH);
  memset(auxUserPssw,'\0',WEB_PW_CREDENTIAL_LENGTH);
  #ifdef WEB_USER_CREDENTIAL
    String(WEB_USER_CREDENTIAL).toCharArray(auxUserName,String(WEB_USER_CREDENTIAL).length()+1);
  #endif
  #ifdef WEB_PW_CREDENTIAL
    String(WEB_PW_CREDENTIAL).toCharArray(auxUserPssw,String(WEB_PW_CREDENTIAL).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x2A8,auxUserName);userName=auxUserName;
  EEPROM.put(0x2B3,auxUserPssw);userPssw=auxUserPssw;
  
  if (debugModeOn) {printLogln("  [factoryConfReset] - Wrote auxUserName='"+String(auxUserName)+"', auxUserPssw='"+String(auxUserPssw)+"'");}
  
  //Now initialize wifiCred.SiteAllow variables
  configVariables=0x0; //Bit 0, notFirstRun=true
  EEPROM.write(0x2BE,configVariables); //All unset the first time EEPROM is written
  wifiCred.SiteAllow[0]=0;wifiCred.SiteAllow[1]=0;wifiCred.SiteAllow[2]=0;

  //Write MQTT server variable
  //Set variables for MQTT Server or null if no config in global_setup.h file
  char auxMQTT[MQTT_SERVER_NAME_MAX_LENGTH];
  memset(auxMQTT,'\0',MQTT_SERVER_NAME_MAX_LENGTH);
  #ifdef MQTT_SERVER
    String(MQTT_SERVER).toCharArray(auxMQTT,String(MQTT_SERVER).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x2BF,auxMQTT);mqttServer=auxMQTT;
  
  //Write MQTT User Credential-related variables
  //Set variables for MQTT User Credentials or null if no config in global_setup.h file
  memset(auxUserName,'\0',MQTT_USER_CREDENTIAL_LENGTH);
  memset(auxUserPssw,'\0',MQTT_PW_CREDENTIAL_LENGTH);
  #ifdef MQTT_USER_CREDENTIAL
    String(MQTT_USER_CREDENTIAL).toCharArray(auxUserName,String(MQTT_USER_CREDENTIAL).length()+1);
  #endif
  #ifdef MQTT_PW_CREDENTIAL
    String(MQTT_PW_CREDENTIAL).toCharArray(auxUserPssw,String(MQTT_PW_CREDENTIAL).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x2FF,auxUserName);mqttUserName=auxUserName;
  EEPROM.put(0x30A,auxUserPssw);mqttUserPssw=auxUserPssw;

  //Write MQTT Topic Name variable
  //Set variables for MQTT Topic or null if no config in global_setup.h file
  char auxMqttTopicPrefix[MQTT_TOPIC_NAME_MAX_LENGTH];
  memset(auxMqttTopicPrefix,'\0',MQTT_TOPIC_NAME_MAX_LENGTH);
  uint8_t auxLength;
  #ifdef MQTT_TOPIC_PREFIX
    mqttTopicPrefix=MQTT_TOPIC_PREFIX;
    if (mqttTopicPrefix.charAt(mqttTopicPrefix.length()-1)!='/') mqttTopicPrefix+="/"; //Adding slash at the end if needed
    auxLength=mqttTopicPrefix.length()+1;
    if (auxLength>MQTT_TOPIC_NAME_MAX_LENGTH-1) { //Substring if greater that max length
      auxLength=MQTT_TOPIC_NAME_MAX_LENGTH-1;
      mqttTopicPrefix=mqttTopicPrefix.substring(0,auxLength);
    }
    mqttTopicPrefix.toCharArray(auxMqttTopicPrefix,mqttTopicPrefix.length()+1);
  #endif
  mqttTopicName=mqttTopicPrefix+device; //Adding the device name to the MQTT Topic name
   //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x315,auxMqttTopicPrefix);

  if (debugModeOn) {printLogln("  [factoryConfReset] - Wrote mqttServer='"+String(auxMQTT)+"', mqttTopicPrefix='"+String(auxMqttTopicPrefix)+"', mqttUserName='"+String(auxUserName)+"', mqttUserPssw='"+String(auxUserPssw)+"'");}

  //Write MQTT Power Topic Name variable
  //Set variables for MQTT Topic or null if no config in global_setup.h file
  memset(auxMqttTopicPrefix,'\0',MQTT_TOPIC_NAME_MAX_LENGTH);
  #ifdef MQTT_POWER_TOPIC
    powerMqttTopic=MQTT_POWER_TOPIC;
    auxLength=powerMqttTopic.length()+1;
    if (auxLength>MQTT_TOPIC_NAME_MAX_LENGTH-1) { //Substring if greater that max length
      auxLength=MQTT_TOPIC_NAME_MAX_LENGTH-1;
      powerMqttTopic=powerMqttTopic.substring(0,auxLength);
    }
    powerMqttTopic.toCharArray(auxMqttTopicPrefix,powerMqttTopic.length()+1);
  #else
    powerMqttTopic=auxMqttTopicPrefix;
  #endif
   //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x53D,auxMqttTopicPrefix);

  //Set iBeacon Proximity
  char auxBLEProximityUUID[BLE_BEACON_UUID_LENGH];
  memset(auxBLEProximityUUID,'\0',BLE_BEACON_UUID_LENGH);
  #ifdef BLE_BEACON_UUID
    String(BLE_BEACON_UUID).toCharArray(auxBLEProximityUUID,String(BLE_BEACON_UUID).length()+1);
  #endif
  //Write varialbes in EEPROM to be available the next boots up
  EEPROM.put(0x3E0,auxBLEProximityUUID);

  if (debugModeOn) {printLogln("  [factoryConfReset] - Wrote BLEProximityUUID='"+String(auxBLEProximityUUID)+"'");}

  //Set iBeacon Major and Minor
  byte mac[6];WiFi.macAddress(mac);
  uint16_t aux=(mac[2]<<8)|mac[3];
  EEPROM.writeUShort(0x417,aux);
  if (debugModeOn) {printLog("  [factoryConfReset] - Wrote BLE Major=0x"+String(aux,HEX));}
  aux=(mac[4]<<8)|mac[5];
  EEPROM.writeUShort(0x419,aux);

  if (debugModeOn) {printLogln(" and BLE Minor=0x"+String(aux,HEX));}
  
  //Set minHeap and minMaxHeapBlockSizeSinceUpgrade
  EEPROM.writeInt(0x41D,minHeapSinceUpgrade);
  EEPROM.writeInt(0x609,minMaxHeapBlockSizeSinceUpgrade);
  if (debugModeOn) {printLogln("  [factoryConfReset] - Wrote Min Heap Seen=0x"+String(minHeapSinceUpgrade,HEX)+", Min Max Heap Block Seen=0x"+String(minMaxHeapBlockSizeSinceUpgrade));}
  
  //Initialize bootCount variable
  bootCount=1;
  EEPROM.write(0x3DE,bootCount); 

  //Initialize reset and error counters
  resetCount=0; //uncontrolled resets
  EEPROM.write(0x3DF,resetCount); 
  resetPreventiveCount=0; //preventive resets (mainly becuase low heap situation)
  EEPROM.write(0x41B,resetPreventiveCount);
  resetSWCount=0; //reset from the HA restart button
  EEPROM.write(0x41C,resetSWCount);
  resetPreventiveWebServerCount=0;
  EEPROM.write(0x531,resetPreventiveWebServerCount); //preventive web server resets (mainly becuase low heap situation)
  resetSWWebCount=0;
  EEPROM.write(0x532,resetSWWebCount); //resets done from the web maintenance page
  resetSWMqttCount=0;
  EEPROM.write(0x533,resetSWMqttCount); //resets done from the HA (mqqtt) page
  resetSWUpgradeCount=0;
  EEPROM.write(0x534,resetSWUpgradeCount); //resets done due to firmware upgrade from maintenance web page
  resetWebServerCnt=0;
  EEPROM.write(0x53C,resetWebServerCnt); //resets done due to Web Server errors (being WiFi connected but not serving web pages)
  errorsWiFiCnt=0;
  EEPROM.write(0x535,errorsWiFiCnt); //Counter for WiFi errors
  errorsNTPCnt=0;
  EEPROM.write(0x536,errorsNTPCnt); // Counter for NTP sync errors
  errorsHTTPUptsCnt=0;
  EEPROM.write(0x537,errorsHTTPUptsCnt); // Counter for HTTP Cloud uploads errors
  errorsMQTTCnt=0;
  EEPROM.write(0x538,errorsMQTTCnt); //Counter for MQTT errors
  SPIFFSErrors=0;
  EEPROM.write(0x539,SPIFFSErrors); //Counter for SPIFFS errors
  errorsConnectivityCnt=0;
  EEPROM.write(0x53A,errorsConnectivityCnt); //Counter for Connectivity errors (being WiFi connected)
  errorsWebServerCnt=0;
  EEPROM.write(0x53B,errorsWebServerCnt); //Counter for Web Server errors (being WiFi connected but not serving web pages)

  //Set Power Threshold 
  powerOnFlameThreshold=BOILER_FLAME_ON_POWER_THRESHOLD;
  EEPROM.writeUShort(0x607,powerOnFlameThreshold); //Power Threshold to decide whether the boiler is burning gas or not (flame)
} //factoryConfReset