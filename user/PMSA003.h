#ifndef __PMSA003_H
#define __PMSA003_H

#include <stdbool.h>
#include <stdint.h>
// #include "inc/hw_memmap.h"
// #include "driverlib/rom.h"
// #include "driverlib/gpio.h"
// #include "Variable.h"
// #include "modbus.h"
// #include "debugprint.h"


// extern void PM25Sensor_Action(void);
// extern void PM25CMDSend(uint8_t *data, int count);
// extern int PM25_TimeOut;
// extern uint16_t PM25ERRORCode,PM25Ver;

// extern float PM01Value,PM25Value,PM10Value,PM25rawdata,PM25CF1;

// typedef enum 
// {
//   PM25_Start      = 0,
//   PM25_Auto       = 1,
//   PM25_Manual     = 2,
//   PM25_ErrStauts  = 3,
//   PM25_Shutdown   = 4,
//   PM25_BootUP     = 5,
//   PM25_ReadValue  = 6,
//   PM25_FanClean   = 7
// }PM25_Action_Status;

// extern PM25_Action_Status PM25_Status;


extern void CalculatePMSA003Value(uint8_t* pbuff, uint16_t len);
extern float GetPMSA003Value_PM25();
extern float GetPMSA003Value_PM10();
extern bool CheckPM25Valid(uint8_t* pbuff, uint16_t len);

#endif