#include <HardwareSerial.h>
#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <Arduino_JSON.h>
#ifdef SYSLOG_SERVER
  #include <PicoSyslog.h>
#endif
#include "SHT2x.h"
#include "wifiConnection.h"
#include "mqttClient.h"
#include "eeprom_utils.h"
#include "webServer.h"
#include "webSocket.h"
#include "esp_partition.h"
#include "esp_log.h"
#include "esp_ota_ops.h"

#ifndef __hex_digits__
  static const char hex_digits[] = "0123456789ABCDEF";
  #define __hex_digits__
#endif

extern RTC_DATA_ATTR byte mac[6];
extern RTC_DATA_ATTR String tempHumSensorType,bootLogs;
extern RTC_DATA_ATTR AsyncMqttClient mqttClient;
extern RTC_DATA_ATTR uint8_t bootCount,resetCount,resetPreventiveCount,resetPreventiveWebServerCount,resetSWCount,resetWebServerCnt,
                              errorsWiFiCnt,errorsNTPCnt,errorsHTTPUptsCnt,errorsMQTTCnt,SPIFFSErrors,errorsWebServerCnt,errorsConnectivityCnt;
extern RTC_DATA_ATTR uint16_t year,previousYear,powerOnFlameThreshold;
extern RTC_DATA_ATTR uint32_t minMaxHeapBlockSizeSinceUpgrade,minHeapSinceUpgrade,minHeapSinceBoot,minMaxHeapBlockSizeSinceBoot,minHeapSeen;
extern RTC_DATA_ATTR uint64_t lastTimeHTTPClouCheck,lastTimeNTPCheck,lastMQTTChangeCheck,lastCloudClockChangeCheck,lastTimeMQTTSampleCheck;
extern RTC_DATA_ATTR boolean wifiEnabled,httpCloudEnabled,mqttServerEnabled,secureMqttEnabled,webServerEnabled,bluetoothEnabled,ntpSynced,
                              OTAUpgradeBinAllowed,SPIFFSUpgradeBinAllowed,logTagged,forceWifiReconnect,forceWebServerInit,powerMeasureEnabled,powerMeasureSubscribed;
extern RTC_DATA_ATTR struct timeOnCounters heaterTimeOnYear,heaterTimeOnPreviousYear,boilerTimeOnYear,boilerTimeOnPreviousYear;
extern RTC_DATA_ATTR HardwareSerial boardSerialPort;

extern bool debugModeOn,logMessageTOFF,logMessageTRL1_ON,logMessageTRL2_ON,logMessageGAP_OFF,
  boilerStatus,thermostateStatus,boilerOn,thermostateOn,thermostateInterrupt,gasClear,gasInterrupt,isBeaconAdvertising,webServerResponding,
  webLogsOn,serialLogsOn,sysLogsOn,eepromUpdate;
extern char activeCookie[COOKIE_SIZE],currentSetCookie[COOKIE_SIZE],firmwareVersion[VERSION_CHAR_LENGTH+1];
extern uint8_t ntpServerIndex,auxLoopCounter,auxLoopCounter2,auxCounter,configVariables,fileUpdateError,errorOnActiveCookie,errorOnWrongCookie;
extern uint16_t rebounds,voltage,power,sysLogServerUDPPort;
extern uint32_t heapSize,heapBlockSize,flashSize,programSize,fileSystemSize,fileSystemUsed;
extern uint64_t whileLoopTimeLeft;
extern int sendHttpRequest(bool debugModeOn, IPAddress server, uint16_t port, String httpRequest,bool fromSetup);
extern float gasSample,gasVoltCalibrated,RS_airCalibrated,RS_CurrentCalibrated,gasRatioSample,current,energyToday,energyYesterday,energyTotal;
extern size_t fileUpdateSize,OTAAvailableSize,SPIFFSAvailableSize;
extern String serverToUploadSamplesString,device,TZEnvVariable,TZName,mqttUserName,mqttUserPssw,mqttTopicPrefix,mqttTopicName,mqttServer,sysLogServer,userName,userPssw,powerMqttTopic;
extern wifiNetworkInfo wifiNet;
extern wifiCredentials wifiCred;
extern String ntpServers[4];
extern SHT2x tempHumSensor; //Temp and Hum sensor
extern IPAddress serverToUploadSamplesIPAddress;
extern JSONVar samples;
#ifdef SYSLOG_SERVER
  extern PicoSyslog::Logger syslog;
#endif

#ifndef _PRINT_LOG_DEFINITION_
  void printLogln(String logMessage, unsigned char base=10);
  void printLog(String logMessage, unsigned char base=10);
  void printLog(uint8_t logMessage, unsigned char base=10);
  void printLogln(tm * timeinfo, const char *format);
  void printLogln(tm * timeinfo, const char *format);
  void printLog(tm * timeinfo, const char *format);
  #define _PRINT_LOG_DEFINITION_
#endif

void detachNetwork(void);
String roundFloattoString(float_t number, uint8_t decimals);
String IpAddress2String(const IPAddress& ipAddress);
IPAddress stringToIPAddress(String stringIPAddress);
size_t getAppOTAPartitionSize(uint8_t type, uint8_t subtype);
bool wifiVariablesInit();
bool initTZVariables();
bool ntpVariablesInit();
bool mqttVariablesInit();
bool syslogVariablesInit();
void EEPROMInit();
void variablesInit();
uint32_t tempSensorInit(boolean debugModeOn);
uint32_t wifiInit(boolean wifiEnabled,boolean debugModeOn);
uint32_t httpCloudInit(boolean wifiEnabled,boolean httpCloudEnabled,enum wifiStatus wifiCurrentStatus,boolean debugModeOn,boolean fromSetup);
uint32_t ntpInit(boolean wifiEnabled,boolean ntpEnabled,enum wifiStatus wifiCurrentStatus,boolean debugModeOn,boolean fromSetup);
uint32_t mqttClientInit(boolean wifiEnabled, boolean mqttServerEnabled, boolean secureMqttEnabled,uint32_t error_setup, bool debugModeOn, bool fromSetup, String mqttTopicName, String device);
uint32_t spiffsInit(boolean debugModeOn,boolean fromSetup);
uint32_t webServerInit(uint32_t error_setup, boolean wifiEnabled,boolean webServerEnabled,enum wifiStatus wifiCurrentStatus,boolean debugModeOn,boolean fromSetup);
uint32_t timeOnCountersInit(uint32_t error_setup,bool debugModeOn,bool fromSetup, bool ntpSynced);
