#include <Arduino.h>
#include <SoftwareSerial.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Arduino_JSON.h>
#include <AsyncMqttClient.h>
#include "SHT2x.h"
#include "global_setup.h"
//#include "wifiConnection.h" //included in loop.h
//#include "mq5_sensor.h"     //included in loop.h
#ifdef SYSLOG_SERVER
  #include <PicoSyslog.h>
#endif
#include "setup.h"
#include "interrupt.h"
#include "loop.h"
#include "eeprom_utils.h"
#include "webServer.h"
#include "webSocket.h"


//Global variable definitions stored in RTC memory to be persistent during Sleep periods. 8 KB Max
RTC_DATA_ATTR boolean firstBoot=true;  //1B - First boot flag.
RTC_DATA_ATTR boolean OTAUpgradeBinAllowed=false,SPIFFSUpgradeBinAllowed=false; //2*1=2B - v1.2.0 To block SPIFFS upgrade if there is something wrong with SPIFFS partition
RTC_DATA_ATTR uint16_t year=0,previousYear=0,powerOnFlameThreshold=0;
RTC_DATA_ATTR uint32_t yesterday=0,today=0;
RTC_DATA_ATTR enum CloudClockStatus CloudClockCurrentStatus; //4B
RTC_DATA_ATTR enum CloudClockStatus CloudClockLastStatus; //4 B
RTC_DATA_ATTR enum CloudSyncStatus CloudSyncCurrentStatus; // 4B
RTC_DATA_ATTR enum CloudSyncStatus CloudSyncLastStatus; // 4B
RTC_DATA_ATTR enum MqttSyncStatus MqttSyncCurrentStatus; // 4B
RTC_DATA_ATTR enum MqttSyncStatus MqttSyncLastStatus; // 4B
RTC_DATA_ATTR enum wifiStatus wifiCurrentStatus;
RTC_DATA_ATTR struct tm startTimeInfo;
RTC_DATA_ATTR char TZEnvVar[TZ_ENV_VARIABLE_MAX_LENGTH];
RTC_DATA_ATTR uint32_t error_setup=NO_ERROR,minMaxHeapBlockSizeSinceBoot=0xFFFFFFFF,minMaxHeapBlockSizeSinceUpgrade=0xFFFFFFFF,minHeapSinceUpgrade=0xFFFFFFFF,minHeapSinceBoot=0xFFFFFFFF; //1*4=4B
RTC_DATA_ATTR uint8_t forceMQTTpublish=0,bootCount=255,resetCount=0,resetPreventiveCount=0,resetPreventiveWebServerCount=0,resetSWCount=0,resetSWWebCount=0,resetSWMqttCount=0,resetSWUpgradeCount=0,resetWebServerCnt=0,
                      errorsWiFiCnt=0,errorsNTPCnt=0,errorsHTTPUptsCnt=0,errorsMQTTCnt=0,SPIFFSErrors=0,errorsWebServerCnt=0,errorsConnectivityCnt=0;
RTC_DATA_ATTR boolean wifiEnabled=true,forceWifiReconnect=false,forceWEBTestCheck=false,forceWebServerInit=false,forceWebEvent=false,
                      ntpEnabled=true,httpCloudEnabled=true,forceNTPCheck=false,ntpSynced=false,
                      mqttServerEnabled=true,forceMQTTConnect=false,secureMqttEnabled=false,bluetoothEnabled=false,webServerEnabled=false,timersEepromUpdate=false,
                      updateHADiscovery=false,deviceReset=false,factoryReset=false,logTagged=false,reconnectWifiAndRestartWebServer=false,resyncNTPServer=false,powerMeasureEnabled=false,powerMeasureSubscribed=false;
RTC_DATA_ATTR byte mac[6];
RTC_DATA_ATTR uint64_t nowTimeGlobal=0,firstLoopTime=0,lastCheckTime=0,lastTimeWifiReconnectionCheck=0,lastTimeHTTPClouCheck=0,lastTimeNTPCheck=0,
                      lastCloudClockChangeCheck=0,lastTimeHeapReadingCheck=0,lastInterruptTime=0,lastGasSample=0,lastTimeMQTTCheck=0,lastTimeConnectiviyCheck=0,
                      lastMQTTChangeCheck=0,lastTimeSecondCheck=0,lastThermostatOnTime=0,lastBoilerOnTime=0,lastTimeTimerEepromUpdateCheck=0;
