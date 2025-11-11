/* Dealing with Web Server stuff
    from version: 1.1.0 IoT CO2 Sensor
*/

#include "webServer.h"

String processorGraphs(const String& var){
  log_v(">> processorGraphs");
  if(var == "GRAPHSBODY") {
    if (httpCloudEnabled) {
      /*String prefixURL=String(CLOUD_SERVICES_URL)+device;
      String co2GraphPath=prefixURL+"-"+String(CO2_GRAPH_IMAGE);
      String tempHumGraphPath=prefixURL+"-"+String(TEMP_HUM_GRAPH_IMAGE);
      String voltageGraphPath=prefixURL+"-"+String(VOLTAGE_GRAPH_IMAGE);
      String chargeGraphPath=prefixURL+"-"+String(CHARGE_GRAPH_IMAGE);

      return String("<img src="+co2GraphPath+" alt=\"CO2 graph\" title=\"CO2 Samples Evolution\" id=\"CO2SamplesEvolution\" style=\"width: 550px; height: 325px;\">\n"+
                    "<img src="+tempHumGraphPath+" alt=\"Temp and Hum graph\" title=\"Temp & Hum Samples Evolution\" id=\"TempHumSamplesEvolution\" style=\"width: 550px; height: 325px;\">\n"+
                    "<img src="+voltageGraphPath+" alt=\"Voltage graph\" title=\"Voltage Samples Evolution\" id=\"VoltageSamplesEvolution\" style=\"width: 550px; height: 325px;\">\n"+
                    "<img src="+chargeGraphPath+" alt=\"Charge graph\" title=\"Charge Samples Evolution\" id=\"ChargeSamplesEvolution\" style=\"width: 550px; height: 325px;\">");
      */
      String URL=String(CLOUD_SERVICES_URL)+"?dev="+device;
      return String("<iframe src=\""+URL+"\" frameBorder=\"0\" width=\"550\" height=\"1530\"></iframe>");
    }
    else {
      return String("<p><br>Enable the <a href=\"cloud.html\"> Cloud Services</a> first to see the evolution graphs</p>");
    }
  } else {
    return String();
  }
  log_v("<< processorGraphs. Exit");
} //processorGraphs

String processorContainer(const String& var){
  /******************************************************
   Function processorContainer
   Target: Serve the container web page
   Parameters:
    String var: It's the tag found in the html page. i.e.: ~INDEX_Text_Update_Color~
   Returns:
    (String) File content
   *****************************************************/ 

  log_v(">> processorContainer");
  if (var == "CONTAINER_HEAD") {
    if (deviceReset) return String("Device Reset");
    else if (factoryReset) return String("Factory Reset");
    else if (fileUpdateError) return String("File Upload");
    else return String("Other");
  } else if (var == "CONTAINER_TEXT") {
    if (deviceReset && updateCommand!=U_FLASH && updateCommand!=U_SPIFFS) {
      if (fileUpdateError==ERROR_UPLOAD_FILE_NOAUTH) {
        deviceReset=false;
        //return String("Authentication is required to reboot the device (0x")+String(fileUpdateError,HEX)+String(").<p></p>Try again: <a href=\"http://"+WiFi.localIP().toString()+"/maintenance.html\" title=\"Maintenance & Troubleshooting\" target=\"_self\">http://"+WiFi.localIP().toString()+"/maintenance.html</a>");
        return String("Authentication is required to reboot the device (0x")+String(fileUpdateError,HEX)+String(").<p></p>Try again: <a href=\"http://"+WiFi.localIP().toString()+lastURI+"\" title=\"Return and try authentication\" target=\"_self\">http://"+WiFi.localIP().toString()+lastURI+"</a>");
      }
      //else return String("Rebooting the device....<p></p><div id=\"timer\"></div><script type=\"text/javascript\">var maxTime=60;var timer = maxTime;var auxTimeout=false;var interval = setInterval(function() {if (timer > 0) document.getElementById(\"timer\").innerHTML='Checking if the device is ready....<br><br>Wait '+timer+' seconds';if (timer === maxTime/4) {var request = new XMLHttpRequest();request.open('GET', 'http://"+WiFi.localIP().toString()+"', true);request.ontimeout = function(){auxTimeout=true;};request.onloadend = function(){if (auxTimeout === true) document.getElementById(\"timer\").innerHTML='Something went wrong. The device is not ready.<br><br>Check if either it is not connected to WiFi of the device IP address is not longer "+WiFi.localIP().toString()+" after rebooting';else {document.getElementById(\"timer\").innerHTML='The device is back and ready. Wait a bit more to connect it....';setTimeout(function() {location.replace('http://"+WiFi.localIP().toString()+"');},1500)};timer=0;};request.onloadstart = function(){document.getElementById(\"timer\").innerHTML='Checking if the device is ready....<br><br>Wait '+timer+' seconds';};request.send();};if (timer === 0) clearInterval(interval);else timer--;},1000);</script>");
      else return String("Rebooting the device....<p></p><div id=\"timer\"></div><script type=\"text/javascript\">var maxTime=60; var timer = maxTime; var maxTimeout=1000;var url = 'http://"+WiFi.localIP().toString()+"/test.html';var ping = new XMLHttpRequest();function printTimer(){document.getElementById(\"timer\").innerHTML='Checking if the device is ready....<br><br>Wait '+timer+' seconds';};printTimer();ping.onreadystatechange = function(){if(ping.readyState == 4){if(ping.status == 200){document.getElementById(\"timer\").innerHTML='The device is back and ready. Wait a bit more to connect it....';console.log(\"Response sucessfully got. Redirect now.\");setTimeout(function() {location.replace('http://"+WiFi.localIP().toString()+"');console.log(\"Redirect done.\");},1500);}}};ping.ontimeout = function(event) {timer-=maxTimeout;if (timer > 0) {printTimer();ping.open(\"GET\", url, true);ping.send();console.log(\"Request re-sent, event=\"+event);}else {document.getElementById(\"timer\").innerHTML='Something went wrong. The device is not ready.<br><br>Check if either it is not connected to WiFi of the device IP address is not longer "+WiFi.localIP().toString()+" after rebooting';console.log(\"End of timeouts. Unsuccesfull exit.\");}};ping.onerror=function(event){console.log(\"Error. event=\"+event);setTimeout(function() {timer--;printTimer();ping.open(\"GET\", url, true);ping.send();console.log(\"Request re-sent.\");},1000);};ping.open(\"GET\", url, true);ping.timeout=maxTimeout;var interval = setInterval(function() {printTimer();if (timer === 50) {clearInterval(interval);ping.send();console.log(\"First request sent\");}else timer--;},1000);</script>");
    }
    else if (deviceReset && (updateCommand==U_FLASH || updateCommand==U_SPIFFS)) {
      String auxString=(updateCommand==U_FLASH) ? String("Binary file has been successfully written in FLASH. Size="):String("System file has been successfully written in SPIFFS. Size=");
      //return String(auxString+fileUpdateSize+" B.<p></p>Rebooting the device....<p></p><div id=\"timer\"></div><script type=\"text/javascript\">var maxTime=60;var timer = maxTime;var auxTimeout=false;var interval = setInterval(function() {if (timer > 0) document.getElementById(\"timer\").innerHTML='Checking if the device is ready....<br><br>Wait '+timer+' seconds';if (timer === maxTime/4) {var request = new XMLHttpRequest();request.open('GET', 'http://"+WiFi.localIP().toString()+"', true);request.ontimeout = function(){auxTimeout=true;};request.onloadend = function(){if (auxTimeout === true) document.getElementById(\"timer\").innerHTML='Something went wrong. The device is not ready.<br><br>Check if either it is not connected to WiFi of the device IP address is not longer "+WiFi.localIP().toString()+" after rebooting';else {document.getElementById(\"timer\").innerHTML='The device is back and ready. Wait a bit more to connect it....';setTimeout(function() {location.replace('http://"+WiFi.localIP().toString()+"');},1500)};timer=0;};request.onloadstart = function(){document.getElementById(\"timer\").innerHTML='Checking if the device is ready....<br><br>Wait '+timer+' seconds';};request.send();};if (timer === 0) clearInterval(interval);else timer--;},1000);</script>");
      return String("Rebooting the device....<p></p><div id=\"timer\"></div><script type=\"text/javascript\">var maxTime=60; var timer = maxTime; var maxTimeout=1000;var url = 'http://"+WiFi.localIP().toString()+"/test.html';var ping = new XMLHttpRequest();function printTimer(){document.getElementById(\"timer\").innerHTML='Checking if the device is ready....<br><br>Wait '+timer+' seconds';};printTimer();ping.onreadystatechange = function(){if(ping.readyState == 4){if(ping.status == 200){document.getElementById(\"timer\").innerHTML='The device is back and ready. Wait a bit more to connect it....';console.log(\"Response sucessfully got. Redirect now.\");setTimeout(function() {location.replace('http://"+WiFi.localIP().toString()+"');console.log(\"Redirect done.\");},1500);}}};ping.ontimeout = function(event) {timer-=maxTimeout;if (timer > 0) {printTimer();ping.open(\"GET\", url, true);ping.send();console.log(\"Request re-sent, event=\"+event);}else {document.getElementById(\"timer\").innerHTML='Something went wrong. The device is not ready.<br><br>Check if either it is not connected to WiFi of the device IP address is not longer "+WiFi.localIP().toString()+" after rebooting';console.log(\"End of timeouts. Unsuccesfull exit.\");}};ping.onerror=function(event){console.log(\"Error. event=\"+event);setTimeout(function() {timer--;printTimer();ping.open(\"GET\", url, true);ping.send();console.log(\"Request re-sent.\");},1000);};ping.open(\"GET\", url, true);ping.timeout=maxTimeout;var interval = setInterval(function() {printTimer();if (timer === 50) {clearInterval(interval);ping.send();console.log(\"First request sent\");}else timer--;},1000);</script>");
    }
    else if (factoryReset) {
      if (fileUpdateError==ERROR_UPLOAD_FILE_NOAUTH) {
        factoryReset=false;
        return String("Authentication is required to reboot the device (0x")+String(fileUpdateError,HEX)+String(").<p></p>Try again: <a href=\"http://"+WiFi.localIP().toString()+lastURI+"\" title=\"Return and try authentication\" target=\"_self\">http://"+WiFi.localIP().toString()+lastURI+"</a>");
      }
      else return String("The configuration of the device has been successfully reset to factory values.<p></p>The WiFi SSIDs need to be setup again as the "+WiFi.SSID()+" SSID has been deleted.<br><br>Web access to the device is not available till the WiFi is setup again.");
    }
    else if (fileUpdateError==ERROR_UPLOAD_FILE_NOAUTH) return (String("Authentication is required to reboot the device (0x")+String(fileUpdateError,HEX)+String(").<p></p>Try again: <a href=\"http://"+WiFi.localIP().toString()+lastURI+"\" title=\"Return and try authentication\" target=\"_self\">http://"+WiFi.localIP().toString()+lastURI+"</a>"));
    else if (fileUpdateError==ERROR_UPLOAD_FILE_NOBIN_FILE) return (String("File is not a valid Binary file (0x")+String(fileUpdateError,HEX)+String(").<p></p>Try again: <a href=\"http://"+WiFi.localIP().toString()+"/maintenance.html\" title=\"Maintenance & Troubleshooting\" target=\"_self\">http://"+WiFi.localIP().toString()+"/maintenance.html</a>"));
    else if (fileUpdateError==ERROR_UPLOAD_FILE_NOSPIFFS_FILE) return (String("File is not a valid SPIFFS partition (0x")+String(fileUpdateError,HEX)+String(").<p></p>Try again: <a href=\"http://"+WiFi.localIP().toString()+"/maintenance.html\" title=\"Maintenance & Troubleshooting\" target=\"_self\">http://"+WiFi.localIP().toString()+"/maintenance.html</a>"));
    else if (fileUpdateError==ERROR_UPLOAD_FILE_NOSPIFFS_FILE_NOSAFE) return (String("File is not a valid SPIFFS partition (0x")+String(fileUpdateError,HEX)+String(").<p></p>Partion in Flash might be corrupted so the built-in web server might not work....<p></p>Reboot and try again: <a href=\"http://"+WiFi.localIP().toString()+"/maintenance.html\" title=\"Maintenance & Troubleshooting\" target=\"_self\">http://"+WiFi.localIP().toString()+"/maintenance.html</a>"));
    else if (fileUpdateError!=ERROR_UPLOAD_FILE_NOERROR) return (String("Something went wrong while uploading the file (0x")+String(fileUpdateError,HEX)+String(").<p></p>You may need to reboot the device before trying again: <a href=\"http://"+WiFi.localIP().toString()+"/maintenance.html\" title=\"Maintenance & Troubleshooting\"\" target=\"_self\">http://"+WiFi.localIP().toString()+"/maintenance.html</a>"));
    //else if (fileUpdateError!=0) return String("Something went wrong in the file upload process.<p></p>Try again.");
    else return String("Other");
  } else {
    return String();
  }
  log_v("<< processorContainer. Exit");
} //processorContainer

