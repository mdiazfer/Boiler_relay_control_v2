#include "mqttClient.h"


void onMqttConnect(bool sessionPresent) {
  String packetInfoId=String(mqttClient.subscribe(MQTT_TOPIC_SUBSCRIPTION,0));
  String packetStatId=String(mqttClient.subscribe(MQTT_HA_B_AND_LWT_TOPIC_PREFIX,0));
  //Subcribe to the-iot-factory/boiler-relay-controlv2-E02940/cmnd/RELAY
  String cmdSubcriptionTopic=String(MQTT_TOPIC_PREFIX+device+"/"+MQTT_TOPIC_CMD_SUFIX_SUBSCRIPTION);
  String packetSwitchId=String(mqttClient.subscribe(cmdSubcriptionTopic.c_str(),0));

  if (debugModeOn) printLogln("\n"+String(millis())+" - [onMqttConnect] - MQTT connected to "+mqttServer+". Session present: "+String(sessionPresent)+
                        "\n  [onMqttConnect] - Subscribing on:"+
                        "\n  [onMqttConnect] - topic "+MQTT_TOPIC_SUBSCRIPTION+", QoS 0, packetId="+packetInfoId+
                        "\n  [onMqttConnect] - topic "+MQTT_HA_B_AND_LWT_TOPIC_PREFIX+", QoS 0, packetId="+packetStatId+ // v1.9.0 - Home Assistant Last Will Testament message (offline)
                        "\n  [onMqttConnect] - topic "+cmdSubcriptionTopic+", QoS 0, packetId="+packetSwitchId); // Relay ON/OFF
  MqttSyncCurrentStatus=MqttSyncOnStatus;
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  if (debugModeOn) printLogln("\n"+String(millis())+" - [onMqttDisconnect] - MQTT disconnected, reason="+(uint8_t)reason);
  MqttSyncCurrentStatus=MqttSyncOffStatus;
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  if (debugModeOn) printLogln("\n"+String(millis())+" - [onMqttSubscribe] - MQTT subscribe acknowledged. packetId="+String(packetId)+", qos="+String(qos));
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  char aux[len+1];strcpy(aux,payload); aux[len]='\0';

  if (debugModeOn) printLogln("\n"+String(millis())+" - [onMqttMessage] - MQTT published message received on topic='"+String(topic)+"', mesage: '"+String(aux)+"', index: "+String(index));
  
  if (String(topic) == String(MQTT_TOPIC_SUBSCRIPTION)) {
    //Pubish device name
    mqttClient.publish(String(mqttTopicPrefix+"device-name").c_str(), 0, false, String("{\"manufacturer\":\"www.the-iotfactory.com\",\"model\":\"boiler-relay-controlv2\",\"name\":\""+device+"\"}").c_str());
  } else if (String(topic) == String(MQTT_HA_B_AND_LWT_TOPIC_PREFIX)) {
    //Home Assistant server published an availability message. Check on it.
    if ((String(aux).equalsIgnoreCase(String("online"))) && mqttServerEnabled) { //Publish the Discovery message for Home Assistan to detect this device
      if (debugModeOn) printLogln(String(millis())+" - [onMqttMessage] - Topic received '"+String(topic)+"' with message: '"+String(aux)+"'. Home Assistant server available. Publish the Discovery message now.");
      mqttClientPublishHADiscovery(mqttTopicPrefix+device,device,WiFi.localIP().toString(),false);
    }
  } else if (String(topic) == String(MQTT_TOPIC_PREFIX+device+"/"+MQTT_TOPIC_CMD_SUFIX_SUBSCRIPTION)) {
    if (debugModeOn) printLogln(String(millis())+" - [onMqttMessage] - Command received: "+String(aux));
    if (String(aux).equalsIgnoreCase(String("REBOOT"))) {
      printLogln(String(millis())+" - [onMqttMessage] - REBOOT");
      //Home Assistant support
      //Publish device is not available
      mqttClient.publish(String(mqttTopicName+"/LWT").c_str(), 0, false, "Offline\0"); //Availability message, not retain in the broker. This makes HA to subscribe to the */SENSOR topic if not already done
      resetSWMqttCount++; //Increase the counter for resets from mqtt
      EEPROM.write(0x533,resetSWMqttCount);
      EEPROM.commit();
      ESP.restart(); //Rebooting
    }
    else if (String(aux).equalsIgnoreCase(String("RESET_TIME_COUNTERS"))) {
      printLogln(String(millis())+" - [onMqttMessage] - RESET_TIME_COUNTERS");
      //Initialize time on counters
      heaterTimeOnYear.year=year; heaterTimeOnPreviousYear.year=year-1;
      for (int i=0;i<12;i++) {heaterTimeOnYear.counterMonths[i]=0;heaterTimeOnPreviousYear.counterMonths[i]=0;}
      heaterTimeOnYear.counterYesterday=0; heaterTimeOnPreviousYear.counterYesterday=0;
      heaterTimeOnYear.counterToday=0; heaterTimeOnPreviousYear.counterToday=0;
      EEPROM.put(0x421,heaterTimeOnYear); EEPROM.put(0x465,heaterTimeOnPreviousYear);
      
      boilerTimeOnYear.year=year; boilerTimeOnPreviousYear.year=year-1;
      for (int i=0;i<12;i++) {boilerTimeOnYear.counterMonths[i]=0;boilerTimeOnPreviousYear.counterMonths[i]=0;}
      boilerTimeOnYear.counterYesterday=0; boilerTimeOnPreviousYear.counterYesterday=0;
      boilerTimeOnYear.counterToday=0; boilerTimeOnPreviousYear.counterToday=0;
      EEPROM.put(0x4A9,boilerTimeOnYear); EEPROM.put(0x4ED,boilerTimeOnPreviousYear);
      EEPROM.commit();
      
      samples["heaterYear"] = "0";
      samples["heaterOnYearJan"] = "0";samples["heaterOnYearFeb"] = "0";samples["heaterOnYearMar"] = "0";samples["heaterOnYearApr"] = "0";samples["heaterOnYearMay"] = "0";samples["heaterOnYearJun"] = "0";
      samples["heaterOnYearJul"] = "0";samples["heaterOnYearAug"] = "0";samples["heaterOnYearSep"] = "0";samples["heaterOnYearOct"] = "0";samples["heaterOnYearNov"] = "0";samples["heaterOnYearDec"] = "0";
      samples["heaterYesterday"] = "0";
      samples["heaterToday"] = "0";
      samples["heaterPreviousYear"] = "0";
      samples["heaterOnPreviousYearJan"] = "0";samples["heaterOnPreviousYearFeb"] = "0";samples["heaterOnPreviousYearMar"] = "0";samples["heaterOnPreviousYearApr"] = "0";samples["heaterOnPreviousYearMay"] = "0";samples["heaterOnPreviousYearJun"] = "0";
      samples["heaterOnPreviousYearJul"] = "0";samples["heaterOnPreviousYearAug"] = "0";samples["heaterOnPreviousYearSep"] = "0";samples["heaterOnPreviousYearOct"] = "0";samples["heaterOnPreviousYearNov"] = "0";samples["heaterOnPreviousYearDec"] = "0";
      samples["boilerYear"] = "0";
      samples["boilerOnYearJan"] = "0";samples["boilerOnYearFeb"] = "0";samples["boilerOnYearMar"] = "0";samples["boilerOnYearApr"] = "0";samples["boilerOnYearMay"] = "0";samples["boilerOnYearJun"] = "0";
      samples["boilerOnYearJul"] = "0";samples["boilerOnYearAug"] = "0";samples["boilerOnYearSep"] = "0";samples["boilerOnYearOct"] = "0";samples["boilerOnYearNov"] = "0";samples["boilerOnYearDec"] = "0";
      samples["boilerYesterday"] = "0";
      samples["boilerToday"] = "0";
      samples["boilerPreviousYear"] = "0";
      samples["boilerOnPreviousYearJan"] = "0";samples["boilerOnPreviousYearFeb"] = "0";samples["boilerOnPreviousYearMar"] = "0";samples["boilerOnPreviousYearApr"] = "0";samples["boilerOnPreviousYearMay"] = "0";samples["boilerOnPreviousYearJun"] = "0";
      samples["boilerOnPreviousYearJul"] = "0";samples["boilerOnPreviousYearAug"] = "0";samples["boilerOnPreviousYearSep"] = "0";samples["boilerOnPreviousYearOct"] = "0";samples["boilerOnPreviousYearNov"] = "0";samples["boilerOnPreviousYearDec"] = "0";
      
      forceMQTTpublish=true; //Force to publish the MQTT message from the loop
    }
    if (gasClear) {
      if (String(aux).equalsIgnoreCase(String("R1_ON"))) {
        digitalWrite(PIN_RL1,LOW);samples["Relay1"] = String("R1_ON");
        printLogln(String(millis())+" - [onMqttMessage] - Set Relay1 OFF"); //Relay1 is set off to allow Ext. Thermostat (R1_ON)
        forceMQTTpublish=true; //Force to publish the MQTT message from the loop
        forceWebEvent=true; //Force to send webEvent from the loop to update Relay Switch Icon
      }
      else if (String(aux).equalsIgnoreCase(String("R1_OFF"))) {
        digitalWrite(PIN_RL1,HIGH);samples["Relay1"] = String("R1_OFF");
        printLogln(String(millis())+" - [onMqttMessage] - Set Relay1 ON"); //Relay1 is set on to not allow Ext. Thermostat (R1_OFF)
        forceMQTTpublish=true; //Force to publish the MQTT message from the loop
        forceWebEvent=true; //Force to send webEvent from the loop to update Relay Switch Icon
      }
      else if (String(aux).equalsIgnoreCase(String("R2_ON"))) {
        digitalWrite(PIN_RL2,HIGH);samples["Relay2"] = String("R2_ON");
        digitalWrite(PIN_RL1,LOW);samples["Relay1"] = String("R1_ON"); //Relay1 is set off to allow Ext. Thermostat (R1_ON) when the Relay2 is set
        printLogln(String(millis())+" - [onMqttMessage] - Set Relay1 OFF"); //Relay1 is set off to allow Ext. Thermostat (R1_ON)
        printLogln(String(millis())+" - [onMqttMessage] - Set Relay2 ON"); //Relay2 is set on to shortcut Ext. Thermostat (R2_ON)
        forceMQTTpublish=true; //Force to publish the MQTT message from the loop
        forceWebEvent=true; //Force to send webEvent from the loop to update Relay Switch Icon
        //thermostateStatus=true;lastThermostatOnTime=millis(); //Only for testing
      }
      else if (String(aux).equalsIgnoreCase(String("R2_OFF"))) {
        digitalWrite(PIN_RL2,LOW);samples["Relay2"] = String("R2_OFF");
        printLogln(String(millis())+" - [onMqttMessage] - Set Relay2 OFF"); //Relay2 is set off to not shortcut Ext. Thermostat (R2_OFF)
        forceMQTTpublish=true; //Force to publish the MQTT message from the loop
        forceWebEvent=true; //Force to send webEvent from the loop to update Relay Switch Icon
        //thermostateStatus=false; //Only for testing
      }
      else printLogln(String(millis())+" - [onMqttMessage] - Unknown command");
    }
    else printLogln(String(millis())+" - [onMqttMessage] - Gas leak situation detected, so no releay activation is allowed for security reasons");
  }
  else {
    //Do nothing for other topics
    if (debugModeOn) printLogln(String(millis())+" - [onMqttMessage] - Topic received ("+String(topic)+") and differs from subscriptions: '"+String(MQTT_TOPIC_SUBSCRIPTION)+"', '"+String(MQTT_HA_B_AND_LWT_TOPIC_PREFIX)+"'. Return");
  }
}