RTC_DATA_ATTR ulong wifiReconnectPeriod=WIFI_RECONNECT_PERIOD;
RTC_DATA_ATTR String tempHumSensorType=String(TEMP_HUM_SENSOR_TYPE);
RTC_DATA_ATTR float valueHum=0,tempSensor=0,valueT=0;
RTC_DATA_ATTR AsyncMqttClient mqttClient;
RTC_DATA_ATTR struct timeOnCounters heaterTimeOnYear,heaterTimeOnPreviousYear,boilerTimeOnYear,boilerTimeOnPreviousYear; //61 B each varialbe (heaterTimeOn or boilerTimeOn) - Time in seconds that the heater signal has been on (thermostat or Relay2)
//RTC_DATA_ATTR AsyncWebServer *webServer=new AsyncWebServer(WEBSERVER_PORT); //1*84=84B
//RTC_DATA_ATTR AsyncWebServer *webServer=nullptr;
//RTC_DATA_ATTR AsyncWebSocket *webSocket=nullptr; // Create a WebSocket object for the web console (send logs and receive commands)
RTC_DATA_ATTR AsyncWebServer webServer(WEBSERVER_PORT); //1*84=84B
RTC_DATA_ATTR AsyncWebSocket webSocket(WEBSOCKET_CONSOLE_URI); // Create a WebSocket object for the web console (send logs and receive commands)
RTC_DATA_ATTR AsyncEventSource webEvents(WEBSERVER_SAMPLES_EVENT); //1*104=104B
RTC_DATA_ATTR HardwareSerial boardSerialPort(0); // Serial port is using UART0
RTC_DATA_ATTR String bootLogs; // Initial logs at boot time

//EEPROM MAP - See eeprom_utils.cpp

//Global variable definitions stored in regular RAM. 520 KB Max
bool debugModeOn=DEBUG_MODE_ON,logMessageTOFF=false,logMessageTRL1_ON=false,logMessageTRL2_ON=false,logMessageGAP_OFF=false,
      thermostateInterrupt=false,gasClear=true,gasInterrupt=false,isBeaconAdvertising=false,webServerResponding=false,
      webLogsOn=false,serialLogsOn=debugModeOn,sysLogsOn=false,eepromUpdate=false,firstHASent=false,blockWebServer=false;
bool boilerStatus=false,boilerOn=false, //boilerStatus => Power > Threshold, boilerOn => Burning gas (flame) due to warming water
      thermostateStatus=false,thermostateOn=false; //thermostateStatus => Thermostate is active (or relay active), thermostateOn => Burning gas due to either warming hot or heater
bool  sentHDAiscovery1=false,sentHDAiscovery2=false,sentHDAiscovery3=false,sentHDAiscovery4=false,sentHDAiscovery5=false,sentHDAiscovery6=false,sentHDAiscovery7=false,sentHDAiscovery8=false;
boolean startTimeConfigure,wifiResuming;
uint8_t ntpServerIndex,configVariables,auxLoopCounter=0,auxLoopCounter2=0,auxCounter=0,fileUpdateError=0,errorOnActiveCookie=0,errorOnWrongCookie=0;
uint16_t rebounds=0,voltage=0,power=0,powerMeasureId=0,sysLogServerUDPPort=0;
uint32_t heapSize=0,minHeapSeen=0,heapBlockSize=0,flashSize=ESP.getFlashChipSize(),programSize=ESP.getSketchSize(),fileSystemSize=0,fileSystemUsed=0;
uint64_t whileLoopTimeLeft=NTP_CHECK_TIMEOUT;
int updateCommand;
float gasSample=0,gasVoltCalibrated=0,RS_airCalibrated=0,RS_CurrentCalibrated=0,gasRatioSample=0,current=0,energyToday=0,energyYesterday=0,energyTotal=0;
size_t fileUpdateSize=0,OTAAvailableSize=0,SPIFFSAvailableSize=0;
struct tm nowTimeInfo;
String TZEnvVariable(NTP_TZ_ENV_VARIABLE),TZName(NTP_TZ_NAME),device(DEVICE_NAME_PREFIX),serverToUploadSamplesString(SERVER_UPLOAD_SAMPLES),
        mqttUserName,mqttUserPssw,mqttTopicPrefix,mqttTopicName,mqttServer,sysLogServer,userName,userPssw,
        iconWifi,iconGasInterrupt,iconThermInterrupt,iconThermStatus,
        ntpServers[4],lastURI,fileUpdateName,powerMqttTopic;
