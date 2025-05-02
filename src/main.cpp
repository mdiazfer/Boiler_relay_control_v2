#include <Arduino.h>
#include <SoftwareSerial.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <Arduino_JSON.h>
#include <AsyncMqttClient.h>
#include "SHT2x.h"
#include "global_setup.h"
//#include "wifiConnection.h" //included in loop.h
//#include "mq5_sensor.h"     //included in loop.h
#include "setup.h"
#include "interrupt.h"
#include "loop.h"
#include "eeprom_utils.h"

//Global variable definitions stored in RTC memory to be persistent during Sleep periods. 8 KB Max
RTC_DATA_ATTR boolean firstBoot=true;  //1B - First boot flag.
RTC_DATA_ATTR int errorsWiFiCnt=0,errorsNTPCnt=0,errorsHTTPUptsCnt=0,errorsMQTTCnt=0;
RTC_DATA_ATTR uint16_t year=0,previousYear=0;
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
RTC_DATA_ATTR uint32_t error_setup=NO_ERROR,minHeapSeen=0xFFFFFFFF; //1*4=4B
RTC_DATA_ATTR uint8_t bootCount=255,resetCount=0,resetPreventiveCount=0,resetSWCount=0;
RTC_DATA_ATTR boolean wifiEnabled=true,forceWifiReconnect=false,forceWEBTestCheck=false,forceWebServerInit=false,forceMQTTpublish=false,
                      ntpEnabled=true,httpCloudEnabled=true,forceNTPCheck=false,ntpSynced=false,
                      mqttServerEnabled=true,forceMQTTConnect=false,secureMqttEnabled=false,bluetoothEnabled=false,webServerEnabled=false,timersEepromUpdate=false,
                      updateHADiscovery=false;
RTC_DATA_ATTR byte mac[6];
RTC_DATA_ATTR uint64_t nowTimeGlobal=0,firstLoopTime=0,lastCheckTime=0,lastTimeWifiReconnectionCheck=0,lastTimeHTTPClouCheck=0,lastTimeNTPCheck=0,
                      lastCloudClockChangeCheck=0,lastInterruptTime=0,lastGasSample=0,lastTimeMQTTCheck=0,
                      lastMQTTChangeCheck=0,lastTimeSecondCheck=0,lastThermostatOnTime=0,lastBoilerOnTime=0,lastTimeTimerEepromUpdateCheck=0;
RTC_DATA_ATTR ulong wifiReconnectPeriod=WIFI_RECONNECT_PERIOD;
RTC_DATA_ATTR String tempHumSensorType=String(TEMP_HUM_SENSOR_TYPE); //16 B
RTC_DATA_ATTR float valueHum=0,tempSensor=0,valueT=0;
RTC_DATA_ATTR AsyncMqttClient mqttClient;
RTC_DATA_ATTR struct timeOnCounters heaterTimeOnYear,heaterTimeOnPreviousYear,boilerTimeOnYear,boilerTimeOnPreviousYear; //61 B each varialbe (heaterTimeOn or boilerTimeOn) - Time in seconds that the heater signal has been on (thermostat or Relay2)

//EEPROM MAP

//Global variable definitions stored in regular RAM. 520 KB Max
bool debugModeOn=DEBUG_MODE_ON,logMessageTOFF=false,logMessageTRL1_ON=false,logMessageTRL2_ON=false,logMessageGAP_OFF=false,
      thermostateStatus=false,thermostateInterrupt=false,gasClear=false,gasInterrupt=false,isBeaconAdvertising;
