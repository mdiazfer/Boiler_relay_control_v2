/* Dealing with variables initialization

*/

#include "setup.h"

void printLogln(String logMessage, unsigned char base) {
  /******************************************************
   Function printLog
   Target: Prints logs in both serial port and web socket (if avialbable). It prints out a
      new line character at the end of the message
   Parameters:
    String logMessage: Message to print out
    unsigned char: base in case it's a number
   Return: Nothing
  *****************************************************/
  
  if (base==HEX) {
    String aux=String(logMessage.toInt(),HEX);
    if (serialLogsOn) boardSerialPort.println(aux);
    if ((wifiCurrentStatus!=wifiOffStatus || WiFi.status()==WL_CONNECTED) && webServerEnabled && !(error_setup & ERROR_WEB_SERVER) &&
         !(error_setup & ERROR_WEB_SOCKET) && webLogsOn) notifyClients(aux+"\n");
    if (millis()<=BOOT_LOGS_TIME) bootLogs+=aux+"\n";
  }
  else {
    if (serialLogsOn) boardSerialPort.println(logMessage);
    if ((wifiCurrentStatus!=wifiOffStatus || WiFi.status()==WL_CONNECTED) && webServerEnabled && !(error_setup & ERROR_WEB_SERVER) &&
         !(error_setup & ERROR_WEB_SOCKET) && webLogsOn) notifyClients(logMessage+"\n");
    if (millis()<=BOOT_LOGS_TIME) bootLogs+=logMessage+"\n";
  }
  
  if (millis()>BOOT_LOGS_TIME && !logTagged) {logTagged=true;bootLogs+="[................................]\n";}
}

void printLog(String logMessage, unsigned char base) {
  /******************************************************
   Function printLog
   Target: Prints logs in both serial port and web socket (if avialbable)
   Parameters:
    String logMessage: Message to print out
    unsigned char: base in case it's a number
   Return: Nothing
  *****************************************************/
  if (base==HEX) {
    String aux=String(logMessage.toInt(),HEX);
    if (serialLogsOn) boardSerialPort.print(aux);
    if ((wifiCurrentStatus!=wifiOffStatus || WiFi.status()==WL_CONNECTED) && webServerEnabled && !(error_setup & ERROR_WEB_SERVER) &&
         !(error_setup & ERROR_WEB_SOCKET) && webLogsOn) notifyClients(aux);
    if (millis()<=BOOT_LOGS_TIME) bootLogs+=aux;
  }
  else {
    if (serialLogsOn) boardSerialPort.print(logMessage);
    if ((wifiCurrentStatus!=wifiOffStatus || WiFi.status()==WL_CONNECTED) && webServerEnabled && !(error_setup & ERROR_WEB_SERVER) &&
         !(error_setup & ERROR_WEB_SOCKET) && webLogsOn) notifyClients(logMessage);
    if (millis()<=BOOT_LOGS_TIME) bootLogs+=logMessage;
  }

  if (millis()>BOOT_LOGS_TIME && !logTagged) {logTagged=true;bootLogs+="[................................]\n";}
}

void printLogln(uint8_t logMessage, unsigned char base) {
  /******************************************************
   Function printLog
   Target: Prints logs in both serial port and web socket (if avialbable). It prints out a
      new line character at the end of the message
   Parameters:
    uint8_t logMessage: Message to print out
    unsigned char: base in case it's a number
   Return: Nothing
  *****************************************************/
  if (base==HEX) {
    String aux=String(logMessage,HEX);
    if (serialLogsOn) boardSerialPort.println(aux);
    if ((wifiCurrentStatus!=wifiOffStatus || WiFi.status()==WL_CONNECTED) && webServerEnabled && !(error_setup & ERROR_WEB_SERVER) &&
         !(error_setup & ERROR_WEB_SOCKET) && webLogsOn) notifyClients(aux+"\n");
    if (millis()<=BOOT_LOGS_TIME) bootLogs+=aux+"\n";
  }
  else {
    if (serialLogsOn) boardSerialPort.println(String(logMessage));
    if ((wifiCurrentStatus!=wifiOffStatus || WiFi.status()==WL_CONNECTED) && webServerEnabled && !(error_setup & ERROR_WEB_SERVER) &&
         !(error_setup & ERROR_WEB_SOCKET) && webLogsOn) notifyClients(String(logMessage)+"\n");
    if (millis()<=BOOT_LOGS_TIME) bootLogs+=logMessage+"\n";
  }

  if (millis()>BOOT_LOGS_TIME && !logTagged) {logTagged=true;bootLogs+="[................................]\n";}
}

void printLog(uint8_t logMessage, unsigned char base) {
  /******************************************************
   Function printLog
   Target: Prints logs in both serial port and web socket (if avialbable)
   Parameters:
    uint8_t logMessage: Message to print out
    unsigned char: base in case it's a number
   Return: Nothing
  *****************************************************/
  if (base==HEX) {
    String aux=String(logMessage,HEX);
    if (serialLogsOn) boardSerialPort.print(aux);
    if ((wifiCurrentStatus!=wifiOffStatus || WiFi.status()==WL_CONNECTED) && webServerEnabled && !(error_setup & ERROR_WEB_SERVER) &&
         !(error_setup & ERROR_WEB_SOCKET) && webLogsOn) notifyClients(aux);
    if (millis()<=BOOT_LOGS_TIME) bootLogs+=aux;
  }
  else {
    if (serialLogsOn) boardSerialPort.print(String(logMessage));
    if ((wifiCurrentStatus!=wifiOffStatus || WiFi.status()==WL_CONNECTED) && webServerEnabled && !(error_setup & ERROR_WEB_SERVER) &&
         !(error_setup & ERROR_WEB_SOCKET) && webLogsOn) notifyClients(String(logMessage));
    if (millis()<=BOOT_LOGS_TIME) bootLogs+=logMessage;
  }

  if (millis()>BOOT_LOGS_TIME && !logTagged) {logTagged=true;bootLogs+="[................................]\n";}
}

void printLogln(tm * timeinfo, const char *format) {
  /******************************************************
   Function printLogln
   Target: Prints logs in both serial port and web socket (if avialbable). It prints out a
      new line character at the end of the message
   Parameters:
    tm * timeinfo: Time structucture to print out
    const char format: Format of the text
   Return: Nothing
  *****************************************************/
  char s[100];
  strftime(s,sizeof(s),format,timeinfo);
  if (serialLogsOn) boardSerialPort.println(String(s));
  if ((wifiCurrentStatus!=wifiOffStatus || WiFi.status()==WL_CONNECTED) && webServerEnabled && !(error_setup & ERROR_WEB_SERVER) &&
       !(error_setup & ERROR_WEB_SOCKET) && webLogsOn) notifyClients(String(s)+"\n");
  if (millis()<=BOOT_LOGS_TIME) bootLogs+=String(s)+"\n";

  if (millis()>BOOT_LOGS_TIME && !logTagged) {logTagged=true;bootLogs+="[................................]\n";}
}

void printLog(tm * timeinfo, const char *format) {
  /******************************************************
   Function printLog
   Target: Prints logs in both serial port and web socket (if avialbable).
   Parameters:
    tm * timeinfo: Time structucture to print out
    const char format: Format of the text
   Return: Nothing
  *****************************************************/
  char s[100];
  strftime(s,sizeof(s),format,timeinfo);
  if (serialLogsOn) boardSerialPort.print(String(s));
  if ((wifiCurrentStatus!=wifiOffStatus || WiFi.status()==WL_CONNECTED) && webServerEnabled && !(error_setup & ERROR_WEB_SERVER) &&
       !(error_setup & ERROR_WEB_SOCKET) && webLogsOn) notifyClients(String(s));
  if (millis()<=BOOT_LOGS_TIME) bootLogs+=String(s);

  if (millis()>BOOT_LOGS_TIME && !logTagged) {logTagged=true;bootLogs+="[................................]\n";}
}

void detachNetwork(void) {
  /******************************************************
   Function detachNetwork
   Target: Close WiFi connection and reinit modules to release memory in the hope to increase heap size
   Parameters: None
   Return: Nothing
  *****************************************************/
  
  
  
  /*webSocket.closeAll();
  webSocket.cleanupClients();
  webServer.removeHandler(&webSocket);
  webServer.reset();
  //webServer.end();
  */
  SPIFFS.end();
  mqttClient.disconnect(true);
  WiFi.disconnect(true,false);
  //WiFi.disconnect();
  wifiCurrentStatus=wifiOffStatus;
  CloudSyncCurrentStatus=CloudSyncOffStatus;
  CloudClockCurrentStatus=CloudClockOffStatus;
  MqttSyncCurrentStatus=MqttSyncOffStatus;
  forceWifiReconnect=true; //Force to reconnect WiFi in the next loop cycle
  forceWebServerInit=true; //Force to reinit the webServer
  //CloudSyncCurrentStatus (External HTTP server) and  CloudClockCurrentStatus (NTP Server) will be init by their own
  delay(WEBSERVER_SEND_DELAY);
  //WiFi.reconnect();
  
  if (debugModeOn) printLogln(String(millis())+" - [detachNetwork] - WiFi related modules stoped. They will be initiated in the next loop cycle");
}

