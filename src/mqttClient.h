#include <Arduino.h>
#include <AsyncMqttClient.h>
#include <Arduino_JSON.h>
#include "global_setup.h"
#include "wifiConnection.h"

extern RTC_DATA_ATTR AsyncMqttClient mqttClient;
extern RTC_DATA_ATTR enum MqttSyncStatus MqttSyncCurrentStatus;
extern RTC_DATA_ATTR boolean debugModeOn,wifiEnabled,mqttServerEnabled,forceMQTTpublish;
extern RTC_DATA_ATTR uint32_t error_setup;
extern RTC_DATA_ATTR uint8_t bootCount,resetCount;
extern RTC_DATA_ATTR int errorsHTTPUptsCnt,errorsMQTTCnt;
extern String mqttTopicPrefix,mqttTopicName,mqttServer,device,iconWifi,iconGasInterrupt,iconThermInterrupt,iconThermStatus;
extern JSONVar samples;
extern bool gasClear;

void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttSubscribe(uint16_t packetId, uint8_t qos);
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
void onMqttUnsubscribe(uint16_t packetId);
void onMqttPublish(uint16_t packetId);
void mqttClientPublishHADiscovery(String mqttTopicName, String device, String ipAddress);

