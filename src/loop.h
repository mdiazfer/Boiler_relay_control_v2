#include <HardwareSerial.h>
#include <WiFi.h>
#include <Arduino_JSON.h>
#include <EEPROM.h>
#include "SHT2x.h"
#include "global_setup.h"
#include "setup.h"
#include "mqttClient.h"
#include "wifiConnection.h"
#include "mq5_sensor.h"

extern RTC_DATA_ATTR boolean forceWifiReconnect,forceWebServerInit,firstBoot,wifiResuming,forceWEBTestCheck,forceNTPCheck,wifiEnabled,timersEepromUpdate,updateHADiscovery;
extern RTC_DATA_ATTR uint8_t bootCount,resetCount,resetPreventiveCount,resetSWCount;
extern RTC_DATA_ATTR uint16_t year;
extern RTC_DATA_ATTR int errorsHTTPUptsCnt,errorsMQTTCnt;
extern RTC_DATA_ATTR uint64_t nowTimeGlobal,lastTimeWifiReconnectionCheck,whileLoopTimeLeft,lastCloudClockChangeCheck,lastTimeMQTTCheck,lastThermostatOnTime,lastTimeSecondCheck,lastTimeTimerEepromUpdateCheck;
extern RTC_DATA_ATTR ulong wifiReconnectPeriod;
extern RTC_DATA_ATTR float valueHum,tempSensor,valueT;
extern RTC_DATA_ATTR enum wifiStatus wifiCurrentStatus;
extern RTC_DATA_ATTR enum CloudClockStatus CloudClockCurrentStatus;
extern RTC_DATA_ATTR enum CloudClockStatus CloudClockLastStatus;
extern RTC_DATA_ATTR enum CloudSyncStatus CloudSyncCurrentStatus;
extern RTC_DATA_ATTR enum CloudSyncStatus CloudSyncLastStatus;
extern RTC_DATA_ATTR enum MqttSyncStatus MqttSyncCurrentStatus; // 4B
extern RTC_DATA_ATTR enum MqttSyncStatus MqttSyncLastStatus; // 4B
extern RTC_DATA_ATTR struct tm startTimeInfo;
extern RTC_DATA_ATTR struct timeOnCounters heaterTimeOnYear,heaterTimeOnPreviousYear,boilerTimeOnYear,boilerTimeOnPreviousYear;

extern bool debugModeOn;
extern HardwareSerial boardSerialPort;
extern uint8_t auxLoopCounter,auxLoopCounter2,auxCounter;
extern enum CloudClockStatus previousCloudClockCurrentStatus;
extern bool thermostateStatus,thermostateInterrupt,gasInterrupt,gasClear,httpCloudEnabled;
extern uint16_t rebounds;
extern float gasSample,gasVoltCalibrated,RS_airCalibrated,RS_CurrentCalibrated,gasRatioSample;
extern SHT2x tempHumSensor; //Temp and Hum sensor
extern JSONVar samples;
extern String device,mqttTopicName,iconWifi,iconGasInterrupt,iconThermInterrupt,iconThermStatus;


extern IPAddress serverToUploadSamplesIPAddress;

void  wifi_reconnect_period(bool debugModeOn);
void ntp_ko_check_period(bool debugModeOn);
int sendHttpRequest(bool debugModeOn, IPAddress server, uint16_t port, String httpRequest, bool fromSetup);
void thermostate_interrupt_triggered(bool debugModeOn);
void gas_sample(bool debugModeOn);
void temperature_sample(bool debugModeOn);
void mqtt_publish_samples(boolean wifiEnabled, boolean mqttServerEnabled, boolean secureMqttEnabled, bool debugModeOn);
void one_second_check_period(bool debugModeOn, uint64_t nowTimeGlobal,bool ntpSynced);
void time_counters_eeprom_update_check_period(bool debugModeOn, uint64_t nowTimeGlobal);