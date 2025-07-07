#include <Arduino.h>
#include <AsyncMqttClient.h>
#include <Arduino_JSON.h>
#include <EEPROM.h>
#include "global_setup.h"
#include "wifiConnection.h"

extern RTC_DATA_ATTR AsyncMqttClient mqttClient;
extern RTC_DATA_ATTR enum MqttSyncStatus MqttSyncCurrentStatus;
extern RTC_DATA_ATTR boolean debugModeOn,wifiEnabled,mqttServerEnabled,forceMQTTpublish,forceWebEvent,thermostateStatus,updateHADiscovery,powerMeasureEnabled,powerMeasureSubscribed;
extern RTC_DATA_ATTR uint32_t error_setup,minHeapSinceUpgrade,minHeapSinceBoot;
extern RTC_DATA_ATTR uint8_t bootCount,resetCount,resetSWCount,resetSWWebCount,resetSWMqttCount,resetSWUpgradeCount,errorsHTTPUptsCnt,errorsMQTTCnt;
extern RTC_DATA_ATTR uint16_t year;
extern RTC_DATA_ATTR uint64_t lastThermostatOnTime,lastBoilerOnTime;
extern RTC_DATA_ATTR struct timeOnCounters heaterTimeOnYear,heaterTimeOnPreviousYear,boilerTimeOnYear,boilerTimeOnPreviousYear;
extern String mqttTopicPrefix,mqttTopicName,mqttServer,device,iconWifi,iconGasInterrupt,iconThermInterrupt,iconThermStatus,powerMqttTopic;
extern JSONVar samples;
extern bool gasClear,thermostateStatus,boilerStatus,boilerOn,thermostateOn,webLogsOn,serialLogsOn,sysLogsOn;
extern uint16_t voltage,power,powerMeasureId;
extern float current,energyToday,energyYesterday,energyTotal;
extern byte mac[6];
extern char bufferTopicHAName[100],bufferPayload[BUFFER_PAYLOAD_SIZE],bufferMqttTopicName[100],bufferDeviceSufix[10],
            bufferIpAddress[20],bufferDevice[50],bufferMqttSensorTopicHAPrefixName[100],bufferMqttBinarySensorTopicHAPrefixName[100],
            bufferMqttButtonTopicHAPrefixName[100],bufferMqttSwitchTopicHAPrefixName[100];

#ifndef _PRINT_LOG_DEFINITION_
  extern void printLogln(String logMessage, unsigned char base=10);
  extern void printLog(String logMessage, unsigned char base=10);
  extern void printLogln(uint8_t logMessage, unsigned char base=10);
  extern void printLog(uint8_t logMessage, unsigned char base=10);
  extern void printLogln(tm * timeinfo, const char *format);
  extern void printLog(tm * timeinfo, const char *format);
  #define _PRINT_LOG_DEFINITION_
#endif
#ifndef __hex_digits__
  static const char hex_digits[] = "0123456789ABCDEF";
  #define __hex_digits__
#endif

void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttSubscribe(uint16_t packetId, uint8_t qos);
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
void onMqttUnsubscribe(uint16_t packetId);
void onMqttPublish(uint16_t packetId);
void mqttClientPublishHADiscovery(String mqttTopicName, String device, String ipAddress, bool removeTopics);
void mqttClientPublishHADiscovery_systemObjects1(String mqttTopicName, String device, String ipAddress, bool removeTopics);
void mqttClientPublishHADiscovery_systemObjects2(String mqttTopicName, String device, String ipAddress, bool removeTopics);
void mqttClientPublishHADiscovery_sensorsObjects(String mqttTopicName, String device, String ipAddress, bool removeTopics);
void mqttClientPublishHADiscovery_boilerTimeOnObjects(String mqttTopicName, String device, String ipAddress, bool removeTopics);
void mqttClientPublishHADiscovery_heaterTimeOnObjects(String mqttTopicName, String device, String ipAddress, bool removeTopics);
void mqttClientPublishHADiscovery_binaryObjects(String mqttTopicName, String device, String ipAddress, bool removeTopics);
void mqttClientPublishHADiscovery_relayObjects(String mqttTopicName, String device, String ipAddress, bool removeTopics);
void mqttClientPublishHADiscovery_buttonObjects(String mqttTopicName, String device, String ipAddress, bool removeTopics);