String roundFloattoString(float_t number, uint8_t decimals) {
  //Round float to "decimals" decimals in String format
  String myString;  

  int ent,dec,auxEnt,auxDec,aux1,aux2;

  if (decimals==1) {
    //Better precision operating without pow()
    aux1=number*100;
    ent=aux1/100;
    aux2=ent*100;
    dec=aux1-aux2; if (dec<0) dec=-dec;
  }
  else 
    if (decimals==2) {
      //Better precision operating without pow()
      aux1=number*1000;
      ent=aux1/1000;
      aux2=ent*1000;
      dec=aux1-aux2; if (dec<0) dec=-dec;
    }
    else {
      ent=int(number);
      dec=abs(number*pow(10,decimals+1)-ent*pow(10,decimals+1));
    }
  auxEnt=int(float(dec/10));
  if (auxEnt>=10) auxEnt=9; //Need adjustment for wrong rounds in xx.98 or xx.99
  auxDec=abs(auxEnt*10-dec);
  if (auxDec>=5) auxEnt++;
  if (auxEnt>=10) {auxEnt=0; ent++;}

  if (decimals==0) myString=String(number).toInt(); 
  else myString=String(ent)+"."+String(auxEnt);

  return myString;
}

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

size_t getAppOTAPartitionSize(uint8_t type, uint8_t subtype) {
  /******************************************************
   Function getAppOTAPartitionSize
   Target: Getting the partition size available for OTA. Should be the maximum binary file size to upload via OTA.
   Parameters:
    type: partition type
    subtype: partition subtype
  *****************************************************/
  
  esp_partition_iterator_t iter;
  const esp_partition_t *partition=nullptr;

  switch (type) {
    case ESP_PARTITION_TYPE_APP:
    { const esp_partition_t* nonRunningPartition=nullptr;
      const esp_partition_t* runningPartition=esp_ota_get_running_partition();
      OTAUpgradeBinAllowed=false; //v1.2.0 To block OTA upgrade if there is only one partition
      if (runningPartition==nullptr) {
        //Something wetn wrong
        if (debugModeOn) {printLogln("  [getAppOTAPartitionSize] - Couldn't get the running partition");}
        return 0;
      }
      
      if (debugModeOn) {printLogln("  [getAppOTAPartitionSize] - Got the running partition");}
      
      //Getting all the APP-type partitions
      iter = esp_partition_find((esp_partition_type_t) type, (esp_partition_subtype_t) subtype, NULL);
      if (debugModeOn) {printLogln("  [getAppOTAPartitionSize] - Name, type, subtype, offset, length");}
      uint8_t appPartitionNumber=0;
      while (iter != nullptr)
      {
        partition = esp_partition_get(iter);
        //if (debugModeOn) {printLogln("  [getAppOTAPartitionSize] - "+String(partition->label)+" app "+String(partition->subtype)+" 0x"+String(partition->address,HEX)+" 0x"+String(partition->size,HEX)+" ("+String(partition->size)+" B)");}
        iter = esp_partition_next(iter);
        appPartitionNumber++;
      }
      esp_partition_iterator_release(iter);

      if (appPartitionNumber!=2) {
        //Wrong number of app partitions by design of this firmware
        if (debugModeOn) {printLogln("  [getAppOTAPartitionSize] - Wrong number of APP partitions. It should be 2 rather than "+String(appPartitionNumber));}
        //OTA upgrade is not allowed
        return 0;
      }

      //Getting the non-running APP-type partition
      iter = esp_partition_find((esp_partition_type_t) type, (esp_partition_subtype_t) subtype, NULL);
      while (iter != nullptr)
      {
        partition = esp_partition_get(iter);
        if (runningPartition!=partition) {
          nonRunningPartition=partition;
          if (debugModeOn) {printLogln("  [getAppOTAPartitionSize] - "+String(partition->label)+" app "+String(partition->subtype)+" 0x"+String(partition->address,HEX)+" 0x"+String(partition->size,HEX)+" ("+String(partition->size)+" B) is the non-running partition");}
          break;
        }
        else {
          if (debugModeOn) {printLogln("  [getAppOTAPartitionSize] - "+String(partition->label)+" app "+String(partition->subtype)+" 0x"+String(partition->address,HEX)+" 0x"+String(partition->size,HEX)+" ("+String(partition->size)+" B) is the running partition");}
        }
        iter = esp_partition_next(iter);
      }
      esp_partition_iterator_release(iter);

      if (nonRunningPartition==nullptr) {
        //Something wetn wrong getting the non-running partion
        if (debugModeOn) {printLogln("  [getAppOTAPartitionSize] - Couldn't get the non-running partition");}
        return 0;
      }

      if (debugModeOn) {printLogln("  [getAppOTAPartitionSize] - Got the non-running partition and size "+String(nonRunningPartition->size)+" B");}
      OTAUpgradeBinAllowed=true;  //v1.2.0 To block OTA upgrade if there is only one partition
      return nonRunningPartition->size;
    }  
    break;
    case ESP_PARTITION_TYPE_DATA:
    { SPIFFSUpgradeBinAllowed=false;  //v1.2.0 To block SPIFFS upgrade if there is something wrong with SPIFFS partition
      if (subtype!=0x82) return 0; //Not SPIFFS partition

      iter = esp_partition_find((esp_partition_type_t) type, (esp_partition_subtype_t) subtype, NULL);
      if (debugModeOn) {printLogln("  [getAppOTAPartitionSize] - Name, type, subtype, offset, length");}
      while (iter != nullptr) //Assuming there is only one SPIFFS partition (Getting the first one)
      {
        partition = esp_partition_get(iter);
        if (debugModeOn) {printLogln("  [getAppOTAPartitionSize] - "+String(partition->label)+" app "+String(partition->subtype)+" 0x"+String(partition->address,HEX)+" 0x"+String(partition->size,HEX)+" ("+String(partition->size)+" B) is the SPIFFS partition");}
        break;  
        iter = esp_partition_next(iter);
      }
      esp_partition_iterator_release(iter);

      if (partition==nullptr) {
        //Something wetn wrong getting the SPIFFS partion
        if (debugModeOn) {printLogln("  [getAppOTAPartitionSize] - Couldn't get the SPIFFS partition");}
        return 0;
      }

      if (debugModeOn) {printLogln("  [getAppOTAPartitionSize] - Got the SPIFFS partition and size "+String(partition->size)+" B");}
      SPIFFSUpgradeBinAllowed=true; //v1.2.0 To block SPIFFS upgrade if there is something wrong with SPIFFS partition
      return partition->size;
    }
    break;
    default:
      //It's supposed never get here
      return 0;
    break;
  }
  
  //Interesting code to get the DATA partition.
  /*iter = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL);
  while (iter != nullptr)
  {
    const esp_partition_t *partition = esp_partition_get(iter);
    if (debugModeOn) {printLogln("  [getAppOTAPartitionSize] - "+String(partition->label)+" data "+String(partition->subtype)+" 0x"+String(partition->address,HEX)+" 0x"+String(partition->size,HEX)+" ("+String(partition->size)+" B)");}
    iter = esp_partition_next(iter);
  }
  esp_partition_iterator_release(iter);
  if (debugModeOn) {printLogln("  [getAppOTAPartitionSize] - Info on running partition");}
  if (debugModeOn) {printLogln("  [getAppOTAPartitionSize] - "+String(partition->label)+" data "+String(partition->subtype)+" 0x"+String(partition->address,HEX)+" 0x"+String(partition->size,HEX)+" ("+String(partition->size)+" B)");}
  if (debugModeOn) {printLogln("  [getAppOTAPartitionSize] - Ending partition conde");}
  */
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
   Function initTZVariables
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

  //Get the MQTT Topic Name for Power Measurement
  memset(auxMqttTopicPrefix,'\0',MQTT_TOPIC_NAME_MAX_LENGTH);EEPROM.get(0x53D,auxMqttTopicPrefix);
  if (String(auxMqttTopicPrefix).compareTo("")==0) {
    #ifdef MQTT_POWER_TOPIC
      powerMqttTopic=MQTT_POWER_TOPIC;
      //Check if MQTT topic name must be updated in EEPROM
      if (powerMqttTopic.compareTo(String(auxMqttTopicPrefix))!=0) {
        uint8_t auxLength=powerMqttTopic.length()+1;
        if (auxLength>MQTT_TOPIC_NAME_MAX_LENGTH-1) { //Substring if greater that max length
          auxLength=MQTT_TOPIC_NAME_MAX_LENGTH-1;
          powerMqttTopic=powerMqttTopic.substring(0,auxLength);
        }
        memset(auxMqttTopicPrefix,'\0',MQTT_TOPIC_NAME_MAX_LENGTH);
        memcpy(auxMqttTopicPrefix,powerMqttTopic.c_str(),auxLength);
        EEPROM.put(0x53D,auxMqttTopicPrefix);
        updateEEPROM=true;
      }
    #else
      powerMqttTopic=auxMqttTopicPrefix;
    #endif
  } else powerMqttTopic=auxMqttTopicPrefix;

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
  char readFirmwareVersion[VERSION_CHAR_LENGTH+1];

  //EEPROM init first
  EEPROM.begin(EEPROM_SIZE); // Initialize EEPROM with predefined size. Config variables ares stored there

  memset(activeCookie,'\0',COOKIE_SIZE); //init variable
  memset(currentSetCookie,'\0',COOKIE_SIZE); //init variable

  //Set existing version from global_setup.h
  memset(firmwareVersion,'\0',VERSION_CHAR_LENGTH+1);
  sprintf(firmwareVersion,VERSION);

  //Check if it is the first run after the very first firmware upload
  memset(readFirmwareVersion,'\0',VERSION_CHAR_LENGTH+1);EEPROM.get(0x00,readFirmwareVersion);
  uint16_t readChecksum,computedChecksum;
  readChecksum=EEPROM.read(7);readChecksum=readChecksum<<8;readChecksum|=EEPROM.read(6);
  computedChecksum=checkSum((byte*)firmwareVersion,VERSION_CHAR_LENGTH);
  
  if (readChecksum!=computedChecksum) {
    //It's the first run after the very first firmware upload
    //Variable inizialization to values configured in global_setup

    if (debugModeOn) {printLogln("  [EEPROMInit] - Writting EEPROM needed as the version checksums differ: readFirmwareVersion="+String(readFirmwareVersion)+", readChecksums="+String(readChecksum)+", firmwareVersion="+String(firmwareVersion)+", computedChecksum="+String(computedChecksum));}

    //Save version and checksum
    for (int i=0; i<VERSION_CHAR_LENGTH; i++) EEPROM.write(i,firmwareVersion[i]);EEPROM.write(VERSION_CHAR_LENGTH,'\0');
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

    //Get the rest of variables from EEPROM
    configVariables=EEPROM.read(0x606);
    powerMeasureEnabled=configVariables & 0x01; //Bit 0, powerMeasureEnabled

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

    
    //Get the WEB and MQTT User Credential-related variables from EEPROM
    char auxUserName[WEB_USER_CREDENTIAL_LENGTH],auxUserPssw[WEB_PW_CREDENTIAL_LENGTH];
    memset(auxUserName,'\0',WEB_USER_CREDENTIAL_LENGTH);EEPROM.get(0x2A8,auxUserName);userName=String(auxUserName);
    memset(auxUserPssw,'\0',WEB_PW_CREDENTIAL_LENGTH);EEPROM.get(0x2B3,auxUserPssw);userPssw=String(auxUserPssw);
    memset(auxUserName,'\0',WEB_USER_CREDENTIAL_LENGTH);EEPROM.get(0x2FF,auxUserName);mqttUserName=String(auxUserName);
    memset(auxUserPssw,'\0',WEB_PW_CREDENTIAL_LENGTH);EEPROM.get(0x30A,auxUserPssw);mqttUserPssw=String(auxUserPssw);

    //Set minHeapSinceUpgrade
    minHeapSinceUpgrade=EEPROM.readInt(0x41D);
    minMaxHeapBlockSizeSinceUpgrade=EEPROM.readInt(0x609);

    //Set the bootCount from EEPROM
    bootCount=bootCount==255?EEPROM.read(0x3DE)+1:1; //bootCount = 255 if VERSION = version in EEPROM. Otherwhise bootCount=0 (need to update value in EEPROM)
    EEPROM.write(0x3DE,bootCount);

    //Set the counters for resets and errors from EEPROM
    resetCount=EEPROM.read(0x3DF); //uncontrolled resets
    resetPreventiveCount=EEPROM.read(0x41B);
    resetPreventiveWebServerCount=EEPROM.read(0x531);
    resetSWCount=EEPROM.read(0x41C); //all ESP.restart() resets
    resetSWWebCount=EEPROM.read(0x532); //resets done from the web maintenance page
    resetSWMqttCount=EEPROM.read(0x533); //resets done from the HA (mqqtt) page
    resetSWUpgradeCount=EEPROM.read(0x534); //resets done due to firmware upgrade from maintenance web page
    errorsWiFiCnt=EEPROM.read(0x535); //Counter for WiFi errors
    errorsNTPCnt=EEPROM.read(0x536); // Counter for NTP sync errors
    errorsHTTPUptsCnt=EEPROM.read(0x537); // Counter for HTTP Cloud uploads errors
    errorsMQTTCnt=EEPROM.read(0x538); //Counter for MQTT errors
    SPIFFSErrors=EEPROM.read(0x539); //Counter for SPIFFS errors
    errorsConnectivityCnt=EEPROM.read(0x53A); //Counter for Connectivity errors (being WiFi connected)
    errorsWebServerCnt=EEPROM.read(0x53B); //Counter for Web Server errors (being WiFi connected but not serving web pages)
    resetWebServerCnt=EEPROM.read(0x53C); //Counter for Web Server resets (being WiFi connected but not serving web pages)

    esp_reset_reason_t resetReason=esp_reset_reason();
    /*
    0  ESP_RST_UNKNOWN,    //!< Reset reason can not be determined
    1  ESP_RST_POWERON,    //!< Reset due to power-on event
    2  ESP_RST_EXT,        //!< Reset by external pin (not applicable for ESP32)
    3  ESP_RST_SW,         //!< Software reset via esp_restart
    4  ESP_RST_PANIC,      //!< Software reset due to exception/panic
    5  ESP_RST_INT_WDT,    //!< Reset (software or hardware) due to interrupt watchdog
    6  ESP_RST_TASK_WDT,   //!< Reset due to task watchdog
    7  ESP_RST_WDT,        //!< Reset due to other watchdogs
    8  ESP_RST_DEEPSLEEP,  //!< Reset after exiting deep sleep mode
    9  ESP_RST_BROWNOUT,   //!< Brownout reset (software or hardware)
    10 ESP_RST_SDIO,       //!< Reset over SDIO
    */
    switch (resetReason) { //v1.2.0 - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/misc_system_api.html#_CPPv418esp_reset_reason_t
      case ESP_RST_UNKNOWN: //Reset reason can not be determined
        //In all these cases, there was a wrong code that triggered the reset. Count it
        resetCount++;EEPROM.write(0x3DF,resetCount);updateEEPROM=true;
        printLogln("  [EEPROMInit] - Reset reason=ESP_RST_UNKNOWN (0). Resetting resetCount");break;
      case ESP_RST_POWERON: //Power-on event
        printLogln("  [EEPROMInit] - Reset reason=ESP_RST_POWERON (1)");break;
      case ESP_RST_EXT: //External pin (not applicable for ESP32)
        printLogln("  [EEPROMInit] - Reset reason=ESP_RST_EXT (2)");break;
      case ESP_RST_SW: //Software reset via esp_restart()
        resetSWCount++; //Total number of ESP.restarts, including preventive resets, firmware upgrade, resets from web and mqtt
        EEPROM.write(0x41C,resetSWCount);updateEEPROM=true;
        printLogln("  [EEPROMInit] - Reset reason=ESP_RST_SW (3)");break;
      case ESP_RST_PANIC: //Software reset due to exception/panic
        //In all these cases, there was a wrong code that triggered the reset. Count it
        resetCount++;EEPROM.write(0x3DF,resetCount);updateEEPROM=true;
        printLogln("  [EEPROMInit] - Reset reason=ESP_RST_PANIC (4). Resetting resetCount");break;
      case ESP_RST_INT_WDT: //Reset (software or hardware) due to interrupt watchdog.
        resetCount++;EEPROM.write(0x3DF,resetCount);updateEEPROM=true;
        printLogln("  [EEPROMInit] - Reset reason=ESP_RST_INT_WDT (5)");break;
      case ESP_RST_TASK_WDT: //Reset due to task watchdog
        //In all these cases, there was a wrong code that triggered the reset. Count it
        resetCount++;EEPROM.write(0x3DF,resetCount);updateEEPROM=true;
        printLogln("  [EEPROMInit] - Reset reason=ESP_RST_TASK_WDT (6). Resetting resetCount");break;
      case ESP_RST_WDT: //Reset due to other watchdogs
        //In all these cases, there was a wrong code that triggered the reset. Count it
        resetCount++;EEPROM.write(0x3DF,resetCount);updateEEPROM=true;
        printLogln("  [EEPROMInit] - Reset reason=ESP_RST_WDT (7). Resetting resetCount");break;
      case ESP_RST_DEEPSLEEP: //Reset after exiting deep sleep mode
        printLogln("  [EEPROMInit] - Reset reason=ESP_RST_DEEPSLEEP (8)");break;
      case ESP_RST_BROWNOUT: //Brownout reset (software or hardware) - Supply voltage goes below safe level
        printLogln("  [EEPROMInit] - Reset reason=ESP_RST_BROWNOUT (9)");break;
      case ESP_RST_SDIO: //Reset over SDIO
        printLogln("  [EEPROMInit] - Reset reason=ESP_RST_SDIO (10)");break;
      default:
        printLogln("  [EEPROMInit] - Reset reason=default");break;
    }
    samples["resetReason"] = String(resetReason);

    //Update time on counters
    EEPROM.get(0x421,heaterTimeOnYear);EEPROM.get(0x465,heaterTimeOnPreviousYear);
    EEPROM.get(0x4A9,boilerTimeOnYear);EEPROM.get(0x4ED,boilerTimeOnPreviousYear);

    //Update Power Threshold 
    EEPROM.get(0x607,powerOnFlameThreshold); //Power Threshold to decide whether the boiler is burning gas or not (flame)
  }

  if (updateEEPROM) 
    {if (debugModeOn) {printLogln("  [EEPROMInit] - No version change, but update EEPROM needed with values taken from global_setup.h");}}
  else 
    {if (debugModeOn) {printLogln("  [EEPROMInit] - No version change, but update EEPROM needed with counter variables");}}
  EEPROM.commit();
} //EEPROMInit

