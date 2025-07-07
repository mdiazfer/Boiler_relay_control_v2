#include "mqttClient.h"


void onMqttConnect(bool sessionPresent) {
  String packetInfoId=String(mqttClient.subscribe(MQTT_TOPIC_SUBSCRIPTION,0));
  String packetStatId=String(mqttClient.subscribe(MQTT_HA_B_AND_LWT_TOPIC_PREFIX,0));
  //Subcribe to the-iot-factory/boiler-relay-controlv2-E02940/cmnd/RELAY
  String cmdSubcriptionTopic=String(MQTT_TOPIC_PREFIX+device+"/"+MQTT_TOPIC_CMD_SUFIX_SUBSCRIPTION);
  String packetSwitchId=String(mqttClient.subscribe(cmdSubcriptionTopic.c_str(),0));
  if (powerMeasureEnabled) powerMeasureId=mqttClient.subscribe(powerMqttTopic.c_str(),0);
  if (powerMeasureId==0) powerMeasureSubscribed=false; else powerMeasureSubscribed=true; // powerMeasureSubscribed=true will be also updated after receiving the first MQTT message (onMqttMessage) 

  if (debugModeOn) printLogln("\n"+String(millis())+" - [onMqttConnect] - MQTT connected to "+mqttServer+". Session present: "+String(sessionPresent)+
                        "\n  [onMqttConnect] - Subscribing on:"+
                        "\n  [onMqttConnect] - topic "+MQTT_TOPIC_SUBSCRIPTION+", QoS 0, packetId="+packetInfoId+
                        "\n  [onMqttConnect] - topic "+MQTT_HA_B_AND_LWT_TOPIC_PREFIX+", QoS 0, packetId="+packetStatId+ // v1.9.0 - Home Assistant Last Will Testament message (offline)
                        "\n  [onMqttConnect] - topic "+cmdSubcriptionTopic+", QoS 0, packetId="+packetSwitchId); // Relay ON/OFF
  if (debugModeOn && powerMeasureEnabled) printLogln("  [onMqttConnect] - topic "+powerMqttTopic+", QoS 0, packetId="+String(powerMeasureId));

  //MQTT HA Discovery buffers init
  memset(bufferMqttTopicName,'\0',sizeof(bufferMqttTopicName));sprintf(bufferMqttTopicName,"%s",String(MQTT_TOPIC_PREFIX+device).c_str()); // the-iot-factory/boiler-relay-controlv2-2254C4
  memset(bufferIpAddress,'\0',sizeof(bufferIpAddress));sprintf(bufferIpAddress,"%s",WiFi.localIP().toString().c_str()); // 192.168.100.215
  memset(bufferDevice,'\0',sizeof(bufferDevice));sprintf(bufferDevice,"%s",device.c_str()); // boiler-relay-controlv2-2254C4
      //Taking the 6 latest MAC's bytes: F8:B3:B7:22:54:C4 is converted into 2254C4
      String macAux=String((char)hex_digits[mac[3]>>4])+String((char)hex_digits[mac[3]&15])+
                    String((char)hex_digits[mac[4]>>4])+String((char)hex_digits[mac[4]&15])+
                    String((char)hex_digits[mac[5]>>4])+String((char)hex_digits[mac[5]&15]);
  memset(bufferDeviceSufix,'\0',sizeof(bufferDeviceSufix));sprintf(bufferDeviceSufix,"%s",macAux.c_str()); // 2254C4
  memset(bufferMqttSensorTopicHAPrefixName,'\0',sizeof(bufferMqttSensorTopicHAPrefixName));sprintf(bufferMqttSensorTopicHAPrefixName,"%s",String(MQTT_HA_SENSOR_TOPIC_PREFIX+String("/")+device+"/"+macAux).c_str()); // homeassistant/sensor/boiler-relay-controlv2-2254C4/2254C4
  memset(bufferMqttBinarySensorTopicHAPrefixName,'\0',sizeof(bufferMqttBinarySensorTopicHAPrefixName));sprintf(bufferMqttBinarySensorTopicHAPrefixName,"%s",String(MQTT_HA_BINARY_SENSOR_TOPIC_PREFIX+String("/")+device+"/"+macAux).c_str()); // homeassistant/binary_sensor/boiler-relay-controlv2-2254C4/2254C4
  memset(bufferMqttButtonTopicHAPrefixName,'\0',sizeof(bufferMqttButtonTopicHAPrefixName));sprintf(bufferMqttButtonTopicHAPrefixName,"%s",String(MQTT_HA_BUTTON_TOPIC_PREFIX+String("/")+device+"/"+macAux).c_str()); // homeassistant/button/boiler-relay-controlv2-2254C4/2254C4
  memset(bufferMqttSwitchTopicHAPrefixName,'\0',sizeof(bufferMqttSwitchTopicHAPrefixName));sprintf(bufferMqttSwitchTopicHAPrefixName,"%s",String(MQTT_HA_SWITCH_TOPIC_PREFIX+String("/")+device+"/"+macAux).c_str()); // homeassistant/switch/boiler-relay-controlv2-2254C4/2254C4

  samples["powerMeasureEnabled"]=powerMeasureEnabled;
  samples["powerMeasureSubscribed"]=powerMeasureSubscribed;
  MqttSyncCurrentStatus=MqttSyncOnStatus;
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  if (debugModeOn) printLogln("\n"+String(millis())+" - [onMqttDisconnect] - MQTT disconnected, reason="+(uint8_t)reason);
  MqttSyncCurrentStatus=MqttSyncOffStatus;
  powerMeasureSubscribed=false;
  samples["powerMeasureEnabled"]=powerMeasureEnabled;
  samples["powerMeasureSubscribed"]=powerMeasureSubscribed;
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  if (debugModeOn) printLogln("\n"+String(millis())+" - [onMqttSubscribe] - MQTT subscribe acknowledged. packetId="+String(packetId)+", qos="+String(qos));
}

void onMqttUnsubscribe(uint16_t packetId) {
  if (debugModeOn) printLogln("\n"+String(millis())+" - [onMqttUnsubscribe] - MQTT unsubscribe acknowledged. packetId="+String(packetId));
}

