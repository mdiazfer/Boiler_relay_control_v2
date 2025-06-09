// Get current sensor readings when the page loads  
window.addEventListener('newSamples', getReadings);

// Create Temperature Gauge
var gaugeTemp = new LinearGauge({
  renderTo: 'gauge-temperature',
  width: 90,
  height: 627,
  units: "Temperature C",
  minValue: -10,
  maxValue: 60,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueDec: 2,
  valueInt: 2,
  majorTicks: [
    "-10",
    "-5",
    "0",
    "5",
    "10",
    "15",
    "20",
    "25",
    "30",
    "35",
    "40",
    "45",
    "50",
    "60"
  ],
  minorTicks: 5,
  strokeTicks: true,
  highlights: [
    {
      "from": 30,
      "to": 40,
      "color": "#f8af05" //Orange
    },
      {
          "from": 41,
          "to": 60,
          "color": "#f80505" //Red
      }
  ],
  //colorPlate: "#89BFFF",
  //colorPlate: "#fff",
  //colorPlate: "#34383b",
    colorPlate: "#43484c",
  //colorNumbers: "#ccc",
  colorNumbers: "#d5ca95",
  colorMajorTicks: "#d5ca95",
  colorMinorTicks: "#d5ca95",
  colorTitle: "#d5ca95",
  colorUnits: "#d5ca95",
  colorBarProgress: "#CC2936",
  //colorBarProgressEnd: "#15d2e0",
  //colorBarProgressEnd: "#00e4f5",
  colorBarProgressEnd: "#00ffff",
  borderShadowWidth: 0,
  borders: false,
  needleType: "arrow",
  needleWidth: 2,
  needleCircleSize: 7,
  needleCircleOuter: true,
  needleCircleInner: false,
  animationDuration: 1500,
  animationRule: "linear",
  barWidth: 10,
  //value: 37
}).draw();

// Create Temperature Gauge
var gaugeHum = new LinearGauge({
  renderTo: 'gauge-humidity',
  width: 90,
  height: 627,
  units: "Humidity (%)",
  minValue: 0,
  maxValue: 100,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueDec: 2,
  valueInt: 2,
  majorTicks: [
    "0",
    "20",
    "40",
    "60",
    "80",
    "100"
  ],
  minorTicks: 4,
  strokeTicks: true,
  highlights: [
      {
        "from": 80,
        "to": 100,
        "color": "#03C0C1"
      }
  ],
  //colorPlate: "#fff",
  colorPlate: "#5e4870",
      colorNumbers: "#d5ca95",
  colorMajorTicks: "#d5ca95",
    colorMinorTicks: "#d5ca95",
    colorTitle: "#d5ca95",
    colorUnits: "#d5ca95",
colorBarProgress: "#327ac0",
  colorBarProgressEnd: "#f5f5f5",
  borderShadowWidth: 0,
  borders: false,
  needleType: "arrow",
  needleWidth: 2,
  animationDuration: 1500,
  animationRule: "linear",
  tickSide: "left",
  numberSide: "left",
  needleSide: "left",
  barStrokeWidth: 4,
  barStrokeColor: "#5e4870",
  barBeginCircle: false,
  //value: 90
}).draw();	
  
// Create LPG Gauge
var gaugeLPG = new RadialGauge({
  renderTo: 'gauge-LPG',
  width: 110,
  height: 110,
  units: "LPG (ppm)",
  minValue: 0,
  maxValue: 16000,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueInt: 3,
  valueDec: 0,
  majorTicks: [
    "0",
    
    "2000",
    "4000",
    "6000",

    "8000",
    "10000",

    "12000",
    "14000",
    "16000"
  ],
  minorTicks: 4,
  strokeTicks: true,
  highlights: [
    { 
      "from": 0,
      "to": 200,
      "color": "#686b68", //Gray
    },
    {
      "from": 201,
      "to": 1500,
      "color": "#00FF44", //Green
    },
    {
          "from": 1501,
          "to": 5000,
          "color": "#FFEA00" //Yellow
      },
      {
        "from": 5001,
        "to": 10000,
        "color": "#FF6200" //Red
    },
    { 
      "from": 10001,
      "to": 16000,
      "color": "#686b68", //Gray
    }
  ],
  colorPlate: "#43484c",
  colorNumbers: "#ccc",
  colorTitle: "#ccc",
  colorUnits: "#ccc",
  borderShadowWidth: 0,
  borders: false,
  needleType: "line",
  colorNeedle: "#2edcff",
  colorNeedleEnd: "#2edcff",
  needleWidth: 2,
  needleCircleSize: 3,
  colorNeedleCircleOuter: "#2edcff",
  needleCircleOuter: true,
  needleCircleInner: false,
  animationDuration: 1500,
  animationRule: "linear"
}).draw();