void variablesInit() {
  /******************************************************
   Function variablesInit
   Target: Init all the variables
   Parameters: None
   Returns: Nothing
  *****************************************************/
  
  //Get the wakeup cause
  esp_sleep_wakeup_cause_t wakeup_reason=esp_sleep_get_wakeup_cause();
  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_UNDEFINED:printLogln(String(millis())+" - [variablesInit] - wakeup reason: ESP_SLEEP_WAKEUP_UNDEFINED (0)");break;
    case ESP_SLEEP_WAKEUP_ALL:printLogln(String(millis())+" - [variablesInit] - wakeup reason: ESP_SLEEP_WAKEUP_ALL (1)");break;
    case ESP_SLEEP_WAKEUP_EXT0:printLogln(String(millis())+" - [variablesInit] - wakeup reason: ESP_SLEEP_WAKEUP_EXT0 (2)");break;
    case ESP_SLEEP_WAKEUP_EXT1:printLogln(String(millis())+" - [variablesInit] - wakeup reason: ESP_SLEEP_WAKEUP_EXT1 (3)");break;
    case ESP_SLEEP_WAKEUP_TIMER:printLogln(String(millis())+" - [variablesInit] - wakeup reason: ESP_SLEEP_WAKEUP_TIMER (4)");break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:printLogln(String(millis())+" - [variablesInit] - wakeup reason: ESP_SLEEP_WAKEUP_TOUCHPAD (5)");break;
    case ESP_SLEEP_WAKEUP_ULP:printLogln(String(millis())+" - [variablesInit] - wakeup reason: ESP_SLEEP_WAKEUP_ULP (6)");break;
    case ESP_SLEEP_WAKEUP_GPIO:printLogln(String(millis())+" - [variablesInit] - wakeup reason: ESP_SLEEP_WAKEUP_GPIO (7)");break;
    case ESP_SLEEP_WAKEUP_UART:printLogln(String(millis())+" - [variablesInit] - wakeup reason: ESP_SLEEP_WAKEUP_UART (8)");break;
    case ESP_SLEEP_WAKEUP_WIFI:printLogln(String(millis())+" - [variablesInit] - wakeup reason: ESP_SLEEP_WAKEUP_WIFI (9)");break;
    case ESP_SLEEP_WAKEUP_COCPU:printLogln(String(millis())+" - [variablesInit] - wakeup reason: ESP_SLEEP_WAKEUP_COCPU (10)");break;
    case ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG:printLogln(String(millis())+" - [variablesInit] - wakeup reason: ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG (11)");break;
    case ESP_SLEEP_WAKEUP_BT:printLogln(String(millis())+" - [variablesInit] - wakeup reason: ESP_SLEEP_WAKEUP_BT (12)");break;
    default:printLogln(String(millis())+" - [variablesInit] - wakeup reason: default");break;
    
    break;
  }

  switch(wakeup_reason) { //Code structure to prepare sleep modes in the future
    case ESP_SLEEP_WAKEUP_UNDEFINED:
    case ESP_SLEEP_WAKEUP_ALL:
    case ESP_SLEEP_WAKEUP_EXT0: //Wake up from Deep Sleep Mode by pressing Button1 or softReset
    case ESP_SLEEP_WAKEUP_EXT1: //Wake up from Hibernate Mode by long pressing Button1
    case ESP_SLEEP_WAKEUP_TIMER: //Scheduled wake up
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
    case ESP_SLEEP_WAKEUP_ULP:
    case ESP_SLEEP_WAKEUP_GPIO:
    case ESP_SLEEP_WAKEUP_UART:
    case ESP_SLEEP_WAKEUP_WIFI:
    case ESP_SLEEP_WAKEUP_COCPU:
    case ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG:
    case ESP_SLEEP_WAKEUP_BT:
    default:
      //Adding the 3 latest mac bytes to the device name (in Hex format)
      WiFi.macAddress(mac);
      device=device+"-"+String((char)hex_digits[mac[3]>>4])+String((char)hex_digits[mac[3]&15])+
        String((char)hex_digits[mac[4]>>4])+String((char)hex_digits[mac[4]&15])+
        String((char)hex_digits[mac[5]>>4])+String((char)hex_digits[mac[5]&15]);

      //EEPROM init
      EEPROMInit(); //Wifi, ntp, web server and mqtt variables updated from that function
      
    //Variable init
      lastMQTTChangeCheck=0,lastCloudClockChangeCheck=0;

      //bool
      debugModeOn=DEBUG_MODE_ON;logMessageTOFF=false;logMessageTRL1_ON=false;logMessageTRL2_ON=false;logMessageGAP_OFF=false;
      boilerStatus=false;thermostateStatus=false;boilerOn=false;thermostateOn=false;thermostateInterrupt=false;gasClear=true;gasInterrupt=false;isBeaconAdvertising=false;webServerResponding=false;
      webLogsOn=false;eepromUpdate=false;powerMeasureSubscribed=false;
      //webLogsOn=false;
      //uint8_t
      auxLoopCounter=0;auxLoopCounter2=0;auxCounter=0;fileUpdateError=0;errorOnActiveCookie=0;errorOnWrongCookie=0;
      //uint16_t
      rebounds=0;voltage=0;power=0;powerMeasureId=0;
      //uint32_t
      heapSize=0;heapBlockSize=0;minMaxHeapBlockSizeSinceBoot=0xFFFFFFFF;minHeapSinceBoot=0xFFFFFFFF;flashSize=ESP.getFlashChipSize();programSize=ESP.getSketchSize();fileSystemSize=0;fileSystemUsed=0;
      //uint64_t
      whileLoopTimeLeft=NTP_CHECK_TIMEOUT;
      //float
      gasSample=0;gasVoltCalibrated=0;RS_airCalibrated=0;RS_CurrentCalibrated=0;gasRatioSample=0;current=0;energyToday=0;energyYesterday=0;energyTotal=0;
      //size_t
      fileUpdateSize=0;OTAAvailableSize=0;SPIFFSAvailableSize=0;
      //String
      TZEnvVariable=String(NTP_TZ_ENV_VARIABLE);TZName=String(NTP_TZ_NAME);serverToUploadSamplesString=String(SERVER_UPLOAD_SAMPLES);

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
    break;
  }
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
      printLog("\n  [tempSensorInit] - Tp/Hm Sensor type: "); printLogln(tempHumSensorType); 
      printLog("  [tempSensorInit] - Tp/Hm Sen. status: "); printLogln(String(statSns),HEX); //boardSerialPort.println(statSns,HEX);
      printLog("  [tempSensorInit] - Tp/Hm Sen.  error: "); printLogln(String(errorSns),HEX); //boardSerialPort.println(errorSns,HEX);
      printLog("  [tempSensorInit] - Tp/Hm Sen. resolu.: "); printLogln(String(tempHumSensor.getResolution())); //boardSerialPort.println(tempHumSensor.getResolution());
      printLogln("  [tempSensorInit] - OK");
    }
    else printLogln(" ...... [OK]");
  }
  else {
    if (debugModeOn) {
      printLog(".\n  [tempSensorInit] - Tp/Hm Sensor type: "); printLog(tempHumSensorType);printLogln(" - Shouldn't be UNKNOWN");
      printLog("  [tempSensorInit] - Tp/Hm Sen. status: "); printLogln(String(statSns),HEX); //boardSerialPort.println(statSns,HEX);
      printLog("  [tempSensorInit] - Tp/Hm Sen.  error: "); printLog(String(errorSns),HEX);printLogln(" - Should be 0"); //boardSerialPort.print(errorSns,HEX);boardSerialPort.println(" - Should be 0");
      printLog("  [tempSensorInit] - Tp/Hm Sen. resolu.: "); printLogln(String(tempHumSensor.getResolution())); //boardSerialPort.println(tempHumSensor.getResolution());
      printLogln("  [tempSensorInit] - KO");
    }
    else printLogln(" ...... [KO]");
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
      printLogln("\n  [wifiInit] - WiFi enabled. Setting WiFi variables ....");
      printLogln("  [wifiInit]    + wifiCred.wifiSSIDs[0]='"+wifiCred.wifiSSIDs[0]+"', wifiCred.wifiPSSWs[0]='"+wifiCred.wifiPSSWs[0]+"', wifiCred.wifiSITEs[0]='"+wifiCred.wifiSITEs[0]+"'");
      printLogln("  [wifiInit]    + wifiCred.wifiSSIDs[1]='"+wifiCred.wifiSSIDs[1]+"', wifiCred.wifiPSSWs[1]='"+wifiCred.wifiPSSWs[1]+"', wifiCred.wifiSITEs[1]='"+wifiCred.wifiSITEs[1]+"'");
      printLogln("  [wifiInit]    + wifiCred.wifiSSIDs[2]='"+wifiCred.wifiSSIDs[2]+"', wifiCred.wifiPSSWs[2]='"+wifiCred.wifiPSSWs[2]+"', wifiCred.wifiSITEs[2]='"+wifiCred.wifiSITEs[2]+"'");
      printLogln("  [wifiInit]    + ntpServers[0]='"+ntpServers[0]+"', ntpServers[1]='"+ntpServers[1]+"', ntpServers[2]='"+ntpServers[2]+"', ntpServers[3]='"+ntpServers[3]+"'");
      printLogln("  [wifiInit]    + TZEnvVariable='"+TZEnvVariable+"', TZName='"+TZName+"'");
    }
    error_setup|=wifiConnect(debugModeOn,&auxLoopCounter,&auxCounter);

    //print Logs
    if ((error_setup & ERROR_WIFI_SETUP)==0 ) { 
      if (debugModeOn) printLogln("[setup] - WiFi connection .......... [OK]");
      else printLogln(" ...... [OK]");

      //WifiNet is updated in printCurrentWiFi(), which is called by wifiConnect(true,X);
      //WiFi.RSSI() might be used instead, but doesn't hurt keeping wifiNet.RSSI instead, as printCurrentWiFi() is required
      // to print logs in here.
      if (wifiNet.RSSI>=WIFI_100_RSSI) wifiCurrentStatus=wifi100Status;
      else if (wifiNet.RSSI>=WIFI_075_RSSI) wifiCurrentStatus=wifi75Status;
      else if (wifiNet.RSSI>=WIFI_050_RSSI) wifiCurrentStatus=wifi50Status;
      else if (wifiNet.RSSI>=WIFI_025_RSSI) wifiCurrentStatus=wifi25Status;
      else if (wifiNet.RSSI<WIFI_000_RSSI) wifiCurrentStatus=wifi0Status;
    }
    else {
      if (debugModeOn) printLogln("  [wifiInit] - WiFi connection .......... [KO]");
      else printLogln(" ...... [KO]");
      wifiCurrentStatus=wifiOffStatus;
    }
  }
  else {//If WiFi is not enabled, then inform
    if (debugModeOn) printLogln("\n  [wifiInit] - WiFi connection .......... [N/E]");
    else printLogln(" ...... [N/E]");
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
      if (debugModeOn) {
        if (fromSetup) {printLogln("\n  [httpCloudInit] - Setting HTTP Cloud server ....");}
        else printLogln(String(millis())+" - [httpCloudInit] - Setting HTTP Cloud server ....");
      }
      String httpRequest=String(GET_REQUEST_TO_UPLOAD_SAMPLES);
      httpRequest=httpRequest+"device="+device+"&local_ip_address="+IpAddress2String(WiFi.localIP())+
        "&relay_status="+String(digitalRead(PIN_THERMOSTATE))+"&counts=0 HTTP/1.1";
      error_setup|=sendHttpRequest(debugModeOn,serverToUploadSamplesIPAddress,SERVER_UPLOAD_PORT,httpRequest,fromSetup); //Send http update
      lastTimeHTTPClouCheck=millis();
      if ((error_setup & ERROR_CLOUD_SERVER)==0) {
        CloudSyncCurrentStatus=CloudSyncOnStatus;
        if (debugModeOn) {
          if (fromSetup) printLogln("  [httpCloudInit] - HTTP Cloud config .......... [OK]");
          else printLogln(String(millis())+" - [httpCloudInit] - HTTP Cloud config .......... [OK]");
        }
      }
      else {
        if (debugModeOn) {
          if (fromSetup) printLogln("  [httpCloudInit] - HTTP Cloud config .......... [KO]");
          else printLogln(String(millis())+"  [httpCloudInit] - HTTP Cloud config .......... [KO]");
        }
      }
    }
    else {
      if (debugModeOn) {
        if (fromSetup) printLogln("\n  [httpCloudInit] - HTTP Cloud server can't be setup: No WiFi connection or WiFi is disabled....");
        else printLogln(String(millis())+"  [httpCloudInit] - HTTP Cloud config .......... [KO-N/E]");
      }
    }
  }
  else {
    if (debugModeOn) {
      if (fromSetup) printLogln("\n  [httpCloudInit] - HTTP Cloud is disabled .......... [N/E]");
      else printLogln(String(millis())+"  [httpCloudInit] - HTTP Cloud is disabled .......... [N/E]");
    }
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
      if (debugModeOn) {
        if (fromSetup) {printLogln("\n  [ntpInit] - Setting NTP server ....");}
        else printLog(String(millis())+" - [ntpInit] - Setting NTP server ....");
      }
      error_setup|=setupNTPConfig(debugModeOn,fromSetup,&auxLoopCounter2,&whileLoopTimeLeft); //Control variables were init in initVariables()
      lastTimeNTPCheck=millis();
      if ((error_setup & ERROR_NTP_SERVER)==0) {
        CloudClockCurrentStatus=CloudClockOnStatus;
        if (debugModeOn) {
          if (fromSetup) printLogln("  [ntpInit] - NTP config .......... [OK]");
          else printLogln(String(millis())+" -   [ntpInit] - NTP config .......... [OK]");
        }
      }
      else {
        CloudClockCurrentStatus=CloudClockOffStatus;
        if (debugModeOn) {
          if (fromSetup) printLogln("  [ntpInit] - NTP config .......... [KO]");
          else printLogln(String(millis())+" -   [ntpInit] - NTP config .......... [KO]");
        }
      }
    }
    else {
      if (debugModeOn) {
        if (fromSetup) printLogln("\n  [ntpInit] - NTP server can't be setup: No WiFi connection or WiFi is disabled .......... [N/E]");
        else printLogln(String(millis())+" -   [ntpInit] - NTP is disabled .......... [KO-N/E]");
      }
    }
  }
  else {
    if (debugModeOn) {
      if (fromSetup) printLogln("\n  [ntpInit] - NTP is disabled .......... [N/E]");
      else printLogln(String(millis())+" -   [ntpInit] - NTP is disabled .......... [N/E]");
    }
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
        if (fromSetup) {printLog("\n");printLog("  [mqttClientInit] - MQTT init.");}
        else printLog(String(millis())+" - [mqttClientInit] - MQTT init.");
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
          printLogln("\n  [mqttClientInit] - Connecting to MQTT SRV: "+mqttServer+", secureMqttEnabled="+String(secureMqttEnabled));
          printLogln("  [mqttClientInit] - Root Topic Name="+mqttTopicName);
          printLogln("  [mqttClientInit] - userName="+userName+", userPssw=******");
        }

        //Connect to MQTT broker
        mqttClient.connect();
        
        //Wait for the MQTT client to get connected or timeout (5sg), whatever happens first
        ulong now=millis();
        while (!mqttClient.connected() && (millis()<=now+5000)) {;}

        if (mqttClient.connected()) {
          MqttSyncCurrentStatus=MqttSyncOnStatus;
          if (fromSetup && debugModeOn) {
            printLogln("  [mqttClientInit] - Connection to MQTT SRV: "+mqttServer+" [OK]");
            printLogln("  [mqttClientInit] - Sending Home Assistant Discovery Messages");
          }
          
          //Publish HA Discovery messages - v1.9
          if (!boilerStatus) { //boilerStatus is true if receiving MQTT messages storm from the SmartPlug
            mqttClientPublishHADiscovery(mqttTopicName,device,WiFi.localIP().toString(),true); //Remove the topics first
            sleep(1); //wait 1 sec.
            mqttClientPublishHADiscovery(mqttTopicName,device,WiFi.localIP().toString(),false); //Update the topics then
          }
        }

        if (MqttSyncCurrentStatus==MqttSyncOnStatus) {
          if (debugModeOn) {
            if (fromSetup) printLogln("  [mqttClientInit] - MQTT config .......... [OK]");
            else printLogln(String(millis())+" -   [mqttClientInit] - MQTT config .......... [OK]");
          }
        }
        else
        {
          if (debugModeOn) {
            if (fromSetup) printLogln("  [mqttClientInit] - MQTT config .......... [KO]");
            else printLogln(String(millis())+" -   [mqttClientInit] - MQTT config .......... [KO]");
          }
          errorMqtt=ERROR_MQTT_SERVER;
        }
      }
      else 
      {
        if (debugModeOn) {
          if (fromSetup) printLogln("\n  [mqttClientInit] - MQTT server can't be setup: No WiFi connection or WiFi is disabled .......... [KO-N/E]");
          else printLogln(String(millis())+" -   [mqttClientInit] - MQTT is disabled .......... [KO-N/E]");
        }
        errorMqtt=ERROR_MQTT_SERVER;
      }
    }
    else {
      if (debugModeOn) {
        if (fromSetup) printLogln("\n  [mqttClientInit] - MQTT server can't be setup: No WiFi connection or WiFi is disabled .......... [KO-N/E]");
        else printLogln(String(millis())+" -   [mqttClientInit] - MQTT is disabled .......... [KO-N/E]"); //This case is supposed not to occur
      } 
      errorMqtt=ERROR_MQTT_SERVER;
    }
  }
  else {
    if (debugModeOn) {
      if (fromSetup) printLogln("\n  [mqttClientInit] - MQTT is disabled .......... [N/E]");
      else printLogln(String(millis())+" -   [mqttClientInit] - MQTT is disabled .......... [N/E]");  //This case is supposed not to occur
    }
    errorMqtt=ERROR_MQTT_SERVER;
  }

  return errorMqtt;
}

