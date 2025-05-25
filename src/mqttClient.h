#include <Arduino.h>
#include <AsyncMqttClient.h>
#include <Arduino_JSON.h>
#include <EEPROM.h>
#include "global_setup.h"
#include "wifiConnection.h"

extern RTC_DATA_ATTR AsyncMqttClient mqttClient;
extern RTC_DATA_ATTR enum MqttSyncStatus MqttSyncCurrentStatus;
extern RTC_DATA_ATTR boolean debugModeOn,wifiEnabled,mqttServerEnabled,forceMQTTpublish,thermostateStatus,updateHADiscovery;
extern RTC_DATA_ATTR uint32_t error_setup,minHeapSinceUpgrade,minHeapSinceBoot;
extern RTC_DATA_ATTR uint8_t bootCount,resetCount,resetSWCount,resetSWWebCount,resetSWMqttCount,resetSWUpgradeCount,errorsHTTPUptsCnt,errorsMQTTCnt;
extern RTC_DATA_ATTR uint16_t year;
extern RTC_DATA_ATTR uint64_t lastThermostatOnTime;
extern RTC_DATA_ATTR struct timeOnCounters heaterTimeOnYear,heaterTimeOnPreviousYear,boilerTimeOnYear,boilerTimeOnPreviousYear;
extern String mqttTopicPrefix,mqttTopicName,mqttServer,device,iconWifi,iconGasInterrupt,iconThermInterrupt,iconThermStatus;
extern JSONVar samples;
extern bool gasClear;

#ifndef _PRINT_LOG_DEFINITION_
  extern void printLogln(String logMessage, unsigned char base=10);
  extern void printLog(String logMessage, unsigned char base=10);
  extern void printLogln(uint8_t logMessage, unsigned char base=10);
  extern void printLog(uint8_t logMessage, unsigned char base=10);
  extern void printLogln(tm * timeinfo, const char *format);
  extern void printLog(tm * timeinfo, const char *format);
  #define _PRINT_LOG_DEFINITION_
#endif

void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttSubscribe(uint16_t packetId, uint8_t qos);
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
void onMqttUnsubscribe(uint16_t packetId);
void onMqttPublish(uint16_t packetId);
void mqttClientPublishHADiscovery(String mqttTopicName, String device, String ipAddress, bool removeTopics);

