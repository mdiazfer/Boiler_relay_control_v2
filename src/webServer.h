#include <Arduino.h>
#include <HardwareSerial.h>
#include "global_setup.h"
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <Arduino_JSON.h>
#include <AsyncMqttClient.h>
#include "Update.h"
#include "wifiConnection.h"

extern RTC_DATA_ATTR boolean deviceReset,factoryReset,OTAUpgradeBinAllowed,SPIFFSUpgradeBinAllowed,httpCloudEnabled,
                      mqttServerEnabled,secureMqttEnabled,forceMQTTConnect,forceWifiReconnect,forceNTPCheck,
                      bluetoothEnabled,wifiEnabled,reconnectWifiAndRestartWebServer,resyncNTPServer,powerMeasureEnabled,powerMeasureSubscribed;
extern RTC_DATA_ATTR uint8_t bootCount,resetCount,resetPreventiveCount,resetPreventiveWebServerCount,resetSWCount,resetSWWebCount,resetSWMqttCount,resetSWUpgradeCount,resetWebServerCnt,
                              errorsWiFiCnt,errorsNTPCnt,errorsHTTPUptsCnt,errorsMQTTCnt,SPIFFSErrors,errorsWebServerCnt,errorsConnectivityCnt;
extern RTC_DATA_ATTR uint32_t error_setup,minHeapSinceUpgrade,minHeapSinceBoot;
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

extern bool webServerResponding,debugModeOn,boilerStatus,thermostateStatus,gasClear;
extern char activeCookie[],currentSetCookie[];
extern uint8_t fileUpdateError,errorOnActiveCookie,errorOnWrongCookie;
extern uint16_t voltage,power;
extern int updateCommand;
extern float current,energyToday,energyYesterday,energyTotal;
extern size_t fileUpdateSize,OTAAvailableSize,SPIFFSAvailableSize;
extern String device,lastURI,fileUpdateName,userName,userPssw,mqttTopicPrefix,mqttTopicName,mqttServer,mqttUserName,mqttUserPssw,
              TZEnvVariable,TZName,ntpServers[4],powerMqttTopic;
extern JSONVar samples;
extern wifiCredentials wifiCred;
extern IPAddress serverToUploadSamplesIPAddress;

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

String processorInfo(const String& var);
uint32_t initWebServer();
