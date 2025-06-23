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

  if (myObj.httpCloudEnabled) document.getElementById("Cloud_on_id").checked=true; else document.getElementById("Cloud_on_id").checked=false;
  if (myObj.httpCloudEnabled) document.getElementById("Cloud_off_id").checked=false; else document.getElementById("Cloud_on_id").checked=true;

  if (myObj.SITE=="") val="NO SITE DEFINED"; else val=myObj.SITE;document.getElementById("SITE_ALLOW_NAME_id").innerHTML=val;
  if (myObj.SITE_BK1=="") val="NO SITE DEFINED"; else val=myObj.SITE_BK1;document.getElementById("SITE_BK1_ALLOW_NAME_id").innerHTML=val;
  if (myObj.SITE_BK2=="") val="NO SITE DEFINED"; else val=myObj.SITE_BK2;document.getElementById("SITE_BK2_ALLOW_NAME_id").innerHTML=val;
  
  if (myObj.SITE=="") document.getElementById("SiteAllowed_id").disabled=true; else if (myObj.SITE_ALLOWED) document.getElementById("SiteAllowed_id").checked=true;
  if (myObj.SITE_BK1=="") document.getElementById("SiteBk1Allowed_id").disabled=true; else if (myObj.SITE_BK1_ALLOWED) document.getElementById("SiteBk1Allowed_id").checked=true;
  if (myObj.SITE_BK2=="") document.getElementById("SiteBk2Allowed_id").disabled=true; else if (myObj.SITE_BK2_ALLOWED) document.getElementById("SiteBk2Allowed_id").checked=true;

  document.getElementById("MQTTSERVER_id").placeholder=myObj.mqttServer;document.getElementById("MQTTSERVER_id").value=myObj.mqttServer;
  document.getElementById("MQTTTOPIC_id").placeholder=myObj.mqttTopicName.split("/")[0]+"/";document.getElementById("MQTTTOPIC_id").value=myObj.mqttTopicName.split("/")[0]+"/";
  if (myObj.mqttServerEnabled) {document.getElementById("MQTTSERVER_id").required=true;document.getElementById("MQTTTOPIC_id").required=true;} else {document.getElementById("MQTTSERVER_id").required=false;document.getElementById("MQTTTOPIC_id").required=false;}

  document.getElementById("MQTT_on_id").checked=myObj.mqttServerEnabled;document.getElementById("MQTT_off_id").checked=!myObj.mqttServerEnabled;
  document.getElementById("Secure_MQTT_on_id").checked=myObj.secureMqttEnabled;document.getElementById("Secure_MQTT_off_id").checked=!myObj.secureMqttEnabled;
  document.getElementById("MQTT_Power_on_id").checked=myObj.powerMeasureEnabled;document.getElementById("MQTT_Power_off_id").checked=!myObj.powerMeasureEnabled;
  if (!myObj.mqttServerEnabled) {document.getElementById("MQTT_Power_on_id").disabled=true;document.getElementById("MQTT_Power_off_id").disabled=true;}

  document.getElementById("MQTTPOWERTOPIC_id").placeholder=myObj.powerMqttTopic;
  document.getElementById("MQTTPOWERTOPIC_id").value=myObj.powerMqttTopic;
  if (myObj.powerMeasureEnabled) document.getElementById("MQTTPOWERTOPIC_id").required=true;
  if (!myObj.mqttServerEnabled ) document.getElementById("MQTTPOWERTOPIC_id").disabled=true;
  
  document.getElementById("POWERTHRESHHOLD_id").placeholder=myObj.powerOnFlameThreshold;document.getElementById("POWERTHRESHHOLD_id").value=myObj.powerOnFlameThreshold;
  if (myObj.powerMeasureEnabled) document.getElementById("POWERTHRESHHOLD_id").required=true;
  if (!myObj.mqttServerEnabled ) document.getElementById("POWERTHRESHHOLD_id").disabled=true;

  document.getElementById("MQTTUserName_id").placeholder=myObj.mqttUserName;document.getElementById("MQTTUserName_id").value=myObj.mqttUserName;
  document.getElementById("MQTTUserPssw_id").placeholder="User password";document.getElementById("MQTTUserPssw_id").value="**********";
}