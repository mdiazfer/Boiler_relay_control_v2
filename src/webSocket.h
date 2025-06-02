#include <Arduino.h>
#include <HardwareSerial.h>
#include "global_setup.h"
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <Arduino_JSON.h>

extern RTC_DATA_ATTR AsyncWebServer webServer;
extern RTC_DATA_ATTR AsyncWebSocket webSocket;
extern RTC_DATA_ATTR HardwareSerial boardSerialPort;
extern RTC_DATA_ATTR bool forceMQTTpublish,forceWebEvent;

extern bool webServerResponding,debugModeOn,webLogsOn;
extern String bootLogs;
extern JSONVar samples;

#ifndef _PRINT_LOG_DEFINITION_
  extern void printLogln(String logMessage, unsigned char base=10);
  extern void printLog(String logMessage, unsigned char base=10);
  extern void printLogln(uint8_t logMessage, unsigned char base=10);
  extern void printLog(uint8_t logMessage, unsigned char base=10);
  extern void printLogln(tm * timeinfo, const char *format);
  extern void printLog(tm * timeinfo, const char *format);
  #define _PRINT_LOG_DEFINITION_
#endif

void notifyClients(String message);
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
uint32_t initWebSocket();