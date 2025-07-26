// Get current sensor readings when the page loads
window.addEventListener('load', init);
var binAvailableSpace;
var spiffsAvailableSpace;
var jsonObject;

function init() { //To hide/show the help when the mouse pass over
  initTipEvents("WIFIERRORS_id", "WIFIERRORS_TIP_id");
  initTipEvents("CONNECTIVITYERRORS_id", "CONNECTIVITYERRORS_TIP_id");
  initTipEvents("SAMPLEUPDTERRORS_id", "SAMPLEUPDTERRORS_TIP_id");
  initTipEvents("MQTTERRORS_id", "MQTTERRORS_TIP_id");
  initTipEvents("NTPERRORS_id", "NTPERRORS_TIP_id");
  initTipEvents("WEBSERVERERRORS_id", "WEBSERVERERRORS_TIP_id");
  initTipEvents("JSONERRORS_id", "JSONERRORS_TIP_id");

  getReadings(); //To get the JSON objects
}
function initTipEvents(elementId, tipId) {
var el = document.getElementById(elementId),
    tip = document.getElementById(tipId),
    showHandler = showTip(tip),
    hideHandler = hideTip(el, tip);

el.onmouseover = showHandler;
el.onfocus = showHandler;
el.onmouseout = hideHandler;
el.onblur = hideHandler;
tip.style.display = "none";
}
function showTip (tipElement) {
  return function () {
      tipElement.style.display = "inline";
  };
}
function hideTip (element, tipElement) {
  return function () {
      if (document.activeElement !== element) {
          tipElement.style.display = "none";
      }
  };
}
// Function to get current readings on the webpage when it loads for the first time
function getReadings(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      jsonObject=JSON.parse(this.responseText);
      updateSite(jsonObject);
    }
  }; 
  xhr.open("GET", "/samples", true);
  xhr.send();
}

//File uploadings stuff
function showHideCheck() {

  if (document.getElementById('select_file_id').value!="") {
      document.getElementById('select_file_id').style.visibility = 'hidden';
      document.getElementById('prg').style.visibility = 'visible';
      document.getElementById('file-upload_id').style.visibility = 'visible';
      document.getElementById('OTA_firmware_id').disabled=true;
      document.getElementById('OTA_system_file_id').disabled=true;
  } else {
    document.getElementById('select_file_id').style.visibility = 'visible';
    document.getElementById('file-upload_id').style.visibility = 'hidden';
    document.getElementById('prg').style.visibility = 'hidden';
    document.getElementById('OTA_firmware_id').disabled=false;
    document.getElementById('OTA_system_file_id').disabled=false;
  }
}

document.getElementById("select_file_id").addEventListener("change", function(){
  var files=document.getElementById("select_file_id").files[0];
  var filename = files.name;

  /* getting file extenstion eg- .jpg,.png, etc */
  var extension = filename.substr(filename.lastIndexOf("."));
  /* define allowed file types */
  //var allowedExtensionsRegx = /(\.bin|\.bin.gz)$/i;
  var allowedExtensionsRegx = /(\.bin)$/i;
  /* testing extension with regular expression */
  var isAllowed = allowedExtensionsRegx.test(extension);
  if(isAllowed){
      /* file upload logic goes here... */
      document.getElementById("prg").innerHTML = filename+" ("+files.size+" B)";
      
      //Binary firmware file
      if (document.getElementById('OTA_firmware_id').checked) {
        if ((binAvailableSpace-files.size)<0) {
          alert("Binary File greater than available space ("+binAvailableSpace+" B)");
          return false;  
        }
      }
      //SPIFFS system file
      else {
        if ((spiffsAvailableSpace-files.size)<0) {
          alert("System File greater than available space ("+spiffsAvailableSpace+" B)");
          return false;  
        }
      }
      showHideCheck();
  }else{
      alert("Only file extension .bin is allowed to be uploaded for either Binary or File Systems.");
      return false;
  }
});

var fileForm=document.getElementById("upload_form");
fileForm.addEventListener("formdata", ({ formData }) => {
  const file = formData.get("select_file");
  formData.delete("select_file");
  if (document.getElementById('OTA_firmware_id').checked) formData.append("select_file", file, "firmware.bin");
  else formData.append("select_file", file, "filesystem.bin");
});


