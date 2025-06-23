// Get current sensor readings when the page loads
window.addEventListener('load', getReadings);

// Function to get current readings on the webpage when it loads for the first time
function getReadings(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      updateSite(JSON.parse(this.responseText));
    }
  }; 
  xhr.open("GET", "/samples", true);
  xhr.send();
}

function updateSite(myObj) {
  //var myObj = JSON.parse(e.data);
  let val="";
  console.log(myObj);

  document.getElementById("TEMPERATURE_id").innerHTML=String((myObj.temperature).toFixed(1))+" ºC";
  document.getElementById("HUMIDITY_id").innerHTML=String((myObj.humidity).toFixed(1))+" %";
  document.getElementById("SSID_id").innerHTML=myObj.SSID;
  document.getElementById("BSSID_id").innerHTML=myObj.BSSID;
  document.getElementById("RSSI_id").innerHTML=String(myObj.SIGNAL)+" dBm ("+String(myObj.RSSI)+"%)";
  document.getElementById("WIFICHANNEL_id").innerHTML=myObj.CHANNEL;
  document.getElementById("DEVICENAME_id").innerHTML=myObj.device_name;
  document.getElementById("FIRMWAREVERSION_id").innerHTML=String(myObj.version);
  document.getElementById("DATE_id").innerHTML=myObj.dateUpdate;
  document.getElementById("UPTIMEDATE_id").innerHTML=myObj.startTime;
  document.getElementById("UPTIME_id").innerHTML=myObj.upTime;
  document.getElementById("TIMEZONENAME_id").innerHTML=myObj.TZName; 
  document.getElementById("TIMEZONE_id").innerHTML=myObj.TZEnvVariable;  
  document.getElementById("SENSORTEMPHUM_id").innerHTML=myObj.tempHumSensorType; 

  document.getElementById("MACADDRESS_id").innerHTML=myObj.macAddress;  
  document.getElementById("IPADDRESS_id").innerHTML=myObj.ipAddress;
  document.getElementById("MASK_id").innerHTML=myObj.netMask; 
  document.getElementById("DEFAULTGW_id").innerHTML=myObj.defaultGW; 
  if (myObj.NTP=="ON") val="Synced"; else val="N/A";document.getElementById("NTPSTATUS_id").innerHTML=val;
  document.getElementById("NTPSTATUS2_id").innerHTML=val;
  document.getElementById("NTPSERVER_id").innerHTML=myObj.ntpServer; 
  document.getElementById("WIFISTATUS_id").innerHTML="Enabled";
  document.getElementById("WIFICURRENTSTATUS_id").innerHTML="On, "+String(myObj.RSSI)+"%";
  document.getElementById("BLUETOOTHSTATUS_id").innerHTML="Disabled";
  document.getElementById("BLUETOOTHCURRENTSTATUS_id").innerHTML="N/E";

  if (myObj.HTTP_CLOUD=="ON") val="Enabled"; else val="Disabled";document.getElementById("CLOUDSERVICESSTATUS_id").innerHTML=val;
  switch (myObj.CloudSyncCurrentStatus) {
    case 0: val="Up"; break;case 1: val="Updating"; break;case 2: val="Down"; break;
  } document.getElementById("CLOUDSERVICESCURRENTSTATUS_id").innerHTML=val;
  document.getElementById("CLOUDSERVICESURL_id").innerHTML=myObj.cloudServicesURL;
  if (myObj.mqttServerEnabled) val="Enabled"; else val="Disabled";document.getElementById("MQTTSERVICESSTATUS_id").innerHTML=val;
  switch(myObj.MqttSyncCurrentStatus) {
    case 0: val="Up"; break;case 1: val="Updating"; break;case 2: val="Down"; break;
  } document.getElementById("MQTTSERVICESCURRENTSTATUS_id").innerHTML=val;
  if (myObj.secureMqttEnabled)  val="Enabled"; else val="Disabled"; document.getElementById("Secure_MQTTSERVICESSTATUS_id").innerHTML=val;
  document.getElementById("MQTTSERVER_id").innerHTML=myObj.mqttServer; 
  document.getElementById("MQTTTOPICNAME_id").innerHTML=myObj.mqttTopicName; 
}