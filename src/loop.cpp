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
    boardSerialPort.println(String(nowTimeGlobal)+" - [loop - WIFI_RECONNECT_PERIOD] - nowTimeGlobal-lastTimeWifiReconnectionCheck >= wifiReconnectPeriod ("+String(nowTimeGlobal-lastTimeWifiReconnectionCheck)+" >= "+String(wifiReconnectPeriod/1000)+" s)");
    boardSerialPort.println("           + lastTimeWifiReconnectionCheck="+String(lastTimeWifiReconnectionCheck));
    boardSerialPort.println("           + forceWifiReconnect="+String(forceWifiReconnect));
    boardSerialPort.println("           + forceWebServerInit="+String(forceWebServerInit));
    boardSerialPort.println("           + !firstBoot="+String(!firstBoot));
    boardSerialPort.println("           + wifiCurrentStatus="+String(wifiCurrentStatus)+", wifiOffStatus=0");
    boardSerialPort.println("           + WiFi.status()="+String(WiFi.status())+", WL_CONNECTED=3");
  }
  else boardSerialPort.println(String(nowTimeGlobal)+" - [loop - WIFI_RECONNECT_PERIOD] - wifiCurrentStatus="+String(wifiCurrentStatus));

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
  if (debugModeOn) {boardSerialPort.println("           + auxLoopCounter="+String(auxLoopCounter)+", auxCounter="+String(auxCounter));}
  
  forceWEBTestCheck=false; //If WiFi reconnection is successfull, then check CLOUD server to update ICON. Decision is done below, if NO_ERROR
  switch(wifiConnect(false,&auxLoopCounter,&auxCounter)) {
    case ERROR_WIFI_SETUP:
      CloudClockCurrentStatus=CloudClockOffStatus;CloudSyncCurrentStatus=CloudSyncOffStatus;MqttSyncCurrentStatus=MqttSyncOffStatus; //To update rest of icons -v1.4.1
      wifiCurrentStatus=wifiOffStatus;
      forceWifiReconnect=false;
      wifiResuming=false;
      if (debugModeOn) {boardSerialPort.println(String(millis())+" - [loop - WIFI_RECONNECT_PERIOD] - wifiConnect() finish with ERROR_WIFI_SETUP. wifiCurrentStatus="+String(wifiCurrentStatus)+", forceWifiReconnect="+String(forceWifiReconnect));}
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
      if (debugModeOn) {boardSerialPort.println(String(millis())+" - [loop - WIFI_RECONNECT_PERIOD] - wifiConnect() finish with NO_ERROR. wifiCurrentStatus="+String(wifiCurrentStatus)+", forceWEBTestCheck="+String(forceWEBTestCheck));}
    break;
  } 

  if (WiFi.status()==WL_CONNECTED && !mqttClient.connected() && mqttServerEnabled) { //Connect to MQTT broker again
    //mqttClient.connect();
    //mqttClientInit(false,debugModeOn,false,mqttTopicName,device,WiFi.localIP().toString());
    MqttSyncCurrentStatus=MqttSyncOffStatus; //Force to init MQTT server
    lastTimeMQTTCheck=nowTimeGlobal-MQTT_CHECK_PERIOD;
  }

  //After getting WiFi connection re-init the web server if needed
  //forceWebServerInit==true if:
  // 1) wake up from sleep, including hibernate (either by pressing buttons or timer)
  // 2) WiFi set ON from the config menu
  // 3) after heap size was below ABSULUTE_MIN_HEAP_THRESHOLD
  // 4) Webserver detected down.
  /*if (wifiEnabled && webServerEnabled && WiFi.status()==WL_CONNECTED && forceWebServerInit) { //v0.9.9 - Re-init the built-in WebServer after waking up from sleep
    if (debugModeOn) boardSerialPort.println("    - After leaving wifiReconnectPeriod entering to re-init the Web Server");    
    if(SPIFFS.begin(true)) {
      if (debugModeOn) boardSerialPort.println("    - wifiConnect()  - SPIFFS.begin() OK, SPIFFSErrors="+String(SPIFFSErrors));
      initWebServer();
      forceWebServerInit=false;
      if (debugModeOn) boardSerialPort.println("    - wifiConnect()  - initWebServer");
      
      fileSystemSize = SPIFFS.totalBytes();
      fileSystemUsed = SPIFFS.usedBytes();
    }
    else {
      SPIFFSErrors++;
      if (debugModeOn) boardSerialPort.println("    - wifiConnect()  - SPIFFS.begin() KO, SPIFFSErrors="+String(SPIFFSErrors));
    }
  }*/

  if (debugModeOn) {boardSerialPort.println(String(millis())+" - [loop - WIFI_RECONNECT_PERIOD] - wifiReconnectPeriod - exit, lastTimeWifiReconnectionCheck="+String(lastTimeWifiReconnectionCheck));}
  else boardSerialPort.println(String(nowTimeGlobal)+" - [loop - WIFI_RECONNECT_PERIOD] - Exit. wifiCurrentStatus="+String(wifiCurrentStatus));
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

  if (debugModeOn) {boardSerialPort.println(String(nowTimeGlobal)+" - [loop - NTP_KO_CHECK_PERIOD] - Last lastTimeNTPCheck="+String(lastTimeNTPCheck)+", NTPResuming="+String(NTPResuming)+", auxLoopCounter2="+String(auxLoopCounter2)+", whileLoopTimeLeft="+String(whileLoopTimeLeft));}
  else boardSerialPort.print(String(nowTimeGlobal)+" - [loop - NTP_KO_CHECK_PERIOD] - CloudClockCurrentStatus="+String(CloudClockCurrentStatus));
    
  //Update at begining to prevent accumulating delays in CHECK periods as this code might take long
  if (!NTPResuming) lastTimeNTPCheck=nowTimeGlobal; //Only if the NTP reconnection didn't ABORT or BREAK in the previous interaction

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
      boardSerialPort.println("           + setupNTPConfig() for NTP Sync");
      if (CloudClockCurrentStatus==CloudClockOffStatus) boardSerialPort.println("           + Reason: CloudClockCurrentStatus==CloudClockOffStatus");
      if (forceNTPCheck) boardSerialPort.println("           + Reason: forceNTPCheck");
      if (auxRandom<2) boardSerialPort.println("           + Reason: auxRandom(="+String(auxRandom)+")<2");
    }
    forceNTPCheck=false;
    if (CloudClockCurrentStatus!=CloudClockSendStatus) CloudClockLastStatus=CloudClockCurrentStatus; //To enter in DISPLAY_ICONS_REFRESH_TIMEOUT in the next loop cycle
    switch(setupNTPConfig(false,false,&auxLoopCounter2,&whileLoopTimeLeft)) { //NTP Sync and CloudClockCurrentStatus update
      case ERROR_NTP_SERVER:
        forceNTPCheck=false;
        NTPResuming=false;
        previousCloudClockCurrentStatus=CloudClockOffStatus; //CloudClock Status to be back after DISPLAY_ICONS_REFRESH_TIMEOUT
        CloudClockCurrentStatus=CloudClockSendStatus;
        if (debugModeOn) {boardSerialPort.println(String(millis())+" - [loop - NTP_KO_CHECK_PERIOD] - setupNTPConfig() finish with ERROR_NTP_SERVER. CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+", forceNTPCheck="+String(forceNTPCheck));}
      break;
      case NO_ERROR:
        forceNTPCheck=false;
        NTPResuming=false;
        previousCloudClockCurrentStatus=CloudClockOnStatus; //CloudClock Status to be back after DISPLAY_ICONS_REFRESH_TIMEOUT
        CloudClockCurrentStatus=CloudClockSendStatus;
        if (debugModeOn) {boardSerialPort.println(String(millis())+" - [loop - NTP_KO_CHECK_PERIOD] - setupNTPConfig() finish with NO_ERROR. CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+", forceNTPCheck="+String(forceNTPCheck));}
      break;
    }
    lastCloudClockChangeCheck=nowTimeGlobal; //Reset Cloud Clock Change Timeout

    getLocalTime(&nowTimeInfo);
    if (debugModeOn) {
      //boardSerialPort.println(String(millis())+" - [loop - NTP_KO_CHECK_PERIOD] - errorsNTPCnt="+String(errorsNTPCnt)+", CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+", lastTimeNTPCheck="+String(lastTimeNTPCheck));
      boardSerialPort.print("        [loop - NTP_KO_CHECK_PERIOD] ");boardSerialPort.println(&nowTimeInfo,"- NTP sync done. Exit - Time: %d/%m/%Y - %H:%M:%S");
    }
    else {boardSerialPort.print(&nowTimeInfo," - NTP sync done. Exit - Time: %d/%m/%Y - %H:%M:%S. ");boardSerialPort.println("CloudClockCurrentStatus="+String(CloudClockCurrentStatus));}
  }
  else {
    if( wifiCurrentStatus==wifiOffStatus || !wifiEnabled) if (forceNTPCheck) forceNTPCheck=false; //v0.9.9 If no WiFi, don't enter in NTP_KO_CHECK_PERIOD even if it was BREAK or ABORT in previous intercation
    getLocalTime(&nowTimeInfo);
    if (debugModeOn) {
      ///boardSerialPort.println(String(millis())+" - [loop - NTP_KO_CHECK_PERIOD] - errorsNTPCnt="+String(errorsNTPCnt)+", CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+", lastTimeNTPCheck="+String(lastTimeNTPCheck));
      boardSerialPort.print("        [loop - NTP_KO_CHECK_PERIOD] ");boardSerialPort.println(&nowTimeInfo,"- No need for NTP sync. Exit - Time: %d/%m/%Y - %H:%M:%S");
    }
    else {boardSerialPort.print(&nowTimeInfo," - No need for NTP sync. Exit - Time: %d/%m/%Y - %H:%M:%S. "); boardSerialPort.println("CloudClockCurrentStatus="+String(CloudClockCurrentStatus));}
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

  if (fromSetup) boardSerialPort.print(String(millis())+" -"); //no from Setup
  else boardSerialPort.print("       "); //from setup
  boardSerialPort.print("[loop - sendHttpRequest] - Sending HTTP request");
  if (!fromSetup) boardSerialPort.println("");
  else boardSerialPort.println(" '"+httpRequest+"', server="+IpAddress2String(server));

  if (httpCloudEnabled && wifiCurrentStatus!=wifiOffStatus && wifiEnabled) {  //Only if HTTP Cloud is enabled and WiFi is connected
    if (client.connect(server, port)) {
      if (debugModeOn) {boardSerialPort.println("       [loop - sendHttpRequest] - connected to web server");}
      // Send a HTTP request:
      client.println(httpRequest);
      client.print("Host: "); client.println(IpAddress2String(server));
      client.println("User-Agent: Arduino/1.0");
      client.println("Accept-Language: es-es");
      client.println("Connection: close");
      client.println();
    }
    else {
      errorsHTTPUptsCnt++;  //Something went wrong. Update error counter for stats
      if (debugModeOn) {boardSerialPort.println("       [loop - sendHttpRequest] - Not connected, errorsSampleUpts="+String(errorsHTTPUptsCnt));}
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
        if (debugModeOn && fromSetup) {boardSerialPort.println("       NO Server answer\n       ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");}
        break;
      }
    }; //wait till there is server answer

    if (CloudSyncCurrentStatus==CloudSyncOnStatus) {
      if (debugModeOn && fromSetup) {boardSerialPort.print("       Server answer\n       ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n       ");}
      while (client.available()) {
        char c = client.read();
        if (debugModeOn && fromSetup) {if (c=='\n') boardSerialPort.print("\n       "); else boardSerialPort.write(c);}
      }
      if (debugModeOn && fromSetup) {boardSerialPort.println("\n       ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");}
    }
    else {
      errorsHTTPUptsCnt++;  //Something went wrong. Update error counter for stats
      return (ERROR_CLOUD_SERVER);
    }

    // if the server's disconnected, stop the client:
    if (!client.connected()) {
      if (debugModeOn && fromSetup) {boardSerialPort.println("       [loop - sendHttpRequest] - Disconnecting from server. Bye!");}
      client.stop();
    }

    return (NO_ERROR);
  }
  else {
    if (debugModeOn) {boardSerialPort.println("       [loop - sendHttpRequest] - No WiFi or HTTP Cloud updates are disabled.");}
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
  int thermostatInterrupt=digitalRead(PIN_THERMOSTATE);
  String auxStatus,httpRequest=String(GET_REQUEST_TO_UPLOAD_SAMPLES);
  thermostateInterrupt=false;
  int32_t auxRebounds; //int32 instead of uint16 because auxRebouds might have negaative values. To construct the http request
  
  if (thermostateStatus) {auxStatus="ON";} else {auxStatus="OFF";}
  boardSerialPort.println(String(millis())+" - [loop - thermostate_interrupt_triggered] - Interrupt detected - Reason: Thermostate="+String(thermostatInterrupt)+", thermostateStatus="+auxStatus);
  
  //Update rebouds according to the current status
  if (thermostatInterrupt==1) {
    auxRebounds=rebounds;
    if (thermostateStatus) {
      if (debugModeOn) {boardSerialPort.println("        - [loop - thermostate_interrupt_triggered] - Strange, thermostate status was already "+auxStatus+", rebounds="+String(rebounds/2)+". Consider to increase THERMOSTATE_INTERRUPT_DELAY ("+String(THERMOSTATE_INTERRUPT_DELAY)+" ms)");}
    }
    else {
      thermostateStatus=true; auxStatus="ON";
      if (debugModeOn) {boardSerialPort.println("        - [loop - thermostate_interrupt_triggered] - Thermostate status goes to "+auxStatus+", rebounds="+String(rebounds/2));}
      rebounds=0;
    }
  }
  else {
    auxRebounds=-rebounds;
    if (thermostateStatus) {
      thermostateStatus=false; auxStatus="OFF";
      if (debugModeOn) {boardSerialPort.println("        - [loop - thermostate_interrupt_triggered] - Thermostate status goes to "+auxStatus+", rebounds="+String(rebounds/2));}  
      rebounds=0;
    }
    else {
      if (debugModeOn) {boardSerialPort.println("        - [loop - thermostate_interrupt_triggered] - Strange, thermostate status was already "+auxStatus+", rebounds="+String(rebounds/2)+". Consider to increase THERMOSTATE_INTERRUPT_DELAY ("+String(THERMOSTATE_INTERRUPT_DELAY)+" ms)");}
    }
  }

  //Send the http cloud update
  /*
    Thermostat ON  - HTTP Request => "GET /lar-to/?device=boiler-temp-relay&local_ip_address=192.168.100.192&relay_status=1&counts=101 HTTP/1.0"
    Thermostat OFF - HTTP Request => "GET /lar-to/?device=boiler-temp-relay&local_ip_address=192.168.100.192&relay_status=0&counts=-1737 HTTP/1.0"
  */
  httpRequest=httpRequest+"device="+device+"&local_ip_address="+IpAddress2String(WiFi.localIP())+
    "&relay_status="+String(thermostatInterrupt)+"&counts="+String(auxRebounds/2)+" HTTP/1.1";
  sendHttpRequest(debugModeOn,serverToUploadSamplesIPAddress,SERVER_UPLOAD_PORT,httpRequest,false); //Send http update
}

void gas_sample(bool debugModeOn) {
 /******************************************************
   Function gas_sample
   Target: Regular actions every SAMPLE_PERIOD seconds to take gas samples
   Parameters:
    debugModeOn: Print out the logs or not
   *****************************************************/ 

  float h2_ppm=0,lpg_ppm=0,ch4_ppm=0,co_ppm=0,alcohol_ppm=0;
  boardSerialPort.print(String(nowTimeGlobal)+" - [loop - SAMPLE_PERIOD] - Taking GAS samples. - ");
  
  //calculate_R0(); //This is to calculate R0 when the MQ5 sensor is replaced. Don't use it for regular working
  gasRatioSample=get_resistence_ratio(debugModeOn); //This is the current ratio RS/R0 - 6.5 for clean air
  if (gasRatioSample>4.5) {
    gasClear=1;
    if (debugModeOn) boardSerialPort.println("\n         [loop - SAMPLE_PERIOD] - Clean air detected. Digital sensor input: "+String(digitalRead(PIN_GAS_SENSOR_D0))+" (1=NO GAS, 0=GAS)");
    else boardSerialPort.println("Clean air detected. Digital sensor input: "+String(digitalRead(PIN_GAS_SENSOR_D0))+" (1=NO GAS, 0=GAS)");
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

    boardSerialPort.println(""); //Just print new line
    if (gasTypes > 0 && gasTypes != 0x20) {
      boardSerialPort.println("         [loop - SAMPLE_PERIOD] - GAS detected. Digital input: "+String(digitalRead(PIN_GAS_SENSOR_D0))+" (1=NO GAS, 0=GAS)");
      if ((gasTypes & 0x1) > 0) boardSerialPort.println("         [loop - SAMPLE_PERIOD] -    H2="+String(h2_ppm)+" ppm");
      if ((gasTypes & 0x2) > 0) boardSerialPort.println("         [loop - SAMPLE_PERIOD] -    LPG="+String(lpg_ppm)+" ppm");
      if ((gasTypes & 0x4) > 0) boardSerialPort.println("         [loop - SAMPLE_PERIOD] -    CH4="+String(ch4_ppm)+" ppm");
      if ((gasTypes & 0x8) > 0) boardSerialPort.println("         [loop - SAMPLE_PERIOD] -    CO="+String(co_ppm)+" ppm");
      if ((gasTypes & 0x10) > 0) boardSerialPort.println("         [loop - SAMPLE_PERIOD] -    ALCOHOL="+String(alcohol_ppm)+" ppm");
    }
    if (gasTypes > 0x20)
      {boardSerialPort.println("         [loop - SAMPLE_PERIOD] - Other GASes detected out of range 200-10000, so not valid. Digital input: "+String(digitalRead(PIN_GAS_SENSOR_D0))+" (1=NO GAS, 0=GAS)");}
    else if (gasTypes == 0x20)
      {boardSerialPort.println("         [loop - SAMPLE_PERIOD] - GAS detected out of range 200-10000, so not valid. Digital input: "+String(digitalRead(PIN_GAS_SENSOR_D0))+" (1=NO GAS, 0=GAS)");}
    if ((h2_ppm>0 && h2_ppm<200) || h2_ppm>10000) boardSerialPort.println("         [loop - SAMPLE_PERIOD] -    H2="+String(h2_ppm)+" ppm. Discard that value.");
    if ((lpg_ppm>0 && lpg_ppm<200) || lpg_ppm>10000) boardSerialPort.println("         [loop - SAMPLE_PERIOD] -    LPG="+String(lpg_ppm)+" ppm. Discard that value.");
    if ((ch4_ppm>0 && ch4_ppm<200) || ch4_ppm>10000) boardSerialPort.println("         [loop - SAMPLE_PERIOD] -    CH4="+String(ch4_ppm)+" ppm. Discard that value.");
    if ((co_ppm>0 && co_ppm<200) || co_ppm>10000) boardSerialPort.println("         [loop - SAMPLE_PERIOD] -    CO="+String(co_ppm)+" ppm. Discard that value.");
    if ((alcohol_ppm>0 && alcohol_ppm<200) || alcohol_ppm>10000) boardSerialPort.println("         [loop - SAMPLE_PERIOD] -    ALCOHOL="+String(alcohol_ppm)+" ppm. Discard that value.");
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
  samples["H2"] = String(h2_ppm);
  samples["LPG"] = String(lpg_ppm);
  samples["CH4"] = String(ch4_ppm);
  samples["CO"] = String(co_ppm);
  samples["ALCOHOL"] = String(alcohol_ppm);
  samples["Clean_air"] = gasClear==1?"OFF":"ON";
  samples["GAS_interrupt"] = gasInterrupt==1?"ON":"OFF";
  iconGasInterrupt=gasInterrupt==1?String("mdi:electric-switch-closed"):String("mdi:electric-switch");
  samples["Thermostate_interrupt"] = thermostateInterrupt==1?"ON":"OFF";
  iconThermInterrupt=thermostateInterrupt==1?String("mdi:electric-switch-closed"):String("mdi:electric-switch");
  samples["Thermostate_auxStatus"] = thermostateStatus==1?"ON":"OFF";
  if (thermostateStatus) iconThermStatus=String("mdi:radiator");
  else {if (digitalRead(PIN_RL1) && !digitalRead(PIN_RL2)) iconThermStatus=String("mdi:radiator-off"); else iconThermStatus=String("mdi:radiator-disabled");}
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
  samples["errorsNTPCnt"] = String(errorsNTPCnt);
  samples["errorsHTTPUptsCnt"] = String(errorsHTTPUptsCnt);
  samples["errorsMQTTCnt"] = String(errorsMQTTCnt);
  samples["bootCount"] = String(bootCount);
  samples["resetCount"] = String(resetCount);
}

void temperature_sample(bool debugModeOn) {
 /******************************************************
   Function temperature_sample
   Target: Take temp and hum samples
   Parameters:
    debugModeOn: Print out the logs or not
   *****************************************************/ 

  boardSerialPort.print(String(millis())+" - [loop - temperature_sample] - Taking Temp & Hum samples.");
  
  tempHumSensor.read();
   
  valueHum=tempHumSensor.getHumidityCompensated();
  tempSensor=tempHumSensor.getTemperature(); //Sample got from the sensor
  valueT=0.9944*tempHumSensor.getTemperature()-0.8073; //Calibrated value from IoT_Co2_Sensor

  if (valueT < -50.0) valueT=-50;  //Discarding potential wrong values
  //if (debugModeOn) {boardSerialPort.println("         [loop - temperature_sample] - valueT="+String(valueT)+", valueHum="+String(valueHum));}
  boardSerialPort.println(" Temp="+String(valueT)+"ºC, Hum="+String(valueHum)+"%");

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
  boardSerialPort.println(String(millis())+" - [loop - mqtt_publish_samples] - Publishing samples.");
  
  if (wifiEnabled && mqttServerEnabled && WiFi.status()==WL_CONNECTED) {
    MqttSyncCurrentStatus=MqttSyncSendStatus;
    
    if (mqttClient.connected()) {
      //MQTT Client connected
      //mqttTopicName=the-iot-factory/boiler-relay-controlv2-E02940

      //Publish sample values message under mqttTopicName (the-iot-factory propietary), not retain in the server
      String packetIddateUpdate=String(mqttClient.publish(String(mqttTopicName+"/dateUpdate").c_str(), 0, false, JSON.stringify(samples["dateUpdate"]).c_str()));
      String packetIddatedevice_name=String(mqttClient.publish(String(mqttTopicName+"/device_name").c_str(), 0, false, JSON.stringify(samples["device_name"]).c_str()));
      String packetIdversion=String(mqttClient.publish(String(mqttTopicName+"/version").c_str(), 0, false, JSON.stringify(samples["version"]).c_str()));
      String packetIdstartTime=String(mqttClient.publish(String(mqttTopicName+"/startTime").c_str(), 0, false, JSON.stringify(samples["startTime"]).c_str()));
      String packetIdupTime=String(mqttClient.publish(String(mqttTopicName+"/upTime").c_str(), 0, false, JSON.stringify(samples["upTime"]).c_str()));
      String packetIdupTimeSeconds=String(mqttClient.publish(String(mqttTopicName+"/upTimeSeconds").c_str(), 0, false, JSON.stringify(samples["upTimeSeconds"]).c_str()));
      String packetIdH2=String(mqttClient.publish(String(mqttTopicName+"/H2").c_str(), 0, false, JSON.stringify(samples["H2"]).c_str()));
      String packetIdLPG=String(mqttClient.publish(String(mqttTopicName+"/LPG").c_str(), 0, false, JSON.stringify(samples["LPG"]).c_str()));
      String packetIdCH4=String(mqttClient.publish(String(mqttTopicName+"/CH4").c_str(), 0, false, JSON.stringify(samples["CH4"]).c_str()));
      String packetIdCO=String(mqttClient.publish(String(mqttTopicName+"/CO").c_str(), 0, false, JSON.stringify(samples["CO"]).c_str()));
      String packetIdALCOHOL=String(mqttClient.publish(String(mqttTopicName+"/ALCOHOL").c_str(), 0, false, JSON.stringify(samples["ALCOHOL"]).c_str()));
      String packetIdClean_air=String(mqttClient.publish(String(mqttTopicName+"/Clean_air").c_str(), 0, false, JSON.stringify(samples["Clean_air"]).c_str()));
      String packetIdGAS_interrupt=String(mqttClient.publish(String(mqttTopicName+"/GAS_interrupt").c_str(), 0, false, JSON.stringify(samples["GAS_interrupt"]).c_str()));
      String packetIdThermostate_interrupt=String(mqttClient.publish(String(mqttTopicName+"/Thermostate_interrupt").c_str(), 0, false, JSON.stringify(samples["Thermostate_interrupt"]).c_str()));
      String packetIdThermostate_status=String(mqttClient.publish(String(mqttTopicName+"/Thermostate_status").c_str(), 0, false, JSON.stringify(samples["Thermostate_status"]).c_str()));
      String packetIdSSID=String(mqttClient.publish(String(mqttTopicName+"/SSID").c_str(), 0, false, JSON.stringify(samples["SSID"]).c_str()));
      String packetIdBSSID=String(mqttClient.publish(String(mqttTopicName+"/BSSID").c_str(), 0, false, JSON.stringify(samples["BSSID"]).c_str()));
      String packetIdSIGNAL=String(mqttClient.publish(String(mqttTopicName+"/SIGNAL").c_str(), 0, false, JSON.stringify(samples["SIGNAL"]).c_str()));
      String packetIdRSSI=String(mqttClient.publish(String(mqttTopicName+"/RSSI").c_str(), 0, false, JSON.stringify(samples["RSSI"]).c_str()));
      String packetIdENCRYPTION=String(mqttClient.publish(String(mqttTopicName+"/ENCRYPTION").c_str(), 0, false, JSON.stringify(samples["ENCRYPTION"]).c_str()));
      String packetIdCHANNEL=String(mqttClient.publish(String(mqttTopicName+"/CHANNEL").c_str(), 0, false, JSON.stringify(samples["CHANNEL"]).c_str()));
      String packetIdMODE=String(mqttClient.publish(String(mqttTopicName+"/MODE").c_str(), 0, false, JSON.stringify(samples["MODE"]).c_str()));
      String packetIdtempSensor=String(mqttClient.publish(String(mqttTopicName+"/tempSensor").c_str(), 0, false, JSON.stringify(samples["tempSensor"]).c_str()));
      String packetIdtemperature=String(mqttClient.publish(String(mqttTopicName+"/temperature").c_str(), 0, false, JSON.stringify(samples["temperature"]).c_str()));
      String packetIdhumidity=String(mqttClient.publish(String(mqttTopicName+"/humidity").c_str(), 0, false, JSON.stringify(samples["humidity"]).c_str()));
      String packetIdNTP=String(mqttClient.publish(String(mqttTopicName+"/NTP").c_str(), 0, false, JSON.stringify(samples["NTP"]).c_str()));
      String packetIdHTTP_CLOUD=String(mqttClient.publish(String(mqttTopicName+"/HTTP_CLOUD").c_str(), 0, false, JSON.stringify(samples["HTTP_CLOUD"]).c_str()));
      String packetIdRelay1=String(mqttClient.publish(String(mqttTopicName+"/Relay1").c_str(), 0, false, JSON.stringify(samples["Relay1"]).c_str()));
      String packetIdRelay2=String(mqttClient.publish(String(mqttTopicName+"/Relay2").c_str(), 0, false, JSON.stringify(samples["Relay2"]).c_str()));
      mqttClient.publish(String(mqttTopicName+"/errorsWiFiCnt").c_str(), 0, false, JSON.stringify(samples["errorsWiFiCnt"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/errorsNTPCnt").c_str(), 0, false, JSON.stringify(samples["errorsNTPCnt"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/errorsHTTPUptsCnt").c_str(), 0, false, JSON.stringify(samples["errorsHTTPUptsCnt"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/errorsMQTTCnt").c_str(), 0, false, JSON.stringify(samples["errorsMQTTCnt"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/bootCount").c_str(), 0, false, JSON.stringify(samples["bootCount"]).c_str());
      mqttClient.publish(String(mqttTopicName+"/resetCount").c_str(), 0, false, JSON.stringify(samples["resetCount"]).c_str());
      
      //Home Assistant support
      //Publish sample values message, not retain in the server
      mqttClient.publish(String(mqttTopicName+"/LWT").c_str(), 0, false, "Online\0"); //Availability message, not retain in the broker. This makes HA to subscribe to the */SENSOR topic if not already done
      getLocalTime(&nowTimeInfo);strftime(s,sizeof(s),"%Y-%m-%dT%H:%M:%S",&nowTimeInfo); //Time in format 2024-08-24T07:56:25
      String message=String("{\"Time\":\""+String(s)+"\",\"SAMPLES\":"+JSON.stringify(samples)+"}");
      String packetIdHA=String(mqttClient.publish(String(mqttTopicName+"/SENSOR").c_str(), 0, false, message.c_str()));

      if (debugModeOn) boardSerialPort.println(String(millis())+" - [loop - mqtt_publish_samples] - new MQTT messages published:\n    "+mqttTopicName+"/dateUpdate "+JSON.stringify(samples["dateUpdate"])+", packetIddateUpdate="+packetIddateUpdate+
                    "\n    "+mqttTopicName+"/device_name "+JSON.stringify(samples["device_name"])+", packetIddatedevice_name="+packetIddatedevice_name+", "+mqttTopicName+"/version "+JSON.stringify(samples["version"])+", packetIdversion="+packetIdversion+
                    "\n    "+mqttTopicName+"/startTime "+JSON.stringify(samples["startTime"])+", packetIdstartTime="+packetIdstartTime+", "+mqttTopicName+"/upTime "+JSON.stringify(samples["upTime"])+", packetIdupTime="+packetIdupTime+", "+mqttTopicName+"/upTimeSeconds "+JSON.stringify(samples["upTimeSeconds"])+", packetIdupTimeSeconds="+packetIdupTimeSeconds+
                    "\n    "+mqttTopicName+"/H2 "+JSON.stringify(samples["H2"])+", packetIdH2="+packetIdH2+", "+mqttTopicName+"/LPG "+JSON.stringify(samples["LPG"])+", packetIdLPG="+packetIdLPG+
                    "\n    "+mqttTopicName+"/CH4 "+JSON.stringify(samples["CH4"])+", packetIdCH4="+packetIdCH4+", "+mqttTopicName+"/CO "+JSON.stringify(samples["CO"])+", packetIdCO="+packetIdCO+
                    "\n    "+mqttTopicName+"/ALCOHOL "+JSON.stringify(samples["ALCOHOL"])+", packetIdALCOHOL="+packetIdALCOHOL+", "+mqttTopicName+"/Clean_air "+JSON.stringify(samples["Clean_air"])+", packetIdClean_air="+packetIdClean_air+
                    "\n    "+mqttTopicName+"/GAS_interrupt "+JSON.stringify(samples["GAS_interrupt"])+", packetIdGAS_interrupt="+packetIdGAS_interrupt+", "+mqttTopicName+"/Thermostate_interrupt "+JSON.stringify(samples["Thermostate_interrupt"])+", packetIdThermostate_interrupt="+packetIdThermostate_interrupt+
                    "\n    "+mqttTopicName+"/Thermostate_status "+JSON.stringify(samples["Thermostate_status"])+", packetIdThermostate_status="+packetIdThermostate_status+", "+mqttTopicName+"/SSID "+JSON.stringify(samples["SSID"])+", packetIdSSID="+packetIdSSID+
                    "\n    "+mqttTopicName+"/BSSID "+JSON.stringify(samples["BSSID"])+", packetIdupBSSID="+packetIdBSSID+", "+mqttTopicName+"/SIGNAL "+JSON.stringify(samples["SIGNAL"])+", packetIdSIGNAL="+packetIdSIGNAL+
                    "\n    "+mqttTopicName+"/RSSI "+JSON.stringify(samples["RSSI"])+", packetIdRSSI="+packetIdRSSI+", "+mqttTopicName+"/ENCRYPTION "+JSON.stringify(samples["ENCRYPTION"])+", packetIdENCRYPTION="+packetIdENCRYPTION+
                    "\n    "+mqttTopicName+"/CHANNEL "+JSON.stringify(samples["CHANNEL"])+", packetIdCHANNEL="+packetIdCHANNEL+", "+mqttTopicName+"/MODE "+JSON.stringify(samples["MODE"])+", packetIdMODE="+packetIdMODE+
                    "\n    "+mqttTopicName+"/tempSensor "+JSON.stringify(samples["tempSensor"])+", packetIdtempSensor="+packetIdtempSensor+", "+mqttTopicName+"/temperature "+JSON.stringify(samples["temperature"])+", packetIdtemperature="+packetIdtemperature+
                    "\n    "+mqttTopicName+"/humidity "+JSON.stringify(samples["humidity"])+", packetIdhumidity="+packetIdhumidity+
                    "\n    "+mqttTopicName+"/NTP "+JSON.stringify(samples["NTP"])+", packetIdNTP="+packetIdNTP+", "+mqttTopicName+"/HTTP_CLOUD "+JSON.stringify(samples["HTTP_CLOUD"])+", packetIdHTTP_CLOUD="+packetIdHTTP_CLOUD+
                    "\n    "+mqttTopicName+"/Relay1 "+JSON.stringify(samples["Relay1"])+", packetIdRelay1="+packetIdRelay1+", "+mqttTopicName+"/Relay2 "+JSON.stringify(samples["Relay2"])+", packetIdRelay2="+packetIdRelay2+
                    "\n    "+mqttTopicName+"/SENSOR '"+message+"', packetIdHA="+packetIdHA);

      //Publish HA Discovery messages at random basis to make sure HA always recives the Discovery Packet
      // even if it didn't receive it after it rebooted due to network issues or whatever - v1.9.2
      if (random(0,33) < 2)  //random < 2 ==> probability ~3%, ==> ~1 every 10 min (at samples/20s rate)) v1.9.2
        mqttClientPublishHADiscovery(mqttTopicName,device,WiFi.localIP().toString()); 

      MqttSyncCurrentStatus=MqttSyncOnStatus;
    }
    else {
      //MQTT Client disconnected
      //Connect to MQTT broker
      if (debugModeOn) boardSerialPort.println(String(millis())+" - [loop - mqtt_publish_samples] - new MQTT messages can't be published as MQTT broker is disconnected. Trying to get connected again...\n             MqttSyncLastStatus("+String(MqttSyncLastStatus)+")!=MqttSyncCurrentStatus("+String(MqttSyncCurrentStatus)+")");
      MqttSyncCurrentStatus=MqttSyncOffStatus;
      errorsMQTTCnt++;
    }
  }
}