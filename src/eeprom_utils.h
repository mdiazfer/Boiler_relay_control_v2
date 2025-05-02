#include <HardwareSerial.h>
#include <EEPROM.h>
#include "wifiConnection.h"

extern RTC_DATA_ATTR boolean wifiEnabled,httpCloudEnabled,mqttServerEnabled,secureMqttEnabled,webServerEnabled,bluetoothEnabled;
extern RTC_DATA_ATTR uint8_t bootCount,resetCount,resetPreventiveCount,resetSWCount;
extern RTC_DATA_ATTR uint32_t minHeapSeen; //1*4=4B
extern RTC_DATA_ATTR uint32_t yesterday,today;
extern RTC_DATA_ATTR struct timeOnCounters heaterTimeOnYear,heaterTimeOnPreviousYear,boilerTimeOnYear,boilerTimeOnPreviousYear;

extern String TZEnvVariable,TZName,device,
        mqttServer,mqttUserName,mqttUserPssw,mqttTopicPrefix,mqttTopicName,userName,userPssw;
extern bool debugModeOn;
extern HardwareSerial boardSerialPort;
extern wifiCredentials wifiCred;

uint16_t checkSum(byte *addr, uint32_t count);
void factoryConfReset();