wifiNetworkInfo wifiNet;
wifiCredentials wifiCred;
enum CloudClockStatus previousCloudClockCurrentStatus;
SHT2x tempHumSensor; //Temp and Hum sensor
JSONVar samples;
IPAddress serverToUploadSamplesIPAddress; //8*2=16
char activeCookie[COOKIE_SIZE],currentSetCookie[COOKIE_SIZE],firmwareVersion[VERSION_CHAR_LENGTH+1];
//Global MQTT buffers
char bufferTopicHAName[100],bufferPayload[BUFFER_PAYLOAD_SIZE],bufferMqttTopicName[100],bufferDeviceSufix[10],
     bufferIpAddress[20],bufferDevice[50],bufferMqttSensorTopicHAPrefixName[100],bufferMqttBinarySensorTopicHAPrefixName[100],
     bufferMqttButtonTopicHAPrefixName[100],bufferMqttSwitchTopicHAPrefixName[100];
#ifdef SYSLOG_SERVER
  PicoSyslog::Logger syslog;
#endif

void setup() {
  pinMode(BOARD_RX,INPUT);  //Pin definition for serial RX
  pinMode(BOARD_TX,OUTPUT); //Pin definition for serial TX
  boardSerialPort.begin(SERIAL_PORT_SPEED);
  randomSeed(analogRead(GPIO_NUM_32));

  printLogln("\n============================[  BOOT  ]================================");
  printLog("[setup] - Doing regular bootup v");printLog(VERSION);printLogln(", debugModeOn="+String(debugModeOn)+" ..........");
  printLogln("[setup] - Serial: OK");

  //GPIO Mode definitons for both the LED and the Relays control pins
  pinMode(PIN_RL1,OUTPUT);digitalWrite(PIN_RL1,LOW); //To force the Relay1 OFF
  pinMode(PIN_RL2,OUTPUT);digitalWrite(PIN_RL2,LOW); //To force the Relay2 OFF
  pinMode(PIN_LED,OUTPUT);digitalWrite(PIN_LED,LOW); //To force the LED OFF
  printLogln("[setup] - LED and Relays set OFF");

  analogReadResolution(12); //12 bits resolution (supposed to be default) 2^12=4096 values
  pinMode(PIN_GAS_SENSOR_A0,ANALOG);
  pinMode(PIN_GAS_SENSOR_D0,INPUT);
  printLogln("[setup] - GAS inputs set. Analog GPIO "+String(PIN_GAS_SENSOR_A0)+", ADC resolution 12 bits. Digital GPIO "+String(PIN_GAS_SENSOR_D0));

  //Init variables
  variablesInit();
  printLogln("[setup] - Device: "+device+", boots since last update="+String(bootCount)+", uncontrolled resets="+String(resetCount)+", task watchdog timeout="+String(ESP_TASK_WDT_TIMEOUT)+" seconds");

  //Hum and Temp sensor init
  printLog("[setup] - Temp & Hum Sensor initialization");
  error_setup|=tempSensorInit(debugModeOn);
  printLogln("[setup] - error_setup="+String(error_setup));

  //WiFi init
  wifiCurrentStatus=wifiOffStatus; //It's updated in wifiInit
  printLog("[setup] - WiFi initialization");
  error_setup=wifiInit(wifiEnabled,debugModeOn);
  printLogln("[setup] - error_setup="+String(error_setup));
  
  //HTTP - Cloud server init
  CloudSyncCurrentStatus=CloudSyncOffStatus; //It's updated in httpCloudInit
  printLog("[setup] - HTTP Cloud initialization");
  error_setup|=httpCloudInit(wifiEnabled,httpCloudEnabled,wifiCurrentStatus,debugModeOn,true);
  printLogln("[setup] - error_setup="+String(error_setup));
  
  //NTP init
  printLog("[setup] - NTP initialization");
  error_setup|=ntpInit(wifiEnabled,ntpEnabled,wifiCurrentStatus,debugModeOn,true);
  printLogln("[setup] - error_setup="+String(error_setup));

  //SPIFFS init
  printLog("[setup] - SPIFFS initialization");
  error_setup|=spiffsInit(debugModeOn,true);
  printLogln("[setup] - error_setup="+String(error_setup));
  
  //WebServer init
  printLog("[setup] - Web Server initialization");
  error_setup|=webServerInit(error_setup,wifiEnabled,webServerEnabled,wifiCurrentStatus,debugModeOn,true);
  if (!(error_setup & ERROR_WEB_SERVER)) webLogsOn=debugModeOn; //Enable web logs only if webServer and webSocket is not error
  printLogln("[setup] - error_setup="+String(error_setup));

  //Time on counters init
  printLog("[setup] - Time on counters initialization");
  error_setup|=timeOnCountersInit(error_setup,debugModeOn,true,ntpSynced);
  printLogln("[setup] - error_setup="+String(error_setup));
  
  //MQTT init
  printLog("[setup] - MQTT initialization");
  error_setup|=mqttClientInit(wifiEnabled,mqttServerEnabled,secureMqttEnabled,error_setup,debugModeOn,true,mqttTopicName,device);
  printLogln("[setup] - error_setup="+String(error_setup));

  printLogln("[setup] - ---------oooooOOOOOoooo-----------");
  if (error_setup==0x00) printLogln("[setup] - Setup ends with no errors. Running loop() now");
  else printLogln("[setup] - Setup ends with errors (0x"+String(error_setup,HEX)+"). Running loop() now");
  printLogln("[setup] - ---------oooooOOOOOoooo-----------");
}

