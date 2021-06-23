#include "global.h"
#include "sensor_server.h"
#include "mesh_event.h"
#include "water_level_mesh.h"
#include "device_bus.h"
#include "bus_rs485.h"
#include "jnc_cmd.h"
#include "mod_bus.h"



void ModBusInit(void)
{
    
}

uchar ModBusProc(void)
{    
    PUCHAR pCmd;    
    pCmd = UsartGetBuff(USART_ID_RX);
    
}