uint32_t initWebServer() {
  /******************************************************
   Function initWebServer
   Target: Init web server
   Parameters:
    None
   Returns:
    uint32_t error: Error code
   *****************************************************/ 

  // Route for root / web page
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    //if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - Http request / received, heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_INDEX_PAGE, String(),false);
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  // Route for root index.html web page
  webServer.on(WEBSERVER_INDEX_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_INDEX_PAGE, String(),false);
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  // Route for root test.html web page
  webServer.on(WEBSERVER_TEST_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    //blockWebServer is not checked to avoid inter-lock during connectivity checks
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_TEST_PAGE, String(),false);
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  // Route for root console.html web page
  webServer.on(WEBSERVER_CONSOLE_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS,WEBSERVER_CONSOLE_PAGE, String(),false);
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });
  
  // Route to load style.css file
  webServer.on(WEBSERVER_CSSSTYLES_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_CSSSTYLES_PAGE, "text/css");
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  // Route to load tswnavbar.css file
  webServer.on(WEBSERVER_CSSNAVBAR_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_CSSNAVBAR_PAGE, "text/css");
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed   
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  // Route to load The_IoT_Factory.png file
  webServer.on(WEBSERVER_LOGO_ICON, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_LOGO_ICON, "image/png");

    //Wait to disconnect the connection
    request->onDisconnect([]{
      //Excute wen the request->send(SPIFFS, WEBSERVER_LOGO_ICON, "image/png"); is finished
      
      //SSID change - Reconnect WiFi
      if (wifiEnabled && reconnectWifiAndRestartWebServer) {
        wifiCurrentStatus=wifiOffStatus;
        forceWifiReconnect=true; //Don't wait next WIFI_RECONNECT_PERIOD interaction. Reconnect in this loop() interaction
        /*WiFi.disconnect(true);
        delay(WEBSERVER_SEND_DELAY);
        WiFi.disconnect(false);*/ //This stuff is done in WIFI_RECONNECT_PERIOD
        reconnectWifiAndRestartWebServer=false;
      }

      //NTP Server - Resync NTP Server
      if(wifiEnabled && resyncNTPServer) {
        //CloudClockCurrentStatus=CloudClockOffStatus; //CloudClockCurrentStatus is updated in ntp_ko_check_period()
        forceNTPCheck=true;
        resyncNTPServer=false;
      }
  
      //Device reset
      if (deviceReset) {
        time_counters_eeprom_update_check_period(true,millis(),true); //Force to write timers
        ESP.restart();
      }
    });

    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  // Route to load favicon.ico file
  webServer.on(WEBSERVER_FAVICON_ICON, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_FAVICON_ICON, "image/x-icon");
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  // Route to load maintenance_upload_firmware/identity file
  webServer.on("/maintenance_upload_firmware/identity", HTTP_GET, [](AsyncWebServerRequest *request) {
    String id = String((uint32_t)ESP.getEfuseMac(), HEX);
    id.toUpperCase();
    
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(200, "application/json", "{\"id\": \""+id+"\", \"hardware\": \"ESP32\"}");
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  webServer.on(WEBSERVER_GRAPHS_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_GRAPHS_PAGE, String(), false, processorGraphs);
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  webServer.on(WEBSERVER_STATS_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_STATS_PAGE, String(), false);
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });
  
  // Route for info.html web page
  webServer.on(WEBSERVER_INFO_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_INFO_PAGE, String(), false);
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });
  
  // Route for  basic.html web page
  webServer.on(WEBSERVER_BASICCONFIG_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_BASICCONFIG_PAGE, String(), false);
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });
  
  // Route for cloud.html web page
  webServer.on(WEBSERVER_CLOUDCONFIG_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_CLOUDCONFIG_PAGE, String(), false);
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });
  
  // Route for bluetooth.html web page
  webServer.on(WEBSERVER_BLUETOOTHCONFIG_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_BLUETOOTHCONFIG_PAGE, String(), false);
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });
  
  // Route for maintenance.html web page
  webServer.on(WEBSERVER_MAINTENANCE_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    fileUpdateError=0; //To check POST parameters & File Upload process

    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    
    //Setup a new response to send Set Cookie headers in the maintenance.html answer
    AsyncWebServerResponse * auxResp=new AsyncFileResponse(SPIFFS, WEBSERVER_MAINTENANCE_PAGE, String(), false);

    //If comes with a valid cookie and there is an ongoing upload, return the same cookie
    //This way, a broser-based abort action can be detected
    if (Update.progress()!=0 && request->hasHeader("Cookie") &&
        (request->getHeader("Cookie")->value().length() <= COOKIE_SIZE-1) &&
        memcmp(device.c_str(),request->getHeader("Cookie")->value().c_str()+3,device.length())==0 ) {
          memcpy(currentSetCookie,request->getHeader("Cookie")->value().c_str(),request->getHeader("Cookie")->value().length()); //End null not included
          if (debugModeOn) printLogln(String(millis())+" - [maintenance GET] - Valid cookie deteced="+String(currentSetCookie)+" kept");
          auxResp->addHeader("Set-Cookie", String(currentSetCookie));
    } 
    else {
      //Create a new cookie if the condition is not met
      long auxRandom1=random(0,2147483647);long auxRandom2=random(0,2147483647);long auxRandom3=random(0,2147483647);long auxRandom4=random(0,2147483647);
      String setCookie=String("id-"+device+"="+String(auxRandom1,HEX)+String(auxRandom2,HEX)+String(auxRandom3,HEX)+String(auxRandom4,HEX));      
      memcpy(currentSetCookie,setCookie.c_str(),setCookie.length()); //End null not included
      if (debugModeOn) printLogln(String(millis())+" - [maintenance GET] - New cookie="+String(currentSetCookie)+" created");
      auxResp->addHeader("Set-Cookie", setCookie);
    }
    request->send(auxResp);
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  // Route for container.html web page
  webServer.on(WEBSERVER_CONTAINER_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow

    updateCommand=-1;
    fileUpdateError=ERROR_UPLOAD_FILE_NOERROR;
    request->send(SPIFFS, WEBSERVER_CONTAINER_PAGE, String(), false, processorContainer);
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed   
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  // Route for /maintenance_default_values web page
  webServer.on(WEBSERVER_DEFAULTCONF_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    
    fileUpdateError=ERROR_UPLOAD_FILE_NOERROR; //To check POST parameters & File Upload process
    request->send(SPIFFS, WEBSERVER_MAINTENANCE_PAGE, String(), false);
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  // Route for /maintenance_device_reset web page
  webServer.on(WEBSERVER_DEVICERESET_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    
    fileUpdateError=ERROR_UPLOAD_FILE_NOERROR; //To check POST parameters & File Upload process
    request->send(SPIFFS, WEBSERVER_MAINTENANCE_PAGE, String(), false);
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed   
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });
  
  // Route for updatefile.html web page
  webServer.on(WEBSERVER_UPLOADFILE_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
  
    fileUpdateError=0; //To check POST parameters & File Upload process
    request->send(SPIFFS, WEBSERVER_MAINTENANCE_PAGE, String(), false);
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  // Route to WEBSERVER_SAMPLES_PAGE file
  webServer.on(WEBSERVER_SAMPLES_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    
    //Update JSON objects needed for this web page
    gas_sample(false,6);   //Get GAS samples
    if (tempHumSensor.isConnected()) temperature_sample(false);  //Get Temp & Hum samples
    request->send(200, "application/json", JSON.stringify(samples));
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  // Route to gauge.min.js file
  webServer.on(WEBSERVER_GAUGESCRIPT_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_GAUGESCRIPT_PAGE, "text/javascript");
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  // Route to WIFI_PSSWs file
  webServer.on("/wpsw", HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(200, "application/json", String("{\"PSSW\":\"")+wifiCred.wifiPSSWs[0]+String("\",\"PSSW_BK1\":\"")+wifiCred.wifiPSSWs[1]+String("\",\"PSSW_BK2\":\"")+wifiCred.wifiPSSWs[2]+String("\"}"));
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  // Route to index_script.js file
  webServer.on(WEBSERVER_INDEX_SCRIPT_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_INDEX_SCRIPT_PAGE, "text/javascript");
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  // Route to stats_script.js file
  webServer.on(WEBSERVER_STATS_SCRIPT_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_STATS_SCRIPT_PAGE, "text/javascript");
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  // Route to info_script.js file
  webServer.on(WEBSERVER_INFO_SCRIPT_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_INFO_SCRIPT_PAGE, "text/javascript");
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  // Route to basic_script.js file
  webServer.on(WEBSERVER_BASIC_SCRIPT_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_BASIC_SCRIPT_PAGE, "text/javascript");
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  // Route to cloud_script.js file
  webServer.on(WEBSERVER_CLOUD_SCRIPT_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_CLOUD_SCRIPT_PAGE, "text/javascript");
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  // Route to maintenance_script.js file
  webServer.on(WEBSERVER_MAINTENANCE_SCRIPT_PAGE, HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, WEBSERVER_MAINTENANCE_SCRIPT_PAGE, "text/javascript");
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });
  
  webServer.on("/boiler-blue.png", HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, "/boiler-blue.png", "image/png");
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  webServer.on("/boiler-orange.png", HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, "/boiler-orange.png", "image/png");
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  webServer.on("/boiler-blue-flame.png", HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, "/boiler-blue-flame.png", "image/png");
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });
  
  webServer.on("/boiler-orange-flame.png", HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, "/boiler-orange-flame.png", "image/png");
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  webServer.on("/boiler-grey.png", HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, "/boiler-grey.png", "image/png");
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  webServer.on("/leaf-circle-green.png", HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, "/leaf-circle-green.png", "image/png");
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  webServer.on("/leaf-circle-red.png", HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, "/leaf-circle-red.png", "image/png");
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });
  
  webServer.on("/radiator-blue.png", HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    //Wait for other blocking tasks in the loop to finish
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, "/radiator-blue.png", "image/png");
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });
  
  webServer.on("/radiator-orange.png", HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, "/radiator-orange.png", "image/png");
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });
  
  webServer.on("/radiator-disabled-off.png", HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, "/radiator-disabled-off.png", "image/png");
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });
  
  webServer.on("/radiator-orange-on.png", HTTP_GET, [](AsyncWebServerRequest *request){
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    request->send(SPIFFS, "/radiator-orange-on.png", "image/png");
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  webServer.on("/basic1", HTTP_POST, [](AsyncWebServerRequest *request) {
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    
    int params = request->params();
    for(int i=0;i<params;i++) {
      const AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        // HTTP POST WiFi_enabled value
        if (String(p->name()).compareTo("WiFi_enabled")==0) {
          if ((String(p->value().c_str()).compareTo("on")==0) && !wifiEnabled) {
            //Never enter here as wifi is disabled. Anyway the code should work
            wifiEnabled=true;
            if(WiFi.status()!=WL_CONNECTED) {
              forceWifiReconnect=true; //Next loop interaction the WiFi connection and network services are done
            }
            uint8_t configVariables=EEPROM.read(0x08);
            configVariables|=0x10; //Set wifiEnabled bit to true (enabled)
            EEPROM.write(0x08,configVariables);
            EEPROM.commit();
          }
          else if ((String(p->value().c_str()).compareTo("off")==0) && wifiEnabled) {
            wifiEnabled=false;
            if(WiFi.status()!=WL_DISCONNECTED) {
              WiFi.disconnect();
              wifiCurrentStatus=wifiOffStatus;
              forceWifiReconnect=false; //To avoid deadlock in WIFI_RECONNECT_PERIOD chck if a previous WiFi reconnection was ongoing
              mqttServerEnabled=false; //To avoid uploading mqtt samples
              httpCloudEnabled=false; //To avoid uploading samples tries
              CloudClockCurrentStatus=CloudClockOffStatus;
            }
            uint8_t configVariables=EEPROM.read(0x08);
            configVariables&=0xEF; //Set wifiEnabled bit to false (disabled)
            EEPROM.write(0x08,configVariables);
            EEPROM.commit();
          }
        }
      }
    }
    //request->send(SPIFFS, WEBSERVER_INDEX_PAGE, String(), false, processor);
    //request->send(SPIFFS, WEBSERVER_INDEX_PAGE, "text/html");
    request->send(SPIFFS, WEBSERVER_INDEX_PAGE, String(),false);
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  webServer.on("/basic4", HTTP_POST, [](AsyncWebServerRequest *request) {
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    
    int params = request->params();
    bool updateEEPROM=false;
    char auxUserName[WEB_USER_CREDENTIAL_LENGTH],auxUserPssw[WEB_PW_CREDENTIAL_LENGTH];
    AsyncWebServerResponse * auxResp=new AsyncFileResponse(SPIFFS, WEBSERVER_CLOUDCONFIG_PAGE, String(), false);

    //Authentication is required
    if(!request->authenticate(userName.c_str(), userPssw.c_str())) {
      //Setup a new response to send AuthenticationRequest headers in the container.html answer
      lastURI=String(WEBSERVER_BASICCONFIG_PAGE); //To go back to the right page from CONTAINER if the authentication fails
      auxResp->setCode(401);
      auxResp->addHeader("WWW-Authenticate", "Basic realm=\"Login Required\"");
      request->send(auxResp);
    }
    else
    {
      for(int i=0;i<params;i++) {
        const AsyncWebParameter* p = request->getParam(i);  
        if(p->isPost()){
          // HTTP POST AdminUser_Form value
          if (String(p->name()).compareTo("UserName")==0) {
            memset(auxUserName,'\0',WEB_USER_CREDENTIAL_LENGTH);
            memcpy(auxUserName,p->value().c_str(),p->value().length()); //End null not included
            if (userName.compareTo(auxUserName)!=0) {
              userName=p->value();
              EEPROM.put(0x2A8,auxUserName);
              updateEEPROM=true;
            }
          }
          else if (String(p->name()).compareTo("UserPssw")==0) {
            memset(auxUserPssw,'\0',WEB_PW_CREDENTIAL_LENGTH);
            memcpy(auxUserPssw,p->value().c_str(),p->value().length()); //End null not included
            if (userPssw.compareTo(auxUserPssw)!=0) {
              userPssw=p->value();
              EEPROM.put(0x2B3,auxUserPssw);
              updateEEPROM=true;
            }
          }
        }
      }
      if (updateEEPROM) EEPROM.commit();
      //request->send(SPIFFS, WEBSERVER_INDEX_PAGE, String(), false, processor);
      //request->send(SPIFFS, WEBSERVER_INDEX_PAGE, "text/html");
      //Update JSON objects needed for this web page
      request->send(SPIFFS, WEBSERVER_INDEX_PAGE, String(),false);
    }

    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });
  
  webServer.on("/basic2", HTTP_POST, [](AsyncWebServerRequest *request) {
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow    
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    
    reconnectWifiAndRestartWebServer=false;
    resyncNTPServer=false;
    int params = request->params();
    bool updateEEPROM=false;
    char auxSSID[WIFI_MAX_SSID_LENGTH],auxPSSW[WIFI_MAX_PSSW_LENGTH],auxSITE[WIFI_MAX_SITE_LENGTH],
         auxNTP[NTP_SERVER_NAME_MAX_LENGTH],auxTZEnvVar[TZ_ENV_VARIABLE_MAX_LENGTH],auxTZName[TZ_ENV_NAME_MAX_LENGTH];
    
    for(int i=0;i<params;i++) {
      const AsyncWebParameter* p = request->getParam(i);
      
      if(p->isPost()){
        // HTTP POST SSID value
        if (String(p->name()).compareTo("SSID")==0) {
          memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);
          memcpy(auxSSID,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiSSIDs[0].compareTo(auxSSID)!=0) {
            wifiCred.wifiSSIDs[0]=String(p->value().c_str());
            EEPROM.put(0x0D,auxSSID);
            reconnectWifiAndRestartWebServer=true; //Always reconnect
            updateEEPROM=true;
          }
        }
        // HTTP POST PSSW value
        else if (String(p->name()).compareTo("PSSW")==0) {
          memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);
          memcpy(auxPSSW,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiPSSWs[0].compareTo(auxPSSW)!=0) {
            wifiCred.wifiPSSWs[0]=String(p->value().c_str());
            EEPROM.put(0x2E,auxPSSW);
            reconnectWifiAndRestartWebServer=true;  //Always reconnect
            updateEEPROM=true;
          }
        }
        // HTTP POST SITE value
        else if (String(p->name()).compareTo("SITE")==0) {
          memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);
          memcpy(auxSITE,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiSITEs[0].compareTo(auxSITE)!=0) {
            wifiCred.wifiSITEs[0]=String(p->value().c_str());
            EEPROM.put(0x6E,auxSITE);
            updateEEPROM=true;
          }
        }
        // HTTP POST SSID_BK1 value
        else if (String(p->name()).compareTo("SSID_BK1")==0) {
          memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);
          memcpy(auxSSID,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiSSIDs[1].compareTo(auxSSID)!=0) {
            wifiCred.wifiSSIDs[1]=String(p->value().c_str());
            EEPROM.put(0x79,auxSSID);
            if (wifiCred.activeIndex>=1) reconnectWifiAndRestartWebServer=true; //Only reconnect if connected to SSID_BK1 or SSID_BK2
            updateEEPROM=true;
          }
        }
        // HTTP POST PSSW_BK1 value
        else if (String(p->name()).compareTo("PSSW_BK1")==0) {
          memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);
          memcpy(auxPSSW,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiPSSWs[1].compareTo(auxPSSW)!=0) {
            wifiCred.wifiPSSWs[1]=String(p->value().c_str());
            EEPROM.put(0x9A,auxPSSW);
            if (wifiCred.activeIndex>=1) reconnectWifiAndRestartWebServer=true; //Only reconnect if connected to SSID_BK1 or SSID_BK2
            updateEEPROM=true;
          }
        }
        // HTTP POST SITE_BK1 value
        else if (String(p->name()).compareTo("SITE_BK1")==0) {
          memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);
          memcpy(auxSITE,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiSITEs[1].compareTo(auxSITE)!=0) {
            wifiCred.wifiSITEs[1]=String(p->value().c_str());
            EEPROM.put(0xDA,auxSITE);
            updateEEPROM=true;
          }
        }
        // HTTP POST SSID_BK2 value
        else if (String(p->name()).compareTo("SSID_BK2")==0) {
          memset(auxSSID,'\0',WIFI_MAX_SSID_LENGTH);
          memcpy(auxSSID,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiSSIDs[2].compareTo(auxSSID)!=0) {
            wifiCred.wifiSSIDs[2]=String(p->value().c_str());
            EEPROM.put(0xE5,auxSSID);
            if (wifiCred.activeIndex==2) reconnectWifiAndRestartWebServer=true; //Only reconnect if connected to SSID_BK2
            updateEEPROM=true;
          }
        }
        // HTTP POST PSSW_BK2 value
        else if (String(p->name()).compareTo("PSSW_BK2")==0) {
          memset(auxPSSW,'\0',WIFI_MAX_PSSW_LENGTH);
          memcpy(auxPSSW,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiPSSWs[2].compareTo(auxPSSW)!=0) {
            wifiCred.wifiPSSWs[2]=String(p->value().c_str());
            EEPROM.put(0x106,auxPSSW);
            if (wifiCred.activeIndex==2) reconnectWifiAndRestartWebServer=true; //Only reconnect if connected to SSID_BK2
            updateEEPROM=true;
          }
        }
        // HTTP POST SITE_BK2 value
        else if (String(p->name()).compareTo("SITE_BK2")==0) {
          memset(auxSITE,'\0',WIFI_MAX_SITE_LENGTH);
          memcpy(auxSITE,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (wifiCred.wifiSITEs[2].compareTo(auxSITE)!=0) {
            wifiCred.wifiSITEs[2]=String(p->value().c_str());
            EEPROM.put(0x146,auxSITE);
            updateEEPROM=true;
          }
        }
        // HTTP POST NTP1 value
        else if (String(p->name()).compareTo("NTP1")==0) {
          memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
          memcpy(auxNTP,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (ntpServers[0].compareTo(auxNTP)!=0) {
            ntpServers[0]=String(p->value().c_str());
            EEPROM.put(0x151,auxNTP);
            resyncNTPServer=true; //Always resync
            updateEEPROM=true;
          }
        }
        // HTTP POST NTP2 value
        else if (String(p->name()).compareTo("NTP2")==0) {
          memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
          memcpy(auxNTP,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (ntpServers[1].compareTo(auxNTP)!=0) {
            ntpServers[1]=String(p->value().c_str());
            EEPROM.put(0x191,auxNTP);
            if (ntpServerIndex>=1) resyncNTPServer=true; //Only resync if connected to NTP with lower precedence
            updateEEPROM=true;
          }
        }
        // HTTP POST NTP3 value
        else if (String(p->name()).compareTo("NTP3")==0) {
          memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
          memcpy(auxNTP,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (ntpServers[2].compareTo(auxNTP)!=0) {
            ntpServers[2]=String(p->value().c_str());
            EEPROM.put(0x1D1,auxNTP);
            if (ntpServerIndex>=2) resyncNTPServer=true; //Only resync if connected to NTP with lower precedence
            updateEEPROM=true;
          }
        }
        // HTTP POST NTP4 value
        else if (String(p->name()).compareTo("NTP4")==0) {
          memset(auxNTP,'\0',NTP_SERVER_NAME_MAX_LENGTH);
          memcpy(auxNTP,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          if (ntpServers[3].compareTo(auxNTP)!=0) {
            ntpServers[3]=String(p->value().c_str());
            EEPROM.put(0x211,auxNTP);
            if (ntpServerIndex>=3) resyncNTPServer=true; //Only resync if connected to NTP with lower precedence
            updateEEPROM=true;
          }
        }
        // HTTP POST TimeZone value
        else if (String(p->name()).compareTo("TimeZone")==0) {
          memset(auxTZEnvVar,'\0',TZ_ENV_VARIABLE_MAX_LENGTH);
          //memcpy(auxTZEnvVar,p->value().c_str(),String(p->value().c_str()).length()); //End null not included
          memcpy(auxTZEnvVar,
                 p->value().substring(p->value().indexOf('$')+1).c_str(),
                 p->value().substring(p->value().indexOf('$')+1).length()); //End null not included
          if (TZEnvVariable.compareTo(auxTZEnvVar)!=0) {
            //TZEnvVariable=p->value();
            TZEnvVariable=p->value().substring(p->value().indexOf('$')+1);
            EEPROM.put(0x251,auxTZEnvVar);
            resyncNTPServer=true; //Always resync NTP
            updateEEPROM=true;
          }

          memset(auxTZName,'\0',TZ_ENV_NAME_MAX_LENGTH);
          memcpy(auxTZName,p->value().substring(0,p->value().indexOf('$')).c_str(),p->value().indexOf('$')); //End null not included
          if (TZName.compareTo(auxTZName)!=0) {
            TZName=p->value().substring(0,p->value().indexOf('$'));
            EEPROM.put(0x28A,auxTZName);
            resyncNTPServer=true; //Always resync NTP
            updateEEPROM=true;
          }
        }
      }
    }
    
    if (updateEEPROM) EEPROM.commit();
    //request->send(200,String("text/html"),String("<!DOCTYPE html><html lang=\"en\"><head><meta http-equiv=\"refresh\" content=\"0.5; URL=http://192.168.100.103\"/></head><body style=\"background-color:#34383b;\"></body></html>"));
    //request->send(SPIFFS, WEBSERVER_INDEX_PAGE, String(), false, processor);
    //request->send(SPIFFS, WEBSERVER_INDEX_PAGE, "text/html");
    request->send(SPIFFS, WEBSERVER_INDEX_PAGE, String(),false);

    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  webServer.on("/cloud", HTTP_POST, [](AsyncWebServerRequest *request) {
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    
    int params = request->params();
    uint8_t currentConfigVariables,configVariables=0;
    bool updateEEPROM=false,connectMqtt=false,disconnectMqtt=false,subscribePowerMqtt=false;
    char auxUserName[MQTT_USER_CREDENTIAL_LENGTH],auxUserPssw[MQTT_PW_CREDENTIAL_LENGTH];
    AsyncWebServerResponse * auxResp=new AsyncFileResponse(SPIFFS, WEBSERVER_CLOUDCONFIG_PAGE, String(), false);
    byte auxCounter=0;

    currentConfigVariables=EEPROM.read(0x2BE);
    wifiCred.SiteAllow[0]=false;wifiCred.SiteAllow[1]=false;wifiCred.SiteAllow[2]=false;

    //Authentication is required
    if(!request->authenticate(userName.c_str(), userPssw.c_str())) {
      //Setup a new response to send AuthenticationRequest headers in the container.html answer
      lastURI=String(WEBSERVER_CLOUDCONFIG_PAGE); //To go back to the right page from CONTAINER if the authentication fails
      auxResp->setCode(401);
      auxResp->addHeader("WWW-Authenticate", "Basic realm=\"Login Required\"");
      request->send(auxResp);
    }
    else 
    {
      //Checking checkbox. Name is sent only if checked. 
      for(int i=0;i<params;i++) {
        const AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST SITE_ALLOW_NAME value
          if (p->name().compareTo("SITE_ALLOW_NAME")==0) { //Check-box checked
            wifiCred.SiteAllow[0]=true;
            configVariables|= 0x01;
          }
          // HTTP POST SITE_BK1_ALLOW_NAME value
          else if (p->name().compareTo("SITE_BK1_ALLOW_NAME")==0) { //Check-box checked
            wifiCred.SiteAllow[1]=true;
            configVariables|=0x02;
          }
          // HTTP POST SITE_BK2_ALLOW_NAME value
          else if (p->name().compareTo("SITE_BK2_ALLOW_NAME")==0) { //Check-box checked
            wifiCred.SiteAllow[2]=true;
            configVariables|=0x04;
          }
        }
      }

      if (currentConfigVariables != configVariables) {
        EEPROM.write(0x2BE,configVariables);
        updateEEPROM=true;
        if (httpCloudEnabled) lastTimeHTTPClouCheck=millis()-CONNECTIVITY_CHECK_PERIOD; //Force to update status of the cloud server (CloudSyncCurrentStatus) in the next loop round
      }
      
      //Checking the rest of parameters different to checkbox
      for(int i=0;i<params;i++) {
        const AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST Cloud_enabled value
          if (p->name().compareTo("Cloud_enabled")==0) {
            if ((p->value().compareTo("on")==0) && !httpCloudEnabled) {
              httpCloudEnabled=true;
              if (httpCloudEnabled) lastTimeHTTPClouCheck=millis()-CONNECTIVITY_CHECK_PERIOD; //Force to update status of the cloud server (CloudSyncCurrentStatus) in the next loop round
              configVariables=EEPROM.read(0x08) | 0x04; //Set httpCloudEnabled bit to true (enabled)
              EEPROM.write(0x08,configVariables);
              updateEEPROM=true;
            }
            else if ((p->value().compareTo("off")==0) && httpCloudEnabled) {
              httpCloudEnabled=false;
              if (CloudSyncCurrentStatus==CloudSyncOnStatus) CloudSyncCurrentStatus=CloudSyncOffStatus;
              configVariables=EEPROM.read(0x08) & 0xFB;
              EEPROM.write(0x08,configVariables);
              updateEEPROM=true;
            }
          }
          // HTTP POST MQTT_enabled value
          else if (p->name().compareTo("MQTT_enabled")==0) {
            if ((p->value().compareTo("on")==0) && !mqttServerEnabled) {
              mqttServerEnabled=true;
              connectMqtt=true;auxCounter|=0x01;
              configVariables=EEPROM.read(0x08) | 0x40; //Set mqttServerEnabled bit to true (enabled)
              EEPROM.write(0x08,configVariables);
              updateEEPROM=true;
            }
            else if ((p->value().compareTo("off")==0) && mqttServerEnabled) {
              mqttServerEnabled=false;
              disconnectMqtt=true;
              configVariables=EEPROM.read(0x08) & 0xBF;
              EEPROM.write(0x08,configVariables);
              updateEEPROM=true;
            }
          }
          // HTTP POST Secure_MQTT_enabled value
          else if (p->name().compareTo("Secure_MQTT_enabled")==0) {
            if ((p->value().compareTo("on")==0) && !secureMqttEnabled) {
              secureMqttEnabled=true;
              if (mqttServerEnabled) {connectMqtt=true;auxCounter|=0x02;}  //Connect with authentication
              configVariables=EEPROM.read(0x08) | 0x80; //Set secureMqttEnabled bit to true (enabled)
              EEPROM.write(0x08,configVariables);
              updateEEPROM=true;
            }
            else if ((p->value().compareTo("off")==0) && secureMqttEnabled) {
              secureMqttEnabled=false;
              if (mqttServerEnabled) {connectMqtt=true;auxCounter|=0x04;} //Connect without authentication
              configVariables=EEPROM.read(0x08) & 0x7F;
              EEPROM.write(0x08,configVariables);
              updateEEPROM=true;
            }
          }
          // HTTP POST MQTTSERVER value
          else if (p->name().compareTo("MQTTSERVER")==0) {
            char auxMQTT[MQTT_SERVER_NAME_MAX_LENGTH];
            memset(auxMQTT,'\0',MQTT_SERVER_NAME_MAX_LENGTH);
            memcpy(auxMQTT,p->value().c_str(),p->value().length()); //End null not included
            if (mqttServer.compareTo(auxMQTT)!=0) {
              mqttServer=p->value();
              EEPROM.put(0x2BF,auxMQTT);
              if (mqttServerEnabled) {connectMqtt=true;auxCounter|=0x08;}
              updateEEPROM=true;
            }
          }
          // HTTP POST MQTTTOPIC value
          else if (p->name().compareTo("MQTTTOPIC")==0) {
            char auxMqttTopicPrefix[MQTT_TOPIC_NAME_MAX_LENGTH];
            memset(auxMqttTopicPrefix,'\0',MQTT_TOPIC_NAME_MAX_LENGTH);
            memcpy(auxMqttTopicPrefix,p->value().c_str(),p->value().length()); //End null not included
            if (mqttTopicPrefix.compareTo(auxMqttTopicPrefix)!=0) {
              mqttTopicPrefix=p->value();
              if (mqttTopicPrefix.charAt(mqttTopicPrefix.length()-1)!='/') mqttTopicPrefix+="/"; //Adding slash at the end if needed
              mqttTopicPrefix.toCharArray(auxMqttTopicPrefix,mqttTopicPrefix.length()+1);
              EEPROM.put(0x315,auxMqttTopicPrefix);
              if (mqttServerEnabled) {connectMqtt=true;auxCounter|=0x10;}
              updateEEPROM=true;
              mqttTopicName=mqttTopicPrefix+device; //Adding the device name to the MQTT Topic name
            }
          }
          // HTTP POST MQTT_Power_enabled value
          else if (p->name().compareTo("MQTT_Power_enabled")==0) {
            if ((p->value().compareTo("on")==0) && !powerMeasureEnabled && mqttClient.connected()) {
              powerMeasureEnabled=true;  //Subscription is done later in MQTTPOWERTOPIC
              if (debugModeOn) printLogln(String(millis())+" - [webServer cloud] - MQTT_Power_enabled - Request to subscribe to "+powerMqttTopic);
              configVariables=EEPROM.read(0x606) | 0x01; //Set powerMeasureEnabled bit to true (enabled)
              EEPROM.write(0x606,configVariables);
              updateEEPROM=true;
            }
            else if ((p->value().compareTo("off")==0) && powerMeasureEnabled && mqttClient.connected()) {
              if (powerMqttTopic.compareTo("")!=0) {
                powerMeasureEnabled=false;
                //Unsubscribe
                if (mqttClient.unsubscribe(powerMqttTopic.c_str())!=0 || !powerMeasureSubscribed)
                {
                  powerMeasureSubscribed=false;
                  if (debugModeOn) printLogln(String(millis())+" - [webServer cloud] - MQTT_Power_enabled - Unsubscribed to "+powerMqttTopic);
                  configVariables=EEPROM.read(0x606) & 0xFE; //Set powerMeasureEnabled bit to false (disabled)
                  EEPROM.write(0x606,configVariables);
                  updateEEPROM=true;
                }
                else {
                  if (debugModeOn) printLogln(String(millis())+" - [webServer cloud] - MQTT_Power_enabled - Wrong unsubscription to "+powerMqttTopic);
                }
              }
            }
          }
          // HTTP POST MQTTPOWERTOPIC value
          else if (p->name().compareTo("MQTTPOWERTOPIC")==0) {
            char auxMqttTopicPrefix[MQTT_TOPIC_NAME_MAX_LENGTH];
            memset(auxMqttTopicPrefix,'\0',MQTT_TOPIC_NAME_MAX_LENGTH);
            memcpy(auxMqttTopicPrefix,p->value().c_str(),p->value().length()); //End null not included
            if (powerMqttTopic.compareTo(auxMqttTopicPrefix)!=0 && p->value().compareTo("")!=0 && //New Topic. Subscribe it
                powerMeasureEnabled && mqttClient.connected()) {
              
              if (powerMeasureSubscribed) {
                if (mqttClient.unsubscribe(powerMqttTopic.c_str())!=0) { //Unsubscribe first to the previous topic
                  if (debugModeOn) printLogln(String(millis())+" - [webServer cloud] - MQTTPOWERTOPIC - Unsubscribed to "+powerMqttTopic);
                  powerMeasureSubscribed=false;
                }
                else {
                  if (debugModeOn) printLogln(String(millis())+" - [webServer cloud] - MQTTPOWERTOPIC - Wrong unsubscription to "+powerMqttTopic);
                }
              }
              if (!powerMeasureSubscribed) {
                // Subscribe now to the new topic
                powerMqttTopic=p->value();
                mqttClient.subscribe(powerMqttTopic.c_str(),0); //Subscribe now to the topic
                if (debugModeOn) printLogln(String(millis())+" - [webServer cloud] - MQTTPOWERTOPIC - Subscribed to \""+p->value()+"\"");
              }
              //UPdate EEPROM
              memset(auxMqttTopicPrefix,'\0',MQTT_TOPIC_NAME_MAX_LENGTH);
              powerMqttTopic.toCharArray(auxMqttTopicPrefix,powerMqttTopic.length()+1);
              EEPROM.put(0x53D,auxMqttTopicPrefix);
              updateEEPROM=true;
            }
            else {
              if (p->value().compareTo("")==0) {
                powerMeasureEnabled=false;
                if (debugModeOn) printLogln(String(millis())+" - [webServer cloud] - MQTTPOWERTOPIC - Can't subscribed to empty topic. Disabling powerMeasureEnabled");
              }
              else {
                if (powerMqttTopic.compareTo(auxMqttTopicPrefix)==0 && powerMeasureEnabled && //New Topic. Subscribe it
                    mqttClient.connected() && !powerMeasureSubscribed) {
                  //No powerMqttTopic change. Just subscribe to the topic after enabling the topic
                  mqttClient.subscribe(powerMqttTopic.c_str(),0); //Subscribe now to the topic
                  if (debugModeOn) printLogln(String(millis())+" - [webServer cloud] - MQTTPOWERTOPIC - No power MQTT topic change. Subscribed to \""+powerMqttTopic+"\"");
                }
              }
            }
          }

          // HTTP POST POWERTHRESHOLD value
          else if (p->name().compareTo("POWERTHRESHOLD")==0) {
            uint16_t auxPowerOnFlameThreshold=p->value().toInt();
            if (auxPowerOnFlameThreshold!=powerOnFlameThreshold) {
              powerOnFlameThreshold=auxPowerOnFlameThreshold;
              EEPROM.writeUShort(0x607,powerOnFlameThreshold);
              updateEEPROM=true;
            }
          }

          // HTTP POST MQTTUserName value
          else if (String(p->name()).compareTo("MQTTUserName")==0) {
            memset(auxUserName,'\0',MQTT_USER_CREDENTIAL_LENGTH);
            memcpy(auxUserName,p->value().c_str(),p->value().length()); //End null not included
            if (mqttUserName.compareTo(auxUserName)!=0) {
              mqttUserName=p->value();
              EEPROM.put(0x2FF,auxUserName);
              updateEEPROM=true;
              if (mqttServerEnabled && secureMqttEnabled) {connectMqtt=true;auxCounter|=0x20;}
            }
          }
          // HTTP POST MQTTUserPssw value
          else if (String(p->name()).compareTo("MQTTUserPssw")==0) {
            memset(auxUserPssw,'\0',MQTT_PW_CREDENTIAL_LENGTH);
            memcpy(auxUserPssw,p->value().c_str(),p->value().length()); //End null not included
            if (mqttUserPssw.compareTo(auxUserPssw)!=0) {
              mqttUserPssw=p->value();
              EEPROM.put(0x30A,auxUserPssw);
              updateEEPROM=true;
              if (mqttServerEnabled && secureMqttEnabled) {connectMqtt=true;auxCounter|=0x40;}
            }
          }
        }
      }
    
      if (debugModeOn) printLogln(String(millis())+" - [webServer cloud] - connectMqtt="+String(connectMqtt)+", disconnectMqtt="+String(disconnectMqtt)+", auxCounter=0x"+String(auxCounter,HEX));
      if (connectMqtt) {
        if (debugModeOn) printLogln("      [webServer cloud] - mqttClient.connected()="+String(mqttClient.connected()));
        //Disconnect first from the current MQTT server
        if (mqttClient.connected()) mqttClient.disconnect(true);
        while (mqttClient.connected()) {}; //Wait till get disconnected
        
        //Connect to the MQTT broker
        if (WiFi.status()==WL_CONNECTED && !mqttClient.connected() && mqttServerEnabled) { //Connect to MQTT broker again
          if (debugModeOn) printLogln("      [webServer cloud] - about to init Mqtt client");
          //mqttClientInit(false,false,false);
          forceMQTTConnect=true; //v1.4.1 Connect from the loop cycle to avoid the error "task_wdt: Task watchdog got triggered"
        }
        if (debugModeOn) printLogln("      [webServer cloud] - about to exit");
      }
      if (disconnectMqtt) {
        /*Code here to unsuscribe from the MQTT broker*/
        //removeTopics=true;
        //mqttClientPublishHADiscovery(mqttTopicName,device,WiFi.localIP().toString(),true); //Remove all topics from Home Assistant
        mqttClient.publish(String(mqttTopicName+"/LWT").c_str(), 0, false, "Offline\0"); //Availability message, not retain in the broker

        //Disconnect to the MQTT broker
        mqttClient.disconnect(true);
      }
      if (updateEEPROM) EEPROM.commit();
      //request->send(SPIFFS, WEBSERVER_INDEX_PAGE, String(), false, processor);
      //request->send(SPIFFS, WEBSERVER_INDEX_PAGE, "text/html");
      request->send(SPIFFS, WEBSERVER_INDEX_PAGE, String(),false);

      //Update powerMeasureEnabled and powerMeasureSubscribed variables anycase
      samples["powerMeasureEnabled"]=powerMeasureEnabled;
      samples["powerMeasureSubscribed"]=powerMeasureSubscribed;
    }
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  }); // /cloud HTTP_POST form
  
  webServer.on(WEBSERVER_DEFAULTCONF_PAGE, HTTP_POST, [](AsyncWebServerRequest *request) {
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow  
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    
    //reconnectWifiAndRestartWebServer=false;
    //resyncNTPServer=false;
    factoryReset=false;
    fileUpdateError=ERROR_UPLOAD_FILE_NOERROR;
    updateCommand=-1;
    AsyncWebServerResponse * auxResp=new AsyncFileResponse(SPIFFS, WEBSERVER_CONTAINER_PAGE, String(), false, processorContainer);

    int params = request->params();
    for(int i=0;i<params;i++) {
      const AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        // HTTP POST Bluetooth_enabled value
        if (p->name().compareTo("Maintenance_Default_Send")==0) {
          factoryReset=true;

          //Authentication is required
          if(!request->authenticate(userName.c_str(), userPssw.c_str())) {
            fileUpdateError=ERROR_UPLOAD_FILE_NOAUTH;
            //Setup a new response to send AuthenticationRequest headers in the container.html answer
            lastURI=String(WEBSERVER_MAINTENANCE_PAGE); //To go back to the right page from CONTAINER if the authentication fails
            auxResp->setCode(401);
            auxResp->addHeader("WWW-Authenticate", "Basic realm=\"Login Required\"");
          } 
          else {   
            //reconnectWifiAndRestartWebServer=true;
            //resyncNTPServer=true;
            factoryConfReset();
            EEPROM.commit();
          }
          //Factory Reset is done after serving WEBSERVER_CONTAINER_PAGE
        }
      }
    }
    //request->send(200,String("text/html"),String("<!DOCTYPE html><html lang=\"en\"><head><meta http-equiv=\"refresh\" content=\"0; URL=http://192.168.100.103\"/></head><body style=\"background-color:#34383b;\"></body></html>"));
    //request->send(SPIFFS, WEBSERVER_CONTAINER_PAGE, String(), false, processor);
    request->send(auxResp);
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });
  
  webServer.on(WEBSERVER_DEVICERESET_PAGE, HTTP_POST, [](AsyncWebServerRequest *request) {
    //This code is run after uploading the file
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    
    deviceReset=false;
    fileUpdateError=ERROR_UPLOAD_FILE_NOERROR;
    updateCommand=-1;
    AsyncWebServerResponse * auxResp=new AsyncFileResponse(SPIFFS, WEBSERVER_CONTAINER_PAGE, String(), false, processorContainer);
    
    int params = request->params();
    for(int i=0;i<params;i++) {
      const AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        // HTTP POST Maintenance_Reset_Send value
        if (p->name().compareTo("Maintenance_Reset_Send")==0) {
          deviceReset=true;

          //Authentication is required
          if(!request->authenticate(userName.c_str(), userPssw.c_str())) {
            fileUpdateError=ERROR_UPLOAD_FILE_NOAUTH;
            //Setup a new response to send AuthenticationRequest headers in the container.html answer
            lastURI=String(WEBSERVER_MAINTENANCE_PAGE); //To go back to the right page from CONTAINER if the authentication fails
            auxResp->setCode(401);
            auxResp->addHeader("WWW-Authenticate", "Basic realm=\"Login Required\"");
          }
          else {
            resetSWWebCount++; //Increase software reset from Web counter
            EEPROM.write(0x532,resetSWWebCount);
            EEPROM.commit();
          }
          //Device Reset is done after serving WEBSERVER_CONTAINER_PAGE
        }
      }
    }
    request->send(auxResp);
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  webServer.on(WEBSERVER_LOGSCONFIG_PAGE, HTTP_POST, [](AsyncWebServerRequest *request) {
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    
    int params = request->params();
    uint8_t configVariables=0;
    bool updateEEPROM=false, sysLogServerChange=false;
    AsyncWebServerResponse * auxResp=new AsyncFileResponse(SPIFFS, WEBSERVER_MAINTENANCE_PAGE, String(), false);
    
    //Authentication is required
    if(!request->authenticate(userName.c_str(), userPssw.c_str())) {
      //Setup a new response to send AuthenticationRequest headers in the container.html answer
      lastURI=String(WEBSERVER_MAINTENANCE_PAGE); //To go back to the right page from CONTAINER if the authentication fails
      auxResp->setCode(401);
      auxResp->addHeader("WWW-Authenticate", "Basic realm=\"Login Required\"");
      request->send(auxResp);
    }
    else 
    {
      //Checking the parameters
      for(int i=0;i<params;i++) {
        const AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST Debug Mode value
          if (p->name().compareTo("debug_mode_enabled")==0) {
            if ((p->value().compareTo("on")==0) && !debugModeOn) {
              debugModeOn=true;
              printLogln(String(millis())+" - [webServer - logs_config] - Debug Mode ON");
              samples["debugModeOn"]="DEBUG_ON";
              configVariables=EEPROM.read(0x606) | 0x04; //Set debugModeOn bit to true (enabled)
              EEPROM.write(0x606,configVariables);
              updateEEPROM=true;forceMQTTpublish=7;
            }
            else if ((p->value().compareTo("off")==0) && debugModeOn) {
              debugModeOn=false;
              samples["debugModeOn"]="DEBUG_OFF";
              printLogln(String(millis())+" - [webServer - logs_config] - Debug Mode OFF");
              configVariables=EEPROM.read(0x606) & 0xFB; //Unset debugModeOn bit (disabled)
              EEPROM.write(0x606,configVariables);
              updateEEPROM=true;forceMQTTpublish=7;
            }
          }
          // HTTP POST Serial Logs value
          else if (p->name().compareTo("serial_logs_enabled")==0) {
            if ((p->value().compareTo("on")==0) && !serialLogsOn) {
              serialLogsOn=true;
              samples["serialLogsOn"]="SERIAL_LOGS_ON";
              printLogln(String(millis())+" - [webServer - logs_config] - Serial Logs ON");
              configVariables=EEPROM.read(0x606) | 0x08; //Set serialLogsOn bit to true (enabled)
              EEPROM.write(0x606,configVariables);
              updateEEPROM=true;forceMQTTpublish=9;
            }
            else if ((p->value().compareTo("off")==0) && serialLogsOn) {
              serialLogsOn=false;
              samples["serialLogsOn"]="SERIAL_LOGS_OFF";
              printLogln(String(millis())+" - [webServer - logs_config] - Serial Logs OFF");
              configVariables=EEPROM.read(0x606) & 0xF7; //Unset serialLogsOn bit (disabled)
              EEPROM.write(0x606,configVariables);
              updateEEPROM=true;forceMQTTpublish=9;
            }
          }
          // HTTP POST Console Logs value
          else if (p->name().compareTo("console_logs_enabled")==0) {
            if ((p->value().compareTo("on")==0) && !webLogsOn) {
              webLogsOn=true;
              samples["webLogsOn"]="WEB_LOGS_ON";
              printLogln(String(millis())+" - [webServer - logs_config] - Web Console Logs ON");
              configVariables=EEPROM.read(0x606) | 0x10; //Set webLogsOn bit to true (enabled)
              EEPROM.write(0x606,configVariables);
              updateEEPROM=true;forceMQTTpublish=11;
            }
            else if ((p->value().compareTo("off")==0) && webLogsOn) {
              webLogsOn=false;
              samples["webLogsOn"]="WEB_LOGS_OFF";
              printLogln(String(millis())+" - [webServer - logs_config] - Web Console Logs OFF");
              configVariables=EEPROM.read(0x606) & 0xEF; //Unset webLogsOn bit (disabled)
              EEPROM.write(0x606,configVariables);
              updateEEPROM=true;forceMQTTpublish=11;
            }
          }
          // HTTP POST Syslogs value
          else if (p->name().compareTo("syslogs_enabled")==0) {
            if ((p->value().compareTo("on")==0) && !sysLogsOn) {
              sysLogsOn=true;
              samples["sysLogsOn"]="SYS_LOGS_ON";
              printLogln(String(millis())+" - [webServer - logs_config] - Syslog ON");
              configVariables=EEPROM.read(0x606) | 0x20; //Set sysLogsOn bit to true (enabled)
              EEPROM.write(0x606,configVariables);
              updateEEPROM=true;forceMQTTpublish=13;
            }
            else if ((p->value().compareTo("off")==0) && sysLogsOn) {
              sysLogsOn=false;
              samples["sysLogsOn"]="SYS_LOGS_OFF";
              printLogln(String(millis())+" - [webServer - logs_config] - Syslog OFF");
              configVariables=EEPROM.read(0x606) & 0xDF; //Unset sysLogsOn bit (disabled)
              EEPROM.write(0x606,configVariables);
              updateEEPROM=true;forceMQTTpublish=13;
            }
          }
          // HTTP POST SYSLOGSERVER value
          else if (p->name().compareTo("SYSLOGSERVER")==0) {
            char auxSYSLOG[SYSLOG_SERVER_NAME_MAX_LENGTH];
            memset(auxSYSLOG,'\0',SYSLOG_SERVER_NAME_MAX_LENGTH);
            memcpy(auxSYSLOG,p->value().c_str(),p->value().length()); //End null not included
            if (sysLogServer.compareTo(auxSYSLOG)!=0) {
              sysLogServer=p->value();
              samples["sysLogServer"]=sysLogServer;
              EEPROM.put(0x60D,auxSYSLOG);
              updateEEPROM=true;forceMQTTpublish=13;
              sysLogServerChange=true;
            }
          }
          // HTTP POST SYSLOGPORT value
          else if (p->name().compareTo("SYSLOGPORT")==0) {
            uint16_t auxSysLogServerUDPPort=p->value().toInt();
            if (auxSysLogServerUDPPort!=sysLogServerUDPPort) {
              sysLogServerUDPPort=auxSysLogServerUDPPort;
              samples["sysLogServerUDPPort"]=sysLogServerUDPPort;
              EEPROM.writeUShort(0x64D,sysLogServerUDPPort);
              updateEEPROM=true;forceMQTTpublish=13;
              sysLogServerChange=true;
            }
          }
        }
      }
    }

    #ifdef SYSLOG_SERVER
      if (sysLogsOn) {
        if (syslog.server.compareTo(sysLogServer)!=0) syslog.server=sysLogServer;
        if (syslog.port!=sysLogServerUDPPort) syslog.port=sysLogServerUDPPort;
      }
    #endif
    if (updateEEPROM) EEPROM.commit();

    request->send(auxResp);
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  }); //HTTP_POST WEBSERVER_LOGSCONFIG_PAGE

  webServer.on(WEBSERVER_COUNTERRESET_PAGE, HTTP_POST, [](AsyncWebServerRequest *request) {
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow  
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    
    //reconnectWifiAndRestartWebServer=false;
    //resyncNTPServer=false;
    AsyncWebServerResponse * auxResp=new AsyncFileResponse(SPIFFS, WEBSERVER_MAINTENANCE_PAGE, String(), false);

    int params = request->params();
    for(int i=0;i<params;i++) {
      const AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        // HTTP POST Bluetooth_enabled value
        if (p->name().compareTo("Reset_Counters_Button")==0) {

          //Authentication is required
          if(!request->authenticate(userName.c_str(), userPssw.c_str())) {
            fileUpdateError=ERROR_UPLOAD_FILE_NOAUTH;
            //Setup a new response to send AuthenticationRequest headers in the container.html answer
            lastURI=String(WEBSERVER_MAINTENANCE_PAGE); //To go back to the right page from CONTAINER if the authentication fails
            auxResp->setCode(401);
            auxResp->addHeader("WWW-Authenticate", "Basic realm=\"Login Required\"");
          } 
          else {   
            //Initialize reset and error counters
            bootCount=1;EEPROM.write(0x3DE,bootCount); 
            resetCount=0; EEPROM.write(0x3DF,resetCount); //uncontrolled resets
            resetPreventiveCount=0; EEPROM.write(0x41B,resetPreventiveCount); //preventive resets (mainly becuase low heap situation)
            resetSWCount=0; EEPROM.write(0x41C,resetSWCount); //reset from the HA restart button
            resetPreventiveWebServerCount=0; EEPROM.write(0x531,resetPreventiveWebServerCount); //preventive web server resets (mainly becuase low heap situation)
            resetSWWebCount=0; EEPROM.write(0x532,resetSWWebCount); //resets done from the web maintenance page
            resetSWMqttCount=0; EEPROM.write(0x533,resetSWMqttCount); //resets done from the HA (mqqtt) page
            resetSWUpgradeCount=0; EEPROM.write(0x534,resetSWUpgradeCount); //resets done due to firmware upgrade from maintenance web page
            resetWebServerCnt=0; EEPROM.write(0x53C,resetWebServerCnt); //Counter for Web Server resets (being WiFi connected but not serving web pages)
            resetPreventiveJSONCount=0; EEPROM.write(0x650,resetPreventiveJSONCount); //Counter for JSON resets due to JSORN error limit exceeded
            errorsWiFiCnt=0; EEPROM.write(0x535,errorsWiFiCnt); //Counter for WiFi errors
            errorsNTPCnt=0; EEPROM.write(0x536,errorsNTPCnt); // Counter for NTP sync errors
            errorsHTTPUptsCnt=0; EEPROM.write(0x537,errorsHTTPUptsCnt); // Counter for HTTP Cloud uploads errors
            errorsMQTTCnt=0; EEPROM.write(0x538,errorsMQTTCnt); //Counter for MQTT errors
            SPIFFSErrors=0; EEPROM.write(0x539,SPIFFSErrors); //Counter for SPIFFS errors
            errorsConnectivityCnt=0; EEPROM.write(0x53A,errorsConnectivityCnt); //Counter for Connectivity errors (being WiFi connected)
            errorsWebServerCnt=0; EEPROM.write(0x53B,errorsWebServerCnt); //Counter for Web Server errors (being WiFi connected but not serving web pages)
            errorsJSONCnt=0; EEPROM.write(0x64F,errorsJSONCnt); // Counter for JSON errors
            forceMQTTpublish=5;
            EEPROM.commit();
          }
          //Factory Reset is done after serving WEBSERVER_CONTAINER_PAGE
        }
      }
    }
    //request->send(200,String("text/html"),String("<!DOCTYPE html><html lang=\"en\"><head><meta http-equiv=\"refresh\" content=\"0; URL=http://192.168.100.103\"/></head><body style=\"background-color:#34383b;\"></body></html>"));
    //request->send(SPIFFS, WEBSERVER_CONTAINER_PAGE, String(), false, processor);
    request->send(auxResp);
    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  }); //HTTP_POST WEBSERVER_COUNTERRESET_PAGE
  
  // Route for maintenance_upload_firmware
  webServer.on(WEBSERVER_UPLOADFILE_PAGE, HTTP_POST, [](AsyncWebServerRequest *request) {
    //lastTimeBLECheck=loopStartTime+millis()+BLE_PERIOD_EXTENSION; //Avoid BLE Advertising during BLE_PERIOD_EXTENSION from now
    if (esp_get_free_heap_size()<WEBSERVER_MIN_HEAP_SIZE) {if (debugModeOn) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize ("+String(esp_get_free_heap_size())+") < WEBSERVER_MIN_HEAP_SIZE ("+String(WEBSERVER_MIN_HEAP_SIZE)+"). Can't serve the web page. Return.");} return;} /*Return if heap is lower than threshold*/ 
    webServerResponding=true;  //This prevents sending iBeacons to prevent heap overflow  
    if (debugModeOn && Update.progress()==0) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+", heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
    while (blockWebServer) {esp_task_wdt_reset(); /*Reset the watchdog*/ vTaskDelay(pdMS_TO_TICKS(300)); /* Delay for 300 milliseconds */}
    //if (isBeaconAdvertising || BLEtoBeLoaded) {delay(WEBSERVER_SEND_DELAY);} //Wait for iBeacon to stop to prevent heap overflow
    
    //This code is run after uploading the file
      int params = request->params();

      //Checking if the POST comes from the browser with the active cookie
      if (memcmp(activeCookie,request->getHeader("Cookie")->value().c_str(),request->getHeader("Cookie")->value().length())!=0) {
        //Wrong cookie
        //It's supposed that errorOnWrongCookie was updated propery during the upload handler
        deviceReset=false; //This flags prevents from serving wrong content in container.html when there's a valid ongoing upload in pararell
        fileUpdateError=errorOnWrongCookie;
        if (errorOnWrongCookie==ERROR_UPLOAD_FILE_NOERROR) if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware POST] - Coming from Wrong cookie upload but there is no ERROR. It's strange (meditation required). Serving container.html.");

        if (fileUpdateError==ERROR_UPLOAD_FILE_NOAUTH) { 
          if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware POST] - ERROR - Still no active cookie: Authentication required but not detected.");
          //Setup a new response to send AuthenticationRequest headers in the container.html answer
          AsyncWebServerResponse * auxResp=new AsyncFileResponse(SPIFFS, WEBSERVER_CONTAINER_PAGE, String(), false, processorContainer);
          auxResp->setCode(401);
          auxResp->addHeader("WWW-Authenticate", "Basic realm=\"Login Required\"");
          request->send(auxResp);
          //fileUpdateError=ERROR_UPLOAD_FILE_NOERROR;
        }
        else {
          if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware POST] - Coming from Wrong cookie upload with ERROR code 0x"+String(fileUpdateError,HEX)+". Serving container.html.");
          request->send(SPIFFS, WEBSERVER_CONTAINER_PAGE, String(), false, processorContainer);
        }

         errorOnWrongCookie=ERROR_UPLOAD_FILE_NOERROR;
      }
      else {
        //Active cookie
        //Let's check the errors and then send the response
        fileUpdateError=errorOnActiveCookie;
        if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware POST] - Coming from Active cookie upload with ERROR code 0x"+String(fileUpdateError,HEX)+". Serving container.html.");

        if (fileUpdateError==ERROR_UPLOAD_FILE_NOERROR) //Check Form parameters only if no errors while uploading the file 
        { 
          int i;
          for(i=0;i<params;i++) {
            const AsyncWebParameter* p = request->getParam(i);
            if(p->isPost()){
              // HTTP POST Maintenance_Reset_Send value
              if (p->name().compareTo("select_file")==0 && !p->isFile())
                fileUpdateError=ERROR_UPLOAD_FILE_NOFILE;
              if (p->name().compareTo("select_file")!=0 && p->name().compareTo("file-upload")!=0) 
                fileUpdateError=ERROR_UPLOAD_FILE_POST_PRM_NAME;
              else if (p->name().compareTo("select_file")==0) {          
                //Everything OK  
                String fileName=p->value();
                if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware POST] - fileName="+fileName);
              }
            }
          }

          if (i!=2) fileUpdateError=ERROR_UPLOAD_FILE_POST_PRM_NUMBER;
        }

        if (fileUpdateError==ERROR_UPLOAD_FILE_NOERROR) {
          //No error. Let's restart the device
          if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware POST] - Upload finished and Post treated with no errors. Should reboot after the file is uploaded if no additional errros.");
          deviceReset=true;
          resetSWUpgradeCount++; //Increase the firmware update counter
          EEPROM.write(0x534,resetSWUpgradeCount);
          EEPROM.commit();
          request->send(SPIFFS, WEBSERVER_CONTAINER_PAGE, String(), false, processorContainer);
        }
        else if (fileUpdateError==ERROR_UPLOAD_FILE_NOAUTH) { 
          if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware POST] - ERROR: Authentication required but not detected.");
          //Setup a new response to send AuthenticationRequest headers in the container.html answer
          AsyncWebServerResponse * auxResp=new AsyncFileResponse(SPIFFS, WEBSERVER_CONTAINER_PAGE, String(), false, processorContainer);
          auxResp->setCode(401);
          auxResp->addHeader("WWW-Authenticate", "Basic realm=\"Login Required\"");
          request->send(auxResp);
          //fileUpdateError=ERROR_UPLOAD_FILE_NOERROR;
        }
        else {
          //Something was wrong with the file update process. Inform about it.
          if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware POST - 0] - ERROR with File update. Serving container.html.");         
          request->send(SPIFFS, WEBSERVER_CONTAINER_PAGE, String(), false, processorContainer);
        }

        errorOnActiveCookie=ERROR_UPLOAD_FILE_NOERROR;
      }
      
      //Init variable to let them ready for the next upload
      updateCommand=-1;
      fileUpdateSize=0;
      fileUpdateError=ERROR_UPLOAD_FILE_NOERROR;
    }, [&](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    //This code is run first
    fileUpdateName=filename;

    //Doing some checks first before allowing the file upload

    //Cookie is needed to allow to upload the file
    if (!request->hasHeader("Cookie")) {
      errorOnWrongCookie=ERROR_UPLOAD_FILE_NOCOOKIE;
      if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware upload] - No Cookie detected. Close socket and Return.");  
      return;
    }

    //Check cookie length and cookie name
    if ((request->getHeader("Cookie")->value().length() > COOKIE_SIZE-1) ||
        0!=String("id-"+device).compareTo(request->getHeader("Cookie")->value().substring(0,request->getHeader("Cookie")->value().indexOf('='))) )
    {
      errorOnWrongCookie=ERROR_UPLOAD_FILE_BADCOOKIE_FORMAT;
      if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware upload] - Bad cookie format: "+request->getHeader("Cookie")->value()+", size "+String(request->getHeader("Cookie")->value().length())+" greater than "+String(COOKIE_SIZE-1)+" or wrong name. Close socket and return. Comparing "+String("id-"+device)+" with "+request->getHeader("Cookie")->value().substring(0,request->getHeader("Cookie")->value().indexOf('=')));
      return;
    }

    if (Update.progress()!=0) {
      //There's one ongoing file upload. Let's check if this is the right cookie
      if (memcmp(activeCookie,request->getHeader("Cookie")->value().c_str(),request->getHeader("Cookie")->value().length())!=0) {
        //No right cookie. Bye
        errorOnWrongCookie=ERROR_UPLOAD_FILE_UPLOAD_ONGOING;
        if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware upload] - There is an ongoing file upload. Return.");
        return;
      }
    }

    //Authentication is needed
    if(!request->authenticate(userName.c_str(), userPssw.c_str())){
      errorOnWrongCookie=ERROR_UPLOAD_FILE_NOAUTH;
      lastURI=String(WEBSERVER_MAINTENANCE_PAGE); //To go back to the right page from CONTAINER if the authentication fails
      if (debugModeOn && index==0) printLogln(String(millis())+" - [maintenance_upload_firmware upload] - Authentication required, index="+String(index)+"\n   - Faulty cookie="+request->getHeader("Cookie")->value());
      return;
    }

    //No further process if there are previous errors
    //No checking errors in this point prevents from stoping right ongloing uploads from
    // someone using a different browser. Cookies conditions are checked below.
    if (memcmp(activeCookie,request->getHeader("Cookie")->value().c_str(),request->getHeader("Cookie")->value().length())!=0) {
      if (errorOnWrongCookie!=ERROR_UPLOAD_FILE_NOERROR) {return;}
    }
    else {
      if (errorOnActiveCookie!=ERROR_UPLOAD_FILE_NOERROR) {return;}
    }

    //Upload handler chunks in data
    if (!index) {
      if (Update.progress()==0) {
        if (memcmp(currentSetCookie,request->getHeader("Cookie")->value().c_str(),request->getHeader("Cookie")->value().length())==0) {
          //It's the same cookie than the one created when serving the maintenance.html page
          //Saving current cookie
          fileUpdateSize=0;
          memcpy(activeCookie,request->getHeader("Cookie")->value().c_str(),request->getHeader("Cookie")->value().length()); //End null not included
          if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware upload] - Cookie="+String(activeCookie));
        }
        else {
          //Not the same cookie than the one created when serving the maintenance.html page
          //Probably the POST comes from a wrong request. Bye
          errorOnWrongCookie=ERROR_UPLOAD_FILE_BADCOOKIE;
          if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware upload] - Bad Cookie received="+request->getHeader("Cookie")->value()+" instead of "+String(currentSetCookie));
          return;
        }
      }
      else {
        //Checking if it is the current cookie
        if (memcmp(activeCookie,request->getHeader("Cookie")->value().c_str(),request->getHeader("Cookie")->value().length())==0) {
          //Coming from abort action from the same broswer. Abort previous upload and start again
          if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware upload] - Detected upload abortion from the same broswer. Reset and starting again");
          Update.abort(); 
          fileUpdateSize=0;
        }
        else {
          //Coming from different browser. But there is an ongoing upload. Error. Bye
          //It's supposed that this point is never reached
          errorOnWrongCookie=ERROR_UPLOAD_FILE_UPLOAD_ONGOING;
          if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware upload] - There is an ongoing file upload. Return.");
          return;
        }
      }

      //Beyond this point the browser has the active cookie

      if (filename.compareTo("firmware.bin")==0) updateCommand=U_FLASH;
      else if (filename.compareTo("filesystem.bin")==0) updateCommand=U_SPIFFS;
      else {
        errorOnActiveCookie=ERROR_UPLOAD_FILE_WRONG_FILE_NAME;
        updateCommand=-1;
        if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware upload] - Error Wrong file name, filename="+filename+", index="+String(index)+", len="+String(len)+", loaded bytes(fileUpdateSize)="+String(fileUpdateSize+len));
        return;
      }

      if (!Update.begin(UPDATE_SIZE_UNKNOWN, updateCommand)) { // Start with max available size
        Update.printError(Serial);
        errorOnActiveCookie=ERROR_UPLOAD_FILE_UPDATE_BEGIN;
        if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware upload] - Error Update.begin()"+String(Serial)+", errorOnActiveCookie="+String(errorOnActiveCookie)+", index="+String(index)+", len="+String(len)+", loaded bytes(fileUpdateSize)="+String(fileUpdateSize+len));
        return;
      }
    }

    // Write chunked data to the free sketch space if it's the same cookie (same upload session)
    if (len) {
      if (memcmp(activeCookie,request->getHeader("Cookie")->value().c_str(),request->getHeader("Cookie")->value().length())!=0) {
        //No same cookie than the one used for starting the file upload
        //It's supposed that this point is never reached
        errorOnWrongCookie=ERROR_UPLOAD_FILE_UPLOAD_ONGOING;
        if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware upload] - Bad cookie detected as there is an ongoing file upload. Cookie detected: "+request->getHeader("Cookie")->value()+" rather than "+String(activeCookie)+" in index="+String(index)+" len="+String(len));
        return;
      }
      
      fileUpdateSize+=len;

      //Check FLASH memory left always (both firmware.bin and spiffs.bin must fit into the remaining space)
      if (filename.compareTo("firmware.bin")==0) {
        //OTA partition for firmware.bin
        if ((int)(OTAAvailableSize-fileUpdateSize)<0) {
          errorOnActiveCookie=ERROR_UPLOAD_FILE_NOFLASHSPACE_LEFT;
          if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware upload] - Error No OTA space left in FLASH memory. OTAAvailableSize ("+String(OTAAvailableSize)+") - fileUpdateSize ("+String(fileUpdateSize)+") = "+String((int)(OTAAvailableSize-fileUpdateSize))+" B"+", index="+String(index)+", len="+String(len));
          Update.abort();
          return;
        }

        //Check header if it's really a binary file
        if (index==0) {
          //Checking the Magic Byte
          uint8_t *dataPtr=data;
          if (*dataPtr != BINARY_HD_MAGIC_NUMBER || len<(BINARY_HD_MAGIC_WORD_ADDR+3)) {
            errorOnActiveCookie=ERROR_UPLOAD_FILE_NOBIN_FILE;
            if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware upload] - Error Binary file. Magic Number=0x"+String(*dataPtr,HEX)+" rather than 0x"+String(BINARY_HD_MAGIC_NUMBER,HEX)+" or len("+String(len)+") < BINARY_HD_MAGIC_WORD_ADDR+3("+String(BINARY_HD_MAGIC_WORD_ADDR+3)+")");
            Update.abort();
            return;
          }
          //Checking the Magic WORD
          for (int i=0; i<BINARY_HD_MAGIC_WORD_ADDR; i++) dataPtr++;
          uint8_t auxByte1=*dataPtr;
          uint32_t receivedMagic=auxByte1;
          dataPtr++;auxByte1=*dataPtr;receivedMagic|=(auxByte1<<8);
          dataPtr++;auxByte1=*dataPtr;receivedMagic|=(auxByte1<<16);
          dataPtr++;auxByte1=*dataPtr;receivedMagic|=(auxByte1<<24);
          if (receivedMagic != ((uint32_t) BINARY_HD_MAGIC_WORD)) {
            errorOnActiveCookie=ERROR_UPLOAD_FILE_NOBIN_FILE;
            if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware upload] - Error Binary file. Magic Word=0x"+String((uint32_t) receivedMagic,HEX)+" rather than 0x"+String(BINARY_HD_MAGIC_WORD,HEX));
            Update.abort();
            return;
          }
        }
      }
      else {
        //SPIFFS partition for spiffs.bin
        if ((int)(SPIFFSAvailableSize-fileUpdateSize)<0) {
          errorOnActiveCookie=ERROR_UPLOAD_FILE_NOSPIFFSPACE_LEFT;
          if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware upload] - Error No SPIFFS space left in FLASH memory. SPIFFSAvailableSize ("+String(SPIFFSAvailableSize)+") - fileUpdateSize ("+String(fileUpdateSize)+") = "+String((int)(SPIFFSAvailableSize-fileUpdateSize))+" B"+", index="+String(index)+", len="+String(len));
          Update.abort();
          return;
        }

        //Check if it's really a SPIFFS partition
        u16_t bix=(u16_t)(fileUpdateSize/SPIFFS_CFG_LOG_BLOCK_SZ); //Current block index
        size_t magicAddr=(bix*SPIFFS_CFG_LOG_BLOCK_SZ)+SPIFFS_MAGIC_ADDR; //Magic address for the current block
        if ((fileUpdateSize-len)<=magicAddr && fileUpdateSize>=(magicAddr+2)) {
          //Magic number received in this chunk
          u16_t calculatedMagic=(0x20140529 ^ SPIFFS_CFG_LOG_PAGE_SZ ^ ((SPIFFSAvailableSize/SPIFFS_CFG_LOG_BLOCK_SZ) - (bix)));
          uint8_t *dataPtr=data;
          for (int i=0; i<magicAddr-(fileUpdateSize-len); i++) dataPtr++;
          uint8_t auxByte1=*dataPtr;
          dataPtr++;
          uint8_t auxByte2=*dataPtr;
          u16_t receivedMagic=(auxByte2<<8) | auxByte1;
          if (receivedMagic!=calculatedMagic) {
            //No SPIFFS partition
            errorOnActiveCookie=bix==0?ERROR_UPLOAD_FILE_NOSPIFFS_FILE:ERROR_UPLOAD_FILE_NOSPIFFS_FILE_NOSAFE;
            if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware upload] - Error No SPIFFS file. Index="+String(index)+", block="+String(bix)+", magic addr=0x"+String(magicAddr,HEX)+", calculatedMagic=0x"+String(calculatedMagic,HEX)+", receivedMagic=0x"+String(receivedMagic,HEX));
            Update.abort();
            return;
          }
        }

        if(!index && len<magicAddr) {
          //File size lower than SPIFFS header, so No SPIFFS partition
          errorOnActiveCookie=bix==0?ERROR_UPLOAD_FILE_NOSPIFFS_FILE:ERROR_UPLOAD_FILE_NOSPIFFS_FILE_NOSAFE;
          if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware upload] - Error No SPIFFS file.");
          Update.abort();
          return;
        }
      }
      
      //Check SPIFFS memory left to upload files into SPIFFS
      //This code should be used if files are uploaded to SPIFFS (no filesystem.bin)
      /*if ((int)(fileSystemSize-fileSystemUsed-fileUpdateSize)<=0) {
        fileUpdateError=ERROR_UPLOAD_FILE_NOSPIFFSPACE_LEFT;
        if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware upload] - Error No SPIFFS memory left. fileSystemSize ("+String(fileSystemSize)+") - fileSystemUsed ("+String(fileSystemUsed)+") - fileUpdateSize ("+String(fileUpdateSize)+") = "+String((int)(fileSystemSize-fileSystemUsed-fileUpdateSize))+" B");
        Update.abort();
        return;
      }*/

      if (Update.write(data, len) != len) {
          Update.printError(Serial);
          errorOnActiveCookie=ERROR_UPLOAD_FILE_UPDATE_WRITE;
          if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware upload] - Error Update.write(), Serial="+String(Serial)+", errorOnActiveCookie="+String(errorOnActiveCookie)+", index="+String(index)+", len="+String(len)+", loaded bytes(fileUpdateSize)="+String(fileUpdateSize));
          Update.abort();
          return;
      }
    }

    if (final) { // if the final flag is set then this is the last frame of data
      if (memcmp(activeCookie,request->getHeader("Cookie")->value().c_str(),request->getHeader("Cookie")->value().length())!=0) {
        //No same cookie than the one used for starting the file upload. Send error message
        //It's supposed that this point is never reached
        errorOnWrongCookie=ERROR_UPLOAD_FILE_UPLOAD_ONGOING;
        if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware upload] - In final block, bad cookie detected as there is an ongoing file upload. Cookie detected: "+request->getHeader("Cookie")->value()+" rather than "+String(activeCookie)+" in index="+String(index)+" len="+String(len));
        return;
      }

      if (errorOnActiveCookie!=ERROR_UPLOAD_FILE_NOERROR) {return;}

      if (Update.end(true)) { //true to set the size to the current progress
        //Inform Reset will be done and then reset the device
        errorOnActiveCookie=ERROR_UPLOAD_FILE_NOERROR;
        if (debugModeOn) printLogln(String(millis())+" - [maintenance_upload_firmware upload] - Update Success - Should reboot now"+", errorOnActiveCookie="+String(errorOnActiveCookie)+", index="+String(index)+", len="+String(len)+", loaded bytes(fileUpdateSize)="+String(fileUpdateSize));
        return;
      } 
      else {
        Update.printError(Serial);
        errorOnActiveCookie=ERROR_UPLOAD_FILE_UPDATE_END;
        if (debugModeOn) {printLogln(String(millis())+" - [maintenance_upload_firmware upload] - Error Update.end()"+String(Serial)+", errorOnActiveCookie="+String(errorOnActiveCookie)+", index="+String(index)+", len="+String(len)+", loaded bytes(fileUpdateSize)="+String(fileUpdateSize));}
        Update.abort();
        return;
      }
    }

    webServerResponding=false;   //WebServer ends, heap is going to be realeased, so BLE iBeacons are allowed agin
    lastTimeWebPageServed=millis();  //ISS019 - v1.1.3 - lastTimeWebPageServed
    if (debugModeOn && Update.progress()==0) {printLogln(String(millis())+" - [webServer.on] - "+request->url()+" exit. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));}
  });

  // Handle Web Server Events
  webEvents.onConnect([](AsyncEventSourceClient *client){
    if (debugModeOn) printLogln(String(millis())+" - [webEvents.onConnect] - Client reconnected! Last message ID that it got is: "+String(client->lastId()));
    if (debugModeOn) printLogln(String(millis())+" - [webEvents.onConnect] - Client reconnected! client->client()->remoteIP()="+client->client()->remoteIP().toString());
    if (debugModeOn) printLogln(String(millis())+" - [webEvents.onConnect] - Client reconnected! client->client()->remotePort()="+String(client->client()->remotePort()));

    if(client->lastId()){
      if (debugModeOn) printLogln(String(millis())+" - [webEvents.onConnect] - Client reconnected! Last message ID that it got is:"+String(client->lastId()));
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  webServer.addHandler(&webEvents);
  
  // Start web server
  if (debugModeOn) {printLogln(String(millis())+" - [initWebServer] - Begin webServer. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));} // ---->
  webServer.begin();

  if (debugModeOn) {printLogln(String(millis())+" - [initWebServer] - Exit now. heapSize="+String(esp_get_free_heap_size())+", heapBlockSize="+String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))+", minHeapSeen="+String(esp_get_minimum_free_heap_size()));} // ---->
  return NO_ERROR;
} // -- initWebServer --