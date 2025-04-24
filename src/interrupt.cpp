/* Dealing with interrupts

*/

#include "interrupt.h"
 
void IRAM_ATTR thermostate_change() {
   /******************************************************
   Function thermostate_change
   Target: Routine to treat the interrupt in PIN_THERMOSTATE
   Parameters: No parameters
   *****************************************************/

  int value=digitalRead(PIN_THERMOSTATE);
  lastInterruptTime=millis();
  rebounds++;
  thermostateInterrupt=true;
  //NO Serial coms in interrupt routing, otherwise CORE exception is triggered beacue watchdog timeout - https://stackoverflow.com/questions/71992044/esp32-core-1-paniced-interrupt-wdt-timeout-on-cpu1
  //if (debugModeOn) {boardSerialPort.println(String(lastInterruptTime)+" - [thermostate_change] - BEGIN, thermostateStatus="+String(thermostateStatus)+", rebounds="+String(rebounds));}
}

void IRAM_ATTR gas_probe_triggered() {
   /******************************************************
   Function gas_probe_triggered
   Target: Routine to treat the interrupt in PIN_GAS_SENSOR_D0
   Parameters: No parameters
   *****************************************************/
  if (gasClear) gasInterrupt=true; //Only flag the interrupt with clear air
  gasClear=false;
  
  //NO Serial coms in interrupt routing, otherwise CORE exception is triggered beacue watchdog timeout - https://stackoverflow.com/questions/71992044/esp32-core-1-paniced-interrupt-wdt-timeout-on-cpu1
  //if (debugModeOn) {boardSerialPort.println(String(millis())+" - [gas_probe_triggered] - BEGIN, gasClear="+String(gasClear));}
}

