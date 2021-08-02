#include "global.h"

//richard Add
/* BG stack headers */
#include "sensor_server.h"
#include "mesh_event.h"
#include "water_level_mesh.h"
#include "device_bus.h"
#include "bus_rs485.h"
#include "jnc_cmd.h"
#include "mod_bus.h"
#include "modbus_to_Mesh.h"
#include "cmd_to_bt_mesh.h"
#include "com_port.h"
//uchar  ComPortBuff[COM_PORT_BUFF_SIZE];   // to receive cmd from COM/RS485

void ComPortInit()
{
    
    ModBusInit();
    JncCmdInit();
}


void ComPortProc(void)
{
 //   uchar response_num;
//    UsartMonitor();
    return;
}



