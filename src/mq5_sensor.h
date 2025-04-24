#include <SoftwareSerial.h>
#include <HardwareSerial.h>
#include "global_setup.h"

extern bool debugModeOn;
extern HardwareSerial boardSerialPort;
extern float gasSample,gasVoltCalibrated,RS_airCalibrated,RS_CurrentCalibrated;

void calculate_R0();
float get_resistence_ratio(boolean debugModeOn);