/* Dealing with WiFi stuff

*/

#include "wifiConnection.h"
#include "esp_sntp.h"
#include "eeprom_utils.h"

int status = WL_IDLE_STATUS;     // the Wifi radio's status

/******************************************************
 Function printNetData
 Target: prints Network parameters (@IP,@MAC, Default GW, Mask, DNS)
 *****************************************************/
void printNetData() {
  // print MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  printLog("  [printNetData] - MAC address: ");
  printLog(mac[0], HEX); printLog(":");
  printLog(mac[1], HEX); printLog(":");
  printLog(mac[2], HEX); printLog(":");
  printLog(mac[3], HEX); printLog(":");
  printLog(mac[4], HEX); printLog(":");
  printLogln(mac[5], HEX);

  // print IP address, etc.
  //IPAddress ip = WiFi.localIP();
  printLog("  [printNetData] - IP Address : ");printLogln(WiFi.localIP().toString());
  printLog("  [printNetData] - Mask       : ");printLogln(WiFi.subnetMask().toString());
  printLog("  [printNetData] - Default GW : ");printLogln(WiFi.gatewayIP().toString());
}

/******************************************************
 Function printCurrentWiFi
 Target: prints WiFi parameters on Serial and returs variable with Wifi parameters
 *****************************************************/
wifiNetworkInfo * printCurrentWiFi(boolean debugModeOn=true, int16_t *numberWiFiNetworks=nullptr) {
  // print current network parameters
  WiFiScanClass wifiScan = WiFiScanClass();
  int16_t countWiFiNetworks=wifiScan.scanNetworks();
  int16_t scanReturn=-1;
  while (scanReturn<0){
    scanReturn=wifiScan.scanComplete();
    switch (scanReturn) {
      case WIFI_SCAN_FAILED:
        if (debugModeOn) printLogln("  [printCurrentWiFi] - Error: Failed to scan WiFi networks");
        return nullptr;
      break;
      case WIFI_SCAN_RUNNING:
        //Waiting for the scan to finish
      break;
      default:
          //printLog("  Number of SSIDs detected: "); printLogln(scanReturn);
          break;  //Scan finished. Exit the while loop
      break;
    }
  } //Wait till scan is finished

  *numberWiFiNetworks=scanReturn;

  int16_t wifiNetworkNode=-1;
  for (int16_t i=0; i<countWiFiNetworks; i++) {
    //String mySSID=String(WIFI_SSID_CREDENTIALS);
    String mySSID=wifiCred.wifiSSIDs[wifiCred.activeIndex];
    wifiScan.getNetworkInfo(i, wifiNet.ssid, wifiNet.encryptionType, wifiNet.RSSI, wifiNet.BSSID, wifiNet.channel);
    if (mySSID.compareTo(wifiNet.ssid) == 0) {
      //if (debugModeOn) {printLog(mySSID);printLog(" is equal than "); printLogln(wifiNet.ssid);
      //             printLog("networkItem=");printLogln(i);}
      wifiNetworkNode=i;
      break;
    }
    else {
      //if (debugModeOn) {printLog(mySSID);printLog(" is not equal than "); printLogln(wifiNet.ssid);}
    }
  }

  if (-1==wifiNetworkNode) {
    //WiFi SSID was lost
    if (debugModeOn) {
      printLog("  [printCurrentWiFi] - SSID: ");printLog(wifiCred.wifiSSIDs[wifiCred.activeIndex]);printLogln(" lost");
    }
    return nullptr;
  }

  // print current network parameters
  if (debugModeOn) {
    printLog("  [printCurrentWiFi] - Wifi Network Node: ");printLog(wifiNetworkNode);
    printLog(" and "); printLog(scanReturn); printLogln(" detected");
    //printLog("SSID: ");printLogln(WiFi.SSID());
    printLog("  [printCurrentWiFi] - SSID: ");printLogln(wifiNet.ssid);

    // print the MAC address of the router you're attached to:
    uint8_t bssid[6];
    //memcpy(bssid, WiFi.BSSID(), 6);
    memcpy(bssid, wifiNet.BSSID, 6);
    printLog("  [printCurrentWiFi] - BSSID: ");
    printLog(bssid[5], HEX); printLog(":");
    printLog(bssid[4], HEX); printLog(":");
    printLog(bssid[3], HEX); printLog(":");
    printLog(bssid[2], HEX); printLog(":");
    printLog(bssid[1], HEX); printLog(":");
    printLogln(bssid[0], HEX);

    // print the received signal strength:
    //printLog("signal strength (RSSI):");printLogln((long) WiFi.RSSI());
    printLog("  [printCurrentWiFi] - Signal strength (RSSI): ");printLogln(String(wifiNet.RSSI)+" dBm");

    // print the encryption type:
    printLog("  [printCurrentWiFi] - Encryption Type: 0x");printLogln(String(wifiNet.encryptionType), HEX);

    // print the channel:
    printLog("  [printCurrentWiFi] - WiFi Channel: ");printLogln(String(wifiNet.channel));
  }

  return &wifiNet;
}

