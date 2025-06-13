/* Dealing with loop interactions

*/

#include "loop.h"

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;

void wifi_reconnect_period(bool debugModeOn) {
  /******************************************************
   Function wifi_reconnect_period
   Target: Regular actions every WIFI_RECONNECT_PERIOD seconds to recover WiFi connection
           forceWifiReconnect==true if:
             1) after ICON_STATUS_REFRESH_PERIOD
             2) after configuring WiFi = ON in the Config Menu
              3) or wake up from sleep (only by pressing buttons, no by timer)
              4) or previous WiFi re-connection try was ABORTED (button pressed) or BREAK (need to refresh display)
              5) after heap size was below ABSULUTE_MIN_HEAP_THRESHOLD
              6) after detection the webServer is down
   Parameters:
    debugModeOn: True to print out logs
   *****************************************************/

  if (debugModeOn) {
    printLogln(String(nowTimeGlobal)+" - [loop - WIFI_RECONNECT_PERIOD] - nowTimeGlobal-lastTimeWifiReconnectionCheck >= wifiReconnectPeriod ("+String(nowTimeGlobal-lastTimeWifiReconnectionCheck)+" >= "+String(wifiReconnectPeriod/1000)+" s)");
    printLogln("           + lastTimeWifiReconnectionCheck="+String(lastTimeWifiReconnectionCheck));
    printLogln("           + forceWifiReconnect="+String(forceWifiReconnect));
    printLogln("           + forceWebServerInit="+String(forceWebServerInit));
    printLogln("           + !firstBoot="+String(!firstBoot));
    printLogln("           + wifiCurrentStatus="+String(wifiCurrentStatus)+", wifiOffStatus=0");
    printLogln("           + WiFi.status()="+String(WiFi.status())+", WL_CONNECTED=3");
  }
  else printLogln(String(nowTimeGlobal)+" - [loop - WIFI_RECONNECT_PERIOD] - wifiCurrentStatus="+String(wifiCurrentStatus));

  //Update at begining to prevent accumulating delays in CHECK periods as this code might take long
  if (!wifiResuming) lastTimeWifiReconnectionCheck=nowTimeGlobal; //Only if the WiFi reconnection didn't ABORT or BREAK in the previous interaction
  
  wifiStatus previousWifiCurrentStatus=wifiCurrentStatus;
  if (CloudSyncLastStatus!=CloudSyncOffStatus) CloudSyncLastStatus=CloudSyncCurrentStatus;      //To update icon -v1.4.1
  if (CloudClockLastStatus!=CloudClockOffStatus) CloudClockLastStatus=CloudClockCurrentStatus;  //To update icon -v1.4.1
  //if (MqttSyncLastStatus!=MqttSyncOffStatus) MqttSyncLastStatus=MqttSyncCurrentStatus;          //To update icon -v1.4.1

  //If WiFi disconnected (wifiOffStatus), then re-connect
  //Conditions for wifiCurrentStatus==wifiOffStatus
  // - no found SSID in init()
  // - no found SSID when ICON_STATUS_REFRESH_PERIOD
  //WiFi.status() gets the WiFi status inmediatly. No need to scann WiFi networks

  //WiFi Reconnection
  if (debugModeOn) {printLogln(String(millis())+" - [loop - WIFI_RECONNECT_PERIOD] - auxLoopCounter="+String(auxLoopCounter)+", auxCounter="+String(auxCounter));}
  
  forceWEBTestCheck=false; //If WiFi reconnection is successfull, then check CLOUD server to update ICON. Decision is done below, if NO_ERROR
  switch(wifiConnect(false,&auxLoopCounter,&auxCounter)) {
    case ERROR_WIFI_SETUP:
      CloudClockCurrentStatus=CloudClockOffStatus;CloudSyncCurrentStatus=CloudSyncOffStatus;MqttSyncCurrentStatus=MqttSyncOffStatus; //To update rest of icons -v1.4.1
      wifiCurrentStatus=wifiOffStatus;
      forceWifiReconnect=false;
      wifiResuming=false;
      if (debugModeOn) {printLogln(String(millis())+" - [loop - WIFI_RECONNECT_PERIOD] - wifiConnect() finish with ERROR_WIFI_SETUP. wifiCurrentStatus="+String(wifiCurrentStatus)+", forceWifiReconnect="+String(forceWifiReconnect));}
    break;
    case NO_ERROR:
    default:
      if (WiFi.RSSI()>=WIFI_100_RSSI) wifiCurrentStatus=wifi100Status;
      else if (WiFi.RSSI()>=WIFI_075_RSSI) wifiCurrentStatus=wifi75Status;
      else if (WiFi.RSSI()>=WIFI_050_RSSI) wifiCurrentStatus=wifi50Status;
      else if (WiFi.RSSI()>=WIFI_025_RSSI) wifiCurrentStatus=wifi25Status;
      else if (WiFi.RSSI()<WIFI_000_RSSI) wifiCurrentStatus=wifi0Status;
      if (forceWifiReconnect) forceNTPCheck=true; //v0.9.9 - Force NTP sync after WiFi Connection
      forceWifiReconnect=false;
      wifiResuming=false;
      //Send HttpRequest to check the server status
      // The request updates CloudSyncCurrentStatus
      forceWEBTestCheck=true; //Will check CLOUD server in the next loop() interaction
      forceNTPCheck=true; //v1.4.1 - Force NTP sync after WiFi Connection
      if (debugModeOn) {printLogln(String(millis())+" - [loop - WIFI_RECONNECT_PERIOD] - wifiConnect() finish with NO_ERROR. wifiCurrentStatus="+String(wifiCurrentStatus)+", forceWEBTestCheck="+String(forceWEBTestCheck));}
    break;
  } 

  if (WiFi.status()==WL_CONNECTED && !mqttClient.connected() && mqttServerEnabled) { //Connect to MQTT broker again
    //mqttClient.connect();
    //mqttClientInit(false,debugModeOn,false,mqttTopicName,device,WiFi.localIP().toString());
    MqttSyncCurrentStatus=MqttSyncOffStatus; //Force to init MQTT server
    lastTimeMQTTCheck=nowTimeGlobal-MQTT_CHECK_PERIOD;
  }

  if (debugModeOn) {printLogln(String(millis())+" - [loop - WIFI_RECONNECT_PERIOD] - wifiReconnectPeriod - exit, lastTimeWifiReconnectionCheck="+String(lastTimeWifiReconnectionCheck));}
  else printLogln(String(nowTimeGlobal)+" - [loop - WIFI_RECONNECT_PERIOD] - Exit. wifiCurrentStatus="+String(wifiCurrentStatus));
}