//bool debugModeOn=false;
boolean NTPResuming,startTimeConfigure,wifiResuming;
uint8_t ntpServerIndex,configVariables,auxLoopCounter=0,auxLoopCounter2=0,auxCounter=0;
uint16_t rebounds=0;
uint32_t lastHeap=0;
uint64_t whileLoopTimeLeft=NTP_CHECK_TIMEOUT;
float gasSample=0,gasVoltCalibrated=0,RS_airCalibrated=0,RS_CurrentCalibrated=0,gasRatioSample=0;
struct tm nowTimeInfo;
String TZEnvVariable(NTP_TZ_ENV_VARIABLE),TZName(NTP_TZ_NAME),device(DEVICE_NAME_PREFIX),serverToUploadSamplesString(SERVER_UPLOAD_SAMPLES),
        mqttUserName,mqttUserPssw,mqttTopicPrefix,mqttTopicName,mqttServer,userName,userPssw,
        iconWifi,iconGasInterrupt,iconThermInterrupt,iconThermStatus,
        ntpServers[4];
wifiNetworkInfo wifiNet;
wifiCredentials wifiCred;
enum CloudClockStatus previousCloudClockCurrentStatus;
HardwareSerial boardSerialPort(0); // Serial port is using UART0
SHT2x tempHumSensor; //Temp and Hum sensor
JSONVar samples;
IPAddress serverToUploadSamplesIPAddress; //8*2=16
char activeCookie[COOKIE_SIZE],currentSetCookie[COOKIE_SIZE],firmwareVersion[VERSION_CHAR_LENGTH+1];

void setup() {
  pinMode(BOARD_RX,INPUT);  //Pin definition for serial RX
  pinMode(BOARD_TX,OUTPUT); //Pin definition for serial TX
  boardSerialPort.begin(SERIAL_PORT_SPEED);
  randomSeed(analogRead(GPIO_NUM_32));

  boardSerialPort.print("\n[setup] - Doing regular bootup v");boardSerialPort.print(VERSION);boardSerialPort.println(", debugModeOn="+String(debugModeOn)+" ..........");
  boardSerialPort.println("[setup] - Serial: OK");

  //GPIO Mode definitons for both the LED and the Relays control pins
  pinMode(PIN_RL1,OUTPUT);digitalWrite(PIN_RL1,LOW); //To force the Relay1 OFF
  pinMode(PIN_RL2,OUTPUT);digitalWrite(PIN_RL2,LOW); //To force the Relay2 OFF
  pinMode(PIN_LED,OUTPUT);digitalWrite(PIN_LED,LOW); //To force the LED OFF
  boardSerialPort.println("[setup] - LED and Relays set OFF");

  analogReadResolution(12); //12 bits resolution (supposed to be default) 2^12=4096 values
  pinMode(PIN_GAS_SENSOR_A0,ANALOG);
  pinMode(PIN_GAS_SENSOR_D0,INPUT);
  boardSerialPort.println("[setup] - GAS inputs set. Analog GPIO "+String(PIN_GAS_SENSOR_A0)+", ADC resolution 12 bits. Digital GPIO "+String(PIN_GAS_SENSOR_D0));

  //Init variables
  variablesInit();
  boardSerialPort.println("[setup] - Device: "+device+", boots since last update="+String(bootCount)+", uncontrolled resets="+String(resetCount));

  //Hum and Temp sensor init
  boardSerialPort.print("[setup] - Temp & Hum Sensor initialization");
  error_setup|=tempSensorInit(debugModeOn);
  boardSerialPort.println("[setup] - error_setup="+String(error_setup));

  //WiFi init
  wifiCurrentStatus=wifiOffStatus; //It's updated in wifiInit
  boardSerialPort.print("[setup] - WiFi initialization");
  error_setup=wifiInit(wifiEnabled,debugModeOn);
  boardSerialPort.println("[setup] - error_setup="+String(error_setup));

  //HTTP - Cloud server init
  CloudSyncCurrentStatus=CloudSyncOffStatus; //It's updated in httpCloudInit
  boardSerialPort.print("[setup] - HTTP Cloud initialization");
  error_setup|=httpCloudInit(wifiEnabled,httpCloudEnabled,wifiCurrentStatus,debugModeOn,true);
  boardSerialPort.println("[setup] - error_setup="+String(error_setup));
  
  //NTP init
  boardSerialPort.print("[setup] - NTP initialization");
  error_setup|=ntpInit(wifiEnabled,ntpEnabled,wifiCurrentStatus,debugModeOn,true);
  boardSerialPort.println("[setup] - error_setup="+String(error_setup));

  //Time on counters init
  boardSerialPort.print("[setup] - Time on counters initialization");
  error_setup|=timeOnCountersInit(error_setup,debugModeOn,true,ntpSynced);
  boardSerialPort.println("[setup] - error_setup="+String(error_setup));
  
  //MQTT init
  boardSerialPort.print("[setup] - MQTT initialization");
  error_setup|=mqttClientInit(wifiEnabled,mqttServerEnabled,secureMqttEnabled,error_setup,debugModeOn,true,mqttTopicName,device);
  boardSerialPort.println("[setup] - error_setup="+String(error_setup));

  boardSerialPort.println("[setup] - ---------oooooOOOOOoooo-----------");
  if (error_setup==0x00) boardSerialPort.println("[setup] - Setup ends with no errors. Running loop() now");
  else boardSerialPort.println("[setup] - Setup ends with errors (0x"+String(error_setup,HEX)+"). Running loop() now");
  boardSerialPort.println("[setup] - ---------oooooOOOOOoooo-----------");
}