void onMqttPublish(uint16_t packetId) {
  if (debugModeOn) printLogln("\n"+String(millis())+" - [onMqttPublish] - MQTT publish acknowledged. packetId="+String(packetId));
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  char aux[len+1];strcpy(aux,payload); aux[len]='\0';

  if (debugModeOn) printLogln("\n"+String(millis())+" - [onMqttMessage] - MQTT published message received on topic='"+String(topic)+"', mesage: '"+String(aux)+"', index: "+String(index)+", heapSize="+String(esp_get_free_heap_size())+"B, heapBlock="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+" B");
  
  if (String(topic) == String(MQTT_TOPIC_SUBSCRIPTION)) {
    //Pubish device name
    mqttClient.publish(String(mqttTopicPrefix+"device-name").c_str(), 0, false, String("{\"manufacturer\":\"www.the-iotfactory.com\",\"model\":\"boiler-relay-controlv2\",\"name\":\""+device+"\"}").c_str());
  } else if (String(topic) == String(MQTT_HA_B_AND_LWT_TOPIC_PREFIX)) {
    //Home Assistant server published an availability message. Check on it.
    if ((String(aux).equalsIgnoreCase(String("online"))) && mqttServerEnabled && !boilerStatus) { //Publish the Discovery message for Home Assistan to detect this device
      //boilerStatus is true if receiving MQTT messages storm from the SmartPlug. Avoid sending HA MQTT messages in this time
      if (debugModeOn) printLogln(String(millis())+" - [onMqttMessage] - Topic received '"+String(topic)+"' with message: '"+String(aux)+"'. Home Assistant server available. Publish the Discovery message now.");
      mqttClientPublishHADiscovery(mqttTopicPrefix+device,device,WiFi.localIP().toString(),false); //===>
    }
  } else if (String(topic) == String(MQTT_TOPIC_PREFIX+device+"/"+MQTT_TOPIC_CMD_SUFIX_SUBSCRIPTION)) {
    if (debugModeOn) printLogln(String(millis())+" - [onMqttMessage] - Command received: "+String(aux));
    if (String(aux).equalsIgnoreCase(String("REBOOT"))) {
      printLogln(String(millis())+" - [onMqttMessage] - REBOOT. HeapSize="+String(esp_get_free_heap_size())+". Reboot now");
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
    else if (String(aux).equalsIgnoreCase(String("DEBUG_ON"))) {
      uint8_t configVariables=0;
      debugModeOn=true;
      forceMQTTpublish=true; //Force to publish the MQTT message from the loop
      samples["debugModeOn"]="DEBUG_ON";
      configVariables=EEPROM.read(0x606) | 0x04; //Set debugModeOn bit to true (enabled)
      EEPROM.write(0x606,configVariables);
      EEPROM.commit();
    }
    else if (String(aux).equalsIgnoreCase(String("SERIAL_LOGS_ON"))) {
      uint8_t configVariables=0;
      serialLogsOn=true;
      forceMQTTpublish=true; //Force to publish the MQTT message from the loop
      samples["serialLogsOn"]="SERIAL_LOGS_ON";
      configVariables=EEPROM.read(0x606) | 0x08; //Set serialLogsOn bit to true (enabled)
      EEPROM.write(0x606,configVariables);
      EEPROM.commit();
    }
    else if (String(aux).equalsIgnoreCase(String("WEB_LOGS_ON"))) {
      uint8_t configVariables=0;
      webLogsOn=true;
      forceMQTTpublish=true; //Force to publish the MQTT message from the loop
      samples["webLogsOn"]="WEB_LOGS_ON";
      configVariables=EEPROM.read(0x606) | 0x10; //Set webLogsOn bit to true (enabled)
      EEPROM.write(0x606,configVariables);
      EEPROM.commit();
    }
    else if (String(aux).equalsIgnoreCase(String("SYS_LOGS_ON"))) {
      uint8_t configVariables=0;
      sysLogsOn=true;
      forceMQTTpublish=true; //Force to publish the MQTT message from the loop
      samples["sysLogsOn"]="SYS_LOGS_ON";
      configVariables=EEPROM.read(0x606) | 0x20; //Set sysLogsOn bit to true (enabled)
      EEPROM.write(0x606,configVariables);
      EEPROM.commit();
    }
    else if (String(aux).equalsIgnoreCase(String("DEBUG_OFF"))) {
      uint8_t configVariables=0;
      debugModeOn=false;
      forceMQTTpublish=true; //Force to publish the MQTT message from the loop
      samples["debugModeOn"]="DEBUG_OFF";
      configVariables=EEPROM.read(0x606) & 0xFB; //Unset debugModeOn bit (disabled)
      EEPROM.write(0x606,configVariables);
      EEPROM.commit();
    }
    else if (String(aux).equalsIgnoreCase(String("SERIAL_LOGS_OFF"))) {
      uint8_t configVariables=0;
      serialLogsOn=false;
      forceMQTTpublish=true; //Force to publish the MQTT message from the loop
      samples["serialLogsOn"]="SERIAL_LOGS_OFF";
      configVariables=EEPROM.read(0x606) & 0xF7; //Unset serialLogsOn bit (disabled)
      EEPROM.write(0x606,configVariables);
      EEPROM.commit();
    }
    else if (String(aux).equalsIgnoreCase(String("WEB_LOGS_OFF"))) {
      uint8_t configVariables=0;
      webLogsOn=false;
      forceMQTTpublish=true; //Force to publish the MQTT message from the loop
      samples["webLogsOn"]="WEB_LOGS_OFF";
      configVariables=EEPROM.read(0x606) & 0xEF; //Unset webLogsOn bit (disabled)
      EEPROM.write(0x606,configVariables);
      EEPROM.commit();
    }
    else if (String(aux).equalsIgnoreCase(String("SYS_LOGS_OFF"))) {
      uint8_t configVariables=0;
      sysLogsOn=false;
      forceMQTTpublish=true; //Force to publish the MQTT message from the loop
      samples["sysLogsOn"]="SYS_LOGS_OFF";
      configVariables=EEPROM.read(0x606) & 0xDF; //Unset sysLogsOn bit (disabled)
      EEPROM.write(0x606,configVariables);
      EEPROM.commit();
    }
    else {
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
        else printLogln(String(millis())+" - [onMqttMessage] - Unknown command: '"+String(aux)+"'");
      }
      else printLogln(String(millis())+" - [onMqttMessage] - Gas leak situation detected, so no releay activation is allowed for security reasons");
    }
  }
  else if (String(topic) == powerMqttTopic) {
    printLogln(String(millis())+" - [onMqttMessage] - "+powerMqttTopic);
    /*
    {"Time":"2025-06-08T10:16:25","ENERGY":{"TotalStartTime":"2024-07-25T11:21:51","Total":0.484,"Yesterday":0.241,"Today":0.088,"Power": 8,"ApparentPower":23,"ReactivePower":22,"Factor":0.36,"Voltage":328,"Current":0.071}}
    */
    JSONVar auxEnergyJson=JSON.parse(aux);
    if (JSON.typeof(auxEnergyJson) == "undefined") {
      printLogln(String(millis())+" - [onMqttMessage] - Parsing input failed!");
    }
    else {
      /*printLogln(String(millis())+" - [onMqttMessage] - Time="+JSON.stringify(auxEnergyJson["Time"])+
                                "\n                           Voltage="+JSON.stringify(auxEnergyJson["ENERGY"]["Voltage"])+" V"+
                                "\n                           Current="+JSON.stringify(auxEnergyJson["ENERGY"]["Current"])+" A"+
                                "\n                           Power="+JSON.stringify(auxEnergyJson["ENERGY"]["Power"])+" W"+
                                "\n                           Energy Today="+JSON.stringify(auxEnergyJson["ENERGY"]["Today"])+" KWh"+
                                "\n                           Energy Yesterday="+JSON.stringify(auxEnergyJson["ENERGY"]["Yesterday"])+" KWh"+
                                "\n                           Energy Total="+JSON.stringify(auxEnergyJson["ENERGY"]["Total"])+" KWh");
      */
      bool updateMQTT=false;
      if (JSON.stringify(auxEnergyJson["ENERGY"]["Power"]).toInt() >= BOILER_FLAME_ON_POWER_THRESHOLD) {
        //Boiler is burning gas (flame) due to heater (thermostateOn=true) or hot water (boilerOn=true)
        // and so it's active (boilerStatus=true)
        /*if (debugModeOn) printLogln(String(millis())+" - [onMqttMessage] - Flame detected: Power ("+JSON.stringify(auxEnergyJson["ENERGY"]["Power"])+") >= BOILER_FLAME_ON_POWER_THRESHOLD ("+String(BOILER_FLAME_ON_POWER_THRESHOLD));*/  //----->
        if (!boilerStatus && !thermostateStatus) updateMQTT=true; //coming from inactive boiler
        if (!boilerOn && !thermostateOn) updateMQTT=true; //coming from no flame
        boilerStatus=true;
        if (thermostateStatus) { //thermostateStatus is updated in thermostate_interrupt_triggered()
          //If thermostate is active, let's assume it's burning gas due to the heater and not the warming water
          
          //Start counting seconds
          if (!thermostateOn) {
            lastThermostatOnTime=millis();
            /*if (debugModeOn) printLogln(String(millis())+" - [onMqttMessage] - Start Heater Time On Counter. lastThermostatOnTime="+String(lastThermostatOnTime));*/  //----->
          }
          thermostateOn=true; boilerOn=false;
        } 
        else {
          //Start counting seconds
          if (!boilerOn) {
            lastBoilerOnTime=millis();
            /*if (debugModeOn) printLogln(String(millis())+" - [onMqttMessage] - Start Boiler Time On Counter. lastBoilerOnTime="+String(lastBoilerOnTime));*/  //----->
          }
          thermostateOn=false; boilerOn=true;
        }
      }
      else if (JSON.stringify(auxEnergyJson["ENERGY"]["Power"]).toInt() >= BOILER_STATUS_ON_POWER_THRESHOLD) {
        //Boiler is active (water flowing due to either heater or hot water) but it isn't burning gas
        if (!boilerStatus && !thermostateStatus) updateMQTT=true; //coming from inactive boiler
        if (boilerOn || thermostateOn) updateMQTT=true; //coming from flame
        boilerStatus=true; thermostateOn=false; boilerOn=false; //No flame and thermostateStatus is updated in thermostate_interrupt_triggered()
        /*if (debugModeOn) printLogln(String(millis())+" - [onMqttMessage] - Boiler is active with no flame: Power ("+JSON.stringify(auxEnergyJson["ENERGY"]["Power"])+") >= BOILER_STATUS_ON_POWER_THRESHOLD ("+String(BOILER_STATUS_ON_POWER_THRESHOLD));*/  //----->
      }
      else {
        //Boiler isn't active
        updateMQTT=true; //Allways update the samples
        if (boilerStatus || thermostateStatus) {
          //Coming from active status
          boilerStatus=false; thermostateOn=false; boilerOn=false; //No flame and thermostateStatus is updated in thermostate_interrupt_triggered()
          /*if (debugModeOn) printLogln(String(millis())+" - [onMqttMessage] - Boiler is stopped: Power ("+JSON.stringify(auxEnergyJson["ENERGY"]["Power"])+")");*/  //----->
        }
        else {
          //Getting active from inactive status or inactive status reported from regular MQTT messages from the SmartPlug
          // Do nothing and wait for the next MQTT update to update status variables
          /*if (debugModeOn) printLogln(String(millis())+" - [onMqttMessage] - Boiler uncertain status. Wait for the next MQTT message to set variables: Power ("+JSON.stringify(auxEnergyJson["ENERGY"]["Power"])+")");*/  //----->
        }
        
      }

      //Variables update
      voltage=JSON.stringify(auxEnergyJson["ENERGY"]["Voltage"]).toInt();
      current=JSON.stringify(auxEnergyJson["ENERGY"]["Current"]).toFloat();
      power=JSON.stringify(auxEnergyJson["ENERGY"]["Power"]).toInt();
      energyToday=JSON.stringify(auxEnergyJson["ENERGY"]["Today"]).toFloat();
      energyYesterday=JSON.stringify(auxEnergyJson["ENERGY"]["Yesterday"]).toFloat();
      energyTotal=JSON.stringify(auxEnergyJson["ENERGY"]["Total"]).toFloat();
      powerMeasureSubscribed=true;
      samples["Voltage"]=voltage;
      samples["Current"]=current;
      samples["Power"]=power;
      samples["EnergyToday"]=energyToday;
      samples["EnergyYesterday"]=energyYesterday;
      samples["EnergyTotal"]=energyTotal;
      samples["Thermostate_status"] = thermostateStatus==true?"ON":"OFF";
      samples["Thermostate_on"] = thermostateOn==true?"ON":"OFF";
      samples["boilerStatus"] = boilerStatus==true?"ON":"OFF";
      samples["boilerOn"] = boilerOn==true?"ON":"OFF";
      samples["powerMeasureEnabled"]=powerMeasureEnabled;
      samples["powerMeasureSubscribed"]=powerMeasureSubscribed;
      struct tm nowTimeInfo; //36 B
      char s[100];getLocalTime(&nowTimeInfo);strftime(s,sizeof(s),"%d/%m/%Y - %H:%M:%S",&nowTimeInfo);
      samples["dateUpdate"] =  String(s);

      //Update web and mqtt only if boiler gets active or inactive.
      //Rest of situatios, updates are done every SAMPLE_PERIO
      if (updateMQTT) {forceMQTTpublish=true; forceWebEvent=true;}
      /*if (debugModeOn) printLogln(String(millis())+" - [onMqttMessage] - Exit - boilerStatus="+String(boilerStatus)+", thermostateStatus="+String(thermostateStatus)+", boilerOn="+String(boilerOn)+", thermostateOn="+String(thermostateOn));*/  //----->
    }
  }
  else {
    //Do nothing for other topics
    if (debugModeOn) printLogln(String(millis())+" - [onMqttMessage] - Topic received ("+String(topic)+") and differs from subscriptions: '"+String(MQTT_TOPIC_SUBSCRIPTION)+"', '"+String(MQTT_HA_B_AND_LWT_TOPIC_PREFIX)+"'. Return");
  }
} // onMqttMessage

