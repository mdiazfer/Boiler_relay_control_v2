#include <Arduino.h>
#include <HardwareSerial.h>
#include "global_setup.h"
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#ifdef SYSLOG_SERVER
  #include <PicoSyslog.h>
#endif
#include <WiFi.h>
#include <EEPROM.h>
#include <Arduino_JSON.h>
#include <AsyncMqttClient.h>
#include "Update.h"
#include "wifiConnection.h"
#include "SHT2x.h"

extern RTC_DATA_ATTR boolean deviceReset,factoryReset,OTAUpgradeBinAllowed,SPIFFSUpgradeBinAllowed,httpCloudEnabled,
                      mqttServerEnabled,secureMqttEnabled,forceMQTTConnect,forceWifiReconnect,forceNTPCheck,
                      bluetoothEnabled,wifiEnabled,reconnectWifiAndRestartWebServer,resyncNTPServer,powerMeasureEnabled,powerMeasureSubscribed;
extern RTC_DATA_ATTR uint8_t bootCount,resetCount,resetPreventiveCount,resetPreventiveWebServerCount,resetSWCount,resetSWWebCount,resetSWMqttCount,resetSWUpgradeCount,resetWebServerCnt,
                              errorsWiFiCnt,errorsNTPCnt,errorsHTTPUptsCnt,errorsMQTTCnt,SPIFFSErrors,errorsWebServerCnt,errorsConnectivityCnt;
extern RTC_DATA_ATTR uint16_t powerOnFlameThreshold;
extern RTC_DATA_ATTR uint32_t error_setup,minHeapSinceUpgrade,minHeapSinceBoot,minMaxHeapBlockSizeSinceBoot,minMaxHeapBlockSizeSinceUpgrade;
extern RTC_DATA_ATTR uint64_t nowTimeGlobal,lastTimeHTTPClouCheck;
extern RTC_DATA_ATTR float valueT,valueHum;
extern RTC_DATA_ATTR AsyncWebServer webServer;
extern RTC_DATA_ATTR AsyncEventSource webEvents;
extern RTC_DATA_ATTR HardwareSerial boardSerialPort;
extern RTC_DATA_ATTR char BLEProximityUUID[];
extern RTC_DATA_ATTR AsyncMqttClient mqttClient;
extern RTC_DATA_ATTR struct tm startTimeInfo;
extern RTC_DATA_ATTR enum CloudSyncStatus CloudSyncCurrentStatus;
extern RTC_DATA_ATTR enum MqttSyncStatus MqttSyncCurrentStatus;
extern RTC_DATA_ATTR String tempHumSensorType;
extern RTC_DATA_ATTR enum CloudClockStatus CloudClockCurrentStatus;
extern RTC_DATA_ATTR enum wifiStatus wifiCurrentStatus;
extern RTC_DATA_ATTR struct timeOnCounters heaterTimeOnYear,boilerTimeOnYear,boilerTimeOnPreviousYear,heaterTimeOnPreviousYear;

extern bool webServerResponding,debugModeOn,webLogsOn,serialLogsOn,sysLogsOn,boilerStatus,thermostateStatus,gasClear,forceMQTTpublish;
extern char activeCookie[],currentSetCookie[];
extern uint8_t fileUpdateError,errorOnActiveCookie,errorOnWrongCookie;
extern uint16_t voltage,power,sysLogServerUDPPort;
extern int updateCommand;
extern float current,energyToday,energyYesterday,energyTotal;
extern size_t fileUpdateSize,OTAAvailableSize,SPIFFSAvailableSize;
extern String device,lastURI,fileUpdateName,userName,userPssw,mqttTopicPrefix,mqttTopicName,mqttServer,mqttUserName,mqttUserPssw,
              TZEnvVariable,TZName,ntpServers[4],powerMqttTopic,sysLogServer;
extern JSONVar samples;
extern wifiCredentials wifiCred;
extern IPAddress serverToUploadSamplesIPAddress;
#ifdef SYSLOG_SERVER
  extern PicoSyslog::Logger syslog;
#endif

#ifndef _PRINT_LOG_DEFINITION_
  extern void printLogln(String logMessage, unsigned char base=10);
  extern void printLog(String logMessage, unsigned char base=10);
  extern void printLogln(uint8_t logMessage, unsigned char base=10);
  extern void printLog(uint8_t logMessage, unsigned char base=10);
  extern void printLogln(tm * timeinfo, const char *format);
  extern void printLog(tm * timeinfo, const char *format);
  #define _PRINT_LOG_DEFINITION_
#endif

extern void factoryConfReset();
extern String roundFloattoString(float_t number, uint8_t decimals);
extern void detachNetwork(void);

extern SHT2x tempHumSensor; //Temp and Hum sensor
extern void gas_sample(bool debugModeOn);
extern void temperature_sample(bool debugModeOn);

extern void mqttClientPublishHADiscovery(String mqttTopicName, String device, String ipAddress, bool removeTopics);

String processorInfo(const String& var);
uint32_t initWebServer();
