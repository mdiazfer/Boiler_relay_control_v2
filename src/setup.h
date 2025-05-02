#include <HardwareSerial.h>
#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <Arduino_JSON.h>
#include "SHT2x.h"
#include "wifiConnection.h"
#include "mqttClient.h"
#include "eeprom_utils.h"


#ifndef __hex_digits__
  static const char hex_digits[] = "0123456789ABCDEF";
  #define __hex_digits__
#endif

extern RTC_DATA_ATTR byte mac[6];
extern RTC_DATA_ATTR String tempHumSensorType;
extern RTC_DATA_ATTR AsyncMqttClient mqttClient;
extern RTC_DATA_ATTR uint8_t bootCount,resetCount,resetPreventiveCount,resetSWCount;
extern RTC_DATA_ATTR uint16_t year,previousYear;
extern RTC_DATA_ATTR uint32_t minHeapSeen;
extern RTC_DATA_ATTR uint64_t lastTimeHTTPClouCheck,lastTimeNTPCheck,lastMQTTChangeCheck,lastCloudClockChangeCheck,lastTimeMQTTSampleCheck;
extern RTC_DATA_ATTR boolean wifiEnabled,httpCloudEnabled,mqttServerEnabled,secureMqttEnabled,webServerEnabled,bluetoothEnabled,ntpSynced;
extern RTC_DATA_ATTR struct timeOnCounters heaterTimeOnYear,heaterTimeOnPreviousYear,boilerTimeOnYear,boilerTimeOnPreviousYear;

extern bool debugModeOn;
extern HardwareSerial boardSerialPort;
extern wifiNetworkInfo wifiNet;
extern wifiCredentials wifiCred;
extern String ntpServers[4];
extern uint8_t ntpServerIndex,auxLoopCounter,auxLoopCounter2,auxCounter,configVariables;
extern uint64_t whileLoopTimeLeft;
extern String device,TZEnvVariable,TZName,mqttUserName,mqttUserPssw,mqttTopicPrefix,mqttTopicName,mqttServer,userName,userPssw;
extern SHT2x tempHumSensor; //Temp and Hum sensor
extern String serverToUploadSamplesString;
extern IPAddress serverToUploadSamplesIPAddress;
extern int sendHttpRequest(bool debugModeOn, IPAddress server, uint16_t port, String httpRequest,bool fromSetup);
extern char activeCookie[COOKIE_SIZE],currentSetCookie[COOKIE_SIZE],firmwareVersion[VERSION_CHAR_LENGTH+1];
extern JSONVar samples;

String IpAddress2String(const IPAddress& ipAddress);
IPAddress stringToIPAddress(String stringIPAddress);
bool wifiVariablesInit();
bool initTZVariables();
bool ntpVariablesInit();
bool mqttVariablesInit();
void EEPROMInit();
void variablesInit();
uint32_t tempSensorInit(boolean debugModeOn);
uint32_t wifiInit(boolean wifiEnabled,boolean debugModeOn);
uint32_t httpCloudInit(boolean wifiEnabled,boolean httpCloudEnabled,enum wifiStatus wifiCurrentStatus,boolean debugModeOn,boolean fromSetup);
uint32_t ntpInit(boolean wifiEnabled,boolean ntpEnabled,enum wifiStatus wifiCurrentStatus,boolean debugModeOn,boolean fromSetup);
uint32_t mqttClientInit(boolean wifiEnabled, boolean mqttServerEnabled, boolean secureMqttEnabled,uint32_t error_setup, bool debugModeOn, bool fromSetup, String mqttTopicName, String device);
uint32_t timeOnCountersInit(uint32_t error_setup,bool debugModeOn,bool fromSetup, bool ntpSynced);