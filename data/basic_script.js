// Get current sensor readings when the page loads
window.addEventListener('load', getWpsw);

function getWpsw(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      updateWpsw(JSON.parse(this.responseText));
    }
  }; 
  xhr.open("GET", "/wpsw", true);
  xhr.send();
  getReadings();
}

function updateWpsw(myObj) {
  //var myObj = JSON.parse(e.data);
  let val="";
  console.log(myObj);
  
  document.getElementById("PSSW_id").value=myObj.PSSW;
  document.getElementById("PSSW_BK1_id").value=myObj.PSSW_BK1;
  document.getElementById("PSSW_BK2_id").value=myObj.PSSW_BK2;
}

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

  document.getElementById("Wifi_on_id").checked=true;document.getElementById("Wifi_off_id").checked=false;
  document.getElementById("UserName_id").placeholder=myObj.userName;document.getElementById("UserName_id").value=myObj.userName;
  document.getElementById("UserPssw_id").placeholder="User password";document.getElementById("UserPssw_id").value="**********";
  document.getElementById("SSID_id").placeholder="Mandatory if WiFi enabled";document.getElementById("SSID_id").value=myObj.SSID;document.getElementById("SSID_id").required=true;
  document.getElementById("PSSW_id").placeholder="Mandatory PSSW";
  document.getElementById("SITE_id").placeholder=myObj.SITE;document.getElementById("SITE_id").value=myObj.SITE;
  document.getElementById("SSID_BK1_id").placeholder=myObj.SSID_BK1;document.getElementById("SSID_BK1_id").value=myObj.SSID_BK1;
  if (myObj.PSSW_BK1==="") document.getElementById("PSSW_BK1_id").placeholder=""; else document.getElementById("PSSW_BK1_id").placeholder="**********";document.getElementById("PSSW_BK1_id").value=myObj.SSID_BK1;
  document.getElementById("SITE_BK1_id").placeholder=myObj.SITE_BK1;document.getElementById("SITE_BK1_id").value=myObj.SITE_BK1;
  document.getElementById("SSID_BK2_id").placeholder=myObj.SSID_BK2;document.getElementById("SSID_BK2_id").value=myObj.SSID_BK2;
  if (myObj.PSSW_BK2==="") document.getElementById("PSSW_BK2_id").placeholder=""; else document.getElementById("PSSW_BK2_id").placeholder="**********";document.getElementById("PSSW_BK2_id").value=myObj.SSID_BK2;
  document.getElementById("SITE_BK2_id").placeholder=myObj.SITE_BK2;document.getElementById("SITE_BK2_id").value=myObj.SITE_BK2;

  if (myObj.ntpServer=="") document.getElementById("NTP1_id").placeholder="Mandatory if WiFi enabled"; else document.getElementById("NTP1_id").placeholder=myObj.ntpServer1;document.getElementById("NTP1_id").value=myObj.ntpServer1;document.getElementById("NTP1_id").required=true;
  if (myObj.ntpServer=="") document.getElementById("NTP2_id").placeholder=myObj.ntpServer2;document.getElementById("NTP2_id").value=myObj.ntpServer2;
  if (myObj.ntpServer=="") document.getElementById("NTP3_id").placeholder=myObj.ntpServer3;document.getElementById("NTP3_id").value=myObj.ntpServer3;
  if (myObj.ntpServer=="") document.getElementById("NTP4_id").placeholder=myObj.ntpServer4;document.getElementById("NTP4_id").value=myObj.ntpServer4;
}