void mqttClientPublishHADiscovery(String mqttTopicName, String device, String ipAddress, bool removeTopics) {
  //Break up the MQTT messages to avoid leaking heap - IS0016 - v0.9.B
  
  //System Objects: Device, Signals, System (Counters), WiFi
  //if (debugModeOn) {printLogln(String(millis())+" - [mqttClientPublishHADiscovery] - heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size())+", Calling mqttClientPublishHADiscovery_systemObjects1");}
  mqttClientPublishHADiscovery_systemObjects1(mqttTopicName,device,ipAddress,removeTopics);
  //if (debugModeOn) {printLogln(String(millis())+" - [mqttClientPublishHADiscovery] - heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size())+", Calling mqttClientPublishHADiscovery_systemObjects2");}
  mqttClientPublishHADiscovery_systemObjects2(mqttTopicName,device,ipAddress,removeTopics);
  //Sesors Objects: Environment GAS, Temperature, Humidity
  //if (debugModeOn) {printLogln(String(millis())+" - [mqttClientPublishHADiscovery] - heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size())+", Calling mqttClientPublishHADiscovery_sensorsObjects");}
  mqttClientPublishHADiscovery_sensorsObjects(mqttTopicName,device,ipAddress,removeTopics);
  //Boiler Time On Counters
  //if (debugModeOn) {printLogln(String(millis())+" - [mqttClientPublishHADiscovery] - heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size())+", Calling mqttClientPublishHADiscovery_boilerTimeOnObjects");}
  mqttClientPublishHADiscovery_boilerTimeOnObjects(mqttTopicName,device,ipAddress,removeTopics);
  //Heater Time On Counters
  //if (debugModeOn) {printLogln(String(millis())+" - [mqttClientPublishHADiscovery] - heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size())+", Calling mqttClientPublishHADiscovery_heaterTimeOnObjects");}
  mqttClientPublishHADiscovery_heaterTimeOnObjects(mqttTopicName,device,ipAddress,removeTopics);
  //Binary Objects: Boiler/Heater Status, Clear Gas flag, etc.
  //if (debugModeOn) {printLogln(String(millis())+" - [mqttClientPublishHADiscovery] - heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size())+", Calling mqttClientPublishHADiscovery_binaryObjects");}
  mqttClientPublishHADiscovery_binaryObjects(mqttTopicName,device,ipAddress,removeTopics);
  //Relay Objects: Relay1, Relay2
  //if (debugModeOn) {printLogln(String(millis())+" - [mqttClientPublishHADiscovery] - heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size())+", Calling mqttClientPublishHADiscovery_relayObjects");}
  mqttClientPublishHADiscovery_relayObjects(mqttTopicName,device,ipAddress,removeTopics);
  //Button Objects: Relay1, Relay2
  //if (debugModeOn) {printLogln(String(millis())+" - [mqttClientPublishHADiscovery] - heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size())+", Calling mqttClientPublishHADiscovery_buttonObjects");}
  mqttClientPublishHADiscovery_buttonObjects(mqttTopicName,device,ipAddress,removeTopics);

  if (removeTopics) {
    if (debugModeOn) {printLogln(String(millis())+" - [mqttClientPublishHADiscovery] - Home Assistant Discovery messages published to remove all the topics");}
    removeTopics=false;
  }
  else {
    if (debugModeOn) {printLogln(String(millis())+" - [mqttClientPublishHADiscovery] - Home Assistant Discovery messages published to publish all the topics");}
    mqttClient.publish(String(mqttTopicName+"/LWT").c_str(), 0, false, "Online\0"); //Availability message, not retain in the broker
    if (debugModeOn) {printLogln(String(millis())+" - [mqttClientPublishHADiscovery] - Availability message published, mqttTopicName="+mqttTopicName+"/LWT");}
  }
} //mqttClientPublishHADiscovery

