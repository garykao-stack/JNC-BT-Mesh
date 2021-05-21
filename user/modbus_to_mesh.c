

#include "global.h"

//richard Add
/* BG stack headers */
#include "init_board.h"
#include "sensor_server.h"
#include "mesh_event.h"
#include "device_bus.h"
#include "bus_rs485.h"
#include "jnc_cmd.h"
#include "mod_bus.h"
#include "com_port.h"
#include "sensor_client.h"
#include "sensor_server.h"

#include "modbus_to_mesh.h"

#if MESH_COLUME_ENABLE

uchar ModbusToMeshStage;


#define BT_MESH_RX_BUFF_NUM             50
#define MODBUS_CMD_SIZE_COL                 24


typedef struct
{
    uchar Size;
    //uchar Buff1[8];
    uchar Buff[MODBUS_CMD_SIZE_COL];
}_MeshModbusBuff,*_PMeshModbusBuff;

_MeshModbusBuff MeshModbusBuff[BT_MESH_RX_BUFF_NUM];
_PMeshModbusBuff pMeshModBusBuffRx,pMeshModBusBuffTx;

uchar BtMeshCountRx,BtMeshCountTx;
uchar MeshModbusWaiting;

uchar MeshRxSize;

//#define MODBUS_CMD_SIZE     8
//uchar ModBusCmdTbl[MODBUS_CMD_SIZE]={01,04,00,00,00,01};
#define CMD_TO_BT_MESH       //for modbus cmd to bt mesh


void ModbusToMeshInit()
{TraceProc();

    #ifdef CMD_TO_BT_MESH   //richard debug
        return;
    #endif

    ModbusToMeshStage = MM_PENDING;
    BtMeshCountRx = BtMeshCountTx = 0;
    MeshModbusWaiting = OFF;
    ToNextTaskStage(MM_PENDING);
    
    //SetEventTaskTimer(TD_SYS_RESET,TIMER_NO_SIGNAL,TIMER_EVENT_REPEAT); // system reset

    /*
    uint16 Crc16=ModbusRtu_CRC16((PUCHAR)ModBusCmdTbl,6);
    *(PUINT16)(&ModBusCmdTbl[6]) = Crc16;
    PrintDataByte("Modbus Cmd", ModBusCmdTbl, 8);
    Trace16_1(Crc16);
    */
    
}

bool ModbusToMeshFormat(PUCHAR p_buff,uchar bytes)
{
    bool ret_code=TRUE;
    if(p_buff == NULL) return FALSE;
    return ret_code;
}

//copy data to Tx buffer for transfer
void SendDataToTxBuff(PUCHAR pBuff,uchar size)
{TraceProc();
    PUCHAR p_tx_buff = UsartGetBuff(USART_ID_TX);
    memcpy(p_tx_buff,pBuff,size);
}


void SendServerNodeData(PUCHAR p_buff, uchar size)
{TraceProc();
    PUCHAR pTxBuff;
    pTxBuff = UsartGetBuff(USART_ID_TX);
    MeshModbusBuff[BtMeshCountRx].Size = size;
    memcpy(&(MeshModbusBuff[BtMeshCountRx].Buff) ,p_buff,size);
   // PrintDataByte("SendServerNodeData 1", p_buff,size);
   // PrintDataByte("SendServerNodeData 2", MeshModbusBuff[BtMeshCountRx].Buff,size);
    if(++BtMeshCountRx >= BT_MESH_RX_BUFF_NUM) BtMeshCountRx = 0; // reset index
    
    //ModbusToMeshStage = MM_MESH_RX_OK;
    MeshModbusWaiting = OFF;
}


#endif // MESH_COLUME_ENABLE