//Update of web fields
function updateSite(myObj) {
  //var myObj = JSON.parse(e.data);
  let debugModeOn=false; if (myObj.debugModeOn=="DEBUG_ON") debugModeOn=true;
  let serialLogsOn=false; if (myObj.serialLogsOn=="SERIAL_LOGS_ON") serialLogsOn=true;
  let webLogsOn=false; if (myObj.webLogsOn=="WEB_LOGS_ON") webLogsOn=true;
  let sysLogsOn=false; if (myObj.sysLogsOn=="SYS_LOGS_ON") sysLogsOn=true;
  console.log(myObj);

  document.getElementById("OTA_firmware_id").disabled=!myObj.OTAUpgradeBinAllowed;
  document.getElementById("OTA_system_file_id").disabled=!myObj.SPIFFSUpgradeBinAllowed;
  document.getElementById("OTA_system_file_id").checked=myObj.SPIFFSUpgradeBinAllowed;
  document.getElementById("select_file_id").disabled=(!myObj.OTAUpgradeBinAllowed && !myObj.SPIFFSUpgradeBinAllowed);
  
  document.getElementById("debug_mode_on_id").checked=debugModeOn;document.getElementById("debug_mode_off_id").checked=!debugModeOn;
  document.getElementById("serial_logs_on_id").checked=serialLogsOn;document.getElementById("serial_logs_off_id").checked=!serialLogsOn;
  document.getElementById("console_logs_on_id").checked=webLogsOn;document.getElementById("console_logs_off_id").checked=!webLogsOn;
  document.getElementById("syslogs_on_id").checked=sysLogsOn;document.getElementById("syslogs_off_id").checked=!sysLogsOn;
  document.getElementById("SYSLOGSERVER_id").placeholder=myObj.sysLogServer;document.getElementById("SYSLOGPORT_id").placeholder=myObj.sysLogServerUDPPort;
  document.getElementById("SYSLOGSERVER_id").value=myObj.sysLogServer;document.getElementById("SYSLOGPORT_id").value=myObj.sysLogServerUDPPort;
  if (debugModeOn) {
    if (sysLogsOn) {
      document.getElementById("SYSLOGSERVER_id").required=true;document.getElementById("SYSLOGPORT_id").required=true;
      document.getElementById("SYSLOGSERVER_id").disabled=false;document.getElementById("SYSLOGPORT_id").disabled=false;
    } else {
      document.getElementById("SYSLOGSERVER_id").required=false;document.getElementById("SYSLOGPORT_id").required=false;
      document.getElementById("SYSLOGSERVER_id").disabled=true;document.getElementById("SYSLOGPORT_id").disabled=true;  
    }
    document.getElementById("serial_logs_on_id").disabled=false;document.getElementById("serial_logs_off_id").disabled=false;
    document.getElementById("console_logs_on_id").disabled=false;document.getElementById("console_logs_off_id").disabled=false;
    document.getElementById("syslogs_on_id").disabled=false;document.getElementById("syslogs_off_id").disabled=false;
    document.getElementById("SYSLOGSERVER_id").disabled=false;document.getElementById("SYSLOGPORT_id").disabled=false;
  } else {
    document.getElementById("serial_logs_on_id").disabled=true;document.getElementById("serial_logs_off_id").disabled=true;
    document.getElementById("console_logs_on_id").disabled=true;document.getElementById("console_logs_off_id").disabled=true;
    document.getElementById("syslogs_on_id").disabled=true;document.getElementById("syslogs_off_id").disabled=true;
    document.getElementById("SYSLOGSERVER_id").disabled=true;document.getElementById("SYSLOGPORT_id").disabled=true;
  }

  document.getElementById("LASTRESETREASON_id").innerHTML=String("0x")+myObj.resetReason.toString(16);
  document.getElementById("BOOTCOUNT_id").innerHTML=myObj.bootCount;
  document.getElementById("NORMALBOOTCOUNT_id").innerHTML=myObj.bootCount-myObj.resetSWCount-myObj.resetCount;
  document.getElementById("SWRESETCOUNT_id").innerHTML=myObj.resetSWCount;
  document.getElementById("SWRESETWEBCOUNT_id").innerHTML=myObj.resetSWWebCount;
  document.getElementById("SWRESETMQTTCOUNT_id").innerHTML=myObj.resetSWMqttCount;
  document.getElementById("SWRESETFIRMWARECOUNT_id").innerHTML=myObj.resetSWUpgradeCount;
  document.getElementById("WEBSERVERRESETCOUNT_id").innerHTML=myObj.resetWebServerCnt;
  document.getElementById("PREVENTIVERESETCOUNT_id").innerHTML=myObj.resetPreventiveCount+myObj.resetPreventiveWebServerCount+myObj.resetPreventiveJSONCount;
  document.getElementById("PREVENTIVEABSOLUTERESETCOUNT_id").innerHTML=myObj.resetPreventiveCount;
  document.getElementById("PREVENTIVEWEBSERVERRESETCOUNT_id").innerHTML=myObj.resetPreventiveWebServerCount;
  document.getElementById("PREVENTIVEJSONRESETCOUNT_id").innerHTML=myObj.resetPreventiveJSONCount;
  document.getElementById("UNCONTROLLEDRESETCOUNT_id").innerHTML=myObj.resetCount;
  document.getElementById("SETUPERRORS_id").innerHTML=String("0x")+myObj.error_setup.toString(16);
  document.getElementById("WIFIERRORS_id").innerHTML="Connection Errors: "+myObj.errorsWiFiCnt+"   ";
  document.getElementById("CONNECTIVITYERRORS_id").innerHTML="Connectivity Errors: "+myObj.errorsConnectivityCnt+"   ";
  document.getElementById("SAMPLEUPDTERRORS_id").innerHTML="Sample Update Errors: "+myObj.errorsHTTPUptsCnt+"   ";
  document.getElementById("MQTTERRORS_id").innerHTML="MQTT Errors: "+myObj.errorsMQTTCnt+"   ";
  document.getElementById("NTPERRORS_id").innerHTML="NTP Sync Errors: "+myObj.errorsNTPCnt+"   "; 
  document.getElementById("WEBSERVERERRORS_id").innerHTML="Web Server Errors: "+myObj.errorsWebServerCnt+"   ";
  document.getElementById("JSONERRORS_id").innerHTML="JSON Errors: "+myObj.errorsJSONCnt+"   ";
  document.getElementById("SPIFFSERRORS_id").innerHTML=myObj.SPIFFSErrors;
  document.getElementById("CURRENTHEAPSIZE_id").innerHTML=myObj.heapSize; 
  document.getElementById("MINMAXHEAPBLOCKSIZE_id").innerHTML=myObj.minMaxHeapBlockSizeSinceBoot; 
  document.getElementById("MINMAXHEAPBLOCKSIZEUPGRADE_id").innerHTML=myObj.minMaxHeapBlockSizeSinceUpgrade;
  document.getElementById("MINHEAPSIZE_id").innerHTML=myObj.minHeapSinceBoot;
  document.getElementById("MINHEAPSIZEUPGRADE_id").innerHTML=myObj.minHeapSinceUpgrade;
  binAvailableSpace=myObj.OTAAvailableSize;
  spiffsAvailableSpace=myObj.SPIFFSAvailableSize;
}