/******************************************************
 Function wifiConnect
 Target: to get connected to the WiFi
 Parameters:
  - debugModeOn: Print or not log messages in Serial line. Diferent prints out are done base on its value
      Value false: no log messages are printed out
      Value true:  log messages are printed out
  - *auxLoopCounter is a pointer for the index of the SSID array to check. It's sent from the main loop
       Value 0: The SSID will start from the very first SSID - First call to the funcion
       Value other: The SSID will resume the connection with the same SSID used in the previous interaction
           which was stoped due to either Button Pressed (ABORT) or Display Refresh (BREAK)
  - *auxCounter is the counter for the while() loop to stop checking the index of he SSID array. It's sent from the main loop
       Value 0: The SSID connection will start from the beginig - First call to the funcion
       Value other: The SSID connection will resume the connection at the same point where the previous interaction
           was stoped due to either Button Pressed (ABORT) or Display Refresh (BREAK)
  - *auxLoopCounter and *auxCounter are global variables. They are modified in here. The calling function
       just send them to this function.
 *****************************************************/
uint32_t wifiConnect(boolean debugModeOn=true, uint8_t* auxLoopCounter=nullptr, uint8_t* auxCounter=nullptr) {
  status = WL_IDLE_STATUS;
  uint8_t counter=0;
  boolean errorWifiConnection;
  
  //to avoid pointer issues
  if (auxLoopCounter==nullptr || auxCounter==nullptr) {
    errorsWiFiCnt++;EEPROM.write(0x535,errorsWiFiCnt);eepromUpdate=true; //Something went wrong. Update error counter for stats
    return(ERROR_WIFI_SETUP);
  }

  // attempt to connect to Wifi network:
  for (uint8_t loopCounter=*auxLoopCounter; loopCounter<(uint8_t)sizeof(wifiCred.wifiSSIDs)/sizeof(String); loopCounter++) {
    counter=*auxCounter;
    errorWifiConnection=false;
    *auxLoopCounter=loopCounter;
    //WiFi.mode(WIFI_STA);
    WiFi.begin(wifiCred.wifiSSIDs[loopCounter].c_str(), wifiCred.wifiPSSWs[loopCounter].c_str());

    long auxWhile=millis();
    while (status != WL_CONNECTED && counter < MAX_CONNECTION_ATTEMPTS) {
      if ((millis()-auxWhile)>=(MAX_CONNECTION_ATTEMPTS*1000/10)) { //Interval to show info and dots: MAX_CONNECTION_ATTEMPTS/10 *1000 milliseconds
        counter++;
        *auxCounter=counter;
        auxWhile=millis();
        if (debugModeOn) {printLog("  [wifiConnect] - Attempting to connect to WPA SSID: ");printLogln(wifiCred.wifiSSIDs[loopCounter].c_str());}
      }
      status = WiFi.status();
    } //End of while() loop

    if (counter>=MAX_CONNECTION_ATTEMPTS) { //Case if while() loop timeout.
      if (debugModeOn) {
        printLog("  [wifiConnect] - WiFi network ERROR: ");
        printLog("No connection to SSID ");printLogln(wifiCred.wifiSSIDs[loopCounter].c_str());
        printLog("  [wifiConnect] - Number of connection attempts ");
        printLog(counter);printLog(">");printLogln(MAX_CONNECTION_ATTEMPTS);
      }
      errorWifiConnection=true;
      *auxCounter=0;  //Reset the while() counter to be ready for the next index of the SSID array  
    }
    else {
      //End of while() due to successful SSID connection
      // (Button Pressed or Display Refresh force the function to return from the while() loop, 
      //  so this point is not reached in those cases)
      wifiCred.activeIndex=loopCounter;
      loopCounter=sizeof(wifiCred.wifiSSIDs); //loop end
      errorWifiConnection=false;
    }
  }//end For() loop

  //This point is reached if either the while() loop timed out or successful SSID connection
  if (errorWifiConnection) {
    //Case for while loop timeout (no successfull SSID connection)
    errorsWiFiCnt++;EEPROM.write(0x535,errorsWiFiCnt);eepromUpdate=true; //Something went wrong. Update error counter for stats
    if (auxLoopCounter!=nullptr) *auxLoopCounter=0;  //To avoid resuming connection the next loop interacion
    if (auxCounter!=nullptr) *auxCounter=0;          //To avoid resuming connection the next loop interacion
    return(ERROR_WIFI_SETUP); //not wifi connection
  }
  
  //Case for successfull SSID connection
  if (debugModeOn) {
    int16_t numberWiFiNetworks;
    printLogln("  [wifiConnect] - Connected to the network");
    printLogln("  [wifiConnect] - WiFi info: "); printCurrentWiFi(true,&numberWiFiNetworks);
    printLog("  [wifiConnect] - Net info: \n");printNetData();
  }

  if (auxLoopCounter!=nullptr) *auxLoopCounter=0;  //To avoid resuming connection the next loop interacion
  if (auxCounter!=nullptr) *auxCounter=0;          //To avoid resuming connection the next loop interacion
  return(NO_ERROR); //WiFi Connection OK
}