uint32_t spiffsInit(boolean debugModeOn,boolean fromSetup) {
  /******************************************************
   Function spiffsInit
   Target: Init the SPIFFS stuff
   Parameters:
    error_setup: Error setup from main code
    debugModeOn: True to print out logs
    fromSetup: True if called from setup
   Return: error_setup
  *****************************************************/
  uint32_t errorSPIFFS=NO_ERROR;

  if (debugModeOn) {
    if (fromSetup) {printLog("\n");printLog("  [spiffsInit] - SPIFFS init.");}
    else printLog(String(millis())+" - [spiffsInit] - SPIFFS init.");
  }
  
  if (SPIFFS.begin(true)) {
    fileSystemSize = SPIFFS.totalBytes();
    fileSystemUsed = SPIFFS.usedBytes();
    if (fromSetup) {
      if (debugModeOn) {printLogln("\n  [spiffsInit] - [OK]");}
      else printLogln(" ...... [OK]");
    }
    else if (debugModeOn) printLogln(" ...... [OK]");

    //Web variables that need to be initialized
    flashSize = ESP.getFlashChipSize();
    programSize = ESP.getSketchSize();
    OTAAvailableSize=getAppOTAPartitionSize(ESP_PARTITION_TYPE_APP,ESP_PARTITION_SUBTYPE_ANY);
    SPIFFSAvailableSize=getAppOTAPartitionSize(ESP_PARTITION_TYPE_DATA,0x82);
    fileSystemSize=SPIFFS.totalBytes();
    fileSystemUsed=SPIFFS.usedBytes();
  }
  else {
    if (fromSetup) {
      if (debugModeOn) {printLogln("\n  [spiffsInit] - [KO]");}
      else printLogln(" ...... [KO]");
    }
    else if (debugModeOn) printLogln(" ...... [KO]");

    //Web variables that need to be initialized
    flashSize = ESP.getFlashChipSize();
    programSize = 0;
    OTAAvailableSize=0;
    SPIFFSAvailableSize=0;
    fileSystemSize=0;
    fileSystemUsed=0;

    errorSPIFFS|=ERROR_SPIFFS_SETUP;
    SPIFFSErrors++;EEPROM.write(0x539,SPIFFSErrors);eepromUpdate=true;
  }

  return errorSPIFFS;
}

