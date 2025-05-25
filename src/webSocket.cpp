#include "webSocket.h"

void notifyClients(String logMessage) {
  /******************************************************
   Function notifyClients
   Target: Notify all the clients with the message
   Parameters:
    String: logMessage to be sent over the web socket
   Return: Nothing
  *****************************************************/
  
  webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
  //webSocket->textAll(logMessage);
  webSocket.textAll(logMessage);
  webServerResponding=false; //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed again
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len, uint32_t id) {
  /******************************************************
   Function handleWebSocketMessage
   Target: Handle what happens when a mmessage is received over the web socket
   Parameters:
    void *arg:  Pointer to the argument
    uint8_t *data: Pointer to the received data
    size_t len: size of the received data buffer
    uint32_t id: client id
   Return: Nothing
  *****************************************************/

  webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String message = (char*)data;
    
    if (debugModeOn) {printLogln(String(millis())+" - [handleWebSocketMessage] - Data received over the web socket");}
    
    //Check if the message is "stop"
    if (strcmp((char*)data, "getBootLogs") == 0) {
      if (debugModeOn) {printLogln(String("        [handleWebSocketMessage] - 'getBootLogs' message received"));}
      //Do something here
      //webSocket->text(id,bootLogs);
      webSocket.text(id,bootLogs);
    }
    else if (strcmp((char*)data, "switchWebLogsOn") == 0) {
      if (debugModeOn) {printLogln(String("        [handleWebSocketMessage] - 'switchWebLogsOn' message received"));}
      //Do something here
      webLogsOn=true;
    }
    else if (strcmp((char*)data, "switchWebLogsOff") == 0) {
      if (debugModeOn) {printLogln(String("        [handleWebSocketMessage] - 'switchWebLogsOff' message received"));}
      //Do something here
      webLogsOn=false;
    }
  }
  webServerResponding=false; //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed againAwsFrameInfo *info = (AwsFrameInfo*)arg;
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  /******************************************************
   Function onEvent
   Target: Handle web socket events
   Parameters:
    AsyncWebSocket *server: pointer to the web socket server
    AsyncWebSocketClient *client: pointer to the web socket client
    AwsEventType type: Event type
    void *arg:  Pointer to the argument
    uint8_t *data: Pointer to the received data
    size_t len: size of the received data buffer
   Return: Nothing
  *****************************************************/

  webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
  switch (type) {
    case WS_EVT_CONNECT:
      if (debugModeOn) {printLogln(String(millis())+" - [onEvent] - WebSocket client #"+String(client->id())+" connected from "+client->remoteIP().toString());}
      //Do something
      //webSocket.textAll(bootLogs); //Send the boot logs
      break;
    case WS_EVT_DISCONNECT:
      if (debugModeOn) {printLogln(String(millis())+"  - [onEvent] - WebSocket client #"+String(client->id())+" disconnected");}
      //Do something
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len, client->id());
      break;
    case WS_EVT_PONG:
      if (debugModeOn) {printLogln(String(millis())+" - [onEvent] - WebSocket PONG event, client #"+String(client->id()));}
      //Do something
      break;
    case WS_EVT_ERROR:
      if (debugModeOn) {printLogln(String(millis())+" - [onEvent] - WebSocket ERROR event, client #"+String(client->id()));}
      //Do something
      break;
  }
  webServerResponding=false; //WebServer ends, heap is goint to be realeased, so BLE iBeacons are allowed again
}

uint32_t initWebSocket() {
  /******************************************************
   Function initWebSocket
   Target: Init the web socket
   Parameters: None
   Returns:
    uint32_t error: Error code
   *****************************************************/ 

  if (debugModeOn) {printLogln(String(millis())+" - [initWebSocket] - Begin webSocket");}
  //webSocket->onEvent(onEvent);
  //if (!webSocket) webServer.addHandler(webSocket);
  webSocket.onEvent(onEvent);
  webServer.addHandler(&webSocket);
     
  return NO_ERROR;
}