void loop() {
  // Loop code
  if (firstLoopTime==0) { //To be ran the very first loop
    firstLoopTime=millis(); //Get the boarad initialization time
    firstBoot=false;

    //Set the interrupt pin
    attachInterrupt(digitalPinToInterrupt(PIN_GAS_SENSOR_D0), gas_probe_triggered, FALLING);
    if (digitalRead(PIN_GAS_SENSOR_D0)==1) {gasClear=true;};
    if (debugModeOn) boardSerialPort.println(String(millis())+" - [loop] - Interrupt set for GPIO pin "+String(PIN_GAS_SENSOR_D0)+", gasClear="+String(gasClear));

    attachInterrupt(digitalPinToInterrupt(PIN_THERMOSTATE), thermostate_change, CHANGE);
    if (digitalRead(PIN_THERMOSTATE)==1) {thermostateStatus=true;} else {thermostateStatus=false;};
    if (debugModeOn) boardSerialPort.println(String(millis())+" - [loop] - Interrupt set for GPIO pin "+String(PIN_THERMOSTATE)+", thermostateStatus="+String(thermostateStatus));
  }

  //Rest of loop interactions

  //Check the heap
  lastHeap=esp_get_minimum_free_heap_size();
  if(lastHeap<minHeapSeen) {
    minHeapSeen=lastHeap; //Track the minimun heap size (bytes)
    EEPROM.writeInt(0x41D,minHeapSeen); //Write in EEPROM
    EEPROM.commit();
  }
  if(lastHeap<ABSULUTE_MIN_HEAP_THRESHOLD) { //Preventive reset to avoid crash
    resetPreventiveCount++; //preventive resets (mainly becuase low heap situation)
    EEPROM.write(0x41B,resetPreventiveCount);
    EEPROM.commit();
    ESP.restart(); //Rebooting
  }

  //Regular actions every ONE_SECOND_PERIOD seconds
  // 1) Check if time on counters need to be updated
  nowTimeGlobal=millis();
  if ((nowTimeGlobal-lastTimeSecondCheck) >= ONE_SECOND_PERIOD) {
    
    one_second_check_period(false,nowTimeGlobal,ntpSynced);
  }

  //Regular actions every TIME_COUNTERS_EEPROM_UPDATE_PERIOD seconds to recover WiFi connection
  nowTimeGlobal=millis();
  if ((nowTimeGlobal-lastTimeTimerEepromUpdateCheck) >= TIME_COUNTERS_EEPROM_UPDATE_PERIOD) {
    
    time_counters_eeprom_update_check_period(true,nowTimeGlobal);
  }

  //Regular actions every WIFI_RECONNECT_PERIOD seconds to recover WiFi connection
  //forceWifiReconnect==true if:
  // 1) after ICON_STATUS_REFRESH_PERIOD
  // 2) after configuring WiFi = ON in the Config Menu
  // 3) or wake up from sleep (only by pressing buttons, no by timer)
  // 4) or previous WiFi re-connection try was ABORTED (button pressed) or BREAK (need to refresh display)
  // 5) after heap size was below ABSULUTE_MIN_HEAP_THRESHOLD
  // 6) after detection the webServer is down
  nowTimeGlobal=millis();
  if ((((nowTimeGlobal-lastTimeWifiReconnectionCheck) >= wifiReconnectPeriod) || forceWifiReconnect ) && 
      wifiEnabled && !firstBoot && (wifiCurrentStatus==wifiOffStatus || WiFi.status()!=WL_CONNECTED) ) {
     
    wifi_reconnect_period(debugModeOn);
  }

  //Regular actions every NTP_KO_CHECK_PERIOD seconds. Cheking if NTP is off or should be checked
  //forceNTPCheck is true if:
  // 1) After NTP server config in firstSetup()
  // 2) If the previous NTP check was aborted due either Button action or Display Refresh
  // 2) WiFi has been setup ON in config menu
  nowTimeGlobal=millis();
  if ((nowTimeGlobal-lastTimeNTPCheck) >= NTP_KO_CHECK_PERIOD || forceNTPCheck) {
    
    ntp_ko_check_period(false);
  }

  //Regular actions after THERMOSTATE_INTERRUPT_DELAY milliseconds the thermostate interrupt was triggered to check the value of Thermostate pin
  //Delay of THERMOSTATE_INTERRUPT_DELAY milliseconds is needed to avoid bouncing
  nowTimeGlobal=millis();
  if (thermostateInterrupt && (nowTimeGlobal-lastInterruptTime >= THERMOSTATE_INTERRUPT_DELAY)) {
    thermostate_interrupt_triggered(debugModeOn); //Sending HTTP Cloud updates is done from that function
  }

  //Regular actions every MQTT_CHECK_PERIOD seconds to check on MQTT connection
  nowTimeGlobal=millis();
  if (nowTimeGlobal-lastTimeMQTTCheck >= MQTT_CHECK_PERIOD) {
    if (wifiEnabled && WiFi.status()==WL_CONNECTED && mqttServerEnabled && MqttSyncCurrentStatus==MqttSyncOffStatus)
      mqttClientInit(wifiEnabled,mqttServerEnabled,secureMqttEnabled,NO_ERROR,false,debugModeOn,mqttTopicName,device);
    lastTimeMQTTCheck=nowTimeGlobal;
  }
  
  //Regular actions every SAMPLE_PERIOD seconds to take gas samples
  nowTimeGlobal=millis();
  if (nowTimeGlobal-lastGasSample >= SAMPLE_PERIOD) {
    gas_sample(false);   //Get GAS samples
    if (tempHumSensor.isConnected()) temperature_sample(false);  //Get Temp & Hum samples

    //JSON object is updated just right after taking the samples.
    //Publish MQTT message with the new samples
    forceMQTTpublish=true;
    lastGasSample=nowTimeGlobal;
  }

  //Check if mqtt samples must be published
  //Situations for forceMQTTpublish=true:
  // - every SAMPLE_PERIOD
  // - Relays ON/OFF from HA (message received in topic: the-iot-factory/boiler-relay-controlv2-2254C4/cmnd/RELAY)
  if (forceMQTTpublish) {
    mqtt_publish_samples(wifiEnabled,mqttServerEnabled,secureMqttEnabled,false);
    forceMQTTpublish=false;
  }
  
  //Check interrupt flags
  if (gasInterrupt) {
    if (debugModeOn) {boardSerialPort.println(String(millis())+"  - [loop] - GAS interrupt detected and gas_probe_triggered routine triggered: gasClear="+String(gasClear));}
    gas_sample(debugModeOn);
    gasInterrupt=false;

    //JSON object is updated just right after taking the samples.
    //Publish MQTT message with the new samples
    forceMQTTpublish=true;
  }
 
}