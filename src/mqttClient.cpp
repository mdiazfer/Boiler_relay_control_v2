#include "mqttClient.h"


void onMqttConnect(bool sessionPresent) {
  String packetInfoId=String(mqttClient.subscribe(MQTT_TOPIC_SUBSCRIPTION,0));
  String packetStatId=String(mqttClient.subscribe(MQTT_HA_B_AND_LWT_TOPIC_PREFIX,0));
  //Subcribe to the-iot-factory/boiler-relay-controlv2-E02940/cmnd/RELAY
  String cmdSubcriptionTopic=String(MQTT_TOPIC_PREFIX+device+"/"+MQTT_TOPIC_CMD_SUFIX_SUBSCRIPTION);
  String packetSwitchId=String(mqttClient.subscribe(cmdSubcriptionTopic.c_str(),0));

  if (debugModeOn) boardSerialPort.println("\n"+String(millis())+" - [onMqttConnect] - MQTT connected to "+mqttServer+". Session present: "+String(sessionPresent)+
                        "\n  [onMqttConnect] - Subscribing on:"+
                        "\n  [onMqttConnect] - topic "+MQTT_TOPIC_SUBSCRIPTION+", QoS 0, packetId="+packetInfoId+
                        "\n  [onMqttConnect] - topic "+MQTT_HA_B_AND_LWT_TOPIC_PREFIX+", QoS 0, packetId="+packetStatId+ // v1.9.0 - Home Assistant Last Will Testament message (offline)
                        "\n  [onMqttConnect] - topic "+cmdSubcriptionTopic+", QoS 0, packetId="+packetSwitchId); // Relay ON/OFF
  MqttSyncCurrentStatus=MqttSyncOnStatus;
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  if (debugModeOn) boardSerialPort.println("\n"+String(millis())+" - [onMqttDisconnect] - MQTT disconnected, reason="+(uint8_t)reason);
  MqttSyncCurrentStatus=MqttSyncOffStatus;
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  if (debugModeOn) boardSerialPort.println("\n"+String(millis())+" - [onMqttSubscribe] - MQTT subscribe acknowledged. packetId="+String(packetId)+", qos="+String(qos));
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  char aux[len+1];strcpy(aux,payload); aux[len]='\0';

  if (debugModeOn) boardSerialPort.println("\n"+String(millis())+" - [onMqttMessage] - MQTT published message received on topic='"+String(topic)+"', mesage: '"+String(aux)+"', index: "+String(index));
  
  if (String(topic) == String(MQTT_TOPIC_SUBSCRIPTION)) {
    //Pubish device name
    mqttClient.publish(String(mqttTopicPrefix+"device-name").c_str(), 0, false, String("{\"manufacturer\":\"www.the-iotfactory.com\",\"model\":\"boiler-relay-controlv2\",\"name\":\""+device+"\"}").c_str());
  } else if (String(topic) == String(MQTT_HA_B_AND_LWT_TOPIC_PREFIX)) {
    //Home Assistant server published an availability message. Check on it.
    if ((String(aux).equalsIgnoreCase(String("online"))) && mqttServerEnabled) { //Publish the Discovery message for Home Assistan to detect this device
      if (debugModeOn) boardSerialPort.println(String(millis())+" - [onMqttMessage] - Topic received '"+String(topic)+"' with message: '"+String(aux)+"'. Home Assistant server available. Publish the Discovery message now.");
      mqttClientPublishHADiscovery(mqttTopicPrefix+device,device,WiFi.localIP().toString());
    }
  } else if (String(topic) == String(MQTT_TOPIC_PREFIX+device+"/"+MQTT_TOPIC_CMD_SUFIX_SUBSCRIPTION)) {
    if (debugModeOn) boardSerialPort.println(String(millis())+" - [onMqttMessage] - Command received: "+String(aux));
    if (gasClear) {
      if (String(aux).equalsIgnoreCase(String("R1_ON"))) {
        digitalWrite(PIN_RL1,LOW);samples["Relay1"] = String("R1_ON");
        boardSerialPort.println(String(millis())+" - [onMqttMessage] - Set Relay1 OFF"); //Relay1 is set off to allow Ext. Thermostat (R1_ON)
        forceMQTTpublish=true; //Force to publish the MQTT message from the loop
      }
      else if (String(aux).equalsIgnoreCase(String("R1_OFF"))) {
        digitalWrite(PIN_RL1,HIGH);samples["Relay1"] = String("R1_OFF");
        boardSerialPort.println(String(millis())+" - [onMqttMessage] - Set Relay1 ON"); //Relay1 is set on to not allow Ext. Thermostat (R1_OFF)
        forceMQTTpublish=true; //Force to publish the MQTT message from the loop
      }
      else if (String(aux).equalsIgnoreCase(String("R2_ON"))) {
        digitalWrite(PIN_RL2,HIGH);samples["Relay2"] = String("R2_ON");
        digitalWrite(PIN_RL1,LOW);samples["Relay1"] = String("R1_ON"); //Relay1 is set off to allow Ext. Thermostat (R1_ON) when the Relay2 is set
        boardSerialPort.println(String(millis())+" - [onMqttMessage] - Set Relay1 OFF"); //Relay1 is set off to allow Ext. Thermostat (R1_ON)
        boardSerialPort.println(String(millis())+" - [onMqttMessage] - Set Relay2 ON"); //Relay2 is set on to shortcut Ext. Thermostat (R2_ON)
        forceMQTTpublish=true; //Force to publish the MQTT message from the loop
      }
      else if (String(aux).equalsIgnoreCase(String("R2_OFF"))) {
        digitalWrite(PIN_RL2,LOW);samples["Relay2"] = String("R2_OFF");
        boardSerialPort.println(String(millis())+" - [onMqttMessage] - Set Relay2 OFF"); //Relay2 is set off to not shortcut Ext. Thermostat (R2_OFF)
        forceMQTTpublish=true; //Force to publish the MQTT message from the loop
      }
      else boardSerialPort.println(String(millis())+" - [onMqttMessage] - Unknown command");
    }
    else boardSerialPort.println(String(millis())+" - [onMqttMessage] - Gas leak situation detected, so no releay activation is allowed for security reasons");
  }
  else {
    //Do nothing for other topics
    if (debugModeOn) boardSerialPort.println(String(millis())+" - [onMqttMessage] - Topic received ("+String(topic)+") and differs from subscriptions: '"+String(MQTT_TOPIC_SUBSCRIPTION)+"', '"+String(MQTT_HA_B_AND_LWT_TOPIC_PREFIX)+"'. Return");
  }
}