void mqttClientPublishHADiscovery_systemObjects1(String mqttTopicName, String device, String ipAddress, bool removeTopics) {
  //Publish Home Assistant Discovery messages
  struct tm nowTimeInfo; //36 B
  char s[100];
  getLocalTime(&nowTimeInfo);strftime(s,sizeof(s),"%Y-%m-%dT%H:%M:%S",&nowTimeInfo); //Time in format 2024-08-24T07:56:25

  //For every sample value, a Discovery Message must be published
  //HA Discovery topic format is as follows: <discovery_prefix>/<component>/[<node_id>]/<object_id>/config
  // In the following exmaple:
  //  <discovery_prefix> = homeassistant
  //  <component> = sensor or switch (relay)
  //  [<node_id>] = boiler-relay-controlv2-E02940
  //  <object_id> = E02940_<SAMPLE_NAME> = E02940_H2, E02940_CO, etc.
  //i.e.: device=boiler-relay-controlv2-E02940                                                                    -      29 Bytes                                       =>  50 Bytes
  //      deviceSufix=E02940                                                                                      -       6 Bytes                                       =>  10 Bytes
  //      mqttTopicName=the-iot-factory/boiler-relay-controlv2-E02940                                             -      70 Bytes                                       => 100 Bytes
  //      mqttSensorTopicHAPrefixName=homeassistant/sensor/boiler-relay-controlv2-E02940/E02940                   -      58 = 57+1 Bytes                                => 100 Bytes
  //      mqttSensorTopicHAName=homeassistant/sensor/boiler-relay-controlv2-E02940/E02940_<SAMPLE_NAME>/config    -     125 = 57+7+60+1 Bytes   <SAMPLE_NAME> ~ 60 Byte => 124 Bytes
  //Sensors
  //mdi:gas-burner,air-filter, mdi:clock-time-eight \"ic\":\"mdi:calendar-clock\",

  
  // 70+ 57 +23+ 57 +72+ 57 +70+ 6 +142+ 6 +35 + 15 +14+ 29 +77+ 22 +20+ 5 +110 +1 = 888 Bytes  => 1000 Bytes
  /*mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //reset_time_counters value
    String("{\"name\":\"Device  : Reset Time Counters\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\""+mqttTopicName+
            "/cmnd/RELAY\",\"payload_press\":\"RESET_TIME_COUNTERS\",\"uniq_id\":\""+deviceSufix+"_reset_time_counters\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+
            device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+
            String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"dev_cla\":\"restart\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['reset_time_counters']}}\"}").c_str()); //Discovery message for Reboot value, not retain in the broker*/
  
  //Device, Environment, GAS, Signals, System (Counters), WiFi
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_startTime/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device  : Boot Time\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_startTime\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"ic\":\"mdi:clock-time-five\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['startTime']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);

  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_dateUpdate/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device  : Last Time\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_dateUpdate\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"ic\":\"mdi:clock-time-eight\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['dateUpdate']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload); //dateUpdate value - Discovery message for dateUpdate value, not retain in the broker
  
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_upTimeSeconds/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device  : upTime\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_upTimeSeconds\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:clock-end\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['upTimeSeconds']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload); //upTimeSeconds value - //Discovery message for upTimeSeconds value, not retain in the broker
  
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_errorsWiFiCnt/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Counter: errorsWiFi\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_errorsWiFiCnt\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"ic\":\"mdi:counter\",\"val_tpl\":\"{{value_json['SAMPLES']['errorsWiFiCnt']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload); //errorsWiFiCnt  value - //Discovery message for errorsWiFiCnt value, not retain in the broker
  
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_errorsConnectivityCnt/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Counter: errorsConnectivity\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_errorsConnectivityCnt\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"ic\":\"mdi:counter\",\"val_tpl\":\"{{value_json['SAMPLES']['errorsConnectivityCnt']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload); //errorsConnectivityCnt  value - //Discovery message for errorsConnectivityCnt value, not retain in the broker
  
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_errorsNTPCnt/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Counter: errorsNTP\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_errorsNTPCnt\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"ic\":\"mdi:counter\",\"val_tpl\":\"{{value_json['SAMPLES']['errorsNTPCnt']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload); //errorsNTPCnt  value - //Discovery message for errorsNTPCnt value, not retain in the broker
  
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_errorsHTTPUptsCnt/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Counter: errorsHTTPUpts\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_errorsHTTPUptsCnt\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"ic\":\"mdi:counter\",\"val_tpl\":\"{{value_json['SAMPLES']['errorsHTTPUptsCnt']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload); //errorsHTTPUptsCnt  value - //Discovery message for errorsHTTPUptsCnt value, not retain in the broker
  
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_errorsMQTTCnt/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Counter: errorsMQTT\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_errorsMQTTCnt\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"ic\":\"mdi:counter\",\"val_tpl\":\"{{value_json['SAMPLES']['errorsMQTTCnt']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload); //errorsMQTTCnt  value - //Discovery message for errorsMQTTCnt value, not retain in the broker
  
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_errorsWebServerCnt/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Counter: errorsWebServerCnt\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_errorsWebServerCnt\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"ic\":\"mdi:counter\",\"val_tpl\":\"{{value_json['SAMPLES']['errorsWebServerCnt']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload); //errorsWebServerCnt  value - //Discovery message for errorsWebServerCnt value, not retain in the broker
  
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_bootCount/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Boots:  since last update\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_bootCount\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"ic\":\"mdi:counter\",\"val_tpl\":\"{{value_json['SAMPLES']['bootCount']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload); //bootCount  value - //Discovery message for bootCount value, not retain in the broker
  
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_resetNormalCount/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Boots: normal resets\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_resetNormalCount\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"ic\":\"mdi:counter\",\"val_tpl\":\"{{value_json['SAMPLES']['resetNormalCount']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload); //resetPreventiveCount  value - //Discovery message for resetNormalCount value, not retain in the broker

} //mqttClientPublishHADiscovery_systemObjects1

void mqttClientPublishHADiscovery_systemObjects2(String mqttTopicName, String device, String ipAddress, bool removeTopics) {
  //Publish Home Assistant Discovery messages
  struct tm nowTimeInfo; //36 B
  char s[100];
  getLocalTime(&nowTimeInfo);strftime(s,sizeof(s),"%Y-%m-%dT%H:%M:%S",&nowTimeInfo); //Time in format 2024-08-24T07:56:25

  //For every sample value, a Discovery Message must be published
  //HA Discovery topic format is as follows: <discovery_prefix>/<component>/[<node_id>]/<object_id>/config
  // In the following exmaple:
  //  <discovery_prefix> = homeassistant
  //  <component> = sensor or switch (relay)
  //  [<node_id>] = boiler-relay-controlv2-E02940
  //  <object_id> = E02940_<SAMPLE_NAME> = E02940_H2, E02940_CO, etc.
  //i.e.: device=boiler-relay-controlv2-E02940                                                                    -      29 Bytes                                       =>  50 Bytes
  //      deviceSufix=E02940                                                                                      -       6 Bytes                                       =>  10 Bytes
  //      mqttTopicName=the-iot-factory/boiler-relay-controlv2-E02940                                             -      70 Bytes                                       => 100 Bytes
  //      mqttSensorTopicHAPrefixName=homeassistant/sensor/boiler-relay-controlv2-E02940/E02940                   -      58 = 57+1 Bytes                                => 100 Bytes
  //      mqttSensorTopicHAName=homeassistant/sensor/boiler-relay-controlv2-E02940/E02940_<SAMPLE_NAME>/config    -     125 = 57+7+60+1 Bytes   <SAMPLE_NAME> ~ 60 Byte => 124 Bytes
  //Sensors
  //mdi:gas-burner,air-filter, mdi:clock-time-eight \"ic\":\"mdi:calendar-clock\",

  
  // 70+ 57 +23+ 57 +72+ 57 +70+ 6 +142+ 6 +35 + 15 +14+ 29 +77+ 22 +20+ 5 +110 +1 = 888 Bytes  => 1000 Bytes
  /*mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //reset_time_counters value
    String("{\"name\":\"Device  : Reset Time Counters\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\""+mqttTopicName+
            "/cmnd/RELAY\",\"payload_press\":\"RESET_TIME_COUNTERS\",\"uniq_id\":\""+deviceSufix+"_reset_time_counters\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+
            device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+
            String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"dev_cla\":\"restart\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['reset_time_counters']}}\"}").c_str()); //Discovery message for Reboot value, not retain in the broker*/
  
  
  //Device, Environment, GAS, Signals, System (Counters), WiFi
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_resetPreventiveCount/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Boots: preventive resets\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_resetPreventiveCount\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"ic\":\"mdi:counter\",\"val_tpl\":\"{{value_json['SAMPLES']['resetPreventiveCount']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_resetPreventiveWebServerCount/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Boots: preventive  web server resets\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_resetPreventiveWebServerCount\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"ic\":\"mdi:counter\",\"val_tpl\":\"{{value_json['SAMPLES']['resetPreventiveWebServerCount']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_resetSWCount/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Boots: total sw resets\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_resetSWCount\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"ic\":\"mdi:counter\",\"val_tpl\":\"{{value_json['SAMPLES']['resetSWCount']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_resetSWWebCount/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Boots: resets from web\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_resetSWWebCount\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"ic\":\"mdi:counter\",\"val_tpl\":\"{{value_json['SAMPLES']['resetSWWebCount']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_resetSWMqttCount/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Boots: resets from mqtt\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_resetSWMqttCount\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"ic\":\"mdi:counter\",\"val_tpl\":\"{{value_json['SAMPLES']['resetSWMqttCount']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_resetSWUpgradeCount/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Boots: resets due to firmware update\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_resetSWUpgradeCount\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"ic\":\"mdi:counter\",\"val_tpl\":\"{{value_json['SAMPLES']['resetSWUpgradeCount']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_resetWebServerCnt/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Boots: resets due to web server KO\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_resetWebServerCnt\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"ic\":\"mdi:counter\",\"val_tpl\":\"{{value_json['SAMPLES']['resetWebServerCnt']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_resetCount/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Boots: uncontrolled resets\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_resetCount\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"ic\":\"mdi:counter\",\"val_tpl\":\"{{value_json['SAMPLES']['resetCount']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_resetReason/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Reset: reason\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_resetReason\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['resetReason']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heapSize/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Heap:   Heap size\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heapSize\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"dev_cla\":\"data_size\",\"unit_of_meas\":\"B\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heapSize']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_minHeapSeen/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Heap: Min heap size\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_minHeapSeen\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"dev_cla\":\"data_size\",\"unit_of_meas\":\"B\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['minHeapSeen']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_minHeapSinceBoot/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Heap: Min heap since boot\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_minHeapSinceBoot\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"dev_cla\":\"data_size\",\"unit_of_meas\":\"B\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['minHeapSinceBoot']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_minHeapSinceUpgrade/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Heap: Min heap since upgrade\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_minHeapSinceUpgrade\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"dev_cla\":\"data_size\",\"unit_of_meas\":\"B\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['minHeapSinceUpgrade']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heapBlockSize/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Heap:  Max heap block size\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heapBlockSize\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"dev_cla\":\"data_size\",\"unit_of_meas\":\"B\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heapBlockSize']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload); //heapBlockSize  value - //Discovery message for heapBlockSize value, not retain in the broker
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_minMaxHeapBlockSizeSinceBoot/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Heap:  Min max block size since boot\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_minMaxHeapBlockSizeSinceBoot\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"dev_cla\":\"data_size\",\"unit_of_meas\":\"B\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['minMaxHeapBlockSizeSinceBoot']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload); //heapBlockSize  value - //Discovery message for heapBlockSize value, not retain in the broker
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_minMaxHeapBlockSizeSinceUpgrade/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device Heap:  Min max block size since upgrade\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_minMaxHeapBlockSizeSinceUpgrade\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"dev_cla\":\"data_size\",\"unit_of_meas\":\"B\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['minMaxHeapBlockSizeSinceUpgrade']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload); //heapBlockSize  value - //Discovery message for heapBlockSize value, not retain in the broker
  
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_BSSID/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"WiFi: BSSID\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_BSSID\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"ic\":\"mdi:radio-tower\",\"val_tpl\":\"{{value_json['SAMPLES']['BSSID']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
    memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_RSSI/config");
    memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"WiFi: RSSI\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_RSSI\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"%\",\"frc_upd\":true,\"ic\":\"",iconWifi.c_str(),"\",\"val_tpl\":\"{{value_json['SAMPLES']['RSSI']}}\"}");
    if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
    else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_SIGNAL/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"WiFi: SIGNAL\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_SIGNAL\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"dBm\",\"dev_cla\":\"signal_strength\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['SIGNAL']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_SSID/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"WiFi: SSID\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_SSID\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"ic\":\"mdi:access-point\",\"val_tpl\":\"{{value_json['SAMPLES']['SSID']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_ipAddress/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"WiFi: IP Address\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_ipAddress\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"ic\":\"mdi:ip-network\",\"val_tpl\":\"{{value_json['SAMPLES']['ipAddress']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);

  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_sysLogServer/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Syslog Server\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_sysLogServer\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"ic\":\"mdi:server-network-outline\",\"val_tpl\":\"{{value_json['SAMPLES']['sysLogServer']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_sysLogServerUDPPort/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Syslog Port\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_sysLogServerUDPPort\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['sysLogServerUDPPort']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
} //mqttClientPublishHADiscovery_systemObjects2