void ntp_ko_check_period(bool debugModeOn) {
  /******************************************************
   Function ntp_ko_check_period
   Target: Regular actions every NTP_KO_CHECK_PERIOD seconds. Cheking if NTP is off or should be checked
           forceNTPCheck is true if:
              1) After NTP server config in firstSetup()
              2) If the previous NTP check was aborted due either Button action or Display Refresh
              2) WiFi has been setup ON in config menu
   Parameters:
    debugModeOn: Print out the logs or not
   *****************************************************/

  if (debugModeOn) {printLogln(String(nowTimeGlobal)+" - [loop - NTP_KO_CHECK_PERIOD] - Last lastTimeNTPCheck="+String(lastTimeNTPCheck)+", auxLoopCounter2="+String(auxLoopCounter2)+", whileLoopTimeLeft="+String(whileLoopTimeLeft));}
  else printLog(String(nowTimeGlobal)+" - [loop - NTP_KO_CHECK_PERIOD] - CloudClockCurrentStatus="+String(CloudClockCurrentStatus));
    
  //Update at begining to prevent accumulating delays in CHECK periods as this code might take long
  lastTimeNTPCheck=nowTimeGlobal;

  //setupNTPConfig if either
  // - Last NTP check failed (and currently there is no NTP sycn) - It's done always (in any Energy Mode)
  // - If in Full Energy Mode (USB power) and WiFI connected:
  //   + Every 6 hours in avarage per day (probability ~ 17% ==> prob= random(1,7)<2) 
  // - If in either Reduce or Save Energy Mode, everytime there's WiFi connecton to minimize the
  //     time drift due to Deep Sleep (0,337 s per Deep Sleep min).
  //   + In average, every wifiReconnectPeriod=300000 (5 min)
  // - If the previous NTP check was aborted due to Button action (forceNTPCheck=true)
  
  long auxRandom=random(0,7);
  if( wifiCurrentStatus!=wifiOffStatus && wifiEnabled && 
      (auxRandom <2 || CloudClockCurrentStatus==CloudClockOffStatus || forceNTPCheck ) ) {
    if (debugModeOn) {
      printLogln("           + setupNTPConfig() for NTP Sync");
      if (CloudClockCurrentStatus==CloudClockOffStatus) printLogln("           + Reason: CloudClockCurrentStatus==CloudClockOffStatus");
      if (forceNTPCheck) printLogln("           + Reason: forceNTPCheck");
      if (auxRandom<2) printLogln("           + Reason: auxRandom(="+String(auxRandom)+")<2");
    }
    forceNTPCheck=false;
    if (CloudClockCurrentStatus!=CloudClockSendStatus) CloudClockLastStatus=CloudClockCurrentStatus; //To enter in DISPLAY_ICONS_REFRESH_TIMEOUT in the next loop cycle
    CloudClockCurrentStatus=CloudClockSendStatus;
    switch(setupNTPConfig(debugModeOn,false,&auxLoopCounter2,&whileLoopTimeLeft)) { //NTP Sync and CloudClockCurrentStatus update
      case ERROR_NTP_SERVER:
        forceNTPCheck=false;
        previousCloudClockCurrentStatus=CloudClockOffStatus; //CloudClock Status to be back after DISPLAY_ICONS_REFRESH_TIMEOUT
        //CloudClockCurrentStatus=CloudClockOffStatus; //CloudClockCurrentStatus is updated in setupNTPConfig with either CloudClockOnStatus or CloudClockOffStatus
        if (debugModeOn) {printLogln(String(millis())+" - [loop - NTP_KO_CHECK_PERIOD] - setupNTPConfig() finish with ERROR_NTP_SERVER. CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+", forceNTPCheck="+String(forceNTPCheck));}
      break;
      case NO_ERROR:
        forceNTPCheck=false;
        previousCloudClockCurrentStatus=CloudClockOnStatus; //CloudClock Status to be back after DISPLAY_ICONS_REFRESH_TIMEOUT
        //CloudClockCurrentStatus=CloudClockOnStatus; //CloudClockCurrentStatus is updated in setupNTPConfig with either CloudClockOnStatus or CloudClockOffStatus
        if (debugModeOn) {printLogln(String(millis())+" - [loop - NTP_KO_CHECK_PERIOD] - setupNTPConfig() finish with NO_ERROR. CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+", forceNTPCheck="+String(forceNTPCheck));}
      break;
    }
    lastCloudClockChangeCheck=nowTimeGlobal; //Reset Cloud Clock Change Timeout

    getLocalTime(&nowTimeInfo);
    if (debugModeOn) {
      //printLogln(String(millis())+" - [loop - NTP_KO_CHECK_PERIOD] - errorsNTPCnt="+String(errorsNTPCnt)+", CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+", lastTimeNTPCheck="+String(lastTimeNTPCheck));
      printLog("        [loop - NTP_KO_CHECK_PERIOD] - "+String(ntpServers[ntpServerIndex])+" - ");printLogln(&nowTimeInfo,"- NTP sync done. Exit - Time: %d/%m/%Y - %H:%M:%S"); //boardSerialPort.println(&nowTimeInfo,"- NTP sync done. Exit - Time: %d/%m/%Y - %H:%M:%S");
    }
    else {printLog(&nowTimeInfo," - NTP sync done. Exit - Time: %d/%m/%Y - %H:%M:%S. ");printLogln(" - "+String(ntpServers[ntpServerIndex])+" - CloudClockCurrentStatus="+String(CloudClockCurrentStatus));}
  }
  else {
    if( wifiCurrentStatus==wifiOffStatus || !wifiEnabled) {if (forceNTPCheck) forceNTPCheck=false;} //v0.9.9 If no WiFi, don't enter in NTP_KO_CHECK_PERIOD even if it was BREAK or ABORT in previous intercation
    getLocalTime(&nowTimeInfo);
    if (debugModeOn) {
      ///printLogln(String(millis())+" - [loop - NTP_KO_CHECK_PERIOD] - errorsNTPCnt="+String(errorsNTPCnt)+", CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+", lastTimeNTPCheck="+String(lastTimeNTPCheck));
      printLog("        [loop - NTP_KO_CHECK_PERIOD] - "+String(ntpServers[ntpServerIndex])+" - ");printLogln(&nowTimeInfo,"- No need for NTP sync. Exit - Time: %d/%m/%Y - %H:%M:%S");
    }
    else {printLog(&nowTimeInfo," - No need for NTP sync. Exit - Time: %d/%m/%Y - %H:%M:%S. "); printLogln(" - "+String(ntpServers[ntpServerIndex])+" - CloudClockCurrentStatus="+String(CloudClockCurrentStatus));}
  }
}

int sendHttpRequest(bool debugModeOn, IPAddress server, uint16_t port, String httpRequest, bool fromSetup) {
 /******************************************************
   Function sendHttpRequest
   Target: Send HTTP updates to pandora when the Thermostate interrupt is triggered
   Parameters:
    debugModeOn: Print out the logs or not
    server: HTTP Cloud web server
    port: HTTP port
    fromSetup: True if called from setup
   Return: status
  *****************************************************/ 

  if (fromSetup) printLog(String(millis())+" -"); //no from Setup
  else printLog("       "); //from setup
  printLog("[loop - sendHttpRequest] - Sending HTTP request");
  if (!fromSetup) printLogln("");
  else printLogln(" '"+httpRequest+"', server="+IpAddress2String(server));

  if (httpCloudEnabled && wifiCurrentStatus!=wifiOffStatus && wifiEnabled) {  //Only if HTTP Cloud is enabled and WiFi is connected
    if (client.connect(server, port)) {
      if (debugModeOn) {printLogln("       [loop - sendHttpRequest] - connected to web server");}
      // Send a HTTP request:
      client.println(httpRequest);
      client.print("Host: "); client.println(IpAddress2String(server));
      client.println("User-Agent: Arduino/1.0");
      client.println("Accept-Language: es-es");
      client.println("Connection: close");
      client.println();
    }
    else {
      errorsHTTPUptsCnt++;EEPROM.write(0x537,errorsHTTPUptsCnt);eepromUpdate=true;  //Something went wrong. Update error counter for stats
      if (debugModeOn) {printLogln("       [loop - sendHttpRequest] - Not connected, errorsSampleUpts="+String(errorsHTTPUptsCnt));}
      return (ERROR_CLOUD_SERVER);
    }

    // if there are incoming bytes available
    // from the server, read them and print them:
    u_long nowMilliseconds,lastMilliseconds=millis();
    CloudSyncCurrentStatus=CloudSyncOnStatus;
    while (!client.available()){
      nowMilliseconds=millis();
      if (nowMilliseconds>=lastMilliseconds+HTTP_ANSWER_TIMEOUT) {
        //Too long with no server answer. Something was wrong. Changing icon
        CloudSyncCurrentStatus=CloudSyncOffStatus;
        if (debugModeOn && fromSetup) {printLogln("       NO Server answer\n       ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");}
        break;
      }
    }; //wait till there is server answer

    if (CloudSyncCurrentStatus==CloudSyncOnStatus) {
      if (debugModeOn && fromSetup) {printLog("       Server answer\n       ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n       ");}
      while (client.available()) {
        char c = client.read();
        if (debugModeOn && fromSetup) {if (c=='\n') printLog("\n       "); else printLog(String(c));}
      }
      if (debugModeOn && fromSetup) {printLogln("\n       ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");}
    }
    else {
      errorsHTTPUptsCnt++; EEPROM.write(0x537,errorsHTTPUptsCnt); eepromUpdate=true; //Something went wrong. Update error counter for stats
      return (ERROR_CLOUD_SERVER);
    }

    // if the server's disconnected, stop the client:
    if (!client.connected()) {
      if (debugModeOn && fromSetup) {printLogln("       [loop - sendHttpRequest] - Disconnecting from server. Bye!");}
      client.stop();
    }

    return (NO_ERROR);
  }
  else {
    if (debugModeOn) {printLogln("       [loop - sendHttpRequest] - No WiFi or HTTP Cloud updates are disabled.");}
    return (ERROR_CLOUD_SERVER);
  }
}
  
  
void thermostate_interrupt_triggered(bool debugModeOn) {
 /******************************************************
   Function thermostate_interrupt_triggered
   Target: Regular actions when the thermostate interrupt is triggered to check the value of Thermostate pin
   Parameters:
    debugModeOn: Print out the logs or not
   *****************************************************/ 
  int auxThermostatInterrupt=digitalRead(PIN_THERMOSTATE);
  String auxStatus,httpRequest=String(GET_REQUEST_TO_UPLOAD_SAMPLES);
  thermostateInterrupt=false;
  int32_t auxRebounds; //int32 instead of uint16 because auxRebouds might have negaative values. To construct the http request
  bool updateCloudServer=false;
  
  if (thermostateStatus) {auxStatus="ON";} else {auxStatus="OFF";}
  printLogln(String(millis())+" - [loop - thermostate_interrupt_triggered] - Interrupt detected - Reason: Thermostate="+String(auxThermostatInterrupt)+" Status (thermostateStatus) was="+auxStatus);
  
  //Update rebouds according to the current status
  if (auxThermostatInterrupt==1) {
    auxRebounds=rebounds;
    if (thermostateStatus) {
      if (debugModeOn) {printLogln("          [loop - thermostate_interrupt_triggered] - Strange, thermostate status was already "+auxStatus+", rebounds="+String(rebounds/2)+". Consider to increase THERMOSTATE_INTERRUPT_DELAY ("+String(THERMOSTATE_INTERRUPT_DELAY)+" ms)");}
    }
    else {
      thermostateStatus=true; auxStatus="ON";
      if (debugModeOn) {printLogln("          [loop - thermostate_interrupt_triggered] - Thermostate status goes to "+auxStatus+", rebounds="+String(rebounds/2));}
      rebounds=0;
      updateCloudServer=true;
    }
  }
  else {
    auxRebounds=-rebounds;
    if (thermostateStatus) {
      thermostateStatus=false; auxStatus="OFF";
      if (debugModeOn) {printLogln("          [loop - thermostate_interrupt_triggered] - Thermostate status goes to "+auxStatus+", rebounds="+String(rebounds/2));}  
      rebounds=0;
      updateCloudServer=true;
    }
    else {
      if (debugModeOn) {printLogln("          [loop - thermostate_interrupt_triggered] - Strange, thermostate status was already "+auxStatus+", rebounds="+String(rebounds/2)+". Consider to increase THERMOSTATE_INTERRUPT_DELAY ("+String(THERMOSTATE_INTERRUPT_DELAY)+" ms)");}
    }
  }

  //if (debugModeOn) {printLogln("          [loop - thermostate_interrupt_triggered] - updateCloudServer="+String(updateCloudServer)+", httpCloudEnabled="+String(httpCloudEnabled)+", CloudSyncCurrentStatus="+String(CloudSyncCurrentStatus)+" (0=CloudSyncOnStatus, 1=CloudSyncSendStatus, 2=CloudSyncOffStatus), wifiCred.activeIndex="+String(wifiCred.activeIndex));}
  //if (debugModeOn) {for (int i=0; i<=2; i++) printLogln("          [loop - thermostate_interrupt_triggered] - wifiCred.wifiSSIDs["+String(i)+"]="+String(wifiCred.wifiSSIDs[i])+", wifiCred.wifiSITEs["+String(i)+"]="+String(wifiCred.wifiSITEs[i])+", wifiCred.SiteAllow["+String(i)+"]="+String(wifiCred.SiteAllow[i]));}
  
  //Send the http cloud update
  if (updateCloudServer && httpCloudEnabled && (CloudSyncCurrentStatus==CloudSyncOnStatus) && wifiCred.SiteAllow[wifiCred.activeIndex]) {
    /*
      Thermostat ON  - HTTP Request => "GET /lar-to/?device=boiler-temp-relay&local_ip_address=192.168.100.192&relay_status=1&counts=101 HTTP/1.0"
      Thermostat OFF - HTTP Request => "GET /lar-to/?device=boiler-temp-relay&local_ip_address=192.168.100.192&relay_status=0&counts=-1737 HTTP/1.0"
    */
    httpRequest=httpRequest+"device="+device+"&local_ip_address="+IpAddress2String(WiFi.localIP())+
      "&relay_status="+String(auxThermostatInterrupt)+"&counts="+String(auxRebounds/2)+" HTTP/1.1";
    error_setup&=!ERROR_CLOUD_SERVER;
    error_setup|=sendHttpRequest(debugModeOn,serverToUploadSamplesIPAddress,SERVER_UPLOAD_PORT,httpRequest,false); //Send http update
    //CloudSyncCurrentStatus is updated in sendHttpRequest()
    lastTimeHTTPClouCheck=millis();
  }
}

void gas_sample(bool debugModeOn) {
 /******************************************************
   Function gas_sample
   Target: Regular actions every SAMPLE_PERIOD seconds to take gas samples
   Parameters:
    debugModeOn: Print out the logs or not
   *****************************************************/ 

  float h2_ppm=0,lpg_ppm=0,ch4_ppm=0,co_ppm=0,alcohol_ppm=0;
  String logMessage=String(nowTimeGlobal)+" - [loop - SAMPLE_PERIOD] - Taking GAS samples. - ";
  printLog(String(nowTimeGlobal)+" - [loop - SAMPLE_PERIOD] - Taking GAS samples. - ");
  
  //calculate_R0(); //This is to calculate R0 when the MQ5 sensor is replaced. Don't use it for regular working
  gasRatioSample=get_resistence_ratio(debugModeOn); //This is the current ratio RS/R0 - 6.5 for clean air
  if (gasRatioSample>4.5) {
    gasClear=1;
    if (debugModeOn) printLogln("\n         [loop - SAMPLE_PERIOD] - Clean air detected. Digital sensor input: "+String(digitalRead(PIN_GAS_SENSOR_D0))+" (1=NO GAS, 0=GAS)");
    else printLogln("Clean air detected. Digital sensor input: "+String(digitalRead(PIN_GAS_SENSOR_D0))+" (1=NO GAS, 0=GAS)");
  }
  else {
    gasClear=0;
    // 0 0 0 0  0 0 0 0 => gasTypes
    //     | |  | | | +----> H2   (0x01)
    //     | |  | | +------> LPG  (0x02)
    //     | |  | +--------> CH4  (0x04)
    //     | |  +----------> CO   (0x08)
    //     | +-------------> Alcohol (0x10)
    //     +---------------> Out of range - Invalid (0x2X)
    uint8_t gasTypes=0;

    h2_ppm=(gasRatioSample-H2_B)/H2_A; if (h2_ppm<0) h2_ppm=0; else if (h2_ppm>=200 && h2_ppm<10000) gasTypes|=0x01; else gasTypes|=0x20;
    lpg_ppm=(gasRatioSample-LPG_B)/LPG_A; if (lpg_ppm<0) lpg_ppm=0; else if (lpg_ppm>=200 && lpg_ppm<10000) gasTypes|=0x02; else gasTypes|=0x20;
    ch4_ppm=(gasRatioSample-CH4_B)/CH4_A; if (ch4_ppm<0) ch4_ppm=0; else if (ch4_ppm>=200 && ch4_ppm<10000) gasTypes|=0x04; else gasTypes|=0x20;
    co_ppm=(gasRatioSample-CO_B)/CO_A; if (co_ppm<0) co_ppm=0; else if (co_ppm>=200 && co_ppm<10000) gasTypes|=0x08; else gasTypes|=0x20;
    alcohol_ppm=(gasRatioSample-ALCOHOL_B)/ALCOHOL_A; if (alcohol_ppm<0) alcohol_ppm=0; else if (alcohol_ppm>=200 && alcohol_ppm<10000) gasTypes|=0x10; else gasTypes|=0x20;

    printLogln(""); //Just print new line
    if (gasTypes > 0 && gasTypes != 0x20) {
      printLogln("         [loop - SAMPLE_PERIOD] - GAS detected. Digital input: "+String(digitalRead(PIN_GAS_SENSOR_D0))+" (1=NO GAS, 0=GAS)");
      if ((gasTypes & 0x1) > 0) printLogln("         [loop - SAMPLE_PERIOD] -    H2="+String(h2_ppm)+" ppm");
      if ((gasTypes & 0x2) > 0) printLogln("         [loop - SAMPLE_PERIOD] -    LPG="+String(lpg_ppm)+" ppm");
      if ((gasTypes & 0x4) > 0) printLogln("         [loop - SAMPLE_PERIOD] -    CH4="+String(ch4_ppm)+" ppm");
      if ((gasTypes & 0x8) > 0) printLogln("         [loop - SAMPLE_PERIOD] -    CO="+String(co_ppm)+" ppm");
      if ((gasTypes & 0x10) > 0) printLogln("         [loop - SAMPLE_PERIOD] -    ALCOHOL="+String(alcohol_ppm)+" ppm");
    }
    if (gasTypes > 0x20)
      {printLogln("         [loop - SAMPLE_PERIOD] - Other GASes detected out of range 200-10000, so not valid. Digital input: "+String(digitalRead(PIN_GAS_SENSOR_D0))+" (1=NO GAS, 0=GAS)");}
    else if (gasTypes == 0x20)
      {printLogln("         [loop - SAMPLE_PERIOD] - GAS detected out of range 200-10000, so not valid. Digital input: "+String(digitalRead(PIN_GAS_SENSOR_D0))+" (1=NO GAS, 0=GAS)");}
    if ((h2_ppm>0 && h2_ppm<200) || h2_ppm>10000) printLogln("         [loop - SAMPLE_PERIOD] -    H2="+String(h2_ppm)+" ppm. Discard that value.");
    if ((lpg_ppm>0 && lpg_ppm<200) || lpg_ppm>10000) printLogln("         [loop - SAMPLE_PERIOD] -    LPG="+String(lpg_ppm)+" ppm. Discard that value.");
    if ((ch4_ppm>0 && ch4_ppm<200) || ch4_ppm>10000) printLogln("         [loop - SAMPLE_PERIOD] -    CH4="+String(ch4_ppm)+" ppm. Discard that value.");
    if ((co_ppm>0 && co_ppm<200) || co_ppm>10000) printLogln("         [loop - SAMPLE_PERIOD] -    CO="+String(co_ppm)+" ppm. Discard that value.");
    if ((alcohol_ppm>0 && alcohol_ppm<200) || alcohol_ppm>10000) printLogln("         [loop - SAMPLE_PERIOD] -    ALCOHOL="+String(alcohol_ppm)+" ppm. Discard that value.");
  }

  //Updating JSON object with samples
  struct tm nowTimeInfo; //36 B
  char s[100];getLocalTime(&nowTimeInfo);strftime(s,sizeof(s),"%d/%m/%Y - %H:%M:%S",&nowTimeInfo);
  samples["dateUpdate"] =  String(s);
  char ss[100];strftime(ss,sizeof(ss),"%d/%m/%Y - %H:%M:%S",&startTimeInfo);
  samples["startTime"] =  String(ss);
  ulong nowInSeconds=millis()/1000; //In seconds
  uint8_t years=nowInSeconds/31536000; //Number of years
  uint8_t months=(nowInSeconds-years*31536000)/2592000; //Number of months
  uint8_t days=(nowInSeconds-years*31536000-months*2592000)/86400; //Number of days
  uint8_t hours=(nowInSeconds-years*31536000-months*2592000-days*86400)/3600; //Number of hours
  uint8_t minutes=(nowInSeconds-years*31536000-months*2592000-days*86400-hours*3600)/60; //Number of minutes
  uint8_t seconds=(nowInSeconds-years*31536000-months*2592000-days*86400-hours*3600-minutes*60); //Number of seconds
  //Uptime in format YY-MM-DDThh:mm:ss
  char upTime[20];
  sprintf(upTime,"%04d-%02d-%02dT%02d:%02d:%02d",years,months,days,hours,minutes,seconds);
  upTime[19]='\0';
  samples["upTime"] =  String(upTime);
  samples["upTimeSeconds"] =  String(nowInSeconds);
  samples["device_name"] = device;
  samples["version"] = String(VERSION);
  samples["ipAddress"] = WiFi.localIP().toString();
  samples["boilerStatus"] = boilerStatus==true?"ON":"OFF";
  samples["boilerOn"] = boilerOn==true?"ON":"OFF";
  samples["H2"] = String(h2_ppm);
  samples["LPG"] = String(lpg_ppm);
  samples["CH4"] = String(ch4_ppm);
  samples["CO"] = String(co_ppm);
  samples["ALCOHOL"] = String(alcohol_ppm);
  samples["Clean_air"] = gasClear==1?"OFF":"ON";
  samples["GAS_interrupt"] = gasInterrupt==1?"ON":"OFF";
  iconGasInterrupt=gasInterrupt==1?String("mdi:electric-switch-closed"):String("mdi:electric-switch");
  samples["Thermostate_interrupt"] = thermostateInterrupt==true?"ON":"OFF";
  iconThermInterrupt=thermostateInterrupt==1?String("mdi:electric-switch-closed"):String("mdi:electric-switch");
  samples["Thermostate_status"] = thermostateStatus==true?"ON":"OFF";
  if (thermostateStatus) iconThermStatus=String("mdi:radiator");
  else {if (digitalRead(PIN_RL1) && !digitalRead(PIN_RL2)) iconThermStatus=String("mdi:radiator-off"); else iconThermStatus=String("mdi:radiator-disabled");}
  samples["Thermostate_on"] = thermostateOn==true?"ON":"OFF";
  samples["SSID"] = WiFi.SSID();
  wifiNet.RSSI=WiFi.RSSI();
  samples["SIGNAL"] = String(wifiNet.RSSI);
  if (wifiNet.RSSI>=WIFI_100_RSSI) wifiCurrentStatus=wifi100Status;
  else if (wifiNet.RSSI>=WIFI_075_RSSI) wifiCurrentStatus=wifi75Status;
  else if (wifiNet.RSSI>=WIFI_050_RSSI) wifiCurrentStatus=wifi50Status;
  else if (wifiNet.RSSI>=WIFI_025_RSSI) wifiCurrentStatus=wifi25Status;
  else if (wifiNet.RSSI<WIFI_000_RSSI) wifiCurrentStatus=wifi0Status;
  switch (wifiCurrentStatus)
  {
    case wifi0Status:
      samples["RSSI"] = "0";
      iconWifi=String("mdi:wifi-strength-alert-outline");
      break;
    case wifi25Status:
      samples["RSSI"] = "25";
      iconWifi=String("mdi:wifi-strength-1-alert");
      break;
    case wifi50Status:
      samples["RSSI"] = "50";
      iconWifi=String("mdi:wifi-strength-2");
      break;
    case wifi75Status:
      samples["RSSI"] = "75";
      iconWifi=String("mdi:wifi-strength-3");
      break;
    case wifi100Status:
      samples["RSSI"] = "100";
      iconWifi=String("mdi:wifi-strength-4");
      break;
    default:
      samples["RSSI"] = "O";
      iconWifi=String("mdi:wifi-strength-alert-outline");
      break;
  }
  samples["BSSID"] = WiFi.BSSIDstr();
  switch (wifiNet.encryptionType)
  {
    case WIFI_AUTH_OPEN:
      samples["ENCRYPTION"] = "OPEN";
      break;
    case WIFI_AUTH_WEP:
      samples["ENCRYPTION"] = "WEP";
      break;
    case WIFI_AUTH_WPA_PSK:
      samples["ENCRYPTION"] = "";
      break;
    case WIFI_AUTH_WPA2_PSK:
      samples["ENCRYPTION"] = "WPA2";
      break;
    case WIFI_AUTH_WPA_WPA2_PSK:
      samples["ENCRYPTION"] = "WPA+WPA2";
      break;
    case WIFI_AUTH_WPA2_ENTERPRISE:
      samples["ENCRYPTION"] = "WPA2-EAP";
      break;
    case WIFI_AUTH_WPA3_PSK:
      samples["ENCRYPTION"] = "WPA3";
      break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
      samples["ENCRYPTION"] = "WPA2+WPA3";
      break;
    case WIFI_AUTH_WAPI_PSK:
      samples["ENCRYPTION"] = "WAPI";
      break;
    default:
      samples["ENCRYPTION"] = "UNKNOWN";
  }
  samples["CHANNEL"] = String(WiFi.channel());
  switch (WiFi.getMode())
  {
    case WIFI_MODE_STA:
      samples["MODE"] = "STATION";
      break;
    case WIFI_MODE_AP:
      samples["MODE"] = "ACCESS POINT";
      break;
    case WIFI_MODE_APSTA:
      samples["MODE"] = "AP+STA";
      break;
    case WIFI_MODE_MAX:
      samples["MODE"] = "MAX";
      break;
    case WIFI_MODE_NULL:
    default:
      samples["MODE"] = "NULL";
      break;
  }
  samples["NTP"] = CloudClockCurrentStatus==CloudClockOffStatus?String("OFF"):String("ON");
  samples["HTTP_CLOUD"] = CloudSyncCurrentStatus==CloudSyncOffStatus?String("OFF"):String("ON");
  samples["Relay1"] = digitalRead(PIN_RL1)==0?String("R1_ON"):String("R1_OFF"); //ON = External Thermostate Not Allowed (OFF)
  samples["Relay2"] = digitalRead(PIN_RL2)==1?String("R2_ON"):String("R2_OFF"); //ON = External Thermostate shortcut (ON)
  samples["errorsWiFiCnt"] = String(errorsWiFiCnt);
  samples["errorsConnectivityCnt"] = String(errorsConnectivityCnt);
  samples["errorsNTPCnt"] = String(errorsNTPCnt);
  samples["errorsHTTPUptsCnt"] = String(errorsHTTPUptsCnt);
  samples["errorsMQTTCnt"] = String(errorsMQTTCnt);
  samples["errorsWebServerCnt"] = String(errorsWebServerCnt);
  samples["bootCount"] = String(bootCount); //Total since last update
  samples["resetNormalCount"] = String(bootCount-resetSWCount-resetCount); //Normal resets - resetSWCount includes preventive Counts
  samples["resetSWCount"] = String(resetSWCount); //Reset due to Restart HA Button, web reset, preventive reset, etc (all ESP.restart cases)
  samples["resetSWWebCount"] = String(resetSWWebCount); //resets done from the web maintenance page
  samples["resetSWMqttCount"] = String(resetSWMqttCount); //resets done from the HA (mqqtt) page
  samples["resetSWUpgradeCount"] = String(resetSWUpgradeCount); //resets done due to firmware upgrade from maintenance web page
  samples["resetWebServerCount"] = String(resetWebServerCnt); //resets due to web server not serving web pages
  samples["resetPreventiveCount"] = String(resetPreventiveCount); //Preventive resets (low heap situation) different than web server low heap
  samples["resetPreventiveWebServerCount"] = String(resetPreventiveWebServerCount); //Preventive web server resets (low heap situation)
  samples["resetCount"] = String(resetCount); //uncontrolled resets
  samples["lastHeap"] = String(esp_get_free_heap_size());
  samples["minHeapSinceBoot"] = String(minHeapSinceBoot);
  samples["minHeapSinceUpgrade"] = String(minHeapSinceUpgrade);
  samples["reboot"] = String("online");
  samples["reset_time_counters"] = String("online");
  
  uint32_t auxTimeOn=0; for (int i=0;i<12;i++) auxTimeOn+=heaterTimeOnYear.counterMonths[i];
  samples["heaterYear"] = String(heaterTimeOnYear.year);
  samples["heaterYesterday"] = String(heaterTimeOnYear.yesterday);
  samples["heaterToday"] = String(heaterTimeOnYear.today);
  samples["heaterOnYear"] = String(auxTimeOn);
  samples["heaterOnYearJan"] = String(heaterTimeOnYear.counterMonths[0]);samples["heaterOnYearFeb"] = String(heaterTimeOnYear.counterMonths[1]);samples["heaterOnYearMar"] = String(heaterTimeOnYear.counterMonths[2]);samples["heaterOnYearApr"] = String(heaterTimeOnYear.counterMonths[3]);samples["heaterOnYearMay"] = String(heaterTimeOnYear.counterMonths[4]);samples["heaterOnYearJun"] = String(heaterTimeOnYear.counterMonths[5]);
  samples["heaterOnYearJul"] = String(heaterTimeOnYear.counterMonths[6]);samples["heaterOnYearAug"] = String(heaterTimeOnYear.counterMonths[7]);samples["heaterOnYearSep"] = String(heaterTimeOnYear.counterMonths[8]);samples["heaterOnYearOct"] = String(heaterTimeOnYear.counterMonths[9]);samples["heaterOnYearNov"] = String(heaterTimeOnYear.counterMonths[10]);samples["heaterOnYearDec"] = String(heaterTimeOnYear.counterMonths[11]);
  samples["heaterOnYesterday"] = String(heaterTimeOnYear.counterYesterday);
  samples["heaterOnToday"] = String(heaterTimeOnYear.counterToday);
  auxTimeOn=0; for (int i=0;i<12;i++) auxTimeOn+=heaterTimeOnPreviousYear.counterMonths[i];
  samples["heaterPreviousYear"] = String(heaterTimeOnPreviousYear.year);
  samples["heaterOnPreviousYear"] = String(auxTimeOn);
  samples["heaterOnPreviousYearJan"] = String(heaterTimeOnPreviousYear.counterMonths[0]);samples["heaterOnPreviousYearFeb"] = String(heaterTimeOnPreviousYear.counterMonths[1]);samples["heaterOnPreviousYearMar"] = String(heaterTimeOnPreviousYear.counterMonths[2]);samples["heaterOnPreviousYearApr"] = String(heaterTimeOnPreviousYear.counterMonths[3]);samples["heaterOnPreviousYearMay"] = String(heaterTimeOnPreviousYear.counterMonths[4]);samples["heaterOnPreviousYearJun"] = String(heaterTimeOnPreviousYear.counterMonths[5]);
  samples["heaterOnPreviousYearJul"] = String(heaterTimeOnPreviousYear.counterMonths[6]);samples["heaterOnPreviousYearAug"] = String(heaterTimeOnPreviousYear.counterMonths[7]);samples["heaterOnPreviousYearSep"] = String(heaterTimeOnPreviousYear.counterMonths[8]);samples["heaterOnPreviousYearOct"] = String(heaterTimeOnPreviousYear.counterMonths[9]);samples["heaterOnPreviousYearNov"] = String(heaterTimeOnPreviousYear.counterMonths[10]);samples["heaterOnPreviousYearDec"] = String(heaterTimeOnPreviousYear.counterMonths[11]);
  
  auxTimeOn=0; for (int i=0;i<12;i++) auxTimeOn+=boilerTimeOnYear.counterMonths[i];
  samples["boilerYear"] = String(boilerTimeOnYear.year);
  samples["boilerYesterday"] = String(boilerTimeOnYear.yesterday);
  samples["boilerToday"] = String(boilerTimeOnYear.today);
  samples["boilerOnYear"] = String(auxTimeOn);
  samples["boilerOnYearJan"] = String(boilerTimeOnYear.counterMonths[0]);samples["boilerOnYearFeb"] = String(boilerTimeOnYear.counterMonths[1]);samples["boilerOnYearMar"] = String(boilerTimeOnYear.counterMonths[2]);samples["boilerOnYearApr"] = String(boilerTimeOnYear.counterMonths[3]);samples["boilerOnYearMay"] = String(boilerTimeOnYear.counterMonths[4]);samples["boilerOnYearJun"] = String(boilerTimeOnYear.counterMonths[5]);
  samples["boilerOnYearJul"] = String(boilerTimeOnYear.counterMonths[6]);samples["boilerOnYearAug"] = String(boilerTimeOnYear.counterMonths[7]);samples["boilerOnYearSep"] = String(boilerTimeOnYear.counterMonths[8]);samples["boilerOnYearOct"] = String(boilerTimeOnYear.counterMonths[9]);samples["boilerOnYearNov"] = String(boilerTimeOnYear.counterMonths[10]);samples["boilerOnYearDec"] = String(boilerTimeOnYear.counterMonths[11]);
  samples["boilerOnYesterday"] = String(boilerTimeOnYear.counterYesterday);
  samples["boilerOnToday"] = String(boilerTimeOnYear.counterToday);
  auxTimeOn=0; for (int i=0;i<12;i++) auxTimeOn+=boilerTimeOnPreviousYear.counterMonths[i];
  samples["boilerPreviousYear"] = String(boilerTimeOnPreviousYear.year);
  samples["boilerOnPreviousYear"] = String(auxTimeOn);
  samples["boilerOnPreviousYearJan"] = String(boilerTimeOnPreviousYear.counterMonths[0]);samples["boilerOnPreviousYearFeb"] = String(boilerTimeOnPreviousYear.counterMonths[1]);samples["boilerOnPreviousYearMar"] = String(boilerTimeOnPreviousYear.counterMonths[2]);samples["boilerOnPreviousYearApr"] = String(boilerTimeOnPreviousYear.counterMonths[3]);samples["boilerOnPreviousYearMay"] = String(boilerTimeOnPreviousYear.counterMonths[4]);samples["boilerOnPreviousYearJun"] = String(boilerTimeOnPreviousYear.counterMonths[5]);
  samples["boilerOnPreviousYearJul"] = String(boilerTimeOnPreviousYear.counterMonths[6]);samples["boilerOnPreviousYearAug"] = String(boilerTimeOnPreviousYear.counterMonths[7]);samples["boilerOnPreviousYearSep"] = String(boilerTimeOnPreviousYear.counterMonths[8]);samples["boilerOnPreviousYearOct"] = String(boilerTimeOnPreviousYear.counterMonths[9]);samples["boilerOnPreviousYearNov"] = String(boilerTimeOnPreviousYear.counterMonths[10]);samples["boilerOnPreviousYearDec"] = String(boilerTimeOnPreviousYear.counterMonths[11]);

  //Get Energy readings from the SmartPlug if: powerMeasureEnabled, exists SmartPlug's IP, exists connectivity
  /*
  http://192.168.100.207/cm?cmnd=powerinfo
    {"PowerInfo":{"Power":8.400,"Voltage":326,"Current":0.070,"ReactivePower":21.200,"ApparentPower":22.848,"Factor":0.368}}
  http://192.168.100.207/cm?cmnd=energyinfo
    {"EnergyInfo":{"TotalStartTime":"2024-07-25T11:21:51","Total":0.759,"Yesterday":0.293,"Today":0.070}}
  
  */
  /*voltage=JSON.stringify(auxEnergyJson["ENERGY"]["Voltage"]).toInt();
  current=JSON.stringify(auxEnergyJson["ENERGY"]["Current"]).toFloat();
  power=JSON.stringify(auxEnergyJson["ENERGY"]["Power"]).toInt();
  energyToday=JSON.stringify(auxEnergyJson["ENERGY"]["Today"]).toFloat();
  energyYesterday=JSON.stringify(auxEnergyJson["ENERGY"]["Yesterday"]).toFloat();
  energyTotal=JSON.stringify(auxEnergyJson["ENERGY"]["Total"]).toFloat();;
  */
  samples["Voltage"]=voltage; 
  samples["Current"]=current;
  samples["Power"]=power;
  samples["EnergyToday"]=energyToday;
  samples["EnergyYesterday"]=energyYesterday;
  samples["EnergyTotal"]=energyTotal;
  samples["powerMeasureEnabled"]=powerMeasureEnabled;
  samples["powerMeasureSubscribed"]=powerMeasureSubscribed;
} // -- gas_sample -- 

void temperature_sample(bool debugModeOn) {
 /******************************************************
   Function temperature_sample
   Target: Take temp and hum samples
   Parameters:
    debugModeOn: Print out the logs or not
   *****************************************************/ 

  printLog(String(millis())+" - [loop - temperature_sample] - Taking Temp & Hum samples.");
  
  tempHumSensor.read();
   
  valueHum=tempHumSensor.getHumidityCompensated();
  tempSensor=tempHumSensor.getTemperature(); //Sample got from the sensor
  valueT=0.9944*tempHumSensor.getTemperature()-0.8073; //Calibrated value from IoT_Co2_Sensor

  if (valueT < -50.0) valueT=-50;  //Discarding potential wrong values
  //if (debugModeOn) {printLogln("         [loop - temperature_sample] - valueT="+String(valueT)+", valueHum="+String(valueHum));}
  printLogln(" Temp="+String(valueT)+"ºC, Hum="+String(valueHum)+"%");

  //Updating JSON object with samples
  samples["tempSensor"] = String(tempSensor); //Non-calibrated temp
  samples["temperature"] = String(valueT);    //Calibrated temp
  samples["humidity"] =  String(valueHum);    //Non-calibrated = calibrated hum
}


void mqtt_publish_samples(boolean wifiEnabled, boolean mqttServerEnabled, boolean secureMqttEnabled, bool debugModeOn) {
  /******************************************************
   Function mqtt_publish_samples
   Target: Publish the samples to the MQTT server
   Parameters:
    wifiEnabled: True if wifi is enabled
    mqttServerEnabled: True if mqtt is enabled
    debugModeOn: Print out the logs or not
   *****************************************************/ 

  struct tm nowTimeInfo; //36 B
  char s[100];
  printLogln(String(millis())+" - [loop - mqtt_publish_samples] - Publishing samples.");
  
  if (wifiEnabled && mqttServerEnabled && WiFi.status()==WL_CONNECTED) {
    MqttSyncCurrentStatus=MqttSyncSendStatus;
    
    if (mqttClient.connected()) {
      //MQTT Client connected
      //mqttTopicName=the-iot-factory/boiler-relay-controlv2-E02940

      //Publish sample values message under mqttTopicName (the-iot-factory propietary), not retain in the server
      mqttClient.publish(String(mqttTopicName+"/dateUpdate").c_str(), 0, false, JSON.stringify(samples["dateUpdate"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/device_name").c_str(), 0, false, JSON.stringify(samples["device_name"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/version").c_str(), 0, false, JSON.stringify(samples["version"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/startTime").c_str(), 0, false, JSON.stringify(samples["startTime"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/upTime").c_str(), 0, false, JSON.stringify(samples["upTime"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/upTimeSeconds").c_str(), 0, false, JSON.stringify(samples["upTimeSeconds"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/H2").c_str(), 0, false, JSON.stringify(samples["H2"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/LPG").c_str(), 0, false, JSON.stringify(samples["LPG"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/CH4").c_str(), 0, false, JSON.stringify(samples["CH4"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/CO").c_str(), 0, false, JSON.stringify(samples["CO"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/ALCOHOL").c_str(), 0, false, JSON.stringify(samples["ALCOHOL"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/Clean_air").c_str(), 0, false, JSON.stringify(samples["Clean_air"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/GAS_interrupt").c_str(), 0, false, JSON.stringify(samples["GAS_interrupt"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/Thermostate_interrupt").c_str(), 0, false, JSON.stringify(samples["Thermostate_interrupt"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/Thermostate_status").c_str(), 0, false, JSON.stringify(samples["Thermostate_status"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/SSID").c_str(), 0, false, JSON.stringify(samples["SSID"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/BSSID").c_str(), 0, false, JSON.stringify(samples["BSSID"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/SIGNAL").c_str(), 0, false, JSON.stringify(samples["SIGNAL"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/RSSI").c_str(), 0, false, JSON.stringify(samples["RSSI"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/ENCRYPTION").c_str(), 0, false, JSON.stringify(samples["ENCRYPTION"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/CHANNEL").c_str(), 0, false, JSON.stringify(samples["CHANNEL"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/MODE").c_str(), 0, false, JSON.stringify(samples["MODE"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/tempSensor").c_str(), 0, false, JSON.stringify(samples["tempSensor"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/temperature").c_str(), 0, false, JSON.stringify(samples["temperature"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/humidity").c_str(), 0, false, JSON.stringify(samples["humidity"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/NTP").c_str(), 0, false, JSON.stringify(samples["NTP"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/HTTP_CLOUD").c_str(), 0, false, JSON.stringify(samples["HTTP_CLOUD"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/Relay1").c_str(), 0, false, JSON.stringify(samples["Relay1"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/Relay2").c_str(), 0, false, JSON.stringify(samples["Relay2"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/errorsWiFiCnt").c_str(), 0, false, JSON.stringify(samples["errorsWiFiCnt"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/errorsConnectivityCnt").c_str(), 0, false, JSON.stringify(samples["errorsConnectivityCnt"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/errorsNTPCnt").c_str(), 0, false, JSON.stringify(samples["errorsNTPCnt"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/errorsHTTPUptsCnt").c_str(), 0, false, JSON.stringify(samples["errorsHTTPUptsCnt"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/errorsMQTTCnt").c_str(), 0, false, JSON.stringify(samples["errorsMQTTCnt"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/errorsWebServerCnt").c_str(), 0, false, JSON.stringify(samples["errorsWebServerCnt"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/bootCount").c_str(), 0, false, JSON.stringify(samples["bootCount"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/resetNormalCount").c_str(), 0, false, JSON.stringify(samples["resetNormalCount"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/resetPreventiveCount").c_str(), 0, false, JSON.stringify(samples["resetPreventiveCount"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/resetPreventiveWebServerCount").c_str(), 0, false, JSON.stringify(samples["resetPreventiveWebServerCount"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/resetSWCount").c_str(), 0, false, JSON.stringify(samples["resetSWCount"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/resetCount").c_str(), 0, false, JSON.stringify(samples["resetCount"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/minHeapSinceBoot").c_str(), 0, false, JSON.stringify(samples["minHeapSinceBoot"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/minHeapSinceUpgrade").c_str(), 0, false, JSON.stringify(samples["minHeapSinceUpgrade"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/lastHeap").c_str(), 0, false, JSON.stringify(samples["lastHeap"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/resetReason").c_str(), 0, false, JSON.stringify(samples["resetReason"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/reboot").c_str(), 0, false, JSON.stringify(samples["reboot"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/heaterYear").c_str(), 0, false, JSON.stringify(samples["heaterYear"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/heaterYesterday").c_str(), 0, false, JSON.stringify(samples["heaterYesterday"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/heaterToday").c_str(), 0, false, JSON.stringify(samples["heaterToday"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/heaterOnYear").c_str(), 0, false, JSON.stringify(samples["heaterOnYear"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/heaterOnYearJan").c_str(), 0, false, JSON.stringify(samples["heaterOnYearJan"]).c_str());mqttClient.publish(String(mqttTopicName+"/heaterOnYearFeb").c_str(), 0, false, JSON.stringify(samples["heaterOnYearFeb"]).c_str());mqttClient.publish(String(mqttTopicName+"/heaterOnYearMar").c_str(), 0, false, JSON.stringify(samples["heaterOnYearMar"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/heaterOnYearApr").c_str(), 0, false, JSON.stringify(samples["heaterOnYearApr"]).c_str());mqttClient.publish(String(mqttTopicName+"/heaterOnYearMay").c_str(), 0, false, JSON.stringify(samples["heaterOnYearMay"]).c_str());mqttClient.publish(String(mqttTopicName+"/heaterOnYearJun").c_str(), 0, false, JSON.stringify(samples["heaterOnYearJun"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/heaterOnYearJul").c_str(), 0, false, JSON.stringify(samples["heaterOnYearJul"]).c_str());mqttClient.publish(String(mqttTopicName+"/heaterOnYearAug").c_str(), 0, false, JSON.stringify(samples["heaterOnYearAug"]).c_str());mqttClient.publish(String(mqttTopicName+"/heaterOnYearSep").c_str(), 0, false, JSON.stringify(samples["heaterOnYearSep"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/heaterOnYearOct").c_str(), 0, false, JSON.stringify(samples["heaterOnYearOct"]).c_str());mqttClient.publish(String(mqttTopicName+"/heaterOnYearNov").c_str(), 0, false, JSON.stringify(samples["heaterOnYearNov"]).c_str());mqttClient.publish(String(mqttTopicName+"/heaterOnYearDec").c_str(), 0, false, JSON.stringify(samples["heaterOnYearDec"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/heaterOnYesterday").c_str(), 0, false, JSON.stringify(samples["heaterOnYesterday"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/heaterOnToday").c_str(), 0, false, JSON.stringify(samples["heaterOnToday"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/heaterPreviousYear").c_str(), 0, false, JSON.stringify(samples["heaterPreviousYear"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/heaterOnPreviousYear").c_str(), 0, false, JSON.stringify(samples["heaterOnPreviousYear"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/heaterOnPreviousYearJan").c_str(), 0, false, JSON.stringify(samples["heaterOnPreviousYearJan"]).c_str());mqttClient.publish(String(mqttTopicName+"/heaterOnPreviousYearFeb").c_str(), 0, false, JSON.stringify(samples["heaterOnPreviousYearFeb"]).c_str());mqttClient.publish(String(mqttTopicName+"/heaterOnPreviousYearMar").c_str(), 0, false, JSON.stringify(samples["heaterOnPreviousYearMar"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/heaterOnPreviousYearApr").c_str(), 0, false, JSON.stringify(samples["heaterOnPreviousYearApr"]).c_str());mqttClient.publish(String(mqttTopicName+"/heaterOnPreviousYearMay").c_str(), 0, false, JSON.stringify(samples["heaterOnPreviousYearMay"]).c_str());mqttClient.publish(String(mqttTopicName+"/heaterOnPreviousYearJun").c_str(), 0, false, JSON.stringify(samples["heaterOnPreviousYearJun"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/heaterOnPreviousYearJul").c_str(), 0, false, JSON.stringify(samples["heaterOnPreviousYearJul"]).c_str());mqttClient.publish(String(mqttTopicName+"/heaterOnPreviousYearAug").c_str(), 0, false, JSON.stringify(samples["heaterOnPreviousYearAug"]).c_str());mqttClient.publish(String(mqttTopicName+"/heaterOnPreviousYearSep").c_str(), 0, false, JSON.stringify(samples["heaterOnPreviousYearSep"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/heaterOnPreviousYearOct").c_str(), 0, false, JSON.stringify(samples["heaterOnPreviousYearOct"]).c_str());mqttClient.publish(String(mqttTopicName+"/heaterOnPreviousYearNov").c_str(), 0, false, JSON.stringify(samples["heaterOnPreviousYearNov"]).c_str());mqttClient.publish(String(mqttTopicName+"/heaterOnPreviousYearDec").c_str(), 0, false, JSON.stringify(samples["heaterOnPreviousYearDec"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/boilerYear").c_str(), 0, false, JSON.stringify(samples["boilerYear"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/boilerYesterday").c_str(), 0, false, JSON.stringify(samples["boilerYesterday"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/boilerToday").c_str(), 0, false, JSON.stringify(samples["boilerToday"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/boilerOnYear").c_str(), 0, false, JSON.stringify(samples["boilerOnYear"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/boilerOnYearJan").c_str(), 0, false, JSON.stringify(samples["boilerOnYearJan"]).c_str());mqttClient.publish(String(mqttTopicName+"/boilerOnYearFeb").c_str(), 0, false, JSON.stringify(samples["boilerOnYearFeb"]).c_str());mqttClient.publish(String(mqttTopicName+"/boilerOnYearMar").c_str(), 0, false, JSON.stringify(samples["boilerOnYearMar"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/boilerOnYearApr").c_str(), 0, false, JSON.stringify(samples["boilerOnYearApr"]).c_str());mqttClient.publish(String(mqttTopicName+"/boilerOnYearMay").c_str(), 0, false, JSON.stringify(samples["boilerOnYearMay"]).c_str());mqttClient.publish(String(mqttTopicName+"/boilerOnYearJun").c_str(), 0, false, JSON.stringify(samples["boilerOnYearJun"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/boilerOnYearJul").c_str(), 0, false, JSON.stringify(samples["boilerOnYearJul"]).c_str());mqttClient.publish(String(mqttTopicName+"/boilerOnYearAug").c_str(), 0, false, JSON.stringify(samples["boilerOnYearAug"]).c_str());mqttClient.publish(String(mqttTopicName+"/boilerOnYearSep").c_str(), 0, false, JSON.stringify(samples["boilerOnYearSep"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/boilerOnYearOct").c_str(), 0, false, JSON.stringify(samples["boilerOnYearOct"]).c_str());mqttClient.publish(String(mqttTopicName+"/boilerOnYearNov").c_str(), 0, false, JSON.stringify(samples["boilerOnYearNov"]).c_str());mqttClient.publish(String(mqttTopicName+"/boilerOnYearDec").c_str(), 0, false, JSON.stringify(samples["boilerOnYearDec"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/boilerOnYesterday").c_str(), 0, false, JSON.stringify(samples["boilerOnYesterday"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/boilerOnToday").c_str(), 0, false, JSON.stringify(samples["boilerOnToday"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/boilerPreviousYear").c_str(), 0, false, JSON.stringify(samples["boilerPreviousYear"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/boilerOnPreviousYear").c_str(), 0, false, JSON.stringify(samples["boilerOnPreviousYear"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/boilerOnPreviousYearJan").c_str(), 0, false, JSON.stringify(samples["boilerOnPreviousYearJan"]).c_str());mqttClient.publish(String(mqttTopicName+"/boilerOnPreviousYearFeb").c_str(), 0, false, JSON.stringify(samples["boilerOnPreviousYearFeb"]).c_str());mqttClient.publish(String(mqttTopicName+"/boilerOnPreviousYearMar").c_str(), 0, false, JSON.stringify(samples["boilerOnPreviousYearMar"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/boilerOnPreviousYearApr").c_str(), 0, false, JSON.stringify(samples["boilerOnPreviousYearApr"]).c_str());mqttClient.publish(String(mqttTopicName+"/boilerOnPreviousYearMay").c_str(), 0, false, JSON.stringify(samples["boilerOnPreviousYearMay"]).c_str());mqttClient.publish(String(mqttTopicName+"/boilerOnPreviousYearJun").c_str(), 0, false, JSON.stringify(samples["boilerOnPreviousYearJun"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/boilerOnPreviousYearJul").c_str(), 0, false, JSON.stringify(samples["boilerOnPreviousYearJul"]).c_str());mqttClient.publish(String(mqttTopicName+"/boilerOnPreviousYearAug").c_str(), 0, false, JSON.stringify(samples["boilerOnPreviousYearAug"]).c_str());mqttClient.publish(String(mqttTopicName+"/boilerOnPreviousYearSep").c_str(), 0, false, JSON.stringify(samples["boilerOnPreviousYearSep"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/boilerOnPreviousYearOct").c_str(), 0, false, JSON.stringify(samples["boilerOnPreviousYearOct"]).c_str());mqttClient.publish(String(mqttTopicName+"/boilerOnPreviousYearNov").c_str(), 0, false, JSON.stringify(samples["boilerOnPreviousYearNov"]).c_str());mqttClient.publish(String(mqttTopicName+"/boilerOnPreviousYearDec").c_str(), 0, false, JSON.stringify(samples["boilerOnPreviousYearDec"]).c_str());
      
      //Home Assistant support
      //Publish sample values message, not retain in the server
      mqttClient.publish(String(mqttTopicName+"/LWT").c_str(), 0, false, "Online\0"); //Availability message, not retain in the broker. This makes HA to subscribe to the */SENSOR topic if not already done
      getLocalTime(&nowTimeInfo);strftime(s,sizeof(s),"%Y-%m-%dT%H:%M:%S",&nowTimeInfo); //Time in format 2024-08-24T07:56:25
      String message=String("{\"Time\":\""+String(s)+"\",\"SAMPLES\":"+JSON.stringify(samples)+"}");
      mqttClient.publish(String(mqttTopicName+"/SENSOR").c_str(), 0, false, message.c_str());

      if (debugModeOn) printLogln(String(millis())+" - [loop - mqtt_publish_samples] - new MQTT messages published");
      /*if (debugModeOn) printLogln(String(millis())+" - [loop - mqtt_publish_samples] - samples[\"boilerToday\"]="+JSON.stringify(samples["boilerToday"])+"samples[\"heaterToday\"]="+JSON.stringify(samples["heaterToday"]));*/ //----->
 
      //Publish HA Discovery messages at random basis to make sure HA always recives the Discovery Packet
      // even if it didn't receive it after it rebooted due to network issues or whatever - v1.9.2
      if ((random(0,33) < 2) || (millis()<HA_ADVST_WINDOW) || updateHADiscovery) { //random < 2 ==> probability ~3%, ==> ~1 every 10 min (at samples/20s rate))
                                                    //updateHADiscovery: True if year changed. Update Timer Counters Year in HA MQTT
                                                    //HADiscovery is sent several times (HA_ADVST_WINDOW) after boot up to make sure all the topics are processed - v0.9.7 - ISS007
        mqttClientPublishHADiscovery(mqttTopicName,device,WiFi.localIP().toString(),false); 
        if (updateHADiscovery) updateHADiscovery=false;
      }

      MqttSyncCurrentStatus=MqttSyncOnStatus;
    }
    else {
      //MQTT Client disconnected
      //Connect to MQTT broker
      if (debugModeOn) printLogln(String(millis())+" - [loop - mqtt_publish_samples] - new MQTT messages can't be published as MQTT broker is disconnected. Trying to get connected again...\n             MqttSyncLastStatus("+String(MqttSyncLastStatus)+")!=MqttSyncCurrentStatus("+String(MqttSyncCurrentStatus)+")");
      MqttSyncCurrentStatus=MqttSyncOffStatus;
      errorsMQTTCnt++;EEPROM.write(0x538,errorsMQTTCnt);eepromUpdate=true;
    }
  }
}

void one_second_check_period(bool debugModeOn, uint64_t nowTimeGlobal, bool ntpSynced) {
  /******************************************************
   Function one_second_check_period
   Target: Regular actions every ONE_SECOND_PERIOD seconds.
    1) Check if time on counters need to be updated
   Parameters:
    debugModeOn: Print out the logs or not
    nowTimeGlobal: current time the function was called in milliseconds
    ntpSynced: True if NTP is synced at least once
   *****************************************************/

  struct tm auxTimeInfo,*auxYesterdayTimeInfo; //36 B
  time_t auxYesterdayEpoch;
  uint32_t auxToday,auxYesterday,auxMonth,auxYear;

  
  /*
    struct tm {
      int	tm_sec;		// seconds after the minute [0-60]
      int	tm_min;		// minutes after the hour [0-59]
      int	tm_hour;	// hours since midnight [0-23]
      int	tm_mday;	// day of the month [1-31]
      int	tm_mon;		// months since January [0-11]
      int	tm_year;	// years since 1900
      int	tm_wday;	// days since Sunday [0-6]
      int	tm_yday;	// days since January 1 [0-365]
      int	tm_isdst;	// Daylight Savings Time flag
      long	tm_gmtoff;	// offset from UTC in seconds
      char	*tm_zone;	// timezone abbreviation
    };
*/
  

  if (debugModeOn) {printLogln(String(nowTimeGlobal)+" - [loop - ONE_SECOND_PERIOD] - Doing actions every second.");}
  /*debugModeOn=true;
    if (debugModeOn) {printLogln(String(millis())+" - [loop - ONE_SECOND_PERIOD] - boilerStatus="+String(boilerStatus)+", thermostateStatus="+String(thermostateStatus)+", boilerOn="+String(boilerOn)+", thermostateOn="+String(thermostateOn)+
        "                                      \nlastThermostatOnTime="+String(lastThermostatOnTime)+", lastBoilerOnTime="+String(lastBoilerOnTime)+
        "                                      \nheaterTimeOnYear.counterToday="+String(heaterTimeOnYear.counterToday)+", heaterTimeOnYear.counterMonths[auxMonth-1]="+String(heaterTimeOnYear.counterMonths[auxMonth-1])+
        "                                      \nboilerTimeOnYear.counterToday="+String(boilerTimeOnYear.counterToday)+", boilerTimeOnYear.counterMonths[auxMonth-1]="+String(boilerTimeOnYear.counterMonths[auxMonth-1])+
        "                                      \nyear="+String(year)+", previousYear="+String(previousYear)+", today="+String(today)+", yesterday="+String(yesterday)+
        "                                      \nheaterTimeOnYear.year="+String(heaterTimeOnYear.year)+", heaterTimeOnYear.today="+String(heaterTimeOnYear.today)+", heaterTimeOnYear.yesterday="+String(heaterTimeOnYear.yesterday)+
        "                                      \nboilerTimeOnYear.year="+String(boilerTimeOnYear.year)+", boilerTimeOnYear.today="+String(boilerTimeOnYear.today)+", boilerTimeOnYear.yesterday="+String(boilerTimeOnYear.yesterday));
  }*/ //----->


  //At this point the variables already updated, even right after boot time.
  if (ntpSynced) {
    //Get time only if NTP was synced at leat once
    getLocalTime(&auxTimeInfo);
    auxToday=(auxTimeInfo.tm_year+1900)*10000+(auxTimeInfo.tm_mon+1)*100+auxTimeInfo.tm_mday;
    //26/Nov/2025 => 20251126, 26/Apr/2025 => 20250426, 2/Apr/2025 => 20250402
    auxYesterdayEpoch=mktime(&auxTimeInfo)-86400; //One day less
    auxYesterdayTimeInfo=localtime(&auxYesterdayEpoch);
    auxYesterday=(auxYesterdayTimeInfo->tm_year+1900)*10000+(auxYesterdayTimeInfo->tm_mon+1)*100+auxYesterdayTimeInfo->tm_mday;
    auxMonth=(today-year*10000)/100;

    /*if (debugModeOn) {printLogln(String(millis())+" - [loop - ONE_SECOND_PERIOD] - auxMonth="+String(auxMonth)+", auxToday="+String(auxToday)+", auxYesterday="+String(auxYesterday));}*/  //----->

    if (thermostateOn) { //Update counters if the heater is on
      heaterTimeOnYear.counterToday+=(nowTimeGlobal-lastThermostatOnTime)/1000;
      heaterTimeOnYear.counterMonths[auxMonth-1]+=(nowTimeGlobal-lastThermostatOnTime)/1000;
      lastThermostatOnTime=nowTimeGlobal;
      timersEepromUpdate=true; //Update EEPROM in the next cycle
    }
    if (boilerOn) { //Update counters if the boiler is on
      boilerTimeOnYear.counterToday+=(nowTimeGlobal-lastBoilerOnTime)/1000;
      boilerTimeOnYear.counterMonths[auxMonth-1]+=(nowTimeGlobal-lastBoilerOnTime)/1000;
      lastBoilerOnTime=nowTimeGlobal;
      timersEepromUpdate=true; //Update EEPROM in the next cycle
    }
    if (!thermostateOn && !boilerOn) { //v0.9.9
      if (today!=auxToday) {
        //New day
        heaterTimeOnYear.counterYesterday=heaterTimeOnYear.counterToday;
        heaterTimeOnYear.counterToday=0;
        
        boilerTimeOnYear.counterYesterday=boilerTimeOnYear.counterToday;
        boilerTimeOnYear.counterToday=0;

        if ((auxTimeInfo.tm_year+1900) > (heaterTimeOnYear.year)) {
          //New year
          memcpy(&heaterTimeOnPreviousYear,&heaterTimeOnYear,sizeof(heaterTimeOnYear)); //Update heaterTimeOnPreviousYear with heaterTimeOnYear
          heaterTimeOnYear.year=auxTimeInfo.tm_year+1900;
          heaterTimeOnYear.today=auxToday;
          heaterTimeOnYear.yesterday=today;
          heaterTimeOnYear.counterYesterday=0;
          heaterTimeOnYear.counterToday=0;

          memcpy(&boilerTimeOnPreviousYear,&boilerTimeOnYear,sizeof(boilerTimeOnYear)); //Update boilerTimeOnPreviousYear with boilerTimeOnYear
          boilerTimeOnYear.year=auxTimeInfo.tm_year+1900;
          boilerTimeOnYear.today=auxToday;
          boilerTimeOnYear.yesterday=today;
          boilerTimeOnYear.counterYesterday=0;
          boilerTimeOnYear.counterToday=0;
          updateHADiscovery=true;
        }
        else {
          //Same year
          //Do nothing
        }
        yesterday=today;
        today=auxToday;
        timersEepromUpdate=true; //Update EEPROM in the next cycle
      }
      else {
        //Same day
        //Do nothing
      }
    }
  }

  uint32_t auxTimeOn=0; for (int i=0;i<12;i++) auxTimeOn+=heaterTimeOnYear.counterMonths[i];
  samples["heaterYear"] = String(heaterTimeOnYear.year);
  samples["heaterYesterday"] = String(heaterTimeOnYear.yesterday);
  samples["heaterToday"] = String(heaterTimeOnYear.today);
  samples["heaterOnYear"] = String(auxTimeOn);
  samples["heaterOnYearJan"] = String(heaterTimeOnYear.counterMonths[0]);samples["heaterOnYearFeb"] = String(heaterTimeOnYear.counterMonths[1]);samples["heaterOnYearMar"] = String(heaterTimeOnYear.counterMonths[2]);samples["heaterOnYearApr"] = String(heaterTimeOnYear.counterMonths[3]);samples["heaterOnYearMay"] = String(heaterTimeOnYear.counterMonths[4]);samples["heaterOnYearJun"] = String(heaterTimeOnYear.counterMonths[5]);
  samples["heaterOnYearJul"] = String(heaterTimeOnYear.counterMonths[6]);samples["heaterOnYearAug"] = String(heaterTimeOnYear.counterMonths[7]);samples["heaterOnYearSep"] = String(heaterTimeOnYear.counterMonths[8]);samples["heaterOnYearOct"] = String(heaterTimeOnYear.counterMonths[9]);samples["heaterOnYearNov"] = String(heaterTimeOnYear.counterMonths[10]);samples["heaterOnYearDec"] = String(heaterTimeOnYear.counterMonths[11]);
  samples["heaterOnYesterday"] = String(heaterTimeOnYear.counterYesterday);
  samples["heaterOnToday"] = String(heaterTimeOnYear.counterToday);
  auxTimeOn=0; for (int i=0;i<12;i++) auxTimeOn+=heaterTimeOnPreviousYear.counterMonths[i];
  samples["heaterPreviousYear"] = String(heaterTimeOnPreviousYear.year);
  samples["heaterOnPreviousYear"] = String(auxTimeOn);
  samples["heaterOnPreviousYearJan"] = String(heaterTimeOnPreviousYear.counterMonths[0]);samples["heaterOnPreviousYearFeb"] = String(heaterTimeOnPreviousYear.counterMonths[1]);samples["heaterOnPreviousYearMar"] = String(heaterTimeOnPreviousYear.counterMonths[2]);samples["heaterOnPreviousYearApr"] = String(heaterTimeOnPreviousYear.counterMonths[3]);samples["heaterOnPreviousYearMay"] = String(heaterTimeOnPreviousYear.counterMonths[4]);samples["heaterOnPreviousYearJun"] = String(heaterTimeOnPreviousYear.counterMonths[5]);
  samples["heaterOnPreviousYearJul"] = String(heaterTimeOnPreviousYear.counterMonths[6]);samples["heaterOnPreviousYearAug"] = String(heaterTimeOnPreviousYear.counterMonths[7]);samples["heaterOnPreviousYearSep"] = String(heaterTimeOnPreviousYear.counterMonths[8]);samples["heaterOnPreviousYearOct"] = String(heaterTimeOnPreviousYear.counterMonths[9]);samples["heaterOnPreviousYearNov"] = String(heaterTimeOnPreviousYear.counterMonths[10]);samples["heaterOnPreviousYearDec"] = String(heaterTimeOnPreviousYear.counterMonths[11]);
  
  auxTimeOn=0; for (int i=0;i<12;i++) auxTimeOn+=boilerTimeOnYear.counterMonths[i];
  samples["boilerYear"] = String(boilerTimeOnYear.year);
  samples["boilerYesterday"] = String(boilerTimeOnYear.yesterday);
  samples["boilerToday"] = String(boilerTimeOnYear.today);
  samples["boilerOnYear"] = String(auxTimeOn);
  samples["boilerOnYearJan"] = String(boilerTimeOnYear.counterMonths[0]);samples["boilerOnYearFeb"] = String(boilerTimeOnYear.counterMonths[1]);samples["boilerOnYearMar"] = String(boilerTimeOnYear.counterMonths[2]);samples["boilerOnYearApr"] = String(boilerTimeOnYear.counterMonths[3]);samples["boilerOnYearMay"] = String(boilerTimeOnYear.counterMonths[4]);samples["boilerOnYearJun"] = String(boilerTimeOnYear.counterMonths[5]);
  samples["boilerOnYearJul"] = String(boilerTimeOnYear.counterMonths[6]);samples["boilerOnYearAug"] = String(boilerTimeOnYear.counterMonths[7]);samples["boilerOnYearSep"] = String(boilerTimeOnYear.counterMonths[8]);samples["boilerOnYearOct"] = String(boilerTimeOnYear.counterMonths[9]);samples["boilerOnYearNov"] = String(boilerTimeOnYear.counterMonths[10]);samples["boilerOnYearDec"] = String(boilerTimeOnYear.counterMonths[11]);
  samples["boilerOnYesterday"] = String(boilerTimeOnYear.counterYesterday);
  samples["boilerOnToday"] = String(boilerTimeOnYear.counterToday);
  auxTimeOn=0; for (int i=0;i<12;i++) auxTimeOn+=boilerTimeOnPreviousYear.counterMonths[i];
  samples["boilerPreviousYear"] = String(boilerTimeOnPreviousYear.year);
  samples["boilerOnPreviousYear"] = String(auxTimeOn);
  samples["boilerOnPreviousYearJan"] = String(boilerTimeOnPreviousYear.counterMonths[0]);samples["boilerOnPreviousYearFeb"] = String(boilerTimeOnPreviousYear.counterMonths[1]);samples["boilerOnPreviousYearMar"] = String(boilerTimeOnPreviousYear.counterMonths[2]);samples["boilerOnPreviousYearApr"] = String(boilerTimeOnPreviousYear.counterMonths[3]);samples["boilerOnPreviousYearMay"] = String(boilerTimeOnPreviousYear.counterMonths[4]);samples["boilerOnPreviousYearJun"] = String(boilerTimeOnPreviousYear.counterMonths[5]);
  samples["boilerOnPreviousYearJul"] = String(boilerTimeOnPreviousYear.counterMonths[6]);samples["boilerOnPreviousYearAug"] = String(boilerTimeOnPreviousYear.counterMonths[7]);samples["boilerOnPreviousYearSep"] = String(boilerTimeOnPreviousYear.counterMonths[8]);samples["boilerOnPreviousYearOct"] = String(boilerTimeOnPreviousYear.counterMonths[9]);samples["boilerOnPreviousYearNov"] = String(boilerTimeOnPreviousYear.counterMonths[10]);samples["boilerOnPreviousYearDec"] = String(boilerTimeOnPreviousYear.counterMonths[11]);

  /*if (debugModeOn) printLogln(String(millis())+" - [loop - mqtt_publish_samples] - Exit - samples[\"boilerToday\"]="+JSON.stringify(samples["boilerToday"])+"samples[\"heaterToday\"]="+JSON.stringify(samples["heaterToday"]));*/ //----->
  
  //No time on counter update if there is no NTP sync  
  //Update with nowTimeGlobal to prevent accumulating delays in CHECK periods as this code might take long
  lastTimeSecondCheck=nowTimeGlobal;
} // -- one_second_check_period --

void time_counters_eeprom_update_check_period(bool debugModeOn, uint64_t nowTimeGlobal) {
  /******************************************************
   Function time_counters_eeprom_update_check_period
   Target: Regular actions every TIME_COUNTERS_EEPROM_UPDATE_PERIOD seconds.
    1) Check if EEPROM must be updated with new values of the time counters
   Parameters:
    debugModeOn: Print out the logs or not
    nowTimeGlobal: current time the function was called in milliseconds
   Returns: Nothing
   *****************************************************/

  if (timersEepromUpdate) {
    //Update EEPROM only if there are new time counter values
    EEPROM.put(0x421,heaterTimeOnYear); EEPROM.put(0x465,heaterTimeOnPreviousYear);
    EEPROM.put(0x4A9,boilerTimeOnYear); EEPROM.put(0x4ED,boilerTimeOnPreviousYear);
    EEPROM.commit();
    if (debugModeOn) printLogln(String(nowTimeGlobal)+" - [loop - eeprom_update_check] - EEPROM updated with variables and counters");
    timersEepromUpdate=false;
  }
  else {
    if (debugModeOn) printLogln(String(nowTimeGlobal)+" - [loop - eeprom_update_check] - No need to update EEPROM with variables and counters");
  }

  lastTimeTimerEepromUpdateCheck=nowTimeGlobal;
}

uint32_t checkURL(boolean debugModeOn,boolean fromSetup,uint32_t error_setup,IPAddress server,uint16_t port,String httpRequest) {
  /******************************************************
   Function checkURL
   Target: Check URL by sending httpRequest
   Parameters:
    debugModeOn: Print out the logs or not
    fromSetup: where the function was called from. Diferent prints out are done base on its value
    error_setup: error during setup
    server: IP address of the HTTP server where to send the HTTP Request to
    port: HTTP Server's Port number
    httpRequest: HTTP request with no HTTP headers. They are set in here.
   Returns: ERROR type or NO_ERROR
   *****************************************************/
  uint64_t timeLeft=0;
  
  if (debugModeOn) {printLogln(String(millis())+" - [checkURL] - Trying connection to "+String(IpAddress2String(server))+" to send httpRequest: '"+httpRequest+"'");}

  if (client.connect(server, port)) {
    if (debugModeOn) {printLogln("      [checkURL] - connected");}
    // Send a HTTP request:
    client.println(httpRequest);
    client.print("Host: "); client.println(IpAddress2String(server));
    client.println("User-Agent: Arduino/1.0");
    client.println("Accept-Language: es-es");
    client.println("Connection: close");
    client.println();
  }
  else {
    if (debugModeOn) {printLog("      [checkURL] - No server connection. ERROR_WEB_SERVER - Exit - Time: ");getLocalTime(&nowTimeInfo);printLogln(&nowTimeInfo, "%d/%m/%Y - %H:%M:%S");}
    return(ERROR_WEB_SERVER); //No WEB server connection
  }

  if (debugModeOn) {printLogln("      [checkURL] - waiting for HTTP client.available()");}
  timeLeft=HTTP_ANSWER_TIMEOUT;
  while ( !client.available() && timeLeft>0) {
    delay(50);
    timeLeft-=50;
    if (timeLeft>HTTP_ANSWER_TIMEOUT) timeLeft=0; //Negative value
  } //end while() loop

  if (timeLeft==0) { //Case if while() loop timeout.
    //Too long with no server answer. Something was wrong
    if (!client.connected()) client.stop();
    if (debugModeOn) {printLog("      [checkURL] - No server connection. ERROR_WEB_SERVER - Exit - Time: ");getLocalTime(&nowTimeInfo);printLogln(&nowTimeInfo, "%d/%m/%Y - %H:%M:%S");}
    return(ERROR_WEB_SERVER); //No WEB server connection
  }
  else { 
    //End of while() due to successful WEB sync
    if (debugModeOn) {printLogln("      [checkURL] Server answer\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");}
    while (client.available()) {//Should be very fast.
      char c = client.read();
      if (debugModeOn) {if (c=='\n') printLog("\n       "); else printLog(String(c));}
    }
    if (debugModeOn) {printLogln("\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");}
  }

  //This point is reached everthing is right
  
  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    if (debugModeOn) {printLogln("      [checkURL] - Disconnecting from server. Bye!");}
    client.stop();
  }

  if (debugModeOn) {printLog(String("      [checkURL] - GOT URL. NO_ERROR - Exit - Time: "));getLocalTime(&nowTimeInfo);printLogln(&nowTimeInfo, "%d/%m/%Y - %H:%M:%S");}
  return(NO_ERROR); //WEB server connection fine
}

uint32_t connectiviy_check_period(bool debugModeOn, uint64_t nowTimeGlobal) {
  /******************************************************
   Function connectiviy_check_period
   Target: Regular actions every CONNECTIVITY_CHECK_PERIOD seconds.
    Check connectivity and reset it if needed, if:
      WiFi.status()==WL_CONNECTED && no FQDN resolved and error in sample_upload && mqttClient not connected and error in checkURL and no GW ping
      NTP status is not check as NTP checks take time (random period) and forcing NTP in here is avoided (as it may take several loop cycles = complexity)
   Parameters:
    debugModeOn: Print out the logs or not
    nowTimeGlobal: current time the function was called in milliseconds
   Returns: Nothing
   *****************************************************/

  if (debugModeOn) {printLogln(String(nowTimeGlobal)+" - [loop - CONNECTIVITY_CHECK_PERIOD] - Checking connectivity.");}
  
  IPAddress auxServer;
  bool connectivityOK=true, webServerOK=true;

  //First try ping to the gateway
  if (Ping.ping(WiFi.gatewayIP(),1)) {
    //Success, so connectivity is right
    //Nothing to do - exit
    if (debugModeOn) {printLogln("     [loop - CONNECTIVITY_CHECK_PERIOD] - Default GW Ping received. Connectivity is OK");}
  }
  else if (WiFi.hostByName(FQDN_TO_CHECK, auxServer)) {
    //Success, so connectivity is right
    //Nothing to do - exit
    if (debugModeOn) {printLogln("     [loop - CONNECTIVITY_CHECK_PERIOD] - "+String(FQDN_TO_CHECK)+"="+auxServer.toString()+". FQDN resoled. Connectivity is OK");}
  }
  else if (mqttClient.connected() && mqttServerEnabled) {
    //Success, so connectivity is right
    //Nothing to do - exit
    if (debugModeOn) {printLogln("     [loop - CONNECTIVITY_CHECK_PERIOD] - Connection to the MQTT server is OK. Connectivity is OK");}
  }
  else if (!(error_setup & ERROR_CLOUD_SERVER) && httpCloudEnabled) {
    //Success, so connectivity is right
    //Nothing to do - exit
    if (debugModeOn) {printLogln("     [loop - CONNECTIVITY_CHECK_PERIOD] - Connection to the Cloud server is OK. Connectivity is OK");}
  }
  else {
      //Check external URL
      if (WiFi.hostByName(SERVER_TO_CHECK, auxServer)) {
        //FQDN resolution is OK
        //auxServer is SERVER_TO_CHECK's IPAddress
      }
      else {
        //Even if FQDN fails, we'll try the URL check to the server
        IPAddress auxServer2=IPAddress(40,112,243,49); //It's SERVER_TO_CHECK's IP address - connectivity.office.com
        auxServer=auxServer2;
      }

      //Now let's check the SERVER_TO_CHECK web site
      if (checkURL(debugModeOn,false,0,auxServer,80,String("GET /"))==ERROR_WEB_SERVER) {
        //Connectivity KO
        errorsConnectivityCnt++;EEPROM.write(0x53A,errorsConnectivityCnt);eepromUpdate=true; //Stats
        connectivityOK=false;
      }
  }

  if (connectivityOK) {
    //Check local web server
    if (checkURL(debugModeOn,false,0,WiFi.localIP(),80,String("GET ")+String(WEBSERVER_TEST_PAGE))==ERROR_WEB_SERVER) {
        //Local web server KO
        errorsWebServerCnt++; //Stats - Variable is written in EEPROM in the main loop
        webServerOK=false;
        if (debugModeOn) {printLogln("     [loop - CONNECTIVITY_CHECK_PERIOD] - Connectivity is OK, but web server is KO.  Reinit network services, heap="+String(esp_get_free_heap_size()));}
    }
    else {
      if (debugModeOn) {printLogln("     [loop - CONNECTIVITY_CHECK_PERIOD] - Connectivity and web server are OK, heap="+String(esp_get_free_heap_size()));}
    }
  }
  else {
    if (debugModeOn) {printLogln("     [loop - CONNECTIVITY_CHECK_PERIOD] - All network checks are KO. Reinit network services, heap="+String(esp_get_free_heap_size()));}
  }
  
  lastTimeConnectiviyCheck=nowTimeGlobal;
  if (!webServerOK) return ERROR_WEB_SERVER;
  else if (!connectivityOK) return ERROR_NO_CONNECTIVITY;
  else return (NO_ERROR);
}