void onMqttUnsubscribe(uint16_t packetId) {
  if (debugModeOn) boardSerialPort.println("\n"+String(millis())+" - [onMqttUnsubscribe] - MQTT unsubscribe acknowledged. packetId="+String(packetId));
}

void onMqttPublish(uint16_t packetId) {
  if (debugModeOn) boardSerialPort.println("\n"+String(millis())+" - [onMqttPublish] - MQTT publish acknowledged. packetId="+String(packetId));
}

void mqttClientPublishHADiscovery(String mqttTopicName, String device, String ipAddress) {
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
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_startTime/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //startTime value
    String("{\"name\":\"Boot Time\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_startTime\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"ic\":\"mdi:clock-time-five\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['startTime']}}\"}").c_str()); //Discovery message for startTime value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_upTimeSeconds/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //upTimeSeconds value
    String("{\"name\":\"upTime\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_upTimeSeconds\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['upTimeSeconds']}}\"}").c_str()); //Discovery message for upTimeSeconds value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_H2/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //H2 value
    String("{\"name\":\"H2\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_H2\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"ppm\",\"ic\":\"mdi:fire\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['H2']}}\"}").c_str()); //Discovery message for H2 value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_LPG/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //H2 value
    String("{\"name\":\"LPG\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_LPG\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"ppm\",\"ic\":\"mdi:fire\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['LPG']}}\"}").c_str()); //Discovery message for LPG value, not retain in the broker  
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_CH4/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //H2 value
    String("{\"name\":\"CH4\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_CH4\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"ppm\",\"ic\":\"mdi:fire\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['CH4']}}\"}").c_str()); //Discovery message for CH4 value, not retain in the broker  
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_CO/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //H2 value
    String("{\"name\":\"CO\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_CO\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"ppm\",\"ic\":\"mdi:fire\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['CO']}}\"}").c_str()); //Discovery message for CO value, not retain in the broker  
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_ALCOHOL/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //H2 value
    String("{\"name\":\"ALCOHOL\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_ALCOHOL\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"ppm\",\"ic\":\"mdi:fire\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['ALCOHOL']}}\"}").c_str()); //Discovery message for ALCOHOL value, not retain in the broker 
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_SSID/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //SSID value
    String("{\"name\":\"SSID\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_SSID\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"ic\":\"mdi:access-point\",\"val_tpl\":\"{{value_json['SAMPLES']['SSID']}}\"}").c_str()); //Discovery message for SSID value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_BSSID/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //BSSID value
    String("{\"name\":\"BSSID\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_BSSID\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"ic\":\"mdi:radio-tower\",\"val_tpl\":\"{{value_json['SAMPLES']['BSSID']}}\"}").c_str()); //Discovery message for BSSID value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_SIGNAL/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //SIGNAL value
    String("{\"name\":\"SIGNAL\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_SIGNAL\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"dBm\",\"dev_cla\":\"signal_strength\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['SIGNAL']}}\"}").c_str()); //Discovery message for SIGNAL value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_RSSI/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //RSSI value
    String("{\"name\":\"RSSI\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_RSSI\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"%\",\"frc_upd\":true,\"ic\":\""+iconWifi+"\",\"val_tpl\":\"{{value_json['SAMPLES']['RSSI']}}\"}").c_str()); //Discovery message for RSSI value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_SIGNAL/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //SIGNAL value
    String("{\"name\":\"SIGNAL\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_SIGNAL\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"dBm\",\"dev_cla\":\"signal_strength\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['SIGNAL']}}\"}").c_str()); //Discovery message for SIGNAL value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_tempSensor/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //tempSensor value
    String("{\"name\":\"TempSensor\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_tempSensor\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"°C\",\"dev_cla\":\"temperature\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['tempSensor']}}\"}").c_str()); //Discovery message for tempSensor value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_Temperature/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //Temperature value
    String("{\"name\":\"Temperature\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_Temperature\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"°C\",\"dev_cla\":\"temperature\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['temperature']}}\"}").c_str()); //Discovery message for Temperature value, not retain in the broker  
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_Humidity/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //Humidity value
    String("{\"name\":\"Humidity\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_Humidity\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"unit_of_meas\":\"%\",\"dev_cla\":\"humidity\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['humidity']}}\"}").c_str()); //Discovery message for Humidity value, not retain in the broker

  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_errorsWiFiCnt/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //errorsWiFiCnt  value
    String("{\"name\":\"Counter: errorsWiFi\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_errorsWiFiCnt\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['errorsWiFiCnt']}}\"}").c_str()); //Discovery message for errorsWiFiCnt value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_errorsNTPCnt/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //errorsNTPCnt  value
    String("{\"name\":\"Counter: errorsNTP\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_errorsNTPCnt\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['errorsNTPCnt']}}\"}").c_str()); //Discovery message for errorsNTPCnt value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_errorsHTTPUptsCnt/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //errorsHTTPUptsCnt  value
    String("{\"name\":\"Counter: errorsHTTPUpts\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_errorsHTTPUptsCnt\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['errorsHTTPUptsCnt']}}\"}").c_str()); //Discovery message for errorsHTTPUptsCnt value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_errorsMQTTCnt/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //errorsMQTTCnt  value
    String("{\"name\":\"Counter: errorsMQTT\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_errorsMQTTCnt\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['errorsMQTTCnt']}}\"}").c_str()); //Discovery message for errorsMQTTCnt value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_bootCount/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //bootCount  value
    String("{\"name\":\"Counter: boots since last update\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_bootCount\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['bootCount']}}\"}").c_str()); //Discovery message for bootCount value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_resetCount/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //resetCount  value
    String("{\"name\":\"Counter: uncontrolled resets\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_resetCount\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['resetCount']}}\"}").c_str()); //Discovery message for resetCount value, not retain in the broker
  
  
  //Binary Sensors
  //i.e.: device=boiler-relay-controlv2-E02940
  //      deviceSufix=E02940
  //      mqttTopicName=the-iot-factory/boiler-relay-controlv2-E02940
  //      mqttSensorTopicHAPrefixName=homeassistant/binary_sensor/boiler-relay-controlv2-E02940/E02940
  //      mqttSensorTopicHAName=homeassistant/binary_sensor/boiler-relay-controlv2-E02940/E02940_<SAMPLE_NAME>/config
  mqttSensorTopicHAPrefixName=String(MQTT_HA_BINARY_SENSOR_TOPIC_PREFIX)+String("/"+device+"/"+deviceSufix);
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_Clean_air/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //Clean_air value
    String("{\"name\":\"Gas Detection\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_Clean_air\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"dev_cla\":\"gas\",\"ic\":\"mdi:meter-gas\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['Clean_air']}}\"}").c_str()); //Discovery message for Clean_air value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_GAS_interrupt/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //GAS_interrupt value
    String("{\"name\":\"Gas Sensor\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_GAS_interrupt\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"ic\":\""+iconGasInterrupt+"\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['GAS_interrupt']}}\"}").c_str()); //Discovery message for GAS_interrupt value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_Thermostate_interrupt/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //Thermostate_interrupt value
    String("{\"name\":\"Therm. Sensor\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_Thermostate_interrupt\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"ic\":\""+iconThermInterrupt+"\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['Thermostate_interrupt']}}\"}").c_str()); //Discovery message for Thermostate_interrupt value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_Thermostate_status/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //Thermostate_status value
    String("{\"name\":\"Heater Active\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_Thermostate_status\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"ic\":\""+iconThermStatus+"\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['Thermostate_status']}}\"}").c_str()); //Discovery message for Thermostate_status value, not retain in the broker
  
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_NTP/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //NTP Sync value
    String("{\"name\":\"NTP Sync\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_NTP\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"ic\":\"mdi:cloud-clock\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['NTP']}}\"}").c_str()); //Discovery message for NTP sync value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_HTTP_CLOUD/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //HTTP_CLOUD Sync value
    String("{\"name\":\"HTTP Cloud Sync\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\""+deviceSufix+"_HTTP_CLOUD\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"ic\":\"mdi:cloud-upload\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['HTTP_CLOUD']}}\"}").c_str()); //Discovery message for HTTP_CLOUD sync value, not retain in the broker

  //Relays
  //i.e.: device=boiler-relay-controlv2-E02940
  //      deviceSufix=E02940
  //      mqttTopicName=the-iot-factory/boiler-relay-controlv2-E02940
  //      mqttSensorTopicHAPrefixName=homeassistant/switch/boiler-relay-controlv2-E02940/E02940
  //      mqttSensorTopicHAName=homeassistant/switch/boiler-relay-controlv2-E02940/E02940_<SAMPLE_NAME>/config
  mqttSensorTopicHAPrefixName=String(MQTT_HA_SWITCH_TOPIC_PREFIX)+String("/"+device+"/"+deviceSufix);
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_Relay1/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //Relay1 value
    String("{\"name\":\"Allow Ext. Therm.\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\""+mqttTopicName+"/cmnd/RELAY\",\"pl_off\":\"R1_OFF\",\"pl_on\":\"R1_ON\",\"uniq_id\":\""+deviceSufix+"_Relay1\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['Relay1']}}\"}").c_str()); //Discovery message for Relay1 value, not retain in the broker
  mqttSensorTopicHAName=mqttSensorTopicHAPrefixName+"_Relay2/config";
  mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //Relay2 value
    String("{\"name\":\"Force Heater\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\""+mqttTopicName+"/cmnd/RELAY\",\"pl_off\":\"R2_OFF\",\"pl_on\":\"R2_ON\",\"uniq_id\":\""+deviceSufix+"_Relay2\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['Relay2']}}\"}").c_str()); //Discovery message for Relay2 value, not retain in the broker
  
  if (debugModeOn) {boardSerialPort.println(String(millis())+" - [mqttClientPublishHADiscovery] - Home Assistant Discovery messages published, last message mqttSensorTopicHAName="+mqttSensorTopicHAName);}
  
  mqttClient.publish(String(mqttTopicName+"/LWT").c_str(), 0, false, "Online\0"); //Availability message, not retain in the broker
  if (debugModeOn) {boardSerialPort.println(String(millis())+" - [mqttClientPublishHADiscovery] - Availability message published, mqttTopicName="+mqttTopicName+"/LWT");}
}