/******************************************************
 Function setupNTPConfig
 Target: to set NTP config. If function is called from firstSetup(), then logs are displayed
 Parameters:
  - debugModeOn: Print or not log messages in Serial line. Diferent prints out are done base on its value
    Value false: no log messages are printed out
    Value true:  log messages are printed out
  - fromSetup: where the function was called from. Diferent prints out are done base on its value
    Value false: from main loop
    Value true:  from the firstSetup() function
  - *auxLoopCounter is a pointer for the index of the NTP server array to check. It's sent from the main loop
    Value 0: The NTP sync will start from the very first NTP server - First call to the funcion
    Value other: The NTP sync will resume the sync with the same NTP server used in the previous interaction
      which was stoped due to either Button Pressed (ABORT) or Display Refresh (BREAK)
  - *whileLoopTimeLeft is the pointer for the time of while() loop to stop checking the index of
    the NTP server array. It's sent from the main loop
    Value NTP_CHECK_TIMEOUT: The NTP sync will start from the beginig - First call to the funcion
    Value other: The NTP sync will resume the sync at the same point where the previous interaction
      was stoped due to either Button Pressed (ABORT) or Display Refresh (BREAK)
  *auxLoopCounter and *whileLoopTimeLeft are global variables. They are modified in here. The calling function
    just send them to this function.
 *****************************************************/
 uint32_t setupNTPConfig(boolean debugModeOn,boolean fromSetup=false,uint8_t* auxLoopCounter=nullptr,uint64_t* whileLoopTimeLeft=nullptr) {
  if (debugModeOn) {if (!fromSetup) {printLog(String(millis()));printLogln("  [setupNTPConfig] - CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+", errorsNTPCnt="+String(errorsNTPCnt)+", auxLoopCounter="+String(*auxLoopCounter)+", whileLoopTimeLeft="+String(*whileLoopTimeLeft));}}

  //to avoid pointer issues
  if (auxLoopCounter==nullptr || whileLoopTimeLeft==nullptr) {
    errorsNTPCnt++; EEPROM.write(0x536,errorsNTPCnt);eepromUpdate=true;//Something went wrong. Update error counter for stats
    return(ERROR_NTP_SERVER);
  }
  
  CloudClockCurrentStatus=CloudClockOffStatus;

  if (wifiCurrentStatus!=wifiOffStatus) {
    uint8_t auxForLoop;
    for (uint8_t loopCounter=*auxLoopCounter; loopCounter<(uint8_t)sizeof(ntpServers)/sizeof(String); loopCounter++) {
      if (ntpServers[loopCounter].charAt(0)=='\0') loopCounter++;
      else {
        if (debugModeOn) {printLogln("  [setupNTPConfig] - Configuring configTzTime("+String(TZEnvVariable.c_str())+","+String(ntpServers[loopCounter].c_str())+") to connecting to NTP Server: "+ntpServers[loopCounter]);}
        configTzTime(TZEnvVariable.c_str(), ntpServers[loopCounter].c_str());

        *auxLoopCounter=loopCounter;
        uint64_t whileStartTime=millis(),auxTime=whileStartTime;
        if (*whileLoopTimeLeft>=NTP_CHECK_TIMEOUT) *whileLoopTimeLeft=NTP_CHECK_TIMEOUT;

        if (debugModeOn) {if (!fromSetup) {printLogln("  [setupNTPConfig] - whileStartTime="+String(whileStartTime)+", *whileLoopTimeLeft="+String(*whileLoopTimeLeft)+", *auxLoopCounter="+String(*auxLoopCounter)+", loopCounter="+String(loopCounter)+", waiting for NTPStatus=SNTP_SYNC_STATUS_COMPLETED");}}
        while ( sntp_get_sync_status()!=SNTP_SYNC_STATUS_COMPLETED &&
              *whileLoopTimeLeft<=NTP_CHECK_TIMEOUT) {
          *whileLoopTimeLeft=*whileLoopTimeLeft-(millis()-auxTime);
          auxTime=millis();
          delay(50);
        } //end while() loop

        if (debugModeOn) {if (!fromSetup) {printLogln(String(millis())+"  [setupNTPConfig] - End of wait - Elapsed Time: "+String(auxTime-whileStartTime));}}
        
        if (*whileLoopTimeLeft>NTP_CHECK_TIMEOUT) { //Case if while() loop timeout.
          if ((debugModeOn && fromSetup) || debugModeOn) {
            printLogln("  [setupNTPConfig] - Time: Failed to get time");
            printLog("  [setupNTPConfig] - NTP: ");printLogln("KO");
          }
        }
        else { 
          //End of while() due to successful NTP sync
          // (Button Pressed or Display Refresh force the function to return from the while() loop, 
          //  so this point is not reached in those cases)
          if ((debugModeOn && fromSetup) || debugModeOn) {
            printLog("  [setupNTPConfig] - NTP: ");printLogln("OK");
            printLog("  [setupNTPConfig] - Time: ");getLocalTime(&nowTimeInfo);printLogln(&nowTimeInfo,"%d/%m/%Y - %H:%M:%S");
          }
          CloudClockCurrentStatus=CloudClockSendStatus;  //Will be CloudClockOnStatus after refreshing Icons
          ntpServerIndex=loopCounter;
          loopCounter=(uint8_t)sizeof(ntpServers)/sizeof(String);
          memset(TZEnvVar,'\0',sizeof(TZEnvVar)); //Fill null character first to avoid overflow
          int tzLength=String(getenv("TZ")).length();
          if (tzLength>sizeof(TZEnvVar)) tzLength=sizeof(TZEnvVar); //Make sure not to overflow TZEnvVar
          memcpy(TZEnvVar,getenv("TZ"),tzLength); //Back Time Zone up to restore it after wakeup
        }
      }
    }//end For() loop
  }

  //This point is reached if either the while() loop timed out or successful NTP sync
  
  if (CloudClockCurrentStatus==CloudClockOffStatus) {
    //Case for while loop timeout (no successfull NTP sync)
    errorsNTPCnt++; EEPROM.write(0x536,errorsNTPCnt); eepromUpdate=true;//Something went wrong. Update error counter for stats   
    if (auxLoopCounter!=nullptr) *auxLoopCounter=0;                 //To avoid resuming connection the next loop interacion
    if (whileLoopTimeLeft!=nullptr) *whileLoopTimeLeft=NTP_CHECK_TIMEOUT;  //To avoid resuming connection the next loop interacion       
    return(ERROR_NTP_SERVER); //not NTP server connection
  }

  //Case for successfull NTP sync
  ntpSynced=true; //This flag is unset at boot time. If true, it means NTP syc was succesfull at least once
  CloudClockCurrentStatus=CloudClockOnStatus;
  if (debugModeOn) {
      if (!fromSetup) {
        printLogln("  [setupNTPConfig] - CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+", lastTimeNTPCheck="+String(lastTimeNTPCheck)+", errorsNTPCnt="+String(errorsNTPCnt));
        printLog(String(millis()));
        printLog("  [setupNTPConfig] - Exit - Time:");getLocalTime(&nowTimeInfo);printLogln(&nowTimeInfo,"%d/%m/%Y - %H:%M:%S");
      }
  }

  //Updating uptime if not done before. Rest the time since loopStatTime
  if (!startTimeConfigure) {
    getLocalTime(&startTimeInfo);
    time_t temTimeT=mktime(&startTimeInfo)-(millis()/1000);
    struct tm *tempTime=localtime(&temTimeT);
    memcpy(&startTimeInfo,tempTime,sizeof(tempTime));
    startTimeConfigure=true;
  }

  if (auxLoopCounter!=nullptr) *auxLoopCounter=0;                 //To avoid resuming connection the next loop interacion
  if (whileLoopTimeLeft!=nullptr) *whileLoopTimeLeft=NTP_CHECK_TIMEOUT;  //To avoid resuming connection the next loop interacion       
  return(NO_ERROR);
}