// Create CO Gauge
var gaugeCO = new RadialGauge({
  renderTo: 'gauge-CO',
  width: 110,
  height: 110,
  units: "CO (ppm)",
  minValue: 0,
  maxValue: 16000,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueInt: 3,
  valueDec: 0,
  majorTicks: [
      "0",
      
      "2000",
      "4000",
      "6000",

      "8000",
      "10000",

      "12000",
      "14000",
      "16000"

  ],
  minorTicks: 4,
  strokeTicks: true,
  highlights: [
    { 
      "from": 0,
      "to": 200,
      "color": "#686b68", //Gray
    },
    {
      "from": 201,
      "to": 500,
      "color": "#00FF44", //Green
    },
    {
          "from": 501,
          "to": 900,
          "color": "#FFEA00" //Yellow
      },
      {
        "from": 901,
        "to": 10000,
        "color": "#FF6200" //Red
    },
    { 
      "from": 10001,
      "to": 16000,
      "color": "#686b68", //Gray
    }
  ],
  colorPlate: "#43484c",
  colorNumbers: "#ccc",
  colorTitle: "#ccc",
  colorUnits: "#ccc",
  borderShadowWidth: 0,
  borders: false,
  needleType: "line",
  colorNeedle: "#2edcff",
  colorNeedleEnd: "#2edcff",
  needleWidth: 2,
  needleCircleSize: 3,
  colorNeedleCircleOuter: "#2edcff",
  needleCircleOuter: true,
  needleCircleInner: false,
  animationDuration: 1500,
  animationRule: "linear"
}).draw();

// Create ALCOHOL Gauge
var gaugeALCOHOL = new RadialGauge({
  renderTo: 'gauge-ALCOHOL',
  width: 110,
  height: 110,
  units: "ALCOHOL (ppm)",
  minValue: 0,
  maxValue: 16000,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueInt: 3,
  valueDec: 0,
  majorTicks: [
      "0",
      
      "2000",
      "4000",
      "6000",

      "8000",
      "10000",

      "12000",
      "14000",
      "16000"

  ],
  minorTicks: 4,
  strokeTicks: true,
  highlights: [
    { 
      "from": 0,
      "to": 200,
      "color": "#686b68", //Gray
    },
    {
      "from": 201,
      "to": 1500,
      "color": "#00FF44", //Green
    },
    {
          "from": 1501,
          "to": 5000,
          "color": "#FFEA00" //Yellow
      },
      {
        "from": 5001,
        "to": 10000,
        "color": "#FF6200" //Red
    },
    { 
      "from": 10001,
      "to": 16000,
      "color": "#686b68", //Gray
    }
  ],
  colorPlate: "#43484c",
  colorNumbers: "#ccc",
  colorTitle: "#ccc",
  colorUnits: "#ccc",
  borderShadowWidth: 0,
  borders: false,
  needleType: "line",
  colorNeedle: "#2edcff",
  colorNeedleEnd: "#2edcff",
  needleWidth: 2,
  needleCircleSize: 3,
  colorNeedleCircleOuter: "#2edcff",
  needleCircleOuter: true,
  needleCircleInner: false,
  animationDuration: 1500,
  animationRule: "linear"
}).draw();

// Create CH4 Gauge
var gaugeCH4 = new RadialGauge({
  renderTo: 'gauge-CH4',
  width: 110,
  height: 110,
  units: "CH4 (ppm)",
  minValue: 0,
  maxValue: 16000,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueInt: 3,
  valueDec: 0,
  majorTicks: [
      "0",
      
      "2000",
      "4000",
      "6000",

      "8000",
      "10000",

      "12000",
      "14000",
      "16000"

  ],
  minorTicks: 4,
  strokeTicks: true,
  highlights: [
    { 
      "from": 0,
      "to": 200,
      "color": "#686b68", //Gray
    },
    {
      "from": 201,
      "to": 1500,
      "color": "#00FF44", //Green
    },
    {
          "from": 1501,
          "to": 5000,
          "color": "#FFEA00" //Yellow
      },
      {
        "from": 5001,
        "to": 10000,
        "color": "#FF6200" //Red
    },
    { 
      "from": 10001,
      "to": 16000,
      "color": "#686b68", //Gray
    }
  ],
  colorPlate: "#43484c",
  colorNumbers: "#ccc",
  colorTitle: "#ccc",
  colorUnits: "#ccc",
  borderShadowWidth: 0,
  borders: false,
  needleType: "line",
  colorNeedle: "#2edcff",
  colorNeedleEnd: "#2edcff",
  needleWidth: 2,
  needleCircleSize: 3,
  colorNeedleCircleOuter: "#2edcff",
  needleCircleOuter: true,
  needleCircleInner: false,
  animationDuration: 1500,
  animationRule: "linear"
}).draw();

