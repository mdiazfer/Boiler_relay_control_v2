#include <HardwareSerial.h>
#include <EEPROM.h>
#include "wifiConnection.h"

extern RTC_DATA_ATTR boolean wifiEnabled,httpCloudEnabled,mqttServerEnabled,secureMqttEnabled,webServerEnabled,bluetoothEnabled,powerMeasureEnabled;
extern RTC_DATA_ATTR uint8_t bootCount,resetCount,resetPreventiveCount,resetSWCount,resetPreventiveWebServerCount,resetSWWebCount,resetSWMqttCount,resetSWUpgradeCount,resetWebServerCnt,
                              errorsWiFiCnt,errorsNTPCnt,errorsHTTPUptsCnt,errorsMQTTCnt,SPIFFSErrors,errorsWebServerCnt,errorsConnectivityCnt;
extern RTC_DATA_ATTR uint32_t minHeapSinceUpgrade,minMaxHeapBlockSizeSinceUpgrade; //1*4=4B
extern RTC_DATA_ATTR uint16_t powerOnFlameThreshold;
extern RTC_DATA_ATTR uint32_t yesterday,today;
extern RTC_DATA_ATTR struct timeOnCounters heaterTimeOnYear,heaterTimeOnPreviousYear,boilerTimeOnYear,boilerTimeOnPreviousYear;
extern RTC_DATA_ATTR HardwareSerial boardSerialPort;

extern String TZEnvVariable,TZName,device,
        mqttServer,mqttUserName,mqttUserPssw,mqttTopicPrefix,mqttTopicName,userName,userPssw,powerMqttTopic,sysLogServer;
extern bool debugModeOn,webLogsOn,serialLogsOn,sysLogsOn;
extern uint16_t sysLogServerUDPPort;
extern wifiCredentials wifiCred;

#ifndef _PRINT_LOG_DEFINITION_
  extern void printLogln(String logMessage, unsigned char base=10);
  extern void printLog(String logMessage, unsigned char base=10);
  extern void printLogln(uint8_t logMessage, unsigned char base=10);
  extern void printLog(uint8_t logMessage, unsigned char base=10);
  extern void printLogln(tm * timeinfo, const char *format);
  extern void printLog(tm * timeinfo, const char *format);
  #define _PRINT_LOG_DEFINITION_
#endif

uint16_t checkSum(byte *addr, uint32_t count);
void factoryConfReset();