void onMqttUnsubscribe(uint16_t packetId) {
  if (debugModeOn) printLogln("\n"+String(millis())+" - [onMqttUnsubscribe] - MQTT unsubscribe acknowledged. packetId="+String(packetId));
}

void onMqttPublish(uint16_t packetId) {
  if (debugModeOn) printLogln("\n"+String(millis())+" - [onMqttPublish] - MQTT publish acknowledged. packetId="+String(packetId));
}

void mqttClientPublishHADiscovery(String mqttTopicName, String device, String ipAddress, bool removeTopics) {
  //Publish Home Assistant Discovery messages - v1.9
  struct tm nowTimeInfo; //36 B
  char s[100];
  getLocalTime(&nowTimeInfo);strftime(s,sizeof(s),"%Y-%m-%dT%H:%M:%S",&nowTimeInfo); //Time in format 2024-08-24T07:56:25
  String deviceSufix=device.substring(23); //Getting the MAC address part of device name
  String mqttSensorTopicHAPrefixName=String(MQTT_HA_SENSOR_TOPIC_PREFIX)+String("/"+device+"/"+deviceSufix);
  String mqttSensorTopicHAName;

  /*Sample variables
    samples["dateUpdate"] =  String(s);
    samples["startTime"] =  String(ss);
    samples["upTime"] =  String(upTime);
    samples["upTimeSeconds"] =  String(nowInSeconds);
    samples["device_name"] = device;
    samples["version"] = String(VERSION);
    samples["H2"] = String(h2_ppm);
    samples["LPG"] = String(lpg_ppm);
    samples["CH4"] = String(ch4_ppm);
    samples["CO"] = String(co_ppm);
    samples["ALCOHOL"] = String(alcohol_ppm);
    samples["Clean_air"] = String(gasClear);
    samples["GAS_interrupt"] = String(gasInterrupt);
    samples["Thermostate_interrupt"] = String(thermostateInterrupt);
    samples["Thermostate_status"] = String(thermostateStatus);
    samples["SSID"] = WiFi.SSID();
    samples["SIGNAL"] = String(wifiNet.RSSI);
    samples["RSSI"] = "50";
    samples["BSSID"] = WiFi.BSSIDstr();
    samples["ENCRYPTION"] = "WPA3";
    samples["CHANNEL"] = String(WiFi.channel());
    samples["MODE"] = "ACCESS POINT";
    samples["NTP"] = CloudClockCurrentStatus==CloudClockOffStatus?String("OFF"):String("ON");
    samples["HTTP_CLOUD"] = CloudSyncCurrentStatus==CloudSyncOffStatus?String("OFF"):String("ON");
    samples["Relay1"] = digitalRead(PIN_RL1)==0?String("R1_ON"):String("R1_OFF");
    samples["Relay2"] = digitalRead(PIN_RL2)==1?String("R2_ON"):String("R2_OFF");
    samples["tempSensor"] = String(tempSensor); //Non-calibrated temp
    samples["temperature"] = String(valueT);    //Calibrated temp
    samples["humidity"] =  String(valueHum);    //Non-calibrated = calibrated hum
  */

  //For every sample value, a Discovery Message must be published
  //HA Discovery topic format is as follows: <discovery_prefix>/<component>/[<node_id>]/<object_id>/config
  // In the followin exmaple:
  //  <discovery_prefix> = homeassistant
  //  <component> = sensor or switch (relay)
  //  [<node_id>] = boiler-relay-controlv2-E02940
  //  <object_id> = E02940_<SAMPLE_NAME> = E02940_H2, E02940_CO, etc.
  //i.e.: device=boiler-relay-controlv2-E02940
  //      deviceSufix=E02940
  //      mqttTopicName=the-iot-factory/boiler-relay-controlv2-E02940
  //      mqttSensorTopicHAPrefixName=homeassistant/sensor/boiler-relay-controlv2-E02940/E02940
  //      mqttSensorTopicHAName=homeassistant/sensor/boiler-relay-controlv2-E02940/E02940_<SAMPLE_NAME>/config
  //Sensors
  //mdi:gas-burner,air-filter, mdi:clock-time-eight \"ic\":\"mdi:calendar-clock\",

  //Device, Environment, GAS, Signals, System (Counters), WiFi
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_startTime/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //startTime value
    String("{\"name\":\"Device  : Boot Time\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_startTime\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"ic\":\"mdi:clock-time-five\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['startTime']}}\"}").c_str()); //Discovery message for startTime value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_dateUpdate/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //dateUpdate value
    String("{\"name\":\"Device  : Last Time\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_dateUpdate\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"ic\":\"mdi:clock-time-eight\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['dateUpdate']}}\"}").c_str()); //Discovery message for dateUpdate value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_upTimeSeconds/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //upTimeSeconds value
    String("{\"name\":\"Device  : upTime\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_upTimeSeconds\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['upTimeSeconds']}}\"}").c_str()); //Discovery message for upTimeSeconds value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_xupTimeSeconds/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Discovery message for upTimeSeconds value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_errorsWiFiCnt/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //errorsWiFiCnt  value
    String("{\"name\":\"Device Counter: errorsWiFi\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_errorsWiFiCnt\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['errorsWiFiCnt']}}\"}").c_str()); //Discovery message for errorsWiFiCnt value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_errorsConnectivityCnt/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //errorsConnectivityCnt  value
    String("{\"name\":\"Device Counter: errorsConnectivity\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_errorsConnectivityCnt\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['errorsConnectivityCnt']}}\"}").c_str()); //Discovery message for errorsConnectivityCnt value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_errorsNTPCnt/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //errorsNTPCnt  value
    String("{\"name\":\"Device Counter: errorsNTP\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_errorsNTPCnt\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['errorsNTPCnt']}}\"}").c_str()); //Discovery message for errorsNTPCnt value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_errorsHTTPUptsCnt/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //errorsHTTPUptsCnt  value
    String("{\"name\":\"Device Counter: errorsHTTPUpts\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_errorsHTTPUptsCnt\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['errorsHTTPUptsCnt']}}\"}").c_str()); //Discovery message for errorsHTTPUptsCnt value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_errorsMQTTCnt/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //errorsMQTTCnt  value
    String("{\"name\":\"Device Counter: errorsMQTT\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_errorsMQTTCnt\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['errorsMQTTCnt']}}\"}").c_str()); //Discovery message for errorsMQTTCnt value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_errorsWebServerCnt/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //errorsWebServerCnt  value
    String("{\"name\":\"Device Counter: errorsWebServerCnt\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_errorsWebServerCnt\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['errorsWebServerCnt']}}\"}").c_str()); //Discovery message for errorsWebServerCnt value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_bootCount/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //bootCount  value
    String("{\"name\":\"Device Boots:  since last update\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_bootCount\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['bootCount']}}\"}").c_str()); //Discovery message for bootCount value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_resetNormalCount/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //resetPreventiveCount  value
    String("{\"name\":\"Device Boots: normal resets\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_resetNormalCount\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['resetNormalCount']}}\"}").c_str()); //Discovery message for resetNormalCount value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_resetPreventiveCount/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //resetPreventiveCount  value
    String("{\"name\":\"Device Boots: preventive resets\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_resetPreventiveCount\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['resetPreventiveCount']}}\"}").c_str()); //Discovery message for resetPreventiveCount value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_resetPreventiveWebServerCount/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //resetPreventiveWebServerCount  value
    String("{\"name\":\"Device Boots: preventive  web server resets\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_resetPreventiveWebServerCount\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['resetPreventiveWebServerCount']}}\"}").c_str()); //Discovery message for resetPreventiveWebServerCount value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_resetSWCount/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //resetSWCount  value
    String("{\"name\":\"Device Boots: total sw resets\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_resetSWCount\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['resetSWCount']}}\"}").c_str()); //Discovery message for resetSWCount value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_resetSWWebCount/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //resetSWWebCount  value
    String("{\"name\":\"Device Boots: resets from web\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_resetSWWebCount\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['resetSWWebCount']}}\"}").c_str()); //Discovery message for resetSWWebCount value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_resetSWMqttCount/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //resetSWMqttCount  value
    String("{\"name\":\"Device Boots: resets from mqtt\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_resetSWMqttCount\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['resetSWMqttCount']}}\"}").c_str()); //Discovery message for resetSWMqttCount value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_resetSWUpgradeCount/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //resetSWUpgradeCount  value
    String("{\"name\":\"Device Boots: resets due to firmware update\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_resetSWUpgradeCount\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['resetSWUpgradeCount']}}\"}").c_str()); //Discovery message for resetSWUpgradeCount value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_resetWebServerCnt/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //resetWebServerCnt  value
    String("{\"name\":\"Device Boots: resets due to web server KO\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_resetWebServerCnt\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['resetWebServerCnt']}}\"}").c_str()); //Discovery message for resetWebServerCnt value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_resetCount/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //resetCount  value
    String("{\"name\":\"Device Boots: uncontrolled resets\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_resetCount\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['resetCount']}}\"}").c_str()); //Discovery message for resetCount value, not retain in the broker
  
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_lastHeap/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //lastHeap  value
    String("{\"name\":\"Device Heap: Heap size\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_lastHeap\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"dev_cla\":\"data_size\",\"unit_of_meas\":\"B\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['lastHeap']}}\"}").c_str()); //Discovery message for lastHeap value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_minHeapSinceBoot/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //minHeapSinceBoot  value
    String("{\"name\":\"Device Heap: Min heap since boot\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_minHeapSinceBoot\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"dev_cla\":\"data_size\",\"unit_of_meas\":\"B\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['minHeapSinceBoot']}}\"}").c_str()); //Discovery message for minHeapSinceBoot value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_minHeapSinceUpgrade/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //minHeapSinceUpgrade  value
    String("{\"name\":\"Device Heap: Min heap since upgrade\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_minHeapSinceUpgrade\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"dev_cla\":\"data_size\",\"unit_of_meas\":\"B\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['minHeapSinceUpgrade']}}\"}").c_str()); //Discovery message for minHeapSinceUpgrade value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_resetReason/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //bootCount  value
    String("{\"name\":\"Device Reset: reason\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_resetReason\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['resetReason']}}\"}").c_str()); //Discovery message for resetReason value, not retain in the broker
  
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_tempSensor/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //tempSensor value
    String("{\"name\":\"Environment: TempSensor\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_tempSensor\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"°C\",\"dev_cla\":\"temperature\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['tempSensor']}}\"}").c_str()); //Discovery message for tempSensor value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_Temperature/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //Temperature value
    String("{\"name\":\"Environment: Temperature\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_Temperature\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"°C\",\"dev_cla\":\"temperature\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['temperature']}}\"}").c_str()); //Discovery message for Temperature value, not retain in the broker  
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_Humidity/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //Humidity value
    String("{\"name\":\"Environment: Humidity\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_Humidity\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"%\",\"dev_cla\":\"humidity\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['humidity']}}\"}").c_str()); //Discovery message for Humidity value, not retain in the broker
    
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_ALCOHOL/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //H2 value
    String("{\"name\":\"GAS: ALCOHOL\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_ALCOHOL\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"ppm\",\"ic\":\"mdi:fire\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['ALCOHOL']}}\"}").c_str()); //Discovery message for ALCOHOL value, not retain in the broker 
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_CH4/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //H2 value
    String("{\"name\":\"GAS: CH4\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_CH4\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"ppm\",\"ic\":\"mdi:fire\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['CH4']}}\"}").c_str()); //Discovery message for CH4 value, not retain in the broker  
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_CO/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //H2 value
    String("{\"name\":\"GAS: CO\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_CO\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"ppm\",\"ic\":\"mdi:fire\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['CO']}}\"}").c_str()); //Discovery message for CO value, not retain in the broker  
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_H2/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //H2 value
    String("{\"name\":\"GAS: H2\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_H2\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"ppm\",\"ic\":\"mdi:fire\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['H2']}}\"}").c_str()); //Discovery message for H2 value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_LPG/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //H2 value
    String("{\"name\":\"GAS: LPG\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_LPG\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"ppm\",\"ic\":\"mdi:fire\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['LPG']}}\"}").c_str()); //Discovery message for LPG value, not retain in the broker  
  
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerPreviousYear/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else {
    if (updateHADiscovery) {mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false);} //Send topic with no payload to publish a new year in the name
    mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnYear  value
      String("{\"name\":\"Boiler Previous Year: \",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerPreviousYear\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerPreviousYear']}}\"}").c_str()); //Discovery message for boilerPreviousYear value, not retain in the broker
  }
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnPreviousYear/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else {
    if (updateHADiscovery) {mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false);} //Send topic with no payload to publish a new year in the name
    mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnPreviousYear  value
      String("{\"name\":\"Time Boiler Previous Year:      \",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnPreviousYear\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYear']}}\"}").c_str()); //Discovery message for boilerOnPreviousYear value, not retain in the broker
  }

  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnPreviousYearJan/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnPreviousYearJan  value
    String("{\"name\":\"Time Boiler:     Month 01\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnPreviousYearJan\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearJan']}}\"}").c_str()); //Discovery message for boilerOnPreviousYearJan value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnPreviousYearFeb/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnPreviousYearFeb  value
    String("{\"name\":\"Time Boiler:     Month 02\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnPreviousYearFeb\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearFeb']}}\"}").c_str()); //Discovery message for boilerOnPreviousYearFeb value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnPreviousYearMar/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnPreviousYearMar  value
    String("{\"name\":\"Time Boiler:     Month 03\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnPreviousYearMar\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearMar']}}\"}").c_str()); //Discovery message for boilerOnPreviousYearMar value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnPreviousYearApr/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnPreviousYearApr  value
    String("{\"name\":\"Time Boiler:     Month 04\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnPreviousYearApr\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearApr']}}\"}").c_str()); //Discovery message for boilerOnPreviousYearApr value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnPreviousYearMay/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnPreviousYearMay  value
    String("{\"name\":\"Time Boiler:     Month 05\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnPreviousYearMay\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearMay']}}\"}").c_str()); //Discovery message for boilerOnPreviousYearMay value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnPreviousYearJun/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnPreviousYearJun  value
    String("{\"name\":\"Time Boiler:     Month 06\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnPreviousYearJun\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearJun']}}\"}").c_str()); //Discovery message for boilerOnPreviousYearJun value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnPreviousYearJul/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnPreviousYearJul  value
    String("{\"name\":\"Time Boiler:     Month 07\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnPreviousYearJul\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearJul']}}\"}").c_str()); //Discovery message for boilerOnPreviousYearJul value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnPreviousYearAug/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnPreviousYearAug  value
    String("{\"name\":\"Time Boiler:     Month 08\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnPreviousYearAug\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearAug']}}\"}").c_str()); //Discovery message for boilerOnPreviousYearAug value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnPreviousYearSep/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnPreviousYearSep  value
    String("{\"name\":\"Time Boiler:     Month 09\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnPreviousYearSep\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearSep']}}\"}").c_str()); //Discovery message for boilerOnPreviousYearSep value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnPreviousYearOct/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnPreviousYearOct  value
    String("{\"name\":\"Time Boiler:     Month 10\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnPreviousYearOct\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearOct']}}\"}").c_str()); //Discovery message for boilerOnPreviousYearOct value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnPreviousYearNov/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnPreviousYearNov  value
    String("{\"name\":\"Time Boiler:     Month 11\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnPreviousYearNov\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearNov']}}\"}").c_str()); //Discovery message for boilerOnPreviousYearNov value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnPreviousYearDec/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnPreviousYearDec  value
    String("{\"name\":\"Time Boiler:     Month 12\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnPreviousYearDec\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearDec']}}\"}").c_str()); //Discovery message for boilerOnPreviousYearDec value, not retain in the broker

  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerYear/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else {
    if (updateHADiscovery) {mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false);} //Send topic with no payload to publish a new year in the name
    mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnYear  value
      String("{\"name\":\"Boiler Current Year: \",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerYear\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerYear']}}\"}").c_str()); //Discovery message for boilerYear value, not retain in the broker
  }
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnYear/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else {
    if (updateHADiscovery) {mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false);} //Send topic with no payload to publish a new year in the name
    mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnYear  value
      String("{\"name\":\"Time Boiler Current Year:      \",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnYear\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYear']}}\"}").c_str()); //Discovery message for boilerOnYear value, not retain in the broker
  }
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnYearJan/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnYearJan  value
    String("{\"name\":\"Time Boiler:   Month 01\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnYearJan\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearJan']}}\"}").c_str()); //Discovery message for boilerOnYearJan value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnYearFeb/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnYearFeb  value
    String("{\"name\":\"Time Boiler:   Month 02\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnYearFeb\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearFeb']}}\"}").c_str()); //Discovery message for boilerOnYearFeb value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnYearMar/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnYearMar  value
    String("{\"name\":\"Time Boiler:   Month 03\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnYearMar\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearMar']}}\"}").c_str()); //Discovery message for boilerOnYearMar value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnYearApr/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnYearApr  value
    String("{\"name\":\"Time Boiler:   Month 04\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnYearApr\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearApr']}}\"}").c_str()); //Discovery message for boilerOnYearApr value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnYearMay/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnYearMay  value
    String("{\"name\":\"Time Boiler:   Month 05\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnYearMay\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearMay']}}\"}").c_str()); //Discovery message for boilerOnYearMay value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnYearJun/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnYearJun  value
    String("{\"name\":\"Time Boiler:   Month 06\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnYearJun\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearJun']}}\"}").c_str()); //Discovery message for boilerOnYearJun value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnYearJul/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnYearJul  value
    String("{\"name\":\"Time Boiler:   Month 07\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnYearJul\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearJul']}}\"}").c_str()); //Discovery message for boilerOnYearJul value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnYearAug/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnYearAug  value
    String("{\"name\":\"Time Boiler:   Month 08\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnYearAug\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearAug']}}\"}").c_str()); //Discovery message for boilerOnYearAug value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnYearSep/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnYearSep  value
    String("{\"name\":\"Time Boiler:   Month 09\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnYearSep\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearSep']}}\"}").c_str()); //Discovery message for boilerOnYearSep value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnYearOct/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnYearOct  value
    String("{\"name\":\"Time Boiler:   Month 10\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnYearOct\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearOct']}}\"}").c_str()); //Discovery message for boilerOnYearOct value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnYearNov/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnYearNov  value
    String("{\"name\":\"Time Boiler:   Month 11\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnYearNov\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearNov']}}\"}").c_str()); //Discovery message for boilerOnYearNov value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnYearDec/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnYearDec  value
    String("{\"name\":\"Time Boiler:   Month 12\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnYearDec\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearDec']}}\"}").c_str()); //Discovery message for boilerOnYearDec value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnYesterday/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnYesterday  value
    String("{\"name\":\"Time Boiler:  Yesterday\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnYesterday\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYesterday']}}\"}").c_str()); //Discovery message for boilerOnYesterday value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_boilerOnToday/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnToday  value
    String("{\"name\":\"Time Boiler: Today\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_boilerOnToday\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnToday']}}\"}").c_str()); //Discovery message for boilerOnToday value, not retain in the broker
  
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterPreviousYear/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else {
    if (updateHADiscovery) {mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false);} //Send topic with no payload to publish a new year in the name
    mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnYear  value
      String("{\"name\":\"Heater Previous Year: \",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterPreviousYear\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterPreviousYear']}}\"}").c_str()); //Discovery message for heaterPreviousYear value, not retain in the broker
  }
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnPreviousYear/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else {
    if (updateHADiscovery) {mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false);} //Send topic with no payload to publish a new year in the name
    mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnPreviousYear  value
      String("{\"name\":\"Time Heater Previous Year:      \",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnPreviousYear\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYear']}}\"}").c_str()); //Discovery message for heaterOnPreviousYear value, not retain in the broker
  }
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnPreviousYearJan/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnPreviousYearJan  value
    String("{\"name\":\"Time Heater:     Month 01\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnPreviousYearJan\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearJan']}}\"}").c_str()); //Discovery message for heaterOnPreviousYearJan value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnPreviousYearFeb/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnPreviousYearFeb  value
    String("{\"name\":\"Time Heater:     Month 02\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnPreviousYearFeb\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearFeb']}}\"}").c_str()); //Discovery message for heaterOnPreviousYearFeb value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnPreviousYearMar/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnPreviousYearMar  value
    String("{\"name\":\"Time Heater:     Month 03\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnPreviousYearMar\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearMar']}}\"}").c_str()); //Discovery message for heaterOnPreviousYearMar value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnPreviousYearApr/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnPreviousYearApr  value
    String("{\"name\":\"Time Heater:     Month 04\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnPreviousYearApr\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearApr']}}\"}").c_str()); //Discovery message for heaterOnPreviousYearApr value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnPreviousYearMay/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnPreviousYearMay  value
    String("{\"name\":\"Time Heater:     Month 05\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnPreviousYearMay\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearMay']}}\"}").c_str()); //Discovery message for heaterOnPreviousYearMay value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnPreviousYearJun/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnPreviousYearJun  value
    String("{\"name\":\"Time Heater:     Month 06\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnPreviousYearJun\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearJun']}}\"}").c_str()); //Discovery message for heaterOnPreviousYearJun value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnPreviousYearJul/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnPreviousYearJul  value
    String("{\"name\":\"Time Heater:     Month 07\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnPreviousYearJul\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearJul']}}\"}").c_str()); //Discovery message for heaterOnPreviousYearJul value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnPreviousYearAug/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnPreviousYearAug  value
    String("{\"name\":\"Time Heater:     Month 08\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnPreviousYearAug\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearAug']}}\"}").c_str()); //Discovery message for heaterOnPreviousYearAug value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnPreviousYearSep/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnPreviousYearSep  value
    String("{\"name\":\"Time Heater:     Month 09\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnPreviousYearSep\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearSep']}}\"}").c_str()); //Discovery message for heaterOnPreviousYearSep value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnPreviousYearOct/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnPreviousYearOct  value
    String("{\"name\":\"Time Heater:     Month 10\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnPreviousYearOct\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearOct']}}\"}").c_str()); //Discovery message for heaterOnPreviousYearOct value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnPreviousYearNov/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnPreviousYearNov  value
    String("{\"name\":\"Time Heater:     Month 11\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnPreviousYearNov\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearNov']}}\"}").c_str()); //Discovery message for heaterOnPreviousYearNov value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnPreviousYearDec/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnPreviousYearDec  value
    String("{\"name\":\"Time Heater:     Month 12\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnPreviousYearDec\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearDec']}}\"}").c_str()); //Discovery message for heaterOnPreviousYearDec value, not retain in the broker

  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterYear/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else {
    if (updateHADiscovery) {mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false);} //Send topic with no payload to publish a new year in the name
    mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //boilerOnYear  value
      String("{\"name\":\"Heater Current Year: \",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterYear\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterYear']}}\"}").c_str()); //Discovery message for heaterYear value, not retain in the broker
  }
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnYear/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else {
    if (updateHADiscovery) {mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false);} //Send topic with no payload to publish a new year in the name
    mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnYear  value
      String("{\"name\":\"Time Heater Current Year:      \",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnYear\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYear']}}\"}").c_str()); //Discovery message for heaterOnYear value, not retain in the broker
  }
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnYearJan/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnYearJan  value
    String("{\"name\":\"Time Heater:   Month 01\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnYearJan\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearJan']}}\"}").c_str()); //Discovery message for heaterOnYearJan value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnYearFeb/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnYearFeb  value
    String("{\"name\":\"Time Heater:   Month 02\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnYearFeb\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearFeb']}}\"}").c_str()); //Discovery message for heaterOnYearFeb value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnYearMar/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnYearMar  value
    String("{\"name\":\"Time Heater:   Month 03\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnYearMar\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearMar']}}\"}").c_str()); //Discovery message for heaterOnYearMar value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnYearApr/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnYearApr  value
    String("{\"name\":\"Time Heater:   Month 04\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnYearApr\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearApr']}}\"}").c_str()); //Discovery message for heaterOnYearApr value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnYearMay/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnYearMay  value
    String("{\"name\":\"Time Heater:   Month 05\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnYearMay\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearMay']}}\"}").c_str()); //Discovery message for heaterOnYearMay value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnYearJun/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnYearJun  value
    String("{\"name\":\"Time Heater:   Month 06\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnYearJun\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearJun']}}\"}").c_str()); //Discovery message for heaterOnYearJun value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnYearJul/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnYearJul  value
    String("{\"name\":\"Time Heater:   Month 07\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnYearJul\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearJul']}}\"}").c_str()); //Discovery message for heaterOnYearJul value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnYearAug/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnYearAug  value
    String("{\"name\":\"Time Heater:   Month 08\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnYearAug\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearAug']}}\"}").c_str()); //Discovery message for heaterOnYearAug value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnYearSep/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnYearSep  value
    String("{\"name\":\"Time Heater:   Month 09\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnYearSep\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearSep']}}\"}").c_str()); //Discovery message for heaterOnYearSep value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnYearOct/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnYearOct  value
    String("{\"name\":\"Time Heater:   Month 10\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnYearOct\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearOct']}}\"}").c_str()); //Discovery message for heaterOnYearOct value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnYearNov/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnYearNov  value
    String("{\"name\":\"Time Heater:   Month 11\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnYearNov\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearNov']}}\"}").c_str()); //Discovery message for heaterOnYearNov value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnYearDec/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnYearDec  value
    String("{\"name\":\"Time Heater:   Month 12\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnYearDec\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearDec']}}\"}").c_str()); //Discovery message for heaterOnYearDec value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnYesterday/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnYesterday  value
    String("{\"name\":\"Time Heater:  Yesterday\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnYesterday\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYesterday']}}\"}").c_str()); //Discovery message for heaterOnYesterday value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_heaterOnToday/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //heaterOnToday  value
    String("{\"name\":\"Time Heater: Today\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_heaterOnToday\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnToday']}}\"}").c_str()); //Discovery message for heaterOnToday value, not retain in the broker

  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_BSSID/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //BSSID value
    String("{\"name\":\"WiFi: BSSID\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_BSSID\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"ic\":\"mdi:radio-tower\",\"val_tpl\":\"{{value_json['SAMPLES']['BSSID']}}\"}").c_str()); //Discovery message for BSSID value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_RSSI/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //RSSI value
    String("{\"name\":\"WiFi: RSSI\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_RSSI\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"%\",\"frc_upd\":true,\"ic\":\""+iconWifi+"\",\"val_tpl\":\"{{value_json['SAMPLES']['RSSI']}}\"}").c_str()); //Discovery message for RSSI value, not retain in the broker
    mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_SIGNAL/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //SIGNAL value
    String("{\"name\":\"WiFi: SIGNAL\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_SIGNAL\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"dBm\",\"dev_cla\":\"signal_strength\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['SIGNAL']}}\"}").c_str()); //Discovery message for SIGNAL value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_SSID/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //SSID value
    String("{\"name\":\"WiFi: SSID\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_SSID\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"ic\":\"mdi:access-point\",\"val_tpl\":\"{{value_json['SAMPLES']['SSID']}}\"}").c_str()); //Discovery message for SSID value, not retain in the broker
 
  
  //Binary Sensors
  //i.e.: device=boiler-relay-controlv2-E02940
  //      deviceSufix=E02940
  //      mqttTopicName=the-iot-factory/boiler-relay-controlv2-E02940
  //      mqttSensorTopicHAPrefixName=homeassistant/binary_sensor/boiler-relay-controlv2-E02940/E02940
  //      mqttSensorTopicHAName=homeassistant/binary_sensor/boiler-relay-controlv2-E02940/E02940_<SAMPLE_NAME>/config
  mqttSensorTopicHAPrefixName=String(MQTT_HA_BINARY_SENSOR_TOPIC_PREFIX)+String("/"+device+"/"+deviceSufix);
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_Clean_air/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //Clean_air value
    String("{\"name\":\"Environment: Gas Detection\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_Clean_air\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"dev_cla\":\"gas\",\"ic\":\"mdi:meter-gas\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['Clean_air']}}\"}").c_str()); //Discovery message for Clean_air value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_Thermostate_status/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //Thermostate_status value
    String("{\"name\":\"Environment: Heater Active\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_Thermostate_status\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"ic\":\""+iconThermStatus+"\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['Thermostate_status']}}\"}").c_str()); //Discovery message for Thermostate_status value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_GAS_interrupt/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //GAS_interrupt value
    String("{\"name\":\"Signal: Gas Sensor\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_GAS_interrupt\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"ic\":\""+iconGasInterrupt+"\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['GAS_interrupt']}}\"}").c_str()); //Discovery message for GAS_interrupt value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_Thermostate_interrupt/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //Thermostate_interrupt value
    String("{\"name\":\"Signal: Therm. Sensor\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_Thermostate_interrupt\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"ic\":\""+iconThermInterrupt+"\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['Thermostate_interrupt']}}\"}").c_str()); //Discovery message for Thermostate_interrupt value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_NTP/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //NTP Sync value
    String("{\"name\":\"WiFi: Sync NTP\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_NTP\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"ic\":\"mdi:cloud-clock\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['NTP']}}\"}").c_str()); //Discovery message for NTP sync value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_HTTP_CLOUD/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //HTTP_CLOUD Sync value
    String("{\"name\":\"WiFi: Sync HTTP Cloud\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_HTTP_CLOUD\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"ic\":\"mdi:cloud-upload\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['HTTP_CLOUD']}}\"}").c_str()); //Discovery message for HTTP_CLOUD sync value, not retain in the broker

  //Relays
  //i.e.: device=boiler-relay-controlv2-E02940
  //      deviceSufix=E02940
  //      mqttTopicName=the-iot-factory/boiler-relay-controlv2-E02940
  //      mqttSensorTopicHAPrefixName=homeassistant/switch/boiler-relay-controlv2-E02940/E02940
  //      mqttSensorTopicHAName=homeassistant/switch/boiler-relay-controlv2-E02940/E02940_<SAMPLE_NAME>/config
  mqttSensorTopicHAPrefixName=String(MQTT_HA_SWITCH_TOPIC_PREFIX)+String("/"+device+"/"+deviceSufix);
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_Relay1/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //Relay1 value
    String("{\"name\":\"Relay: Allow Ext. Therm.\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\""+mqttTopicName+"/cmnd/RELAY\",\"pl_off\":\"R1_OFF\",\"pl_on\":\"R1_ON\",\"uniq_id\":\""+deviceSufix+"_Relay1\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['Relay1']}}\"}").c_str()); //Discovery message for Relay1 value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_Relay2/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //Relay2 value
    String("{\"name\":\"Relay: Force Heater\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\""+mqttTopicName+"/cmnd/RELAY\",\"pl_off\":\"R2_OFF\",\"pl_on\":\"R2_ON\",\"uniq_id\":\""+deviceSufix+"_Relay2\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['Relay2']}}\"}").c_str()); //Discovery message for Relay2 value, not retain in the broker
  
  //Buttons
  //i.e.: device=boiler-relay-controlv2-E02940
  //      deviceSufix=E02940
  //      mqttTopicName=the-iot-factory/boiler-relay-controlv2-E02940
  //      mqttSensorTopicHAPrefixName=homeassistant/button/boiler-relay-controlv2-E02940/E02940
  //      mqttSensorTopicHAName=homeassistant/button/boiler-relay-controlv2-E02940/E02940_<SAMPLE_NAME>/config
  mqttSensorTopicHAPrefixName=String(MQTT_HA_BUTTON_TOPIC_PREFIX)+String("/"+device+"/"+deviceSufix);
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_reboot/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //reboot value
    String("{\"name\":\"Device  : Reboot\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\""+mqttTopicName+"/cmnd/RELAY\",\"payload_press\":\"REBOOT\",\"uniq_id\":\""+deviceSufix+"_reboot\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"dev_cla\":\"restart\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['reboot']}}\"}").c_str()); //Discovery message for Reboot value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_reset_time_counters/config";
  if (removeTopics) mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false); //Send topic with no payload
  else mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //reset_time_counters value
    String("{\"name\":\"Device  : Reset Time Counters\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\""+mqttTopicName+"/cmnd/RELAY\",\"payload_press\":\"RESET_TIME_COUNTERS\",\"uniq_id\":\""+deviceSufix+"_reset_time_counters\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"dev_cla\":\"restart\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['reset_time_counters']}}\"}").c_str()); //Discovery message for Reboot value, not retain in the broker

  if (removeTopics) {
    if (debugModeOn) {printLogln(String(millis())+" - [mqttClientPublishHADiscovery] - Home Assistant Discovery messages published to remove all the topics");}
    removeTopics=false;
  }
  else {
    if (debugModeOn) {printLogln(String(millis())+" - [mqttClientPublishHADiscovery] - Home Assistant Discovery messages published to publish all the topics");}
    mqttClient.publish(String(mqttTopicName+"/LWT").c_str(), 0, false, "Online\0"); //Availability message, not retain in the broker
    if (debugModeOn) {printLogln(String(millis())+" - [mqttClientPublishHADiscovery] - Availability message published, mqttTopicName="+mqttTopicName+"/LWT");}
  }
}