//Handle radio button checks
function handleRadioButtonClick(radioButton) {
  switch (radioButton.id) {
    case "debug_mode_on_id":
      document.getElementById("serial_logs_on_id").disabled=false;document.getElementById("serial_logs_off_id").disabled=false;
      document.getElementById("console_logs_on_id").disabled=false;document.getElementById("console_logs_off_id").disabled=false;
      document.getElementById("syslogs_on_id").disabled=false;document.getElementById("syslogs_off_id").disabled=false;
      if (document.getElementById("syslogs_on_id").checked) {document.getElementById("SYSLOGSERVER_id").disabled=false;document.getElementById("SYSLOGPORT_id").disabled=false;}
      else {document.getElementById("SYSLOGSERVER_id").disabled=true;document.getElementById("SYSLOGPORT_id").disabled=true;}
      break;
    case "debug_mode_off_id":
      document.getElementById("serial_logs_on_id").disabled=true;document.getElementById("serial_logs_off_id").disabled=true;
      document.getElementById("console_logs_on_id").disabled=true;document.getElementById("console_logs_off_id").disabled=true;
      document.getElementById("syslogs_on_id").disabled=true;document.getElementById("syslogs_off_id").disabled=true;
      document.getElementById("SYSLOGSERVER_id").disabled=true;document.getElementById("SYSLOGPORT_id").disabled=true;
      break;
    case "serial_logs_on_id":
      break;
    case "serial_logs_off_id":
      break;
    case "console_logs_on_id":
      break;
    case "console_logs_off_id":
      break;
    case "syslogs_on_id":
      document.getElementById("SYSLOGSERVER_id").disabled=false;document.getElementById("SYSLOGPORT_id").disabled=false;
      break;
    case "syslogs_off_id":
      document.getElementById("SYSLOGSERVER_id").disabled=true;document.getElementById("SYSLOGPORT_id").disabled=true;
      break;
    default:
      break;
  }
}

//Handle reset button check
function handleResetButton(resetButton) {
  switch (resetButton.id) {
    case "Button_Reset_id":
      let debugModeOn=false; if (jsonObject.debugModeOn=="DEBUG_ON") debugModeOn=true;
      let serialLogsOn=false; if (jsonObject.serialLogsOn=="SERIAL_LOGS_ON") serialLogsOn=true;
      let webLogsOn=false; if (jsonObject.webLogsOn=="WEB_LOGS_ON") webLogsOn=true;
      let sysLogsOn=false; if (jsonObject.sysLogsOn=="SYS_LOGS_ON") sysLogsOn=true;
      document.getElementById("debug_mode_on_id").checked=debugModeOn;document.getElementById("debug_mode_off_id").checked=!debugModeOn;
      document.getElementById("serial_logs_on_id").checked=serialLogsOn;document.getElementById("serial_logs_off_id").checked=!serialLogsOn;
      document.getElementById("console_logs_on_id").checked=webLogsOn;document.getElementById("console_logs_off_id").checked=!webLogsOn;
      document.getElementById("syslogs_on_id").checked=sysLogsOn;document.getElementById("syslogs_off_id").checked=!sysLogsOn;
      document.getElementById("SYSLOGSERVER_id").placeholder=jsonObject.sysLogServer;document.getElementById("SYSLOGPORT_id").placeholder=jsonObject.sysLogServerUDPPort;
      document.getElementById("SYSLOGSERVER_id").value=jsonObject.sysLogServer;document.getElementById("SYSLOGPORT_id").value=jsonObject.sysLogServerUDPPort;
    break;
  }
}