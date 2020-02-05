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
#include "com_port.h"
//uchar  ComPortBuff[COM_PORT_BUFF_SIZE];   // to receive cmd from COM/RS485

void ComPortInit()
{
    
    ModBusInit();
    JncCmdInit();
    ModbusToMeshInit();
    SetNodeStatus(STATUS_MODBUS_MESH,ON); // debug for
}


void ComPortProc(void)
{//TraceProc();
    uchar response_num;
    UsartMonitor();
    return;
    
    if(GetNodeStatus(STATUS_CLIENT))
        ModbusToMeshClientProc();
    else
        ModbusToMeshServerProc();
    return;
    
    if(UsartGetStatusRxEnd())
        {
        //TraceDec1("CounterRx", CounterRx); Trace(RxBuff);
        
        response_num = ProtocolProc();
        if(response_num) 
          {            
           // ComPortSendCmd(response_num);
          }
        }

    return;        
}



// 1. ModBus
// 2. JNC Protocol
uchar ProtocolProc()
{    
    uchar loop;
    PUCHAR pTxBuff;    

    if(GetNodeStatus(STATUS_MOD_BUS))
        ModBusProc();
    else if(GetNodeStatus(STATUS_JNC_CMD))       
        JncCmdProc();
    else if(GetNodeStatus(STATUS_MODBUS_MESH))
        ModbusToMeshClientProc();
    else TraceErr("ProtocolProc 1");

    
    //pTxBuff = UsartGetBuff(USART_ID_TX);    
    //for(loop=0; loop<24; loop++)  *(pTxBuff+loop)=0x40+loop;
    return 10;
    
}

//
// 1. To BT Mesh
// 2. To COM port Tx
//
void ComPortSendCmd(uchar size)
{TraceProc();
    UsartSendCmd(size);
}