void loop() {
  // Loop code
  if (firstLoopTime==0) { //To be ran the very first loop
    firstLoopTime=millis(); //Get the board initialization time
    firstBoot=false;

    //Set the interrupt pin
    attachInterrupt(digitalPinToInterrupt(PIN_GAS_SENSOR_D0), gas_probe_triggered, FALLING);
    if (digitalRead(PIN_GAS_SENSOR_D0)==1) {gasClear=true;};
    if (debugModeOn) printLogln(String(millis())+" - [loop] - Interrupt set for GPIO pin "+String(PIN_GAS_SENSOR_D0)+", gasClear="+String(gasClear));

    attachInterrupt(digitalPinToInterrupt(PIN_THERMOSTATE), thermostate_change, CHANGE);
    if (digitalRead(PIN_THERMOSTATE)==1) {thermostateStatus=true;} else {thermostateStatus=false;};
    if (debugModeOn) printLogln(String(millis())+" - [loop] - Interrupt set for GPIO pin "+String(PIN_THERMOSTATE)+", thermostateStatus="+String(thermostateStatus));

    //Update JSON samples variable the very first time
    samples["debugModeOn"]=debugModeOn?"DEBUG_ON":"DEBUG_OFF"; //debugModeOn must be update out of gas_sample, as it is function parameter
    gas_sample(false,1);   //Get GAS samples
    if (tempHumSensor.isConnected()) temperature_sample(false);  //Get Temp & Hum samples
  }

  //Rest of loop interactions

  //Check and update the heap variables
  heapSize=esp_get_free_heap_size();
  minHeapSeen=esp_get_minimum_free_heap_size();
  heapBlockSize=heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
  if(heapBlockSize<minMaxHeapBlockSizeSinceBoot) minMaxHeapBlockSizeSinceBoot=heapBlockSize;
  if(minHeapSeen<minHeapSinceBoot) minHeapSinceBoot=minHeapSeen;
  if(minHeapSeen<minHeapSinceUpgrade) {
    blockWebServer=true; //Avoid serving web pages till this task if finish to avoid heap leaks
    minHeapSinceUpgrade=minHeapSeen; //Track the minimun heap size (bytes)
    EEPROM.writeInt(0x41D,minHeapSinceUpgrade); //Write in EEPROM
    eepromUpdate=true; //EEPROM to be updated at the end of the cycle.
    blockWebServer=false;
  }
  if(heapBlockSize<minMaxHeapBlockSizeSinceUpgrade) {
    blockWebServer=true; //Avoid serving web pages till this task if finish to avoid heap leaksminMaxHeapBlockSizeSinceUpgrade=heapBlockSize;
    EEPROM.writeInt(0x609,minMaxHeapBlockSizeSinceUpgrade); //Write in EEPROM
    blockWebServer=false;eepromUpdate=true; //EEPROM to be updated at the end of the cycle.
  }
  if (heapSize<ABSULUTE_MIN_HEAP_THRESHOLD || minMaxHeapBlockSizeSinceBoot<ABSULUTE_MIN_MAX_HEAP_BLOCK_THRESHOLD) { //Preventive reset to avoid crash
    blockWebServer=true; //Avoid serving web pages till this task if finish to avoid heap leaks
    printLogln(String(millis())+" - [loop - heapCheck] - HeapSize ("+String(heapSize)+")<ABSULUTE_MIN_HEAP_THRESHOLD ("+String(ABSULUTE_MIN_HEAP_THRESHOLD)+"). Restart needed");
    resetPreventiveCount++; //preventive resets (mainly becuase low heap situation)
    EEPROM.write(0x41B,resetPreventiveCount);
    EEPROM.commit();
    ESP.restart(); //Rebooting
  }
  /*else if(heapSize<WEBSERVER_MIN_HEAP_SIZE) {
    blockWebServer=true; //Avoid serving web pages till this task if finish to avoid heap leaks
    printLogln(String(millis())+" - [loop - heapCheck] - HeapSize ("+String(heapSize)+")<WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Restart needed");
    resetPreventiveWebServerCount++; //preventive web reset resets (mainly becuase low heap situation)
    EEPROM.write(0x531,resetPreventiveWebServerCount);
    EEPROM.commit();
    ESP.restart(); //Rebooting
  }*/ //Check don in webServer.cpp

  //Pubish the first MQTT message and HA Discovery. Wait HA_ADVST_WINDOW seconds to avoid heap leaking
  nowTimeGlobal=millis();
  if ((nowTimeGlobal > firstLoopTime+HA_ADVST_WINDOW) && !firstHASent && !webServerResponding) {
    //HA Discovery is sent again just right after booting up cause sometimes, some objects are missed.
    blockWebServer=true; //Avoid serving web pages till this task if finish to avoid heap leaks
    
    if (WiFi.status()==WL_CONNECTED && mqttClient.connected()) { //IS0016 - v0.9.B
      //JSON object is updated just right after taking the samples.
      //Get readings for the index.html page to be up-to-date
      gas_sample(false,2);   //Get GAS samples
      if (tempHumSensor.isConnected()) temperature_sample(false);  //Get Temp & Hum samples

      //Force sending HA Discovery to make sure it's processed by HA. 
      updateHADiscovery=true;
      mqtt_publish_samples(wifiEnabled,mqttServerEnabled,secureMqttEnabled,false,19);
      firstHASent=true;
    }
    blockWebServer=false;
  }

  //Regular actions every WIFI_RECONNECT_PERIOD seconds to recover WiFi connection
  //forceWifiReconnect==true if:
  // 1) after heap size was below ABSULUTE_MIN_HEAP_THRESHOLD
  // 2) after detection the webServer is down
  nowTimeGlobal=millis();
  if ((((nowTimeGlobal-lastTimeWifiReconnectionCheck) >= wifiReconnectPeriod) || forceWifiReconnect ) && 
      wifiEnabled && !firstBoot && (wifiCurrentStatus==wifiOffStatus || WiFi.status()!=WL_CONNECTED)) {
     
    if (!forceWifiReconnect && wifiEnabled && !firstBoot && WiFi.status()!=WL_CONNECTED) {
      errorsWiFiCnt++;EEPROM.write(0x535,errorsWiFiCnt);eepromUpdate=true;
      printLogln(String(millis())+" - [loop - WIFI_RECONNECT_PERIOD] - WiFi not connected. Wifi restart required. error_setup=0x"+String(error_setup,HEX)+"), errorsWiFiCnt="+String(errorsWiFiCnt)+", heapSize="+String(heapSize));
    }
    else {
      printLogln(String(millis())+" - [loop - WIFI_RECONNECT_PERIOD] - WiFi restart required to either free heap memory up or restart web server. error_setup=0x"+String(error_setup,HEX)+"), heapSize="+String(heapSize));
    }
    bool auxWebLogsOn=webLogsOn,auxSysLogsOn=sysLogsOn;
    blockWebServer=true; //Avoid serving web pages till this task if finish to avoid heap leaks
    webLogsOn=false;
    sysLogsOn=false;
    detachNetwork(); //detach the network and network services before restarting them again
    forceWifiReconnect=false;
    wifi_reconnect_period(debugModeOn); //restart network
    
    if (WiFi.status()==WL_CONNECTED) {  //restart network services
      error_setup&=!ERROR_WIFI_SETUP;
      error_setup&=!ERROR_CLOUD_SERVER; error_setup|=httpCloudInit(wifiEnabled,httpCloudEnabled,wifiCurrentStatus,false,false);
      error_setup&=!ERROR_NTP_SERVER; error_setup|=ntpInit(wifiEnabled,ntpEnabled,wifiCurrentStatus,false,false);
      error_setup&=!ERROR_SPIFFS_SETUP; error_setup|=spiffsInit(false,false);
      //error_setup&=!ERROR_WEB_SERVER; error_setup&=!ERROR_WEB_SOCKET;error_setup|=webServerInit(error_setup,wifiEnabled,webServerEnabled,wifiCurrentStatus,debugModeOn,false);
      if ((error_setup & ERROR_EEPROM_VARIABLES_INIT)!=0) {error_setup&=!ERROR_EEPROM_VARIABLES_INIT; error_setup|=timeOnCountersInit(error_setup,false,false,ntpSynced);} //This is in case during first boot was not wifi
      error_setup&=!ERROR_MQTT_SERVER; error_setup|=mqttClientInit(wifiEnabled,mqttServerEnabled,secureMqttEnabled,error_setup,false,false,mqttTopicName,device);
      webLogsOn=auxWebLogsOn;sysLogsOn=auxSysLogsOn;
      if (error_setup==0) printLogln(String(millis())+" - [loop - WIFI_RECONNECT_PERIOD] - WiFi and network services successfully restarted . error_setup=0x"+String(error_setup,HEX)+"), heap="+String(ESP.getFreeHeap()));
      else printLogln(String(millis())+" - [loop - WIFI_RECONNECT_PERIOD] - WiFi successfully restarted but there are network services errors: error_setup=0x"+String(error_setup,HEX)+"), heap="+String(ESP.getFreeHeap()));
    }
    else {
      error_setup|=ERROR_WIFI_SETUP;
      if (error_setup==0) printLogln(String(millis())+" - [loop - WIFI_RECONNECT_PERIOD] - WiFi and network services unsuccessfully restarted . error_setup=0x"+String(error_setup,HEX)+"), heap="+String(ESP.getFreeHeap()));
    }
    blockWebServer=false;
  }
  heapBlockSize=heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);if(heapBlockSize<minMaxHeapBlockSizeSinceBoot) minMaxHeapBlockSizeSinceBoot=heapBlockSize;

  //Regular actions every NTP_KO_CHECK_PERIOD seconds. Cheking if NTP is off or should be checked
  //forceNTPCheck is true if:
  // 1) After NTP server config in firstSetup()
  nowTimeGlobal=millis();
  if (((nowTimeGlobal-lastTimeNTPCheck) >= NTP_KO_CHECK_PERIOD || forceNTPCheck) && !webServerResponding) {
    
    blockWebServer=true; //Avoid serving web pages till this task if finish to avoid heap leaks
    ntp_ko_check_period(false);
    if ((error_setup & ERROR_EEPROM_VARIABLES_INIT)!=0) {error_setup&=!ERROR_EEPROM_VARIABLES_INIT; error_setup|=timeOnCountersInit(error_setup,false,false,ntpSynced);}
    blockWebServer=false;
  }
  heapBlockSize=heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);if(heapBlockSize<minMaxHeapBlockSizeSinceBoot) minMaxHeapBlockSizeSinceBoot=heapBlockSize;
  
  //Regular actions every ONE_SECOND_PERIOD seconds
  // 1) Check if time on counters need to be updated
  nowTimeGlobal=millis();
  if (((nowTimeGlobal-lastTimeSecondCheck) >= ONE_SECOND_PERIOD) && !webServerResponding) {
    
    one_second_check_period(false,nowTimeGlobal,ntpSynced);
  }
  heapBlockSize=heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);if(heapBlockSize<minMaxHeapBlockSizeSinceBoot) minMaxHeapBlockSizeSinceBoot=heapBlockSize;

  //Regular actions every TIME_COUNTERS_EEPROM_UPDATE_PERIOD seconds
  nowTimeGlobal=millis();
  if (((nowTimeGlobal-lastTimeTimerEepromUpdateCheck) >= TIME_COUNTERS_EEPROM_UPDATE_PERIOD) && !webServerResponding) {
    
    time_counters_eeprom_update_check_period(true,nowTimeGlobal);
  }
  heapBlockSize=heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);if(heapBlockSize<minMaxHeapBlockSizeSinceBoot) minMaxHeapBlockSizeSinceBoot=heapBlockSize;

  //Regular actions every MQTT_CHECK_PERIOD seconds to check on MQTT connection
  // or due to forceMQTTConnect (from webServer (/cloud form))
  nowTimeGlobal=millis();
  if (((nowTimeGlobal-lastTimeMQTTCheck >= MQTT_CHECK_PERIOD) || forceMQTTConnect) && !webServerResponding) {

    blockWebServer=true; //Avoid serving web pages till this task if finish to avoid heap leaks
    //Connect to MQTT broker as it was enabled from the webserver
    //forceMQTTConnect is set from webServer (/cloud form)
    if (WiFi.status()==WL_CONNECTED && !mqttClient.connected() && mqttServerEnabled && forceMQTTConnect) {
      if (debugModeOn) printLogln(String(millis())+" - [loop - forceMQTTConnect] - Connecting to the MQTT broker as it was set in the web form.");
      error_setup&=!ERROR_MQTT_SERVER;
      error_setup|=mqttClientInit(wifiEnabled,mqttServerEnabled,secureMqttEnabled,error_setup,debugModeOn,false,mqttTopicName,device);
      forceMQTTConnect=false;
    }
    else if (wifiEnabled && WiFi.status()==WL_CONNECTED && mqttServerEnabled && MqttSyncCurrentStatus==MqttSyncOffStatus) {
      //Regular action every MQTT_CHECK_PERIOD seconds
      error_setup&=!ERROR_MQTT_SERVER;
      error_setup|=mqttClientInit(wifiEnabled,mqttServerEnabled,secureMqttEnabled,error_setup,debugModeOn,false,mqttTopicName,device);
    }
    blockWebServer=false;
    lastTimeMQTTCheck=nowTimeGlobal;
  }
  heapBlockSize=heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);if(heapBlockSize<minMaxHeapBlockSizeSinceBoot) minMaxHeapBlockSizeSinceBoot=heapBlockSize;
  
  //Regular actions every SAMPLE_PERIOD if gas burning or SAMPLE_LONG_PERIOD seconds to take gas samples
  nowTimeGlobal=millis();
  if ((((boilerOn || thermostateOn || !gasClear) && (nowTimeGlobal-lastGasSample >= SAMPLE_PERIOD)) || 
      ((!boilerOn && !thermostateOn && gasClear) && (nowTimeGlobal-lastGasSample >= SAMPLE_LONG_PERIOD))) 
      && !webServerResponding) { //
    blockWebServer=true; //Avoid serving web pages till this task if finish to avoid heap leaks
    if (nowTimeGlobal-lastGasSample >= SAMPLE_LONG_PERIOD) gas_sample(false,4);   //Get GAS samples
    else gas_sample(false,3);   //Get GAS samples
    if (tempHumSensor.isConnected()) temperature_sample(false);  //Get Temp & Hum samples

    //JSON object is updated just right after taking the samples.
    //Publish MQTT message and webEvent with the new samples
    forceMQTTpublish=2;
    forceWebEvent=true;
    blockWebServer=false;
    lastGasSample=nowTimeGlobal;
  }
  heapBlockSize=heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);if(heapBlockSize<minMaxHeapBlockSizeSinceBoot) minMaxHeapBlockSizeSinceBoot=heapBlockSize;

  //Regular actions every CONNECTIVITY_CHECK_PERIOD seconds
  nowTimeGlobal=millis();
  if (((nowTimeGlobal-lastTimeConnectiviyCheck >= CONNECTIVITY_CHECK_PERIOD) && WiFi.status()==WL_CONNECTED && 
          wifiEnabled && !firstBoot ) && !webServerResponding && Update.progress()==0) {
    //Check connectivity and reset it if needed
    // WiFi.status()==WL_CONNECTED && no FQDN resolved and error in sample_upload && mqttClient not connected and error in checkURL and no GW ping
    // NTP status is not check as NTP checks take time (random period) and forcing NTP in here is avoided (as it may take several loop cycles = complexity)
    blockWebServer=true; //Avoid serving web pages till this task if finish to avoid heap leaks
    switch (connectiviy_check_period(debugModeOn,nowTimeGlobal)) {
      case ERROR_WEB_SERVER:
        printLogln(String(millis())+" - [loop - CONNECTIVITY_CHECK_PERIOD] - ERROR_WEB_SERVER after connectiviy_check_period(). HeapSize ("+String(esp_get_free_heap_size())+"). Restart needed");
        resetWebServerCnt++; //Stats
        EEPROM.write(0x53B,errorsWebServerCnt);
        EEPROM.write(0x53C,resetWebServerCnt);
        EEPROM.commit();
        ESP.restart(); //Rebooting
      break;
      case ERROR_NO_CONNECTIVITY:
        forceWifiReconnect=true; //Next loop cycle restart wifi}
      break;
      case NO_ERROR:
      default:
        //Do nothing
      break;
    }
    blockWebServer=false;
  }
  heapBlockSize=heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);if(heapBlockSize<minMaxHeapBlockSizeSinceBoot) minMaxHeapBlockSizeSinceBoot=heapBlockSize;
  
  //Check interrupt flags
  if (gasInterrupt && !webServerResponding) {
    if (debugModeOn) {printLogln(String(millis())+"  - [loop] - GAS interrupt detected and gas_probe_triggered routine triggered: gasClear flag was '"+String(gasClear)+"' (1=NO GAS, 0=GAS)");}
    blockWebServer=true; //Avoid serving web pages till this task if finish to avoid heap leaks
    gas_sample(debugModeOn,5); //gasClear flag is updated
    
    //JSON object is updated just right after taking the samples.
    //Publish MQTT message with the new samples
    if (!gasClear) {
      if (debugModeOn) {printLogln(String(millis())+"  - [loop] - GAS detected after getting readings. MQTT message to be sent");}
      forceMQTTpublish=3;
      forceWebEvent=true;
    }
    else {
      if (debugModeOn) {printLogln(String(millis())+"  - [loop] - GAS clear after getting readings, so GAS interrupt was wrong, probably cause thermostast interrupt. Don't send MQTT message.");}
    }
    gasInterrupt=false;
    blockWebServer=false;
  }
  heapBlockSize=heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);if(heapBlockSize<minMaxHeapBlockSizeSinceBoot) minMaxHeapBlockSizeSinceBoot=heapBlockSize;

  //Regular actions after THERMOSTATE_INTERRUPT_DELAY milliseconds the thermostate interrupt was triggered to check the value of Thermostate pin
  //Delay of THERMOSTATE_INTERRUPT_DELAY milliseconds is needed to avoid bouncing
  nowTimeGlobal=millis();
  if ((thermostateInterrupt && (nowTimeGlobal-lastInterruptTime >= THERMOSTATE_INTERRUPT_DELAY)) && !webServerResponding) {
    blockWebServer=true; //Avoid serving web pages till this task if finish to avoid heap leaks
    thermostate_interrupt_triggered(debugModeOn); //Sending HTTP Cloud, mqtt and web client updates is done from that function
    blockWebServer=false;
  }
  heapBlockSize=heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);if(heapBlockSize<minMaxHeapBlockSizeSinceBoot) minMaxHeapBlockSizeSinceBoot=heapBlockSize;

  //Check if mqtt samples must be published
  nowTimeGlobal=millis();
  if ((forceMQTTpublish>0) && !webServerResponding) {
    /* Situations for forceMQTTpublish != 0
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
  */
    blockWebServer=true; //Avoid serving web pages till this task if finish to avoid heap leaks
    mqtt_publish_samples(wifiEnabled,mqttServerEnabled,secureMqttEnabled,false,forceMQTTpublish);
    forceMQTTpublish=0;
    blockWebServer=false;
  }
  heapBlockSize=heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);if(heapBlockSize<minMaxHeapBlockSizeSinceBoot) minMaxHeapBlockSizeSinceBoot=heapBlockSize;

  if ((forceWebEvent && wifiEnabled && webServerEnabled && WiFi.status()==WL_CONNECTED && webEvents.count()>0) && !webServerResponding) {
    //webEvents.count()>0 it means clients connected
    webEvents.send("ping",NULL,nowTimeGlobal);
    webEvents.send(JSON.stringify(samples).c_str(),"new_samples",nowTimeGlobal);
    if (debugModeOn) {printLogln(String(millis())+" - [loop] - new_samples event sent to web clients. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    forceWebEvent=false;
  }
  heapBlockSize=heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);if(heapBlockSize<minMaxHeapBlockSizeSinceBoot) minMaxHeapBlockSizeSinceBoot=heapBlockSize;

  if (eepromUpdate) {EEPROM.commit(); eepromUpdate=false;}
}