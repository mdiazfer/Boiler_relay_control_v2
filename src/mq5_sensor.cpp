/* Dealing with MQ5 Gas sensor

*/

#include "mq5_sensor.h"

/******************************************************
 Function calculate_R0
 Target: Calculate value of R0 in MQ5 Sensor according to https://www.rxelectronics.sg/datasheet/a7/101020056.pdf
 Parameters: No parmeters
 *****************************************************/
void calculate_R0() {
  gasSample=0;gasVoltCalibrated=0;
  float gasVolt=0,RS_air=0,R0=0,R0Calibrated=0; //This values are based on analogRead() function rather than analogReadMilliVolts(). Should be less accurate

  for (uint8_t i=0; i<ADC_SAMPLES; i++) {gasSample+=analogRead(PIN_GAS_SENSOR_A0); gasVoltCalibrated+=analogReadMilliVolts(PIN_GAS_SENSOR_A0);}
  gasSample=gasSample/ADC_SAMPLES;
  gasVoltCalibrated=gasVoltCalibrated/ADC_SAMPLES;
  gasVolt=gasSample/4096*3.3; //Sample multiplied by number of volt steps (volt max=3.3v in ESP32)
  RS_air=(3.3-gasVolt)/gasVolt;
  RS_airCalibrated=(3300-gasVoltCalibrated)/gasVoltCalibrated;
  R0=RS_air/6.5; // The ratio of RS/R0 is 6.5 in a clear air as per datasheet
  R0Calibrated=RS_airCalibrated/6.5; // The ratio of RS/R0 is 6.5 in a clear air as per datasheet

  if (debugModeOn) {
    boardSerialPort.println("         [calculate_R0] - GAS Samples taken.");
    boardSerialPort.println("                            gasSample="+String(gasSample)+", gasVolt="+String(gasVolt)+"v, RS_air="+String(RS_air)+", R0="+String(R0));
    boardSerialPort.println("                            gasSample="+String(gasSample)+", gasVoltCalibrated="+String(gasVoltCalibrated)+"mv, RS_airCalibrated="+String(RS_airCalibrated)+", R0Calibrated="+String(R0Calibrated));
  }
}

/******************************************************
 Function get_resistence_ratio
 Target: Calculate the RS_Current/R0 ratio of of the MQ5 Sensor according to https://www.rxelectronics.sg/datasheet/a7/101020056.pdf
 Parameters: No parmeters
 *****************************************************/
float get_resistence_ratio(boolean debugModeOn) {
  gasVoltCalibrated=0;
  float ratio=0;

  for (uint8_t i=0; i<ADC_SAMPLES; i++) {gasSample+=analogRead(PIN_GAS_SENSOR_A0); gasVoltCalibrated+=analogReadMilliVolts(PIN_GAS_SENSOR_A0);}
  gasVoltCalibrated=gasVoltCalibrated/ADC_SAMPLES;
  RS_CurrentCalibrated=(3300-gasVoltCalibrated)/gasVoltCalibrated;
  ratio=RS_CurrentCalibrated/R0_CALIBRATED;
  if (debugModeOn) {
    boardSerialPort.println("\n         [get_resistence_ratio] - GAS Samples taken.");
    boardSerialPort.println("              gasVoltCalibrated="+String(gasVoltCalibrated)+"mv, RS_CurrentCalibrated="+String(RS_CurrentCalibrated)+", R0_CALIBRATED="+String(R0_CALIBRATED)+", ratio RS_CurrentCalibrated/R0_CALIBRATED="+String(ratio));
  }
  return ratio;
}