void mqttClientPublishHADiscovery_sensorsObjects(String mqttTopicName, String device, String ipAddress, bool removeTopics) {
  //Publish Home Assistant Discovery messages
  struct tm nowTimeInfo; //36 B
  char s[100];
  getLocalTime(&nowTimeInfo);strftime(s,sizeof(s),"%Y-%m-%dT%H:%M:%S",&nowTimeInfo); //Time in format 2024-08-24T07:56:25

  //For every sample value, a Discovery Message must be published
  //HA Discovery topic format is as follows: <discovery_prefix>/<component>/[<node_id>]/<object_id>/config
  // In the following exmaple:
  //  <discovery_prefix> = homeassistant
  //  <component> = sensor or switch (relay)
  //  [<node_id>] = boiler-relay-controlv2-E02940
  //  <object_id> = E02940_<SAMPLE_NAME> = E02940_H2, E02940_CO, etc.
  //i.e.: device=boiler-relay-controlv2-E02940                                                                    -      29 Bytes                                       =>  50 Bytes
  //      deviceSufix=E02940                                                                                      -       6 Bytes                                       =>  10 Bytes
  //      mqttTopicName=the-iot-factory/boiler-relay-controlv2-E02940                                             -      70 Bytes                                       => 100 Bytes
  //      mqttSensorTopicHAPrefixName=homeassistant/sensor/boiler-relay-controlv2-E02940/E02940                   -      58 = 57+1 Bytes                                => 100 Bytes
  //      mqttSensorTopicHAName=homeassistant/sensor/boiler-relay-controlv2-E02940/E02940_<SAMPLE_NAME>/config    -     125 = 57+7+60+1 Bytes   <SAMPLE_NAME> ~ 60 Byte => 124 Bytes
  //Sensors
  //mdi:gas-burner,air-filter, mdi:clock-time-eight \"ic\":\"mdi:calendar-clock\",

  
  // 70+ 57 +23+ 57 +72+ 57 +70+ 6 +142+ 6 +35 + 15 +14+ 29 +77+ 22 +20+ 5 +110 +1 = 888 Bytes  => 1000 Bytes
  /*mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //reset_time_counters value
    String("{\"name\":\"Device  : Reset Time Counters\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\""+mqttTopicName+
            "/cmnd/RELAY\",\"payload_press\":\"RESET_TIME_COUNTERS\",\"uniq_id\":\""+deviceSufix+"_reset_time_counters\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+
            device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+
            String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"dev_cla\":\"restart\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['reset_time_counters']}}\"}").c_str()); //Discovery message for Reboot value, not retain in the broker*/
  
  
  //Device, Environment, GAS, Signals, System (Counters), WiFi
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_tempSensor/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Environment: TempSensor\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_tempSensor\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"°C\",\"dev_cla\":\"temperature\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['tempSensor'] | float  | round (1)}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_Temperature/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Environment: Temperature\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_Temperature\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"°C\",\"dev_cla\":\"temperature\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['temperature'] | float  | round (1)}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_Humidity/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Environment: Humidity\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_Humidity\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"%\",\"dev_cla\":\"humidity\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['humidity'] | float  | round (1)}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_ALCOHOL/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"GAS: ALCOHOL\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_ALCOHOL\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"ppm\",\"ic\":\"mdi:fire\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['ALCOHOL'] | float  | round (2)}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_CH4/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"GAS: CH4\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_CH4\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"ppm\",\"ic\":\"mdi:fire\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['CH4'] | float  | round (2)}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_CO/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"GAS: CO\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_CO\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"ppm\",\"ic\":\"mdi:fire\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['CO'] | float  | round (2)}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_H2/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"GAS: H2\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_H2\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"ppm\",\"ic\":\"mdi:fire\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['H2'] | float  | round (2)}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_LPG/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"GAS: LPG\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_LPG\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"ppm\",\"ic\":\"mdi:fire\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['LPG'] | float  | round (2)}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
} //mqttClientPublishHADiscovery_sensorsObjects

