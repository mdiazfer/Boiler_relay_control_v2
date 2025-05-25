#include <SoftwareSerial.h>
#include <HardwareSerial.h>
#include "global_setup.h"

extern RTC_DATA_ATTR HardwareSerial boardSerialPort;

extern bool debugModeOn;
extern float gasSample,gasVoltCalibrated,RS_airCalibrated,RS_CurrentCalibrated;

#ifndef _PRINT_LOG_DEFINITION_
  extern void printLogln(String logMessage, unsigned char base=10);
  extern void printLog(String logMessage, unsigned char base=10);
  extern void printLogln(uint8_t logMessage, unsigned char base=10);
  extern void printLog(uint8_t logMessage, unsigned char base=10);
  extern void printLogln(tm * timeinfo, const char *format);
  extern void printLog(tm * timeinfo, const char *format);
  #define _PRINT_LOG_DEFINITION_
#endif

void calculate_R0();
float get_resistence_ratio(boolean debugModeOn);
