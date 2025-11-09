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
    printLogln("           + heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));
  }
  else printLogln(String(nowTimeGlobal)+" - [loop - WIFI_RECONNECT_PERIOD] - wifiCurrentStatus="+String(wifiCurrentStatus)+". heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));

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

  if (debugModeOn) {printLogln(String(millis())+" - [loop - WIFI_RECONNECT_PERIOD] - wifiReconnectPeriod - exit, lastTimeWifiReconnectionCheck="+String(lastTimeWifiReconnectionCheck)+". heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  else printLogln(String(nowTimeGlobal)+" - [loop - WIFI_RECONNECT_PERIOD] - Exit. wifiCurrentStatus="+String(wifiCurrentStatus)+". heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));
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

  if (debugModeOn) {printLogln(String(nowTimeGlobal)+" - [loop - NTP_KO_CHECK_PERIOD] - Last lastTimeNTPCheck="+String(lastTimeNTPCheck)+", auxLoopCounter2="+String(auxLoopCounter2)+", whileLoopTimeLeft="+String(whileLoopTimeLeft)+". heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  else printLog(String(nowTimeGlobal)+" - [loop - NTP_KO_CHECK_PERIOD] - CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+". heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));
    
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
      printLog("        [loop - NTP_KO_CHECK_PERIOD] - "+String(ntpServers[ntpServerIndex])+" - ");printLog(&nowTimeInfo,"- NTP sync done. Exit - Time: %d/%m/%Y - %H:%M:%S"); //boardSerialPort.println(&nowTimeInfo,"- NTP sync done. Exit - Time: %d/%m/%Y - %H:%M:%S");
      printLogln(". heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size())); //----->
    }
    else {printLog(&nowTimeInfo," - NTP sync done. Exit - Time: %d/%m/%Y - %H:%M:%S. ");printLogln(" - "+String(ntpServers[ntpServerIndex])+" - CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+". heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  }
  else {
    if( wifiCurrentStatus==wifiOffStatus || !wifiEnabled) {if (forceNTPCheck) forceNTPCheck=false;} //v0.9.9 If no WiFi, don't enter in NTP_KO_CHECK_PERIOD even if it was BREAK or ABORT in previous intercation
    getLocalTime(&nowTimeInfo);
    if (debugModeOn) {
      ///printLogln(String(millis())+" - [loop - NTP_KO_CHECK_PERIOD] - errorsNTPCnt="+String(errorsNTPCnt)+", CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+", lastTimeNTPCheck="+String(lastTimeNTPCheck));
      printLog("        [loop - NTP_KO_CHECK_PERIOD] - "+String(ntpServers[ntpServerIndex])+" - ");printLog(&nowTimeInfo,"- No need for NTP sync. Exit - Time: %d/%m/%Y - %H:%M:%S");
      printLogln(". heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size())); //----->
    }
    else {printLog(&nowTimeInfo," - No need for NTP sync. Exit - Time: %d/%m/%Y - %H:%M:%S. "); printLogln(" - "+String(ntpServers[ntpServerIndex])+" - CloudClockCurrentStatus="+String(CloudClockCurrentStatus)+". heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
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

  if (fromSetup) printLog(String(millis())+" - "); //from Setup
  else printLog("       "); //no from setup
  printLog("[loop - sendHttpRequest] - Sending HTTP request");
  if (!fromSetup) printLogln(". heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));
  else printLogln(" '"+httpRequest+"', server="+IpAddress2String(server)+". heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));

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
      if (debugModeOn) {printLogln("       [loop - sendHttpRequest] - Disconnecting from server. Bye!. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
      client.stop();
    }

    return (NO_ERROR);
  }
  else {
    if (debugModeOn) {printLogln("       [loop - sendHttpRequest] - No WiFi or HTTP Cloud updates are disabled. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
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
  bool updateNeeded=false;
  
  if (thermostateStatus) {auxStatus="ON";} else {auxStatus="OFF";}
  printLogln(String(millis())+" - [loop - thermostate_interrupt_triggered] - Interrupt detected - Reason: Thermostate="+String(auxThermostatInterrupt)+" Status (thermostateStatus) was="+auxStatus+". heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));
  
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
      updateNeeded=true;
    }
  }
  else {
    auxRebounds=-rebounds;
    if (thermostateStatus) {
      thermostateStatus=false; auxStatus="OFF";
      if (debugModeOn) {printLogln("          [loop - thermostate_interrupt_triggered] - Thermostate status goes to "+auxStatus+", rebounds="+String(rebounds/2));}  
      rebounds=0;
      updateNeeded=true;
    }
    else {
      if (debugModeOn) {printLogln("          [loop - thermostate_interrupt_triggered] - Strange, thermostate status was already "+auxStatus+", rebounds="+String(rebounds/2)+". Consider to increase THERMOSTATE_INTERRUPT_DELAY ("+String(THERMOSTATE_INTERRUPT_DELAY)+" ms)");}
    }
  }

  //if (debugModeOn) {printLogln("          [loop - thermostate_interrupt_triggered] - updateNeeded="+String(updateNeeded)+", httpCloudEnabled="+String(httpCloudEnabled)+", CloudSyncCurrentStatus="+String(CloudSyncCurrentStatus)+" (0=CloudSyncOnStatus, 1=CloudSyncSendStatus, 2=CloudSyncOffStatus), wifiCred.activeIndex="+String(wifiCred.activeIndex));}
  //if (debugModeOn) {for (int i=0; i<=2; i++) printLogln("          [loop - thermostate_interrupt_triggered] - wifiCred.wifiSSIDs["+String(i)+"]="+String(wifiCred.wifiSSIDs[i])+", wifiCred.wifiSITEs["+String(i)+"]="+String(wifiCred.wifiSITEs[i])+", wifiCred.SiteAllow["+String(i)+"]="+String(wifiCred.SiteAllow[i]));}
  
  //Send the updates to http cloud, web client and mqtt broker
  if (updateNeeded) {
    samples["Thermostate_status"] = thermostateStatus?"ON":"OFF";
    forceWebEvent=true; forceMQTTpublish=true;  //Web client and MQTT update to inform about the thermostateStatus status
    if (httpCloudEnabled && (CloudSyncCurrentStatus==CloudSyncOnStatus) && wifiCred.SiteAllow[wifiCred.activeIndex]) {
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
  printLogln(String(millis())+" - [loop - thermostate_interrupt_triggered] - Exit now. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));
}

void gas_sample(bool debugModeOn, uint8_t reason) {
 /******************************************************
   Function gas_sample
   Target: Regular actions every SAMPLE_PERIOD seconds to take gas samples. 
           With boiler burning gas (either boiler or heater), gas samples are updated every SAMPLE_PERIOD (short period, typically 20s)
           With no gas burning (neither boiler nor heater), gas samples are updated every SAMPLE_LONG_PERIOD (long period, typically 5m)
   Parameters:
    debugModeOn: Print out the logs or not
    reason: reason for sample readings
      1: Frist loop
      2: Send HA Discovery
      3: SAMPLE_PERIOD
      4: SAMPLE_LONG_PERIOD
      5: gasInterrupt detected
      6: /samples requested from web page
   *****************************************************/ 

  float h2_ppm=0,lpg_ppm=0,ch4_ppm=0,co_ppm=0,alcohol_ppm=0;
  /*boilerStatus => Power > Threshold (50 w)
    boilerOn => Burning gas (flame), due to warming water 
    thermostateStatus => Thermostate is active (or relay active)
    thermostateOn => Burning gas due to heater */
  if (gasInterrupt) printLog(String(nowTimeGlobal)+" - [loop - gas_sample] - GAS interrupt detected. Checking on GAS samples. reason="+String(reason)+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size())+" - ");
  else if (boilerOn || thermostateOn) printLog(String(nowTimeGlobal)+" - [loop - SAMPLE_PERIOD] - Boiler burning gas. Taking GAS samples. reason="+String(reason)+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size())+" - ");
  else if (nowTimeGlobal-firstLoopTime < 2*HA_ADVST_WINDOW) printLog(String(nowTimeGlobal)+" - [loop - gas_sample] - First sample reading to send MQTT message. reason="+String(reason)+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size())+" - ");
  else if (nowTimeGlobal-lastGasSample >= SAMPLE_LONG_PERIOD) printLog(String(nowTimeGlobal)+" - [loop - SAMPLE_LONG_PERIOD - gas_sample] - No gas burning. Taking GAS samples. reason="+String(reason)+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size())+" - ");
  else printLog(String(nowTimeGlobal)+" - [loop - gas_sample] - Taking samples to update web request. heapSize="+String(esp_get_free_heap_size())+", reason="+String(reason)+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size())+" - ");
  
  //calculate_R0(); //This is to calculate R0 when the MQ5 sensor is replaced. Don't use it for regular working
  gasRatioSample=get_resistence_ratio(debugModeOn); //This is the current ratio RS/R0 - 6.5 for clean air
  if (gasRatioSample>4.5) {
    gasClear=true;
    if (debugModeOn) printLogln("\n         [loop - gas_sample] - Clean air detected. Digital sensor input: "+String(digitalRead(PIN_GAS_SENSOR_D0))+" (1=NO GAS, 0=GAS)");
    else printLogln("Clean air detected. Digital sensor input: "+String(digitalRead(PIN_GAS_SENSOR_D0))+" (1=NO GAS, 0=GAS)");
  }
  else {
    gasClear=false;
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
      printLogln("         [loop - gas_sample] - GAS detected. Digital input: "+String(digitalRead(PIN_GAS_SENSOR_D0))+" (1=NO GAS, 0=GAS)");
      if ((gasTypes & 0x1) > 0) printLogln("         [loop - gas_sample] -    H2="+String(h2_ppm)+" ppm");
      if ((gasTypes & 0x2) > 0) printLogln("         [loop - gas_sample] -    LPG="+String(lpg_ppm)+" ppm");
      if ((gasTypes & 0x4) > 0) printLogln("         [loop - gas_sample] -    CH4="+String(ch4_ppm)+" ppm");
      if ((gasTypes & 0x8) > 0) printLogln("         [loop - gas_sample] -    CO="+String(co_ppm)+" ppm");
      if ((gasTypes & 0x10) > 0) printLogln("         [loop - gas_sample] -    ALCOHOL="+String(alcohol_ppm)+" ppm");
    }
    if (gasTypes > 0x20)
      {printLogln("         [loop - gas_sample] - Other GASes detected out of range 200-10000, so not valid. Digital input: "+String(digitalRead(PIN_GAS_SENSOR_D0))+" (1=NO GAS, 0=GAS)");}
    else if (gasTypes == 0x20)
      {printLogln("         [loop - gas_sample] - GAS detected out of range 200-10000, so not valid. Digital input: "+String(digitalRead(PIN_GAS_SENSOR_D0))+" (1=NO GAS, 0=GAS)");}
    if ((h2_ppm>0 && h2_ppm<200) || h2_ppm>10000) printLogln("         [loop - gas_sample] -    H2="+String(h2_ppm)+" ppm. Discard that value.");
    if ((lpg_ppm>0 && lpg_ppm<200) || lpg_ppm>10000) printLogln("         [loop - gas_sample] -    LPG="+String(lpg_ppm)+" ppm. Discard that value.");
    if ((ch4_ppm>0 && ch4_ppm<200) || ch4_ppm>10000) printLogln("         [loop - gas_sample] -    CH4="+String(ch4_ppm)+" ppm. Discard that value.");
    if ((co_ppm>0 && co_ppm<200) || co_ppm>10000) printLogln("         [loop - gas_sample] -    CO="+String(co_ppm)+" ppm. Discard that value.");
    if ((alcohol_ppm>0 && alcohol_ppm<200) || alcohol_ppm>10000) printLogln("         [loop - gas_sample] -    ALCOHOL="+String(alcohol_ppm)+" ppm. Discard that value.");
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
  samples["upTimeSeconds"] =  nowInSeconds;
  samples["device_name"] = device;
  samples["version"] = String(VERSION);
  samples["ipAddress"] = WiFi.localIP().toString();
  samples["boilerStatus"] = boilerStatus?"ON":"OFF";
  samples["boilerOn"] = boilerOn?"ON":"OFF";
  samples["H2"] = h2_ppm;
  samples["LPG"] = lpg_ppm;
  samples["CH4"] = ch4_ppm;
  samples["CO"] = co_ppm;
  samples["ALCOHOL"] = alcohol_ppm;
  samples["Clean_air"] = gasClear?"OFF":"ON";
  samples["GAS_interrupt"] = gasInterrupt?"ON":"OFF";
  iconGasInterrupt=gasInterrupt?String("mdi:electric-switch-closed"):String("mdi:electric-switch");
  samples["Thermostate_interrupt"] = thermostateInterrupt?"ON":"OFF";
  iconThermInterrupt=thermostateInterrupt?String("mdi:electric-switch-closed"):String("mdi:electric-switch");
  samples["Thermostate_status"] = thermostateStatus?"ON":"OFF";
  if (thermostateStatus) iconThermStatus=String("mdi:radiator");
  else {if (digitalRead(PIN_RL1) && !digitalRead(PIN_RL2)) iconThermStatus=String("mdi:radiator-off"); else iconThermStatus=String("mdi:radiator-disabled");}
  samples["Thermostate_on"] = thermostateOn?"ON":"OFF";
  samples["SSID"] = WiFi.SSID();
  wifiNet.RSSI=WiFi.RSSI();
  samples["SIGNAL"] = wifiNet.RSSI;
  if (wifiNet.RSSI>=WIFI_100_RSSI) wifiCurrentStatus=wifi100Status;
  else if (wifiNet.RSSI>=WIFI_075_RSSI) wifiCurrentStatus=wifi75Status;
  else if (wifiNet.RSSI>=WIFI_050_RSSI) wifiCurrentStatus=wifi50Status;
  else if (wifiNet.RSSI>=WIFI_025_RSSI) wifiCurrentStatus=wifi25Status;
  else if (wifiNet.RSSI<WIFI_000_RSSI) wifiCurrentStatus=wifi0Status;
  switch (wifiCurrentStatus)
  {
    case wifi0Status:
      samples["RSSI"] = 0;
      iconWifi=String("mdi:wifi-strength-alert-outline");
      break;
    case wifi25Status:
      samples["RSSI"] = 25;
      iconWifi=String("mdi:wifi-strength-1-alert");
      break;
    case wifi50Status:
      samples["RSSI"] = 50;
      iconWifi=String("mdi:wifi-strength-2");
      break;
    case wifi75Status:
      samples["RSSI"] = 75;
      iconWifi=String("mdi:wifi-strength-3");
      break;
    case wifi100Status:
      samples["RSSI"] = 100;
      iconWifi=String("mdi:wifi-strength-4");
      break;
    default:
      samples["RSSI"] = 0;
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
  samples["CHANNEL"] = WiFi.channel();
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
  samples["errorsWiFiCnt"] = errorsWiFiCnt;
  samples["errorsConnectivityCnt"] = errorsConnectivityCnt;
  samples["errorsNTPCnt"] = errorsNTPCnt;
  samples["errorsHTTPUptsCnt"] = errorsHTTPUptsCnt;
  samples["errorsMQTTCnt"] = errorsMQTTCnt;
  samples["errorsWebServerCnt"] = errorsWebServerCnt;
  samples["errorsJSONCnt"] = errorsJSONCnt;
  samples["bootCount"] = bootCount; //Total since last update
  samples["resetNormalCount"] = bootCount-resetSWCount-resetCount; //Normal resets - resetSWCount includes preventive Counts
  samples["resetSWCount"] = resetSWCount; //Reset due to Restart HA Button, web reset, preventive reset, etc (all ESP.restart cases)
  samples["resetSWWebCount"] = resetSWWebCount; //resets done from the web maintenance page
  samples["resetSWMqttCount"] = resetSWMqttCount; //resets done from the HA (mqqtt) page
  samples["resetSWUpgradeCount"] = resetSWUpgradeCount; //resets done due to firmware upgrade from maintenance web page
  samples["resetWebServerCnt"] = resetWebServerCnt; //resets due to web server not serving web pages
  samples["resetPreventiveJSONCount"] = resetPreventiveJSONCount; //resets due to JSON errors limit exceeded
  samples["resetPreventiveCount"] = resetPreventiveCount; //Preventive resets (low heap situation) different than web server low heap
  samples["resetPreventiveWebServerCount"] = resetPreventiveWebServerCount; //Preventive web server resets (low heap situation)
  samples["resetCount"] = resetCount; //uncontrolled resets
  samples["heapSize"] = esp_get_free_heap_size();
  samples["minHeapSeen"] = minHeapSeen;
  samples["minHeapSinceBoot"] = minHeapSinceBoot;
  samples["minHeapSinceUpgrade"] = minHeapSinceUpgrade;
  samples["heapBlockSize"]=heapBlockSize;
  samples["minMaxHeapBlockSizeSinceBoot"]=minMaxHeapBlockSizeSinceBoot;
  samples["minMaxHeapBlockSizeSinceUpgrade"]=minMaxHeapBlockSizeSinceUpgrade;
  samples["reboot"] = String("online");
  samples["reset_time_counters"] = String("online");
  
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

  //Additional Objectes needed in info.html
  samples["TZName"]=TZName;
  samples["TZEnvVariable"]=TZEnvVariable;
  samples["tempHumSensorType"]=tempHumSensorType;
  samples["macAddress"]=String(WiFi.macAddress());
  samples["netMask"]=WiFi.subnetMask().toString();
  samples["defaultGW"]=WiFi.gatewayIP().toString();
  samples["ntpServer"]=CloudClockCurrentStatus==CloudClockOnStatus?ntpServers[ntpServerIndex]:String("Not Available");
  samples["CloudSyncCurrentStatus"]=CloudSyncCurrentStatus;
  samples["cloudServicesURL"]=String("http://"+serverToUploadSamplesIPAddress.toString())+String(GET_REQUEST_TO_UPLOAD_SAMPLES).substring(4,String(GET_REQUEST_TO_UPLOAD_SAMPLES).length()-1);
  samples["mqttServerEnabled"]=mqttServerEnabled;
  samples["MqttSyncCurrentStatus"]=MqttSyncCurrentStatus;
  samples["secureMqttEnabled"]=secureMqttEnabled;
  samples["mqttServer"]=mqttServer;
  samples["mqttTopicName"]=mqttTopicName;

  //Additonal objectes needed in basic.html
  samples["userName"]=userName;
  samples["SSID_BK1"]=wifiCred.wifiSSIDs[1];
  samples["SSID_BK2"]=wifiCred.wifiSSIDs[2];
  samples["SITE"]=wifiCred.wifiSITEs[0];
  samples["SITE_BK1"]=wifiCred.wifiSITEs[1];
  samples["SITE_BK2"]=wifiCred.wifiSITEs[2];
  samples["SITE_ALLOWED"]=wifiCred.SiteAllow[0];
  samples["SITE_BK1_ALLOWED"]=wifiCred.SiteAllow[1];
  samples["SITE_BK2_ALLOWED"]=wifiCred.SiteAllow[2];
  samples["SITE_ACTIVE"]=wifiCred.activeIndex;
  samples["ntpServer1"]=ntpServers[0];
  samples["ntpServer2"]=ntpServers[1];
  samples["ntpServer3"]=ntpServers[2];
  samples["ntpServer4"]=ntpServers[3];

  //Additonal objectes needed in cloud.html
  samples["httpCloudEnabled"]=httpCloudEnabled;
  if (powerMqttTopic!=null) samples["powerMqttTopic"]=powerMqttTopic;
  else printLogln(String(millis())+" - [loop - gas_sample] - Variable powerMqttTopic is null. Strange. Investigate why. samples[\"powerMqttTopic\"] isn't save"); //----->
  samples["powerOnFlameThreshold"]=powerOnFlameThreshold;
  samples["mqttUserName"]=mqttUserName;

  //Additonal objectes needed in maintenance.html
  samples["OTAUpgradeBinAllowed"]=OTAUpgradeBinAllowed;samples["SPIFFSUpgradeBinAllowed"]=SPIFFSUpgradeBinAllowed;
  samples["OTAAvailableSize"]=OTAAvailableSize;samples["SPIFFSAvailableSize"]=SPIFFSAvailableSize;
  samples["error_setup"]=error_setup;samples["SPIFFSErrors"]=SPIFFSErrors;
  samples["serialLogsOn"]=serialLogsOn?"SERIAL_LOGS_ON":"SERIAL_LOGS_OFF";
  samples["webLogsOn"]=webLogsOn?"WEB_LOGS_ON":"WEB_LOGS_OFF";samples["sysLogsOn"]=sysLogsOn?"SYS_LOGS_ON":"SYS_LOGS_OFF";
  samples["sysLogServer"]=sysLogServer;samples["sysLogServerUDPPort"]=sysLogServerUDPPort;


  if (samples["boilerOn"]==null) printLogln(String(millis())+" - [loop - gas_sample] - Variable samples[\"boilerOn\"] is null. Strange. Investigate why."); //----->
  if (samples["powerMqttTopic"]==null) printLogln(String(millis())+" - [loop - gas_sample] - Variable samples[\"powerMqttTopic\"] is null. Strange. Investigate why."); //----->
  printLogln(String(millis())+" - [loop - gas_sample] - Exit now. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size())); //----->
} // -- gas_sample -- 

void temperature_sample(bool debugModeOn) {
 /******************************************************
   Function temperature_sample
   Target: Take temp and hum samples
   Parameters:
    debugModeOn: Print out the logs or not
   *****************************************************/ 

  printLog(String(millis())+" - [loop - temperature_sample] - Taking Temp & Hum samples. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));
  
  tempHumSensor.read();
   
  valueHum=tempHumSensor.getHumidityCompensated();
  tempSensor=tempHumSensor.getTemperature(); //Sample got from the sensor
  valueT=0.9944*tempHumSensor.getTemperature()-0.8073; //Calibrated value from IoT_Co2_Sensor

  if (valueT < -50.0) valueT=-50;  //Discarding potential wrong values
  //if (debugModeOn) {printLogln("         [loop - temperature_sample] - valueT="+String(valueT)+", valueHum="+String(valueHum));}
  printLogln(" Temp="+String(valueT)+"ºC, Hum="+String(valueHum)+"%");

  //Updating JSON object with samples
  samples["tempSensor"] = tempSensor; //Non-calibrated temp
  samples["temperature"] = valueT;    //Calibrated temp
  samples["humidity"] =  valueHum;    //Non-calibrated = calibrated hum

  printLogln(String(millis())+" - [loop - temperature_sample] - Exit now. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size())); //----->
} // -- temperature_sample --


void mqtt_publish_samples(boolean wifiEnabled, boolean mqttServerEnabled, boolean secureMqttEnabled, bool debugModeOn, uint8_t reason) {
  /******************************************************
   Function mqtt_publish_samples
   Target: Publish the samples to the MQTT server
   Parameters:
    wifiEnabled: True if wifi is enabled
    mqttServerEnabled: True if mqtt is enabled
    debugModeOn: Print out the logs or not
    reason: Event that triggered the need to message publish
       0: Don't publish mqtt
       1: Thermostate interrupt
       2: SAMPLE_PERIOD
       3: Gas detected (gas interrupt)
       4: Reset time counters (from Home Assistant - MQTT)
       5: Reset time counters (from web)
       6: Configure debug flag (from Home Assistant - MQTT)
       7: Configure debug flag (from web)
       8: Configure serial logs (from Home Assistant - MQTT)
       9: Configure serial logs (from web)
      10: Configure web logs (from Home Assistant - MQTT)
      11: Configure web logs (from web)
      12: Configure sys logs (from Home Assistant - MQTT)
      13: Configure sys logs (from web)
      14: Configure R1 (from Home Assistant - MQTT)
      15: Configure R1 (from web)
      16: Configure R2 (from Home Assistant - MQTT)
      17: Configure R2 (from web)
      18: MQTT POWER message received
      19: Sending the first HA Discovery after booting up
      255: Default, so SAMPLE_PERIOD
   *****************************************************/ 

  struct tm nowTimeInfo; //36 B
  char s[100];
  printLogln(String(millis())+" - [loop - mqtt_publish_samples] - Publishing message, reason="+String(reason)+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));
  if (reason==0) {printLogln(String(millis())+" - [loop - mqtt_publish_samples] - Don't publish message. Exit . heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));return;}
  
  if (wifiEnabled && mqttServerEnabled && WiFi.status()==WL_CONNECTED) {
    MqttSyncCurrentStatus=MqttSyncSendStatus;
    
    if (mqttClient.connected()) {
      //MQTT Client connected
      //mqttTopicName=the-iot-factory/boiler-relay-controlv2-E02940

      //Publish sample values message under mqttTopicName (the-iot-factory/boiler-relay-controlv2-XXXXXX/SENSOR propietary), not retain in the server
      //Publish sample values message, not retain in the server
      mqttClient.publish(String(mqttTopicName+"/LWT").c_str(), 0, false, "Online\0"); //Availability message, not retain in the broker. This makes HA to subscribe to the */SENSOR topic if not already done
      getLocalTime(&nowTimeInfo);strftime(s,sizeof(s),"%Y-%m-%dT%H:%M:%S",&nowTimeInfo); //Time in format 2024-08-24T07:56:25
      String message=String("{\"Time\":\""+String(s)+"\",\"SAMPLES\":"+JSON.stringify(samples)+"}");
      printLogln(String(millis())+" - [loop - mqtt_publish_samples] - Publish samples message now. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size())); //---->
      mqttClient.publish(String(mqttTopicName+"/SENSOR").c_str(), 0, false, message.c_str());

      if (debugModeOn) printLogln(String(millis())+" - [loop - mqtt_publish_samples] - samples MQTT messages published");
      /*if (debugModeOn) printLogln(String(millis())+" - [loop - mqtt_publish_samples] - samples[\"boilerToday\"]="+JSON.stringify(samples["boilerToday"])+"samples[\"heaterToday\"]="+JSON.stringify(samples["heaterToday"]));*/ //----->
 
      //Home Assistant support
      //Publish HA Discovery messages at random basis to make sure HA always recives the Discovery Packet
      // even if it didn't receive it after it rebooted due to network issues or whatever - v1.9.2
      //long auxRandom=random(0,4);
      //if (((auxRandom < 2) || updateHADiscovery) && !boilerStatus) { //random < 2 ==> probability ~50%, ==> ~1 every 10 min (at samples/5m rate))
      long auxRandom=random(0,47); 
      if (((auxRandom < 1) || updateHADiscovery) && !boilerStatus) { //random < 1 ==> probability ~2,1%, ==> ~1 every 240 min (4 hours) (at samples/5m rate))
                                                    //updateHADiscovery: True if year changed or firstLoop. Update Timer Counters Year in HA MQTT
                                                    //HADiscovery is sent several times (HA_ADVST_WINDOW) after boot up to make sure all the topics are processed - v0.9.7 - ISS007
                                                    //boilerStatus is true if receiving MQTT messages storm from the SmartPlug
        /*uint32_t auxHeap=heap_caps_get_largest_free_block(MALLOC_CAP_8BIT); //ESP.getMaxAllocHeap(); //ESP.getFreeHeap();
        if (auxHeap >= 40000) { //Based on guess
          //printLogln(String(millis())+" - [loop - mqtt_publish_samples] - Sending HADiscovery. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+" > 40000. minHeapSeen="+String(esp_get_minimum_free_heap_size())); //----->
          mqttClientPublishHADiscovery(mqttTopicName,device,WiFi.localIP().toString(),false); 
          printLogln(String(millis())+" - [loop - mqtt_publish_samples] - HADiscovery sent. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size())); //----->
        }
        else printLogln(String(millis())+" - [loop - mqtt_publish_samples] - NOT sending HADiscovery due to heap constraints, heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(auxHeap)+" < 40000. minHeapSeen="+String(esp_get_minimum_free_heap_size())); //----->
        */
        //Heap checks done in mqttClientPublishHADiscovery()
        printLogln(String(millis())+" - [loop - mqtt_publish_samples] - Sending HADiscovery. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size())); //----->
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

  printLogln(String(millis())+" - [loop - mqtt_publish_samples] - Exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));
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

    struct timeOnCounters {
      uint16_t year;                 //Year of the counters. i.e.: 2025
      uint32_t today;               //Current day of the today counter. i.e.: 20250427
      uint32_t yesterday;           //Current day of the yesterday counter. i.e.: 20250426
      uint32_t counterMonths[12];   //Total time on (seconds) of the month. Months 0-11
      uint32_t counterYesterday;    //Total time on (seconds) of yesterday
      uint32_t counterToday;        //Total time on (seconds) of today
    }; //68 B
*/
  

  if (debugModeOn) {printLogln(String(nowTimeGlobal)+" - [loop - ONE_SECOND_PERIOD] - Doing actions every second. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
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
        heaterTimeOnYear.today=auxToday;
        heaterTimeOnYear.yesterday=today;
        heaterTimeOnYear.counterYesterday=heaterTimeOnYear.counterToday;
        heaterTimeOnYear.counterToday=0;
        
        boilerTimeOnYear.today=auxToday;
        boilerTimeOnYear.yesterday=today;
        boilerTimeOnYear.counterYesterday=boilerTimeOnYear.counterToday;
        boilerTimeOnYear.counterToday=0;

        if ((auxTimeInfo.tm_year+1900) > (heaterTimeOnYear.year)) {
          //New year
          previousYear=year;
          year=auxTimeInfo.tm_year+1900;
          memcpy(&heaterTimeOnPreviousYear,&heaterTimeOnYear,sizeof(heaterTimeOnYear)); //Update heaterTimeOnPreviousYear with heaterTimeOnYear
          heaterTimeOnYear.year=year;
          for (uint8_t i=0; i<12; i++) heaterTimeOnYear.counterMonths[i]=0;          
          heaterTimeOnYear.counterYesterday=0;
          heaterTimeOnYear.counterToday=0;

          memcpy(&boilerTimeOnPreviousYear,&boilerTimeOnYear,sizeof(boilerTimeOnYear)); //Update boilerTimeOnPreviousYear with boilerTimeOnYear
          boilerTimeOnYear.year=year;
          for (uint8_t i=0; i<12; i++) boilerTimeOnYear.counterMonths[i]=0;
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

  /*if (debugModeOn) printLogln(String(millis())+" - [loop - mqtt_publish_samples] - Exit - samples[\"boilerToday\"]="+JSON.stringify(samples["boilerToday"])+"samples[\"heaterToday\"]="+JSON.stringify(samples["heaterToday"]));*/ //----->
  if (debugModeOn) printLogln(String(millis())+" - [loop - ONE_SECOND_PERIOD] - Exit now. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size())); //----->
  
  //No time on counter update if there is no NTP sync  
  //Update with nowTimeGlobal to prevent accumulating delays in CHECK periods as this code might take long
  lastTimeSecondCheck=nowTimeGlobal;
} // -- one_second_check_period --

void time_counters_eeprom_update_check_period(bool debugModeOn, uint64_t nowTimeGlobal, bool forceUpdateTimers) {
  /******************************************************
   Function time_counters_eeprom_update_check_period
   Target: Regular actions every TIME_COUNTERS_EEPROM_UPDATE_PERIOD seconds.
    1) Check if EEPROM must be updated with new values of the time counters
   Parameters:
    debugModeOn: Print out the logs or not
    nowTimeGlobal: current time the function was called in milliseconds
   Returns: Nothing
   *****************************************************/

  if (timersEepromUpdate || forceUpdateTimers) {
    //Update EEPROM only if there are new time counter values
    EEPROM.put(0x421,heaterTimeOnYear); EEPROM.put(0x465,heaterTimeOnPreviousYear);
    EEPROM.put(0x4A9,boilerTimeOnYear); EEPROM.put(0x4ED,boilerTimeOnPreviousYear);
    EEPROM.commit();
    if (debugModeOn) printLogln(String(nowTimeGlobal)+" - [loop - eeprom_update_check] - EEPROM updated with variables and counters. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));
    timersEepromUpdate=false;
  }
  else {
    if (debugModeOn) printLogln(String(nowTimeGlobal)+" - [loop - eeprom_update_check] - No need to update EEPROM with variables and counters. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));
  }

  if (!forceUpdateTimers) lastTimeTimerEepromUpdateCheck=nowTimeGlobal;
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
  
  if (debugModeOn) {printLogln(String(millis())+" - [checkURL] - Trying connection to "+String(IpAddress2String(server))+" to send httpRequest: '"+httpRequest+"'. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}

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
    if (debugModeOn) {printLog("      [checkURL] - No server connection. ERROR_WEB_SERVER - Exit - Time: ");getLocalTime(&nowTimeInfo);printLog(&nowTimeInfo, "%d/%m/%Y - %H:%M:%S");printLogln(". heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
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
    if (debugModeOn) {printLog("      [checkURL] - No server connection. ERROR_WEB_SERVER - Exit - Time: ");getLocalTime(&nowTimeInfo);printLog(&nowTimeInfo, "%d/%m/%Y - %H:%M:%S");printLogln(". heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
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
    if (debugModeOn) {printLogln("      [checkURL] - Disconnecting from server. Bye!.");}
    client.stop();
  }

  if (debugModeOn) {printLog(String("      [checkURL] - GOT URL. NO_ERROR - Exit - Time: "));getLocalTime(&nowTimeInfo);printLog(&nowTimeInfo, "%d/%m/%Y - %H:%M:%S");printLogln(". heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
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

  if (debugModeOn) {printLogln(String(nowTimeGlobal)+" - [loop - CONNECTIVITY_CHECK_PERIOD] - Checking connectivity. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  
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
        if (debugModeOn) {printLogln("     [loop - CONNECTIVITY_CHECK_PERIOD] - Connectivity is OK, but web server is KO.  Reinit network services. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    }
    else {
      if (debugModeOn) {printLogln("     [loop - CONNECTIVITY_CHECK_PERIOD] - Connectivity and web server are OK. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    }
  }
  else {
    if (debugModeOn) {printLogln("     [loop - CONNECTIVITY_CHECK_PERIOD] - All network checks are KO. Reinit network services. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  }
  
  lastTimeConnectiviyCheck=nowTimeGlobal;
  if (!webServerOK) return ERROR_WEB_SERVER;
  else if (!connectivityOK) return ERROR_NO_CONNECTIVITY;
  else return (NO_ERROR);
}