uint32_t webServerInit(uint32_t error_setup,boolean wifiEnabled,boolean webServerEnabled,enum wifiStatus wifiCurrentStatus,boolean debugModeOn,boolean fromSetup) {
  /******************************************************
   Function webServerInit
   Target: Init the Web Server stuff
   Parameters:
    error_setup: Error setup from main code
    wifiEnabled: True if WiFi is enabled
    webServerEnabled: True if Web server is enabled
    wifiCurrentStatus: enum wifiStatus {wifiOffStatus,wifi0Status,wifi25Status,wifi50Status,wifi75Status,wifi100Status};
    debugModeOn: True to print out logs
    fromSetup: True if called from setup
   Return: error_setup
  *****************************************************/
  uint32_t errorWebServer=NO_ERROR;
  forceWebServerInit=false;

  if (webServerEnabled && !(error_setup & ERROR_SPIFFS_SETUP) ) {//Only if Web server is enabled and not SPIFFS errors
    //if (forceWebServerInit) webServer=AsyncWebServer::AsyncWebServer(WEBSERVER_PORT);
    
    if (wifiCurrentStatus!=wifiOffStatus && wifiEnabled) { 
      if (debugModeOn) {
        if (fromSetup) {printLog("\n");printLog("  [webServerInit] - Web Server init, error_setup="+String(error_setup,HEX));}
        else printLog(String(millis())+" - [webServerInit] - Web Server init, error_setup="+String(error_setup,HEX));
      }
      errorWebServer|=initWebServer();

      if (!errorWebServer) {
        //if (forceWebServerInit) webSocket=new AsyncWebSocket(WEBSOCKET_CONSOLE_URI);
        errorWebServer|=initWebSocket();
        if (!errorWebServer) {
          if (debugModeOn) {
            if (fromSetup) printLogln("\n  [webServerInit] - Web Server .......... [OK]");
            else printLogln(String(millis())+" -   [webServerInit] - Web Server .......... [OK]");
          }
          return NO_ERROR;
        }
      }
      
      //At this point, errorWebServer=ERROR_WEB_SERVER or ERROR_WEB_SOCKET
      if (debugModeOn) {
        if (fromSetup) printLogln("\n  [webServerInit] - Web Server .......... [KO]");
        else printLogln(String(millis())+" -   [webServerInit] - Web Server .......... [KO]");
      }
    }
    else {
      if (debugModeOn) {
        if (fromSetup) printLogln("\n  [webServerInit] - Web Server can't be setup: No WiFi connection or WiFi is disabled .......... [KO-N/E]");
        else printLogln(String(millis())+" -   [webServerInit] - Web Server is disabled .......... [KO-N/E]");
      }
      errorWebServer=ERROR_WEB_SERVER;
    }
  }
  else {
    if (debugModeOn) {
      if (fromSetup) printLogln("\n  [webServerInit] - Web Server is disabled .......... [N/E]");
      else printLogln(String(millis())+" -   [webServerInit] - Web Server is disabled .......... [N/E]");
    }
    errorWebServer=ERROR_WEB_SERVER;
  }

  return errorWebServer;
}