void mqttClientPublishHADiscovery_boilerTimeOnObjects(String mqttTopicName, String device, String ipAddress, bool removeTopics) {
  //Publish Home Assistant Discovery messages
  struct tm nowTimeInfo; //36 B
  char s[100];
  getLocalTime(&nowTimeInfo);strftime(s,sizeof(s),"%Y-%m-%dT%H:%M:%S",&nowTimeInfo); //Time in format 2024-08-24T07:56:25
  
  //For every sample value, a Discovery Message must be published
  //HA Discovery topic format is as follows: <discovery_prefix>/<component>/[<node_id>]/<object_id>/config
  // In the following exmaple:
  //  <discovery_prefix> = homeassistant
  //  <component> = sensor or switch (relay)
  //  [<node_id>] = boiler-relay-controlv2-E02940
  //  <object_id> = E02940_<SAMPLE_NAME> = E02940_H2, E02940_CO, etc.
  //i.e.: device=boiler-relay-controlv2-E02940                                                                    -      29 Bytes                                       =>  50 Bytes
  //      deviceSufix=E02940                                                                                      -       6 Bytes                                       =>  10 Bytes
  //      mqttTopicName=the-iot-factory/boiler-relay-controlv2-E02940                                             -      70 Bytes                                       => 100 Bytes
  //      mqttSensorTopicHAPrefixName=homeassistant/sensor/boiler-relay-controlv2-E02940/E02940                   -      58 = 57+1 Bytes                                => 100 Bytes
  //      mqttSensorTopicHAName=homeassistant/sensor/boiler-relay-controlv2-E02940/E02940_<SAMPLE_NAME>/config    -     125 = 57+7+60+1 Bytes   <SAMPLE_NAME> ~ 60 Byte => 124 Bytes
  //Sensors
  //mdi:gas-burner,air-filter, mdi:clock-time-eight \"ic\":\"mdi:calendar-clock\",

  
  // 70+ 57 +23+ 57 +72+ 57 +70+ 6 +142+ 6 +35 + 15 +14+ 29 +77+ 22 +20+ 5 +110 +1 = 888 Bytes  => 1000 Bytes
  /*mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //reset_time_counters value
    String("{\"name\":\"Device  : Reset Time Counters\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\""+mqttTopicName+
            "/cmnd/RELAY\",\"payload_press\":\"RESET_TIME_COUNTERS\",\"uniq_id\":\""+deviceSufix+"_reset_time_counters\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+
            device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+
            String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"dev_cla\":\"restart\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['reset_time_counters']}}\"}").c_str()); //Discovery message for Reboot value, not retain in the broker*/
  
  //Device, Environment, GAS, Signals, System (Counters), WiFi
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerPreviousYear/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Boiler Previous Year: \",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerPreviousYear\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerPreviousYear']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else {
    if (updateHADiscovery) {mqttClient.publish(bufferTopicHAName, 0, false);} //Send topic with no payload to publish a new year in the name
    mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  }
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnPreviousYear/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler Previous Year:      \",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnPreviousYear\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYear']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else {
    if (updateHADiscovery) {mqttClient.publish(bufferTopicHAName, 0, false);} //Send topic with no payload to publish a new year in the name
    mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  }

  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnPreviousYearJan/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 01\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnPreviousYearJan\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearJan']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnPreviousYearFeb/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 02\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnPreviousYearFeb\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearFeb']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnPreviousYearMar/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 03\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnPreviousYearMar\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearMar']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnPreviousYearApr/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 04\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnPreviousYearApr\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearApr']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnPreviousYearMay/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 05\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnPreviousYearMay\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearMay']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnPreviousYearJun/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 06\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnPreviousYearJun\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearJun']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnPreviousYearJul/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 07\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnPreviousYearJul\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearJul']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnPreviousYearAug/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 08\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnPreviousYearAug\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearAug']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnPreviousYearSep/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 09\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnPreviousYearSep\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearSep']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnPreviousYearOct/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 10\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnPreviousYearOct\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearOct']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnPreviousYearNov/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 11\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnPreviousYearNov\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearNov']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnPreviousYearDec/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 12\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnPreviousYearDec\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnPreviousYearDec']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  

  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerYear/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Boiler Current Year: \",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerYear\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerYear']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else {
    if (updateHADiscovery) {mqttClient.publish(bufferTopicHAName, 0, false);} //Send topic with no payload to publish a new year in the name
    mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  }
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnYear/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler Current Year:      \",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnYear\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYear']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else {
    if (updateHADiscovery) {mqttClient.publish(bufferTopicHAName, 0, false);} //Send topic with no payload to publish a new year in the name
    mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  }
  
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnYearJan/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 01\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnYearJan\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearJan']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnYearFeb/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 02\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnYearFeb\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearFeb']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnYearMar/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 03\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnYearMar\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearMar']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnYearApr/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 04\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnYearApr\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearApr']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnYearMay/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 05\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnYearMay\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearMay']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnYearJun/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 06\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnYearJun\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearJun']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnYearJul/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 07\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnYearJul\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearJul']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnYearAug/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 08\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnYearAug\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearAug']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnYearSep/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 09\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnYearSep\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearSep']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnYearOct/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 10\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnYearOct\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearOct']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnYearNov/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 11\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnYearNov\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearNov']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnYearDec/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:     Month 12\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnYearDec\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYearDec']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  
  
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnYesterday/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler:  Yesterday\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnYesterday\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnYesterday']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_boilerOnToday/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Boiler: Today\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOnToday\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOnToday']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
} //mqttClientPublishHADiscovery_boilerTimeOnObjects

void mqttClientPublishHADiscovery_heaterTimeOnObjects(String mqttTopicName, String device, String ipAddress, bool removeTopics) {
  //Publish Home Assistant Discovery messages
  struct tm nowTimeInfo; //36 B
  char s[100];
  getLocalTime(&nowTimeInfo);strftime(s,sizeof(s),"%Y-%m-%dT%H:%M:%S",&nowTimeInfo); //Time in format 2024-08-24T07:56:25
  
  //For every sample value, a Discovery Message must be published
  //HA Discovery topic format is as follows: <discovery_prefix>/<component>/[<node_id>]/<object_id>/config
  // In the following exmaple:
  //  <discovery_prefix> = homeassistant
  //  <component> = sensor or switch (relay)
  //  [<node_id>] = boiler-relay-controlv2-E02940
  //  <object_id> = E02940_<SAMPLE_NAME> = E02940_H2, E02940_CO, etc.
  //i.e.: device=boiler-relay-controlv2-E02940                                                                    -      29 Bytes                                       =>  50 Bytes
  //      deviceSufix=E02940                                                                                      -       6 Bytes                                       =>  10 Bytes
  //      mqttTopicName=the-iot-factory/boiler-relay-controlv2-E02940                                             -      70 Bytes                                       => 100 Bytes
  //      mqttSensorTopicHAPrefixName=homeassistant/sensor/boiler-relay-controlv2-E02940/E02940                   -      58 = 57+1 Bytes                                => 100 Bytes
  //      mqttSensorTopicHAName=homeassistant/sensor/boiler-relay-controlv2-E02940/E02940_<SAMPLE_NAME>/config    -     125 = 57+7+60+1 Bytes   <SAMPLE_NAME> ~ 60 Byte => 124 Bytes
  //Sensors
  //mdi:gas-burner,air-filter, mdi:clock-time-eight \"ic\":\"mdi:calendar-clock\",

  
  // 70+ 57 +23+ 57 +72+ 57 +70+ 6 +142+ 6 +35 + 15 +14+ 29 +77+ 22 +20+ 5 +110 +1 = 888 Bytes  => 1000 Bytes
  /*mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //reset_time_counters value
    String("{\"name\":\"Device  : Reset Time Counters\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\""+mqttTopicName+
            "/cmnd/RELAY\",\"payload_press\":\"RESET_TIME_COUNTERS\",\"uniq_id\":\""+deviceSufix+"_reset_time_counters\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+
            device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+
            String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"dev_cla\":\"restart\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['reset_time_counters']}}\"}").c_str()); //Discovery message for Reboot value, not retain in the broker*/
  
  
  //Device, Environment, GAS, Signals, System (Counters), WiFi
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterPreviousYear/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Heater Previous Year: \",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterPreviousYear\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterPreviousYear']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else {
    if (updateHADiscovery) {mqttClient.publish(bufferTopicHAName, 0, false);} //Send topic with no payload to publish a new year in the name
    mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  }
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnPreviousYear/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater Previous Year:      \",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnPreviousYear\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYear']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else {
    if (updateHADiscovery) {mqttClient.publish(bufferTopicHAName, 0, false);} //Send topic with no payload to publish a new year in the name
    mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  }

  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnPreviousYearJan/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 01\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnPreviousYearJan\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearJan']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnPreviousYearFeb/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 02\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnPreviousYearFeb\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearFeb']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnPreviousYearMar/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 03\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnPreviousYearMar\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearMar']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnPreviousYearApr/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 04\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnPreviousYearApr\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearApr']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnPreviousYearMay/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 05\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnPreviousYearMay\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearMay']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnPreviousYearJun/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 06\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnPreviousYearJun\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearJun']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnPreviousYearJul/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 07\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnPreviousYearJul\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearJul']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnPreviousYearAug/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 08\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnPreviousYearAug\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearAug']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnPreviousYearSep/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 09\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnPreviousYearSep\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearSep']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnPreviousYearOct/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 10\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnPreviousYearOct\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearOct']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnPreviousYearNov/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 11\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnPreviousYearNov\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearNov']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnPreviousYearDec/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 12\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnPreviousYearDec\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnPreviousYearDec']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  

  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterYear/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Heater Current Year: \",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterYear\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterYear']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else {
    if (updateHADiscovery) {mqttClient.publish(bufferTopicHAName, 0, false);} //Send topic with no payload to publish a new year in the name
    mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  }
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnYear/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater Current Year:      \",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnYear\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYear']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else {
    if (updateHADiscovery) {mqttClient.publish(bufferTopicHAName, 0, false);} //Send topic with no payload to publish a new year in the name
    mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  }
  
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnYearJan/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 01\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnYearJan\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearJan']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnYearFeb/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 02\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnYearFeb\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearFeb']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnYearMar/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 03\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnYearMar\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearMar']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnYearApr/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 04\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnYearApr\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearApr']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnYearMay/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 05\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnYearMay\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearMay']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnYearJun/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 06\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnYearJun\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearJun']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnYearJul/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 07\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnYearJul\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearJul']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnYearAug/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 08\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnYearAug\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearAug']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnYearSep/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 09\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnYearSep\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearSep']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnYearOct/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 10\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnYearOct\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearOct']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnYearNov/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 11\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnYearNov\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearNov']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnYearDec/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:     Month 12\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnYearDec\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYearDec']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnYesterday/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater:  Yesterday\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnYesterday\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnYesterday']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSensorTopicHAPrefixName,"_heaterOnToday/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Time Heater: Today\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_heaterOnToday\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"unit_of_meas\":\"s\",\"dev_cla\":\"duration\",\"ic\":\"mdi:calendar-clock-outline\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['heaterOnToday']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
} //mqttClientPublishHADiscovery_heaterTimeOnObjects

