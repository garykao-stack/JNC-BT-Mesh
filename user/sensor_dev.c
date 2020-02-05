
//richard Add
 /* BG stack headers */
#include "global.h"

#include "device_bus.h"
#include "AD7147.h"
#include "sensor_dev.h"

void SensorInit_2()
{TraceProc();
}

void SensorDevInit()
{TraceProc();
    SensorInit_2();
    AD7147Init();
}





