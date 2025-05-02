/* Dealing with WiFi stuff

*/

#include "wifiConnection.h"
#include "esp_sntp.h"

int status = WL_IDLE_STATUS;     // the Wifi radio's status

/******************************************************
 Function printNetData
 Target: prints Network parameters (@IP,@MAC, Default GW, Mask, DNS)
 *****************************************************/
void printNetData() {
  // print MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  boardSerialPort.print("  [printNetData] - MAC address: ");
  boardSerialPort.print(mac[0], HEX); boardSerialPort.print(":");
  boardSerialPort.print(mac[1], HEX); boardSerialPort.print(":");
  boardSerialPort.print(mac[2], HEX); boardSerialPort.print(":");
  boardSerialPort.print(mac[3], HEX); boardSerialPort.print(":");
  boardSerialPort.print(mac[4], HEX); boardSerialPort.print(":");
  boardSerialPort.println(mac[5], HEX);

  // print IP address, etc.
  //IPAddress ip = WiFi.localIP();
  boardSerialPort.print("  [printNetData] - IP Address : ");boardSerialPort.println(WiFi.localIP().toString());
  boardSerialPort.print("  [printNetData] - Mask       : ");boardSerialPort.println(WiFi.subnetMask().toString());
  boardSerialPort.print("  [printNetData] - Default GW : ");boardSerialPort.println(WiFi.gatewayIP().toString());
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
        if (debugModeOn) boardSerialPort.println("  [printCurrentWiFi] - Error: Failed to scan WiFi networks");
        return nullptr;
      break;
      case WIFI_SCAN_RUNNING:
        //Waiting for the scan to finish
      break;
      default:
          //boardSerialPort.print("  Number of SSIDs detected: "); boardSerialPort.println(scanReturn);
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
      //if (debugModeOn) {boardSerialPort.print(mySSID);boardSerialPort.print(" is equal than "); boardSerialPort.println(wifiNet.ssid);
      //             boardSerialPort.print("networkItem=");boardSerialPort.println(i);}
      wifiNetworkNode=i;
      break;
    }
    else {
      //if (debugModeOn) {boardSerialPort.print(mySSID);boardSerialPort.print(" is not equal than "); boardSerialPort.println(wifiNet.ssid);}
    }
  }

  if (-1==wifiNetworkNode) {
    //WiFi SSID was lost
    if (debugModeOn) {
      boardSerialPort.print("  [printCurrentWiFi] - SSID: ");boardSerialPort.print(wifiCred.wifiSSIDs[wifiCred.activeIndex]);boardSerialPort.println(" lost");
    }
    return nullptr;
  }

  // print current network parameters
  if (debugModeOn) {
    boardSerialPort.print("  [printCurrentWiFi] - Wifi Network Node: ");boardSerialPort.print(wifiNetworkNode);
    boardSerialPort.print(" and "); boardSerialPort.print(scanReturn); boardSerialPort.println(" detected");
    //boardSerialPort.print("SSID: ");boardSerialPort.println(WiFi.SSID());
    boardSerialPort.print("  [printCurrentWiFi] - SSID: ");boardSerialPort.println(wifiNet.ssid);

    // print the MAC address of the router you're attached to:
    uint8_t bssid[6];
    //memcpy(bssid, WiFi.BSSID(), 6);
    memcpy(bssid, wifiNet.BSSID, 6);
    boardSerialPort.print("  [printCurrentWiFi] - BSSID: ");
    boardSerialPort.print(bssid[5], HEX); boardSerialPort.print(":");
    boardSerialPort.print(bssid[4], HEX); boardSerialPort.print(":");
    boardSerialPort.print(bssid[3], HEX); boardSerialPort.print(":");
    boardSerialPort.print(bssid[2], HEX); boardSerialPort.print(":");
    boardSerialPort.print(bssid[1], HEX); boardSerialPort.print(":");
    boardSerialPort.println(bssid[0], HEX);

    // print the received signal strength:
    //boardSerialPort.print("signal strength (RSSI):");boardSerialPort.println((long) WiFi.RSSI());
    boardSerialPort.print("  [printCurrentWiFi] - Signal strength (RSSI): ");boardSerialPort.println(wifiNet.RSSI);

    // print the encryption type:
    boardSerialPort.print("  [printCurrentWiFi] - Encryption Type: ");boardSerialPort.println(wifiNet.encryptionType, HEX);

    // print the channel:
    boardSerialPort.print("  [printCurrentWiFi] - WiFi Channel: ");boardSerialPort.println(wifiNet.channel);
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
    errorsWiFiCnt++; //Something went wrong. Update error counter for stats
    return(ERROR_WIFI_SETUP);
  }

  // attempt to connect to Wifi network:
  for (uint8_t loopCounter=*auxLoopCounter; loopCounter<(uint8_t)sizeof(wifiCred.wifiSSIDs)/sizeof(String); loopCounter++) {
    counter=*auxCounter;
    errorWifiConnection=false;
    *auxLoopCounter=loopCounter;
    WiFi.begin(wifiCred.wifiSSIDs[loopCounter].c_str(), wifiCred.wifiPSSWs[loopCounter].c_str());

    long auxWhile=millis();
    while (status != WL_CONNECTED && counter < MAX_CONNECTION_ATTEMPTS) {
      if ((millis()-auxWhile)>=(MAX_CONNECTION_ATTEMPTS*1000/10)) { //Interval to show info and dots: MAX_CONNECTION_ATTEMPTS/10 *1000 milliseconds
        counter++;
        *auxCounter=counter;
        auxWhile=millis();
        if (debugModeOn) {boardSerialPort.print("  [wifiConnect] - Attempting to connect to WPA SSID: ");boardSerialPort.println(wifiCred.wifiSSIDs[loopCounter].c_str());}
      }
      status = WiFi.status();
    } //End of while() loop

    if (counter>=MAX_CONNECTION_ATTEMPTS) { //Case if while() loop timeout.
      if (debugModeOn) {
        boardSerialPort.print("  [wifiConnect] - WiFi network ERROR: ");
        boardSerialPort.print("No connection to SSID ");boardSerialPort.println(wifiCred.wifiSSIDs[loopCounter].c_str());
        boardSerialPort.print("  [wifiConnect] - Number of connection attempts ");
        boardSerialPort.print(counter);boardSerialPort.print(">");boardSerialPort.println(MAX_CONNECTION_ATTEMPTS);
      }
      errorWifiConnection=true;
      errorsWiFiCnt++; //Something went wrong. Update error counter for stats
      *auxCounter=0;  //Reset the while() counter to be ready for the next index of the SSID array  
    }
    else {
      //End of while() due to successful SSID connection
      // (Button Pressed or Display Refresh force the function to return from the while() loop, 
      //  so this point is not reached in those cases)
      wifiCred.activeIndex=loopCounter;
      loopCounter=sizeof(wifiCred.wifiSSIDs); //loop end
    }
  }//end For() loop

  //This point is reached if either the while() loop timed out or successful SSID connection
  if (errorWifiConnection) {
    //Case for while loop timeout (no successfull SSID connection)
    if (auxLoopCounter!=nullptr) *auxLoopCounter=0;  //To avoid resuming connection the next loop interacion
    if (auxCounter!=nullptr) *auxCounter=0;          //To avoid resuming connection the next loop interacion
    return(ERROR_WIFI_SETUP); //not wifi connection
  }

  //Case for successfull SSID connection
  if (debugModeOn) {
    int16_t numberWiFiNetworks;
    boardSerialPort.println("  [wifiConnect] - Connected to the network");
    boardSerialPort.println("  [wifiConnect] - WiFi info: "); printCurrentWiFi(true,&numberWiFiNetworks);
    boardSerialPort.print("  [wifiConnect] - Net info: \n");printNetData();
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
  if (debugModeOn) {if (!fromSetup) {boardSerialPort.print(String(millis()));boardSerialPort.println("  [setupNTPConfig] - CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+", errorsNTPCnt="+String(errorsNTPCnt)+", auxLoopCounter="+String(*auxLoopCounter)+", whileLoopTimeLeft="+String(*whileLoopTimeLeft));}}

  //to avoid pointer issues
  if (auxLoopCounter==nullptr || whileLoopTimeLeft==nullptr) {
    errorsNTPCnt++; //Something went wrong. Update error counter for stats
    return(ERROR_NTP_SERVER);
  }
  
  CloudClockCurrentStatus=CloudClockOffStatus;

  if (wifiCurrentStatus!=wifiOffStatus) {
    uint8_t auxForLoop;
    for (uint8_t loopCounter=*auxLoopCounter; loopCounter<(uint8_t)sizeof(ntpServers)/sizeof(String); loopCounter++) {
      if (ntpServers[loopCounter].charAt(0)=='\0') loopCounter++;
      else {
        if (debugModeOn) {boardSerialPort.println("  [setupNTPConfig] - Configuring configTzTime("+String(TZEnvVariable.c_str())+","+String(ntpServers[loopCounter].c_str())+") to connecting to NTP Server: "+ntpServers[loopCounter]);}
        configTzTime(TZEnvVariable.c_str(), ntpServers[loopCounter].c_str());

        *auxLoopCounter=loopCounter;
        uint64_t whileStartTime=millis(),auxTime=whileStartTime;
        if (*whileLoopTimeLeft>=NTP_CHECK_TIMEOUT) *whileLoopTimeLeft=NTP_CHECK_TIMEOUT;

        if (debugModeOn) {if (!fromSetup) {boardSerialPort.println("  [setupNTPConfig] - whileStartTime="+String(whileStartTime)+", *whileLoopTimeLeft="+String(*whileLoopTimeLeft)+", *auxLoopCounter="+String(*auxLoopCounter)+", loopCounter="+String(loopCounter)+", waiting for NTPStatus=SNTP_SYNC_STATUS_COMPLETED");}}
        while ( sntp_get_sync_status()!=SNTP_SYNC_STATUS_COMPLETED &&
              *whileLoopTimeLeft<=NTP_CHECK_TIMEOUT) {
          *whileLoopTimeLeft=*whileLoopTimeLeft-(millis()-auxTime);
          auxTime=millis();
          delay(50);
        } //end while() loop

        if (debugModeOn) {if (!fromSetup) {boardSerialPort.println(String(millis())+"  [setupNTPConfig] - End of wait - Elapsed Time: "+String(auxTime-whileStartTime));}}
        
        if (*whileLoopTimeLeft>NTP_CHECK_TIMEOUT) { //Case if while() loop timeout.
          if ((debugModeOn && fromSetup) || debugModeOn) {
            boardSerialPort.println("  [setupNTPConfig] - Time: Failed to get time");
            boardSerialPort.print("  [setupNTPConfig] - NTP: ");boardSerialPort.println("KO");
          }
        }
        else { 
          //End of while() due to successful NTP sync
          // (Button Pressed or Display Refresh force the function to return from the while() loop, 
          //  so this point is not reached in those cases)
          if ((debugModeOn && fromSetup) || debugModeOn) {
            boardSerialPort.print("  [setupNTPConfig] - NTP: ");boardSerialPort.println("OK");
            boardSerialPort.print("  [setupNTPConfig] - Time: ");getLocalTime(&nowTimeInfo);boardSerialPort.println(&nowTimeInfo,"%d/%m/%Y - %H:%M:%S");
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
    errorsNTPCnt++; //Something went wrong. Update error counter for stats   
    if (auxLoopCounter!=nullptr) *auxLoopCounter=0;                 //To avoid resuming connection the next loop interacion
    if (whileLoopTimeLeft!=nullptr) *whileLoopTimeLeft=NTP_CHECK_TIMEOUT;  //To avoid resuming connection the next loop interacion       
    return(ERROR_NTP_SERVER); //not NTP server connection
  }

  //Case for successfull NTP sync
  ntpSynced=true; //This flag is unset at boot time. If true, it means NTP syc was succesfull at least once
  if (debugModeOn) {
      if (!fromSetup) {
        boardSerialPort.println("  [setupNTPConfig] - CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+", lastTimeNTPCheck="+String(lastTimeNTPCheck)+", errorsNTPCnt="+String(errorsNTPCnt));
        boardSerialPort.print(String(millis()));
        boardSerialPort.print("  [setupNTPConfig] - Exit - Time:");getLocalTime(&nowTimeInfo);boardSerialPort.println(&nowTimeInfo,"%d/%m/%Y - %H:%M:%S");
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