uint32_t timeOnCountersInit(uint32_t error_setup,bool debugModeOn,bool fromSetup, bool ntpSynced) {
  /******************************************************
   Function timeOnCountersInit
   Target: Init the Time on counters stuff
   Parameters:
    error_setup: errors so far
    debugModeOn: True to print out logs
    fromSetup: True if it is called from setup time
    ntpSynced: True if NTP is synced at least once
   Return: error_setup
  *****************************************************/

  if (debugModeOn) {
    if (fromSetup) {printLog("\n");printLog("  [timeOnCountersInit] - Time on counters init.");}
    else printLog(String(millis())+" - [timeOnCountersInit] - Time on counters init.");
  }
  
  bool updateEeprom=false;
  struct tm auxTimeInfo,*yesterdayTimeInfo;
  time_t yesterdayEpoch;
  getLocalTime(&auxTimeInfo);
  year=auxTimeInfo.tm_year+1900;
  previousYear=year-1;
  today=year*10000+(auxTimeInfo.tm_mon+1)*100+auxTimeInfo.tm_mday;
  yesterdayEpoch=mktime(&auxTimeInfo)-86400;
  yesterdayTimeInfo=localtime(&yesterdayEpoch);
  yesterday=(yesterdayTimeInfo->tm_year+1900)*10000+(yesterdayTimeInfo->tm_mon+1)*100+yesterdayTimeInfo->tm_mday;

  if (!ntpSynced) {
    //No NTP, so no real date available. Let's skip counters stuff
    today=0;yesterday=0;year=0;previousYear=0; //If no NTP, real date not avaiable, so avoid propagating wrong values with those variables. v0.9.9
    
    heaterTimeOnYear.year=0; heaterTimeOnPreviousYear.year=0;
    heaterTimeOnYear.yesterday=0; heaterTimeOnPreviousYear.yesterday=0;
    heaterTimeOnYear.today=0; heaterTimeOnPreviousYear.today=0;
    for (int i=0;i<12;i++) {heaterTimeOnYear.counterMonths[i]=0;heaterTimeOnPreviousYear.counterMonths[i]=0;}
    heaterTimeOnYear.counterYesterday=0;heaterTimeOnPreviousYear.counterYesterday=0;
    heaterTimeOnYear.counterToday=0;heaterTimeOnPreviousYear.counterToday=0;
    
    boilerTimeOnYear.year=0; boilerTimeOnPreviousYear.year=0;
    boilerTimeOnYear.yesterday=0; boilerTimeOnPreviousYear.yesterday=0;
    boilerTimeOnYear.today=0; boilerTimeOnPreviousYear.today=0;
    for (int i=0;i<12;i++) {boilerTimeOnYear.counterMonths[i]=0;boilerTimeOnPreviousYear.counterMonths[i]=0;}
    boilerTimeOnYear.counterYesterday=0;boilerTimeOnPreviousYear.counterYesterday=0;
    boilerTimeOnYear.counterToday=0;boilerTimeOnPreviousYear.counterToday=0;
    
    //Update JSON variable
    samples["heaterYear"] = 0;
    samples["heaterOnYearJan"] = 0;samples["heaterOnYearFeb"] = 0;samples["heaterOnYearMar"] = 0;samples["heaterOnYearApr"] = 0;samples["heaterOnYearMay"] = 0;samples["heaterOnYearJun"] = 0;
    samples["heaterOnYearJul"] = 0;samples["heaterOnYearAug"] = 0;samples["heaterOnYearSep"] = 0;samples["heaterOnYearOct"] = 0;samples["heaterOnYearNov"] = 0;samples["heaterOnYearDec"] = 0;
    samples["heaterYesterday"] = 0;
    samples["heaterToday"] = 0;
    samples["heaterPreviousYear"] = 0;
    samples["heaterOnPreviousYearJan"] = 0;samples["heaterOnPreviousYearFeb"] = 0;samples["heaterOnPreviousYearMar"] = 0;samples["heaterOnPreviousYearApr"] = 0;samples["heaterOnPreviousYearMay"] = 0;samples["heaterOnPreviousYearJun"] = 0;
    samples["heaterOnPreviousYearJul"] = 0;samples["heaterOnPreviousYearAug"] = 0;samples["heaterOnPreviousYearSep"] = 0;samples["heaterOnPreviousYearOct"] = 0;samples["heaterOnPreviousYearNov"] = 0;samples["heaterOnPreviousYearDec"] = 0;
    
    samples["boilerYear"] = 0;
    samples["boilerOnYearJan"] = 0;samples["boilerOnYearFeb"] = 0;samples["boilerOnYearMar"] = 0;samples["boilerOnYearApr"] = 0;samples["boilerOnYearMay"] = 0;samples["boilerOnYearJun"] = 0;
    samples["boilerOnYearJul"] = 0;samples["boilerOnYearAug"] = 0;samples["boilerOnYearSep"] = 0;samples["boilerOnYearOct"] = 0;samples["boilerOnYearNov"] = 0;samples["boilerOnYearDec"] = 0;
    samples["boilerYesterday"] = 0;
    samples["boilerToday"] = 0;
    samples["boilerPreviousYear"] = 0;
    samples["boilerOnPreviousYearJan"] = 0;samples["boilerOnPreviousYearFeb"] = 0;samples["boilerOnPreviousYearMar"] = 0;samples["boilerOnPreviousYearApr"] = 0;samples["boilerOnPreviousYearMay"] = 0;samples["boilerOnPreviousYearJun"] = 0;
    samples["boilerOnPreviousYearJul"] = 0;samples["boilerOnPreviousYearAug"] = 0;samples["boilerOnPreviousYearSep"] = 0;samples["boilerOnPreviousYearOct"] = 0;samples["boilerOnPreviousYearNov"] = 0;samples["boilerOnPreviousYearDec"] = 0;

    if (debugModeOn) {
      if (fromSetup) {printLogln("\n  [timeOnCountersInit] - There is no real date as there is no NTP sync.\n  [timeOnCountersInit] - [KO]");}
      else printLogln(String(millis())+" [timeOnCountersInit] - [KO]");
    }
          
    error_setup|=ERROR_EEPROM_VARIABLES_INIT;
    return error_setup;
  }

  //Check variable coherency. If not, it means timers in EEPROM were never stored before. Init them.
  EEPROM.get(0x421,heaterTimeOnYear);EEPROM.get(0x465,heaterTimeOnPreviousYear);
  EEPROM.get(0x4A9,boilerTimeOnYear); EEPROM.get(0x4ED,boilerTimeOnPreviousYear);
  
  /*heaterTimeOnPreviousYear.year=2024;
  heaterTimeOnPreviousYear.yesterday=20240530;heaterTimeOnPreviousYear.today=20240531;
  heaterTimeOnPreviousYear.counterYesterday=0;heaterTimeOnPreviousYear.counterToday=0;
  heaterTimeOnPreviousYear.counterMonths[0]=2178360;heaterTimeOnPreviousYear.counterMonths[1]=156600;heaterTimeOnPreviousYear.counterMonths[2]=100440;heaterTimeOnPreviousYear.counterMonths[3]=41400;heaterTimeOnPreviousYear.counterMonths[4]=16920;heaterTimeOnPreviousYear.counterMonths[5]=0;
  heaterTimeOnPreviousYear.counterMonths[6]=0;heaterTimeOnPreviousYear.counterMonths[7]=0;heaterTimeOnPreviousYear.counterMonths[8]=0;heaterTimeOnPreviousYear.counterMonths[9]=14760;heaterTimeOnPreviousYear.counterMonths[10]=115200;heaterTimeOnPreviousYear.counterMonths[11]=228960;
  heaterTimeOnYear.year=2025;
  heaterTimeOnYear.yesterday=20250420;heaterTimeOnYear.today=20250421;
  heaterTimeOnYear.counterMonths[0]=1060200;heaterTimeOnYear.counterMonths[1]=181800;heaterTimeOnYear.counterMonths[2]=273960;heaterTimeOnYear.counterMonths[3]=46080;heaterTimeOnYear.counterMonths[4]=0;heaterTimeOnYear.counterMonths[5]=0;
  heaterTimeOnYear.counterMonths[6]=0;heaterTimeOnYear.counterMonths[7]=0;heaterTimeOnYear.counterMonths[8]=0;heaterTimeOnYear.counterMonths[9]=0;heaterTimeOnYear.counterMonths[10]=0;heaterTimeOnYear.counterMonths[11]=0;
  heaterTimeOnYear.counterYesterday=0;heaterTimeOnYear.counterToday=0;
  heaterTimeOnYear.year=0xff;*/

  if (heaterTimeOnYear.year != (heaterTimeOnYear.yesterday/10000) || 
      heaterTimeOnYear.year != (heaterTimeOnYear.today/10000) ||
      heaterTimeOnPreviousYear.year != (heaterTimeOnPreviousYear.yesterday/10000) || 
      heaterTimeOnPreviousYear.year != (heaterTimeOnPreviousYear.today/10000) ||
      boilerTimeOnYear.year != (boilerTimeOnYear.yesterday/10000) || 
      boilerTimeOnYear.year != (boilerTimeOnYear.today/10000) ||
      boilerTimeOnPreviousYear.year != (boilerTimeOnPreviousYear.yesterday/10000) || 
      boilerTimeOnPreviousYear.year != (boilerTimeOnPreviousYear.today/10000)) {

        //No coherency. Variables needs to be initiated and stored in EEPROM
        if (debugModeOn) {if (fromSetup) {printLog("\n");}printLogln("  [timeOnCountersInit] - No EEPROM coherency. Writting counters in EEPROM");}

        //Initialize time on counters
        heaterTimeOnYear.year=year; heaterTimeOnPreviousYear.year=previousYear;
        heaterTimeOnYear.yesterday=yesterday; heaterTimeOnPreviousYear.yesterday=yesterday-10000; 
        heaterTimeOnYear.today=today; heaterTimeOnPreviousYear.today=today-10000;
        for (int i=0; i<12; i++) {heaterTimeOnYear.counterMonths[i]=0;heaterTimeOnPreviousYear.counterMonths[i]=0;}
        heaterTimeOnYear.counterYesterday=0; heaterTimeOnPreviousYear.counterYesterday=0;
        heaterTimeOnYear.counterToday=0; heaterTimeOnPreviousYear.counterToday=0;

        //Initialize time on counters
        boilerTimeOnYear.year=year; boilerTimeOnPreviousYear.year=year-1;
        boilerTimeOnYear.yesterday=yesterday; boilerTimeOnPreviousYear.yesterday=yesterday-10000;
        boilerTimeOnYear.today=today; boilerTimeOnPreviousYear.today=today-10000;
        for (int i=0; i<12; i++) {boilerTimeOnYear.counterMonths[i]=0;boilerTimeOnPreviousYear.counterMonths[i]=0;}
        boilerTimeOnYear.counterYesterday=0; boilerTimeOnPreviousYear.counterYesterday=0;
        boilerTimeOnYear.counterToday=0; boilerTimeOnPreviousYear.counterToday=0;
        
        updateEeprom=true;
  }
  else {
    if (debugModeOn) {if (fromSetup) {printLog("\n");} printLogln("  [timeOnCountersInit] - EEPROM variables are coherent. Not writting counters in EEPROM.");}
    if (today==heaterTimeOnYear.today) {
      //Boot same day than date in EEPROM
      //Do nothing. Variables already got from EEPROM
      if (debugModeOn) printLogln("  [timeOnCountersInit] - Date saved in EEPROM is same than today. Do nothing else.");
    }
    else {
      //Boot different day than date in EEPROM
      if (yesterday==heaterTimeOnYear.today) {
        //Boot the next day than date in EEPROMtersInit] - year="+String(year)+", heaterTimeOnYear.year="+String(heaterTimeOnYear.year));
        if (year==heaterTimeOnYear.year) {
          //Boot the next day than date in EEPROM and same year
          heaterTimeOnYear.counterYesterday=heaterTimeOnYear.counterToday; boilerTimeOnYear.counterYesterday=boilerTimeOnYear.counterToday;
        }
        else {
          //It's January 1st or a new year
          memcpy(&heaterTimeOnPreviousYear,&heaterTimeOnYear,sizeof(heaterTimeOnYear)); memcpy(&boilerTimeOnPreviousYear,&boilerTimeOnYear,sizeof(boilerTimeOnYear));
          heaterTimeOnYear.year=year; boilerTimeOnYear.year=year;
          for (int i=0; i<12; i++) {heaterTimeOnYear.counterMonths[i]=0;boilerTimeOnYear.counterMonths[i]=0;}
          heaterTimeOnYear.counterYesterday=heaterTimeOnYear.counterToday; boilerTimeOnYear.counterYesterday=boilerTimeOnYear.counterToday;
        }
      }
      else {
        //No next day than in EEPROM
        if (year==heaterTimeOnYear.year) {
          //i.e.: Boot one week after than date in EEPROM and same year
          heaterTimeOnYear.counterYesterday=0;boilerTimeOnYear.counterYesterday=0;
        }
        else {
          //Boot in different year
          if (previousYear==heaterTimeOnYear.year) {
            //i.e.: Boot 14 months after than date in EEPROM
            memcpy(&heaterTimeOnPreviousYear,&heaterTimeOnYear,sizeof(heaterTimeOnYear)); memcpy(&boilerTimeOnPreviousYear,&boilerTimeOnYear,sizeof(boilerTimeOnYear));
          }
          else {
            //i.e.: Boot 3 years after than date in EEPROM
            heaterTimeOnPreviousYear.year=previousYear; boilerTimeOnPreviousYear.year=previousYear;
            heaterTimeOnPreviousYear.yesterday=yesterday-10000; boilerTimeOnPreviousYear.yesterday=yesterday-10000; 
            heaterTimeOnPreviousYear.today=today-10000; boilerTimeOnPreviousYear.today=today-10000;
            for (int i=0; i<12; i++) {heaterTimeOnPreviousYear.counterMonths[i]=0;boilerTimeOnPreviousYear.counterMonths[i]=0;}
            heaterTimeOnPreviousYear.counterYesterday=0;boilerTimeOnPreviousYear.counterYesterday=0;
            heaterTimeOnPreviousYear.counterToday=0;boilerTimeOnPreviousYear.counterToday=0;
          }
          heaterTimeOnYear.year=year; boilerTimeOnYear.year=year;
          for (int i=0; i<12; i++) {heaterTimeOnYear.counterMonths[i]=0;boilerTimeOnYear.counterMonths[i]=0;}
          heaterTimeOnYear.counterYesterday=0;boilerTimeOnYear.counterYesterday=0;
        }
      }

      heaterTimeOnYear.yesterday=yesterday; boilerTimeOnYear.yesterday=yesterday;
      heaterTimeOnYear.today=today; boilerTimeOnYear.today=today;
      heaterTimeOnYear.counterToday=0;boilerTimeOnYear.counterToday=0;

      updateEeprom=true;
    }
  } 

  //Save variables in EEPROM
  if (updateEeprom) {
    EEPROM.put(0x421,heaterTimeOnYear); EEPROM.put(0x465,heaterTimeOnPreviousYear);
    EEPROM.put(0x4A9,boilerTimeOnYear); EEPROM.put(0x4ED,boilerTimeOnPreviousYear);
    EEPROM.commit();
    if (debugModeOn) printLogln("  [timeOnCountersInit] - EEPROM updated with variables and counters");
  }

  //Update JSON variable
  uint32_t auxTimeOn=0; for (int i=0;i<12;i++) auxTimeOn+=heaterTimeOnYear.counterMonths[i];
  samples["heaterYear"] = heaterTimeOnYear.year;
  samples["heaterYesterday"] = heaterTimeOnYear.yesterday;
  samples["heaterToday"] = heaterTimeOnYear.today;
  samples["heaterOnYear"] = auxTimeOn;
  samples["heaterOnYearJan"] = heaterTimeOnYear.counterMonths[0];samples["heaterOnYearFeb"] = heaterTimeOnYear.counterMonths[1];samples["heaterOnYearMar"] = heaterTimeOnYear.counterMonths[2];samples["heaterOnYearApr"] = heaterTimeOnYear.counterMonths[3];samples["heaterOnYearMay"] = heaterTimeOnYear.counterMonths[4];samples["heaterOnYearJun"] = heaterTimeOnYear.counterMonths[5];
  samples["heaterOnYearJul"] = heaterTimeOnYear.counterMonths[6];samples["heaterOnYearAug"] = heaterTimeOnYear.counterMonths[7];samples["heaterOnYearSep"] = heaterTimeOnYear.counterMonths[8];samples["heaterOnYearOct"] = heaterTimeOnYear.counterMonths[9];samples["heaterOnYearNov"] = heaterTimeOnYear.counterMonths[10];samples["heaterOnYearDec"] = heaterTimeOnYear.counterMonths[11];
  samples["heaterOnYesterday"] = heaterTimeOnYear.counterYesterday;
  samples["heaterOnToday"] = heaterTimeOnYear.counterToday;
  auxTimeOn=0; for (int i=0;i<12;i++) auxTimeOn+=heaterTimeOnPreviousYear.counterMonths[i];
  samples["heaterPreviousYear"] = heaterTimeOnPreviousYear.year;
  samples["heaterOnPreviousYear"] = auxTimeOn;
  samples["heaterOnPreviousYearJan"] = heaterTimeOnPreviousYear.counterMonths[0];samples["heaterOnPreviousYearFeb"] = heaterTimeOnPreviousYear.counterMonths[1];samples["heaterOnPreviousYearMar"] = heaterTimeOnPreviousYear.counterMonths[2];samples["heaterOnPreviousYearApr"] = heaterTimeOnPreviousYear.counterMonths[3];samples["heaterOnPreviousYearMay"] = heaterTimeOnPreviousYear.counterMonths[4];samples["heaterOnPreviousYearJun"] = heaterTimeOnPreviousYear.counterMonths[5];
  samples["heaterOnPreviousYearJul"] = heaterTimeOnPreviousYear.counterMonths[6];samples["heaterOnPreviousYearAug"] = heaterTimeOnPreviousYear.counterMonths[7];samples["heaterOnPreviousYearSep"] = heaterTimeOnPreviousYear.counterMonths[8];samples["heaterOnPreviousYearOct"] = heaterTimeOnPreviousYear.counterMonths[9];samples["heaterOnPreviousYearNov"] = heaterTimeOnPreviousYear.counterMonths[10];samples["heaterOnPreviousYearDec"] = heaterTimeOnPreviousYear.counterMonths[11];
  
  auxTimeOn=0; for (int i=0;i<12;i++) auxTimeOn+=boilerTimeOnYear.counterMonths[i];
  samples["boilerYear"] = boilerTimeOnYear.year;
  samples["boilerYesterday"] = boilerTimeOnYear.yesterday;
  samples["boilerToday"] = boilerTimeOnYear.today;
  samples["boilerOnYear"] = auxTimeOn;
  samples["boilerOnYearJan"] = boilerTimeOnYear.counterMonths[0];samples["boilerOnYearFeb"] = boilerTimeOnYear.counterMonths[1];samples["boilerOnYearMar"] = boilerTimeOnYear.counterMonths[2];samples["boilerOnYearApr"] = boilerTimeOnYear.counterMonths[3];samples["boilerOnYearMay"] = boilerTimeOnYear.counterMonths[4];samples["boilerOnYearJun"] = boilerTimeOnYear.counterMonths[5];
  samples["boilerOnYearJul"] = boilerTimeOnYear.counterMonths[6];samples["boilerOnYearAug"] = boilerTimeOnYear.counterMonths[7];samples["boilerOnYearSep"] = boilerTimeOnYear.counterMonths[8];samples["boilerOnYearOct"] = boilerTimeOnYear.counterMonths[9];samples["boilerOnYearNov"] = boilerTimeOnYear.counterMonths[10];samples["boilerOnYearDec"] = boilerTimeOnYear.counterMonths[11];
  samples["boilerOnYesterday"] = boilerTimeOnYear.counterYesterday;
  samples["boilerOnToday"] = boilerTimeOnYear.counterToday;
  auxTimeOn=0; for (int i=0;i<12;i++) auxTimeOn+=boilerTimeOnPreviousYear.counterMonths[i];
  samples["boilerPreviousYear"] = boilerTimeOnPreviousYear.year;
  samples["boilerOnPreviousYear"] = auxTimeOn;
  samples["boilerOnPreviousYearJan"] = boilerTimeOnPreviousYear.counterMonths[0];samples["boilerOnPreviousYearFeb"] = boilerTimeOnPreviousYear.counterMonths[1];samples["boilerOnPreviousYearMar"] = boilerTimeOnPreviousYear.counterMonths[2];samples["boilerOnPreviousYearApr"] = boilerTimeOnPreviousYear.counterMonths[3];samples["boilerOnPreviousYearMay"] = boilerTimeOnPreviousYear.counterMonths[4];samples["boilerOnPreviousYearJun"] = boilerTimeOnPreviousYear.counterMonths[5];
  samples["boilerOnPreviousYearJul"] = boilerTimeOnPreviousYear.counterMonths[6];samples["boilerOnPreviousYearAug"] = boilerTimeOnPreviousYear.counterMonths[7];samples["boilerOnPreviousYearSep"] = boilerTimeOnPreviousYear.counterMonths[8];samples["boilerOnPreviousYearOct"] = boilerTimeOnPreviousYear.counterMonths[9];samples["boilerOnPreviousYearNov"] = boilerTimeOnPreviousYear.counterMonths[10];samples["boilerOnPreviousYearDec"] = boilerTimeOnPreviousYear.counterMonths[11];

  //samples.printTo(boardSerialPort); Print out the JSON variable

  if (debugModeOn) {
    if (fromSetup) {printLogln("  [timeOnCountersInit] - Counter variables got from EEPROM.\n  [timeOnCountersInit] - [OK]");}
    else printLogln(String(millis())+" [timeOnCountersInit] - [OK]");
  }
 
  return error_setup;
}