void mqttClientPublishHADiscovery_binaryObjects(String mqttTopicName, String device, String ipAddress, bool removeTopics) {
  //Publish Home Assistant Discovery messages
  struct tm nowTimeInfo; //36 B
  char s[100];
  getLocalTime(&nowTimeInfo);strftime(s,sizeof(s),"%Y-%m-%dT%H:%M:%S",&nowTimeInfo); //Time in format 2024-08-24T07:56:25
  
  //For every sample value, a Discovery Message must be published
  //HA Discovery topic format is as follows: <discovery_prefix>/<component>/[<node_id>]/<object_id>/config
  // In the following exmaple:
  //  <discovery_prefix> = homeassistant
  //  <component> = sensor or switch (relay)
  //  [<node_id>] = boiler-relay-controlv2-E02940
  //  <object_id> = E02940_<SAMPLE_NAME> = E02940_H2, E02940_CO, etc.
  //i.e.: device=boiler-relay-controlv2-E02940                                                                    -      29 Bytes                                       =>  50 Bytes
  //      deviceSufix=E02940                                                                                      -       6 Bytes                                       =>  10 Bytes
  //      mqttTopicName=the-iot-factory/boiler-relay-controlv2-E02940                                             -      70 Bytes                                       => 100 Bytes
  //      mqttSensorTopicHAPrefixName=homeassistant/sensor/boiler-relay-controlv2-E02940/E02940                   -      58 = 57+1 Bytes                                => 100 Bytes
  //      mqttSensorTopicHAName=homeassistant/sensor/boiler-relay-controlv2-E02940/E02940_<SAMPLE_NAME>/config    -     125 = 57+7+60+1 Bytes   <SAMPLE_NAME> ~ 60 Byte => 124 Bytes
  //Sensors
  //mdi:gas-burner,air-filter, mdi:clock-time-eight \"ic\":\"mdi:calendar-clock\",

  
  // 70+ 57 +23+ 57 +72+ 57 +70+ 6 +142+ 6 +35 + 15 +14+ 29 +77+ 22 +20+ 5 +110 +1 = 888 Bytes  => 1000 Bytes
  /*mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //reset_time_counters value
    String("{\"name\":\"Device  : Reset Time Counters\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\""+mqttTopicName+
            "/cmnd/RELAY\",\"payload_press\":\"RESET_TIME_COUNTERS\",\"uniq_id\":\""+deviceSufix+"_reset_time_counters\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+
            device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+
            String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"dev_cla\":\"restart\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['reset_time_counters']}}\"}").c_str()); //Discovery message for Reboot value, not retain in the broker*/
  
  
  //Device, Environment, GAS, Signals, System (Counters), WiFi
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttBinarySensorTopicHAPrefixName,"_Clean_air/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Environment: Gas Detection\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_Clean_air\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"dev_cla\":\"gas\",\"ic\":\"mdi:meter-gas\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['Clean_air']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttBinarySensorTopicHAPrefixName,"_boilerOn/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Environment: Boiler Flame\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerOn\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"ic\":\"",iconThermStatus.c_str(),"\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerOn']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttBinarySensorTopicHAPrefixName,"_boilerStatus/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Environment: Boiler Active\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerStatus\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"ic\":\"",iconThermStatus.c_str(),"\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerStatus']}}\"}");
  //memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Environment: Boiler Active\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_boilerStatus\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"icon_template\":\">{% if is_state('binary_sensor.boiler_relay_controlv2_2254c4_environment_boiler_active', 'off') %} mdi:water-boiler {% elif is_state('binary_sensor.boiler_relay_controlv2_2254c4_environment_boiler_flame', 'on') %} mdi:water-boiler-alert {% elif is_state('binary_sensor.boiler_relay_controlv2_2254c4_environment_heater_flame', 'on') %} mdi:water-boiler-alert {% else %}mdi:water-boiler{% endif %}\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['boilerStatus']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttBinarySensorTopicHAPrefixName,"_Thermostate_on/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Environment: Heater Flame\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_Thermostate_on\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"ic\":\"",iconThermStatus.c_str(),"\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['Thermostate_on']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttBinarySensorTopicHAPrefixName,"_Thermostate_status/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Environment: Heater Active\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_Thermostate_status\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"ic\":\"",iconThermStatus.c_str(),"\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['Thermostate_status']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttBinarySensorTopicHAPrefixName,"_GAS_interrupt/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Signal: Gas Sensor\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_GAS_interrupt\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"ic\":\"",iconGasInterrupt.c_str(),"\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['GAS_interrupt']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttBinarySensorTopicHAPrefixName,"_Thermostate_interrupt/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Signal: Therm. Sensor\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_Thermostate_interrupt\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"ic\":\"",iconThermInterrupt.c_str(),"\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['Thermostate_interrupt']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttBinarySensorTopicHAPrefixName,"_NTP/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"WiFi: Sync NTP\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_NTP\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"ic\":\"mdi:cloud-clock\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['NTP']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttBinarySensorTopicHAPrefixName,"_HTTP_CLOUD/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"WiFi: Sync HTTP Cloud\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"",bufferDeviceSufix,"_HTTP_CLOUD\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"ic\":\"mdi:cloud-upload\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['HTTP_CLOUD']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
} //mqttClientPublishHADiscovery_binaryObjects

void mqttClientPublishHADiscovery_relayObjects(String mqttTopicName, String device, String ipAddress, bool removeTopics) {
  //Publish Home Assistant Discovery messages
  struct tm nowTimeInfo; //36 B
  char s[100];
  getLocalTime(&nowTimeInfo);strftime(s,sizeof(s),"%Y-%m-%dT%H:%M:%S",&nowTimeInfo); //Time in format 2024-08-24T07:56:25
  
  //For every sample value, a Discovery Message must be published
  //HA Discovery topic format is as follows: <discovery_prefix>/<component>/[<node_id>]/<object_id>/config
  // In the following exmaple:
  //  <discovery_prefix> = homeassistant
  //  <component> = sensor or switch (relay)
  //  [<node_id>] = boiler-relay-controlv2-E02940
  //  <object_id> = E02940_<SAMPLE_NAME> = E02940_H2, E02940_CO, etc.
  //i.e.: device=boiler-relay-controlv2-E02940                                                                    -      29 Bytes                                       =>  50 Bytes
  //      deviceSufix=E02940                                                                                      -       6 Bytes                                       =>  10 Bytes
  //      mqttTopicName=the-iot-factory/boiler-relay-controlv2-E02940                                             -      70 Bytes                                       => 100 Bytes
  //      mqttSensorTopicHAPrefixName=homeassistant/sensor/boiler-relay-controlv2-E02940/E02940                   -      58 = 57+1 Bytes                                => 100 Bytes
  //      mqttSensorTopicHAName=homeassistant/sensor/boiler-relay-controlv2-E02940/E02940_<SAMPLE_NAME>/config    -     125 = 57+7+60+1 Bytes   <SAMPLE_NAME> ~ 60 Byte => 124 Bytes
  //Sensors
  //mdi:gas-burner,air-filter, mdi:clock-time-eight \"ic\":\"mdi:calendar-clock\",

  
  // 70+ 57 +23+ 57 +72+ 57 +70+ 6 +142+ 6 +35 + 15 +14+ 29 +77+ 22 +20+ 5 +110 +1 = 888 Bytes  => 1000 Bytes
  /*mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //reset_time_counters value
    String("{\"name\":\"Device  : Reset Time Counters\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\"",bufferMqttTopicName,
            "/cmnd/RELAY\",\"payload_press\":\"RESET_TIME_COUNTERS\",\"uniq_id\":\""+deviceSufix+"_reset_time_counters\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+
            device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+
            String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"dev_cla\":\"restart\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['reset_time_counters']}}\"}").c_str()); //Discovery message for Reboot value, not retain in the broker*/
  
  
  //Device, Environment, GAS, Signals, System (Counters), WiFi
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSwitchTopicHAPrefixName,"_Relay1/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Relay: Allow Ext. Therm.\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\"",bufferMqttTopicName,"/cmnd/RELAY\",\"pl_off\":\"R1_OFF\",\"pl_on\":\"R1_ON\",\"uniq_id\":\"",bufferDeviceSufix,"_Relay1\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['Relay1']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSwitchTopicHAPrefixName,"_Relay2/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Relay: Force Heater\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\"",bufferMqttTopicName,"/cmnd/RELAY\",\"pl_off\":\"R2_OFF\",\"pl_on\":\"R2_ON\",\"uniq_id\":\"",bufferDeviceSufix,"_Relay2\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['Relay2']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);

  //Debug relay objects
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSwitchTopicHAPrefixName,"_debugModeOn/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Logs: Debug Mode\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\"",bufferMqttTopicName,"/cmnd/RELAY\",\"pl_off\":\"DEBUG_OFF\",\"pl_on\":\"DEBUG_ON\",\"uniq_id\":\"",bufferDeviceSufix,"_debugModeOn\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['debugModeOn']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSwitchTopicHAPrefixName,"_serialLogsOn/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Logs: Serial\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\"",bufferMqttTopicName,"/cmnd/RELAY\",\"pl_off\":\"SERIAL_LOGS_OFF\",\"pl_on\":\"SERIAL_LOGS_ON\",\"uniq_id\":\"",bufferDeviceSufix,"_serialLogsOn\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['serialLogsOn']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSwitchTopicHAPrefixName,"_sysLogsOn/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Logs: Syslog\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\"",bufferMqttTopicName,"/cmnd/RELAY\",\"pl_off\":\"SYS_LOGS_OFF\",\"pl_on\":\"SYS_LOGS_ON\",\"uniq_id\":\"",bufferDeviceSufix,"_sysLogsOn\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['sysLogsOn']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttSwitchTopicHAPrefixName,"_webLogsOn/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Logs: Web\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\"",bufferMqttTopicName,"/cmnd/RELAY\",\"pl_off\":\"WEB_LOGS_OFF\",\"pl_on\":\"WEB_LOGS_ON\",\"uniq_id\":\"",bufferDeviceSufix,"_webLogsOn\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['webLogsOn']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
} //mqttClientPublishHADiscovery_relayObjects

void mqttClientPublishHADiscovery_buttonObjects(String mqttTopicName, String device, String ipAddress, bool removeTopics) {
  //Publish Home Assistant Discovery messages
  struct tm nowTimeInfo; //36 B
  char s[100];
  getLocalTime(&nowTimeInfo);strftime(s,sizeof(s),"%Y-%m-%dT%H:%M:%S",&nowTimeInfo); //Time in format 2024-08-24T07:56:25
  
  //For every sample value, a Discovery Message must be published
  //HA Discovery topic format is as follows: <discovery_prefix>/<component>/[<node_id>]/<object_id>/config
  // In the following exmaple:
  //  <discovery_prefix> = homeassistant
  //  <component> = sensor or switch (relay)
  //  [<node_id>] = boiler-relay-controlv2-E02940
  //  <object_id> = E02940_<SAMPLE_NAME> = E02940_H2, E02940_CO, etc.
  //i.e.: device=boiler-relay-controlv2-E02940                                                                    -      29 Bytes                                       =>  50 Bytes
  //      deviceSufix=E02940                                                                                      -       6 Bytes                                       =>  10 Bytes
  //      mqttTopicName=the-iot-factory/boiler-relay-controlv2-E02940                                             -      70 Bytes                                       => 100 Bytes
  //      mqttSensorTopicHAPrefixName=homeassistant/sensor/boiler-relay-controlv2-E02940/E02940                   -      58 = 57+1 Bytes                                => 100 Bytes
  //      mqttSensorTopicHAName=homeassistant/sensor/boiler-relay-controlv2-E02940/E02940_<SAMPLE_NAME>/config    -     125 = 57+7+60+1 Bytes   <SAMPLE_NAME> ~ 60 Byte => 124 Bytes
  //Sensors
  //mdi:gas-burner,air-filter, mdi:clock-time-eight \"ic\":\"mdi:calendar-clock\",

  
  // 70+ 57 +23+ 57 +72+ 57 +70+ 6 +142+ 6 +35 + 15 +14+ 29 +77+ 22 +20+ 5 +110 +1 = 888 Bytes  => 1000 Bytes
  /*mqttClient.publish(String(mqttSensorTopicHAName).c_str(), 0, false, //reset_time_counters value
    String("{\"name\":\"Device  : Reset Time Counters\",\"stat_t\":\""+mqttTopicName+"/SENSOR\",\"avty_t\":\""+mqttTopicName+"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\"",bufferMqttTopicName,
            "/cmnd/RELAY\",\"payload_press\":\"RESET_TIME_COUNTERS\",\"uniq_id\":\""+deviceSufix+"_reset_time_counters\",\"dev\":{\"ids\":[\""+deviceSufix+"\"],\"configuration_url\":\"http://"+ipAddress+"\",\"name\":\""+
            device+"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\""+
            String(DEVICE_NAME_PREFIX)+"\",\"sw_version\":\""+String(VERSION)+"\"},\"dev_cla\":\"restart\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['reset_time_counters']}}\"}").c_str()); //Discovery message for Reboot value, not retain in the broker*/
  
  
  //Device, Environment, GAS, Signals, System (Counters), WiFi
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttButtonTopicHAPrefixName,"_reboot/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device  : Reboot\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\"",bufferMqttTopicName,"/cmnd/RELAY\",\"payload_press\":\"REBOOT\",\"uniq_id\":\"",bufferDeviceSufix,"_reboot\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"dev_cla\":\"restart\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['reboot']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
  memset(bufferTopicHAName,'\0',sizeof(bufferTopicHAName));sprintf(bufferTopicHAName,"%s%s",bufferMqttButtonTopicHAPrefixName,"_reset_time_counters/config");
  memset(bufferPayload,'\0',sizeof(bufferPayload));sprintf(bufferPayload,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","{\"name\":\"Device  : Reset Time Counters\",\"stat_t\":\"",bufferMqttTopicName,"/SENSOR\",\"avty_t\":\"",bufferMqttTopicName,"/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"cmd_t\":\"",bufferMqttTopicName,"/cmnd/RELAY\",\"payload_press\":\"RESET_TIME_COUNTERS\",\"uniq_id\":\"",bufferDeviceSufix,"_reset_time_counters\",\"dev\":{\"ids\":[\"",bufferDeviceSufix,"\"],\"configuration_url\":\"http://",bufferIpAddress,"\",\"name\":\"",bufferDevice,"\",\"manufacturer\":\"The IoT Factory - www.the-iotfactory.com\",\"model\":\"",DEVICE_NAME_PREFIX,"\",\"sw_version\":\"",VERSION,"\"},\"dev_cla\":\"restart\",\"frc_upd\":true,\"val_tpl\":\"{{value_json['SAMPLES']['reset_time_counters']}}\"}");
  if (removeTopics) mqttClient.publish(bufferTopicHAName, 0, false); //Send topic with no payload
  else mqttClient.publish(bufferTopicHAName, 0, false, bufferPayload);
} //mqttClientPublishHADiscovery_buttonObjects