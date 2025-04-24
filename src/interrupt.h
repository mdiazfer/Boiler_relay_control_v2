#include <HardwareSerial.h>
#include "global_setup.h"

extern RTC_DATA_ATTR uint64_t lastInterruptTime;
extern uint16_t rebounds;
extern bool thermostateStatus,thermostateInterrupt,gasInterrupt,debugModeOn;
extern HardwareSerial boardSerialPort;
extern bool gasClear;

//Routine to treat the interrupt
void IRAM_ATTR thermostate_change();
void IRAM_ATTR gas_probe_triggered();