// Create H2 Gauge
var gaugeH2 = new RadialGauge({
  renderTo: 'gauge-H2',
  width: 110,
  height: 110,
  units: "H2 (ppm)",
  minValue: 0,
  maxValue: 16000,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueInt: 3,
  valueDec: 0,
  majorTicks: [
      "0",
      
      "2000",
      "4000",
      "6000",

      "8000",
      "10000",

      "12000",
      "14000",
      "16000"
  ],
  minorTicks: 4,
  strokeTicks: true,
  highlights: [
    { 
      "from": 0,
      "to": 200,
      "color": "#686b68", //Gray
    },
    {
      "from": 201,
      "to": 1500,
      "color": "#00FF44", //Green
    },
    {
          "from": 1501,
          "to": 5000,
          "color": "#FFEA00" //Yellow
      },
      {
        "from": 5001,
        "to": 10000,
        "color": "#FF6200" //Red
    },
    { 
      "from": 10001,
      "to": 16000,
      "color": "#686b68", //Gray
    }
  ],
  colorPlate: "#43484c",
  colorNumbers: "#ccc",
  colorTitle: "#ccc",
  colorUnits: "#ccc",
  borderShadowWidth: 0,
  borders: false,
  needleType: "line",
  colorNeedle: "#2edcff",
  colorNeedleEnd: "#2edcff",
  needleWidth: 2,
  needleCircleSize: 3,
  colorNeedleCircleOuter: "#2edcff",
  needleCircleOuter: true,
  needleCircleInner: false,
  animationDuration: 1500,
  animationRule: "linear"
}).draw();

var dateUpdate;
var gasClear=true;
var boilerStatus=false;
var thermostatStatus=false;
var Relay1=true;
var Relay2=false;
var ipAddress="";

// Function to get current readings on the webpage when it loads for the first time
function getReadings(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var myObj = JSON.parse(this.responseText);
      console.log(myObj);
      var temp = myObj.temperature;
      var hum = myObj.humidity;
      var LPG = myObj.LPG;
      var CO = myObj.CO;
      var ALCOHOL = myObj.ALCOHOL;
      var CH4 = myObj.CH4;
      var H2 = myObj.H2;
      var date_Update=myObj.dateUpdate;
      gaugeTemp.value = temp;
      gaugeHum.value = hum;
      gaugeLPG.value = LPG;
      gaugeCO.value = CO;
      gaugeALCOHOL.value = ALCOHOL;
      gaugeCH4.value = CH4;
      gaugeH2.value = H2;
      dateUpdate=date_Update;
      Relay1=myObj.Relay1==="R1_ON"?true:false;
      Relay2=myObj.Relay2==="R2_ON"?true:false;
      ipAddress=myObj.ipAddress;
      document.getElementById("latestUpdate").innerHTML = dateUpdate;
      document.getElementById('thermostatSwitch').checked=Relay1;
      document.getElementById('forceSwitch').checked=Relay2;
    }
  }; 
  xhr.open("GET", "/samples", true);
  xhr.send();
}

function switchThermClicked(){
  let message="";
  if (document.getElementById('thermostatSwitch').checked) {  
    message="R1_ON"; //Send ON message to not allow external thermostat (relay ON position)
    console.log(document.getElementById('thermostatSwitch')+' is checked');
    document.getElementById('thermostatSwitch').checked=false; //Keep it false. It will change upon receiving the new_samples event
    
  }
  else {
    message="R1_OFF"; //Send OFF message to allow external thermostat (relay OFF position)
    console.log(document.getElementById('thermostatSwitch')+' is NOT checked');
    document.getElementById('thermostatSwitch').checked=true; //Keep it true. It will change upon receiving the new_samples event
  }

  //Send the message through WebSocket to switch the Relay
  let socket = new WebSocket("ws://"+ipAddress+"/wsconsole"); //Socket must be declared after knowing IoT's IP address
  socket.onerror = function(error) {
    alert('WS error with the IoT: '+error);
  };
  socket.onclose = function(event) {
    console.log("WS closed: code="+event.code+", reason="+event.reason+", wasClean="+event.wasClean);
    if (!event.wasClean) {
      // ej. El proceso del servidor se detuvo o la red está caída
      // event.code es usualmente 1006 en este caso
      alert('WD connection closed suddenly. Code: '+event.code+", Reason: "+event.reason);
    }
  };
  socket.onopen = function(e) {
    socket.send(message); //Send the new switch status to the IoT
  };
  //Socket will be closed by the server upgon receiving the relay message
}

function switchForceClicked(){
  let message="";
  if (document.getElementById('forceSwitch').checked) {
    message="R2_ON"; //Send ON message to force heater on (relay ON position)
    console.log(document.getElementById('forceSwitch')+' is checked');
    document.getElementById('forceSwitch').checked=false; //Keep it false. It will change upon receiving the new_samples event
  }
  else {
    message="R2_OFF"; //Send OFF message to not force heater on (relay OFF position)
    console.log(document.getElementById('forceSwitch')+' is NOT checked');
    document.getElementById('forceSwitch').checked=true; //Keep it true. It will change upon receiving the new_samples event
  }

  //Send the message through WebSocket to switch the Relay
  let socket = new WebSocket("ws://"+ipAddress+"/wsconsole"); //Socket must be declared after knowing IoT's IP address
  socket.onerror = function(error) {
    alert('WS error with the IoT: '+error);
  };
  socket.onclose = function(event) {
    console.log("WS closed: code="+event.code+", reason="+event.reason+", wasClean="+event.wasClean);
    if (!event.wasClean) {
      // ej. El proceso del servidor se detuvo o la red está caída
      // event.code es usualmente 1006 en este caso
      alert('WD connection closed suddenly. Code: '+event.code+", Reason: "+event.reason);
    }
  };
  socket.onopen = function(e) {
    socket.send(message); //Send the new switch status to the IoT
  };
  //Socket will be closed by the server upgon receiving the relay message
}

// Get current sensor readings when the page loads
window.addEventListener('load', getReadings);

if (!!window.EventSource) {
  var source = new EventSource('/sampleEvents');
  
  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);
  
  source.addEventListener('message', function(e) {
    console.log("message", e.data);
  }, false);
  
  source.addEventListener('new_samples', function(e) {
    console.log("new_samples", e.data);
    var myObj = JSON.parse(e.data);
    console.log(myObj);
    gaugeTemp.value = myObj.temperature;
    gaugeHum.value = myObj.humidity;
    gaugeLPG.value = myObj.LPG;
      if (myObj.LPG==0) document.getElementById("id-card-title-LPG").style.color="#78B0FF"; //Blue
      else if (myObj.LPG<=200) document.getElementById("id-card-title-LPG").style.color="#555755"; //Grey
      else if (myObj.LPG<=1500) document.getElementById("id-card-title-LPG").style.color="#6ED81A"; //Green
      else if (myObj.LPG<=5000) document.getElementById("id-card-title-LPG").style.color="#FEE909"; //Yellow
      else if (myObj.LPG<=10000) document.getElementById("id-card-title-LPG").style.color="#C30016"; //Red
      else document.getElementById("id-card-title-LPG").style.color="#555755"; //Grey
    gaugeCO.value = myObj.CO;
      if (myObj.CO==0) document.getElementById("id-card-title-CO").style.color="#78B0FF"; //Blue
      else if (myObj.CO<=200) document.getElementById("id-card-title-CO").style.color="#555755"; //Grey
      else if (myObj.CO<=500) document.getElementById("id-card-title-CO").style.color="#6ED81A"; //Green
      else if (myObj.CO<=900) document.getElementById("id-card-title-CO").style.color="#FEE909"; //Yellow
      else if (myObj.CO<=10000) document.getElementById("id-card-title-CO").style.color="#C30016"; //Red
      else document.getElementById("id-card-title-CO").style.color="#555755"; //Grey
    gaugeALCOHOL.value = myObj.ALCOHOL;
      if (myObj.ALCOHOL==0) document.getElementById("id-card-title-ALCOHOL").style.color="#78B0FF"; //Blue
      else if (myObj.ALCOHOL<=200) document.getElementById("id-card-title-ALCOHOL").style.color="#555755"; //Grey
      else if (myObj.ALCOHOL<=1500) document.getElementById("id-card-title-ALCOHOL").style.color="#6ED81A"; //Green
      else if (myObj.ALCOHOL<=5000) document.getElementById("id-card-title-ALCOHOL").style.color="#FEE909"; //Yellow
      else if (myObj.ALCOHOL<=10000) document.getElementById("id-card-title-ALCOHOL").style.color="#C30016"; //Red
      else document.getElementById("id-card-title-ALCOHOL").style.color="#555755"; //Grey
    gaugeCH4.value = myObj.CH4;
      if (myObj.CH4==0) document.getElementById("id-card-title-CH4").style.color="#78B0FF"; //Blue
      else if (myObj.CH4<=200) document.getElementById("id-card-title-CH4").style.color="#555755"; //Grey
      else if (myObj.CH4<=1500) document.getElementById("id-card-title-CH4").style.color="#6ED81A"; //Green
      else if (myObj.CH4<=5000) document.getElementById("id-card-title-CH4").style.color="#FEE909"; //Yellow
      else if (myObj.CH4<=10000) document.getElementById("id-card-title-CH4").style.color="#C30016"; //Red
      else document.getElementById("id-card-title-CH4").style.color="#555755"; //Grey
    gaugeH2.value = myObj.H2;
      if (myObj.H2==0) document.getElementById("id-card-title-H2").style.color="#78B0FF"; //Blue
      else if (myObj.H2<=200) document.getElementById("id-card-title-H2").style.color="#555755"; //Grey
      else if (myObj.H2<=1500) document.getElementById("id-card-title-H2").style.color="#6ED81A"; //Green
      else if (myObj.H2<=5000) document.getElementById("id-card-title-H2").style.color="#FEE909"; //Yellow
      else if (myObj.H2<=10000) document.getElementById("id-card-title-H2").style.color="#C30016"; //Red
      else document.getElementById("id-card-title-H2").style.color="#555755"; //Grey
    dateUpdate=myObj.dateUpdate;
    previousGasClear=gasClear;
    gasClear=myObj.Clean_air==="OFF"?true:false;
    if (previousGasClear!=gasClear)
      {if (gasClear) {
        document.getElementById("cleanair").src="leaf-circle-green.png";
        document.getElementById("cleanair").alt="Clean AIR";
        document.getElementById("id-card-title-GAS").innerHTML="Clean AIR";
        document.getElementById("id-card-title-GAS").style.color="#6ED81A"; //Gren
      } else {
        document.getElementById("cleanair").src="leaf-circle-red.png";
        document.getElementById("cleanair").alt="GAS leak";
        document.getElementById("id-card-title-GAS").innerHTML="GAS leak";
        document.getElementById("id-card-title-GAS").style.color="#C30016"; //Red
      }
    }
    previousBoilerStatus=boilerStatus;
    boilerStatus=myObj.boilerStatus==="OFF"?false:true;
    if (previousBoilerStatus!=boilerStatus)
      {if (boilerStatus) document.getElementById("boilerStatusIcon").src="boiler-orange.png";
      else document.getElementById("boilerStatusIcon").src="boiler-blue.png"}
    previousThermostatStatus=thermostatStatus;
    thermostatStatus=myObj.Thermostate_status==="OFF"?false:true;
    if (previousThermostatStatus!=thermostatStatus)
      {if (thermostatStatus) document.getElementById("heaterStatusIcon").src="radiator-orange.png";
      else document.getElementById("heaterStatusIcon").src="radiator-blue.png"}
    Relay1=myObj.Relay1==="R1_ON"?true:false;
    Relay2=myObj.Relay2==="R2_ON"?true:false;
    ipAddress=myObj.ipAddress;
    voltage=myObj.Voltage; document.getElementById("voltageId").innerHTML=voltage;
    current=myObj.Current; document.getElementById("currentId").innerHTML=current.toFixed(3);
    power=myObj.Power; document.getElementById("powerId").innerHTML=power;
    energyToday=myObj.EnergyToday; document.getElementById("energyTodayId").innerHTML=energyToday.toFixed(3);
    energyYesterday=myObj.EnergyYesterday; document.getElementById("energyYesterdayId").innerHTML=energyYesterday.toFixed(3);
    energyTotal=myObj.EnergyTotal; document.getElementById("energyTotalId").innerHTML=energyTotal.toFixed(3);
    document.getElementById("latestUpdate").innerHTML = dateUpdate;
    document.getElementById('thermostatSwitch').checked=Relay1;
    document.getElementById('forceSwitch').checked=Relay2;
  }, false);
}