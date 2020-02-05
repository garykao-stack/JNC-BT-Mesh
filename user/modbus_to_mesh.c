#include "global.h"

//richard Add
/* BG stack headers */
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

uchar ModbusToMeshStage;


#define BT_MESH_RX_BUFF_NUM             50
#define MODBUS_CMD_SIZE                 24


typedef struct
{
    uchar Size;
    //uchar Buff1[8];
    uchar Buff[MODBUS_CMD_SIZE];
}_MeshModbusBuff,*_PMeshModbusBuff;

_MeshModbusBuff MeshModbusBuff[BT_MESH_RX_BUFF_NUM];
_PMeshModbusBuff pMeshModBusBuffRx,pMeshModBusBuffTx;

uchar BtMeshCountRx,BtMeshCountTx;
uchar MeshModbusWaiting;

uchar MeshRxSize;

#define TaskTiimerToCount(x) (x/TIMER_DEVICE_TASK)  // base 10ms

#define ToNextTaskStage(stage)  (pDeviceTask->TaskStage = stage)
#define ToTaskWatingStage(id)   {ToNextTaskStage(id); pDeviceTask->TaskTimeOut = pDeviceTask->TaskTimeOutValue;}
#define CheckTaskTimeOut()      (pDeviceTask->TaskTimeOut)
#define CurrTaskStage()         (pDeviceTask->TaskStage)

#define MODBUS_CMD_SIZE     8
uchar ModBusCmdTbl[MODBUS_CMD_SIZE]={01,04,00,00,00,01};


void ModbusToMeshInit()
{TraceProc();
    ModbusToMeshStage = MM_PENDING;
    BtMeshCountRx = BtMeshCountTx = 0;
    MeshModbusWaiting = OFF;
    ToNextTaskStage(MM_PENDING);

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

#if 1

uint32 WaitingCounter;


// for client node
// USART Rx => Mesh Tx => Waiting Mesh Tx Response => Mesh Rx => USART Tx
void ModbusToMeshClientProc()
{//TraceProc();
    uchar rx_counter,size;
    PUCHAR p_rx_buff,p_tx_buff;
    if(!GetNodeStatus(STATUS_CLIENT)) return;
    switch(CurrTaskStage())
        {
         case MM_PENDING: //Trace("Client:MM_PENDING");
            if(UsartGetStatusRxEnd() == TRUE ) 
                {Trace("MM_PENDING: ****** USART Rx Ending ******");
                    if(UsartGetRxCounter() == 8)
                      {
                        ToNextTaskStage(MM_USART_RX);
                        BtMeshCountTx = BtMeshCountRx = 0;MeshModbusWaiting = OFF;
                        SetNodeStatus(STATUS_MODBUS_MESH_PENDING,OFF);
                        //UsartClose();
                      }
                    else UsartSetStage(USART_STAGE_RX_CLEAN);
                }
            break;
         case MM_USART_RX: Trace("Client:MM_USART_RX");            
            ModbusToMeshFormat(p_rx_buff,rx_counter);
            ToNextTaskStage(MM_MESH_TX);
            break;
         case MM_MESH_TX: Trace("Client:MM_MESH_TX");    // send data to server node

            p_rx_buff = UsartGetBuff(USART_ID_RX);
            rx_counter = UsartGetRxCounter();
            PrintDataByte("ModbusToMeshProc Rx", p_rx_buff, rx_counter);
            
            result = Cmd_ms_client_get_column(SENSOR_ELEMENT, PUBLISH_ADDRESS, IGNORED, NO_FLAGS,JNC_MODBUS_CMD,
            //result = Cmd_ms_client_get_column(SENSOR_ELEMENT, 6, IGNORED, NO_FLAGS,JNC_MODBUS_CMD,
                                              rx_counter,p_rx_buff)->result;
            if(result == RESULT_OK) 
                {
                UsartSetStage(USART_STAGE_RX_CLEAN);
                ToTaskWatingStage(MM_MESH_RX_WAITING);
                //ToTaskWatingStage()
                MeshModbusWaiting = ON;
                WaitingCounter = 0;
                }
            else
                { ShowResult("ModbusToMeshClientProc==>Cmd_ms_client_get_column", result);
                   //gecko_bgapi_class_mesh_sensor_client_init(); // gecko_cmd_mesh_sensor_client_init();
                   //gecko_cmd_mesh_node_reset();
                   gecko_cmd_system_reset(0);
                   UsartSetStage(USART_STAGE_RX_CLEAN);//for Bug
                   ToNextTaskStage(MM_ENDING);
                }
            break;
         case MM_MESH_RX_WAITING: //Trace("Client:MM_MESH_RX_WAITING");
                //WaitingCounter++;
            if(CheckTaskTimeOut() == 0){Trace("MM_MESH_TX_WAITING: Time Out");
                ToNextTaskStage(MM_WAITING_ERROR);
                }
            else if(MeshModbusWaiting == OFF) ToNextTaskStage(MM_MESH_RX_OK);
            
            break;
         case MM_MESH_RX_OK: Trace("Client:MM_MESH_RX_OK");
            ToNextTaskStage(MM_USART_TX);
            SetEventTaskTimer(TIMER_ID_SYS_RESET,TIMER_NO_SIGNAL,TIMER_EVENT_REPEAT); // system reset
            break;
         case MM_USART_TX://Trace("Client:MM_USART_TX");

            if(UsartGetStatus(USART_TX_ING)) {//Trace("TX_ING"); 
                return ;}

            if(BtMeshCountTx != BtMeshCountRx)
                {//Trace("Send Tx to Host");
                
                 p_rx_buff = MeshModbusBuff[BtMeshCountTx].Buff;
                 size = MeshModbusBuff[BtMeshCountTx].Size;
                 SendDataToTxBuff(p_rx_buff,size);
                 ComPortSendCmd(size);
                 BtMeshCountTx++;
                }
            else {ToNextTaskStage(MM_ENDING); WaitingCounter = 0;}

            //ComPortSendCmd(MeshRxSize);
            //PrintDataByte("Rs485ToMeshProc Tx", UsartGetBuff(USART_ID_TX), MeshRxSize);
            //ModbusToMeshStage = MM_ENDING;
            break;
         case MM_ENDING://Trace("Client:MM_ENDING");
             //ModbusToMeshStage = MM_PENDING;
             BtMeshCountTx = BtMeshCountRx = 0;
             memset(MeshModbusBuff,0,sizeof(MeshModbusBuff));
             //if(WaitingCounter++ > 10)  
             ToNextTaskStage(MM_PENDING);
            break;
         case MM_WAITING_ERROR: TraceErr("Client:MM_WAITING_ERROR");
            UsartSetStage(USART_STAGE_RX_CLEAN);
            ToNextTaskStage(MM_ENDING);
            break;
            
         default:  TraceErr1("ModbusToMeshClientProc 1",ModbusToMeshStage); 
            break; 
        };    

    
}



//*************************************** For Server Node *****************************************************

#define pServerMeshBuffRx       (MeshModbusBuff[0].Buff)
#define ServerMeshSizeRx        (MeshModbusBuff[0].Size)

#define pServerMeshBuffTx       (MeshModbusBuff[1].Buff)
#define ServerMeshSizeTx        (MeshModbusBuff[1].Size)


//#define WATER_LEVEL_DEBUG

//#ifdef WATER_LEVEL_DEBUG

#define     RETURN_DEVICE_INFO_SIZE     23
#define     RETURN_WATER_LEVEL_SIZE     11
#define     RETURN_MAX_LEVEL            16

const uchar ModbusCmdGetInfoID[8]={0x01, 0x03, 0x00, 0x00, 0x00, 0x09, 0x85, 0xCC};
const uchar ReturnModbusInfo[RETURN_DEVICE_INFO_SIZE]=
{0x01, 0x03, 0x12, 0x4C, 0x57, 0x2D, 0x53, 0x31, 0x42, 0x00, 0x7B, 0x00, 0x01, 0x00, 0x06, 0x00, 0x00, 0x00, 0x62, 0x00, 0x00, 0xE8, 0x8D};

const uchar ModbusCmdGetWaterLevel[8]={0x01, 0x04, 0x00, 0x00, 0x00, 0x03, 0xB0, 0x0B};
const uchar ReturnWaterLevelTbl[RETURN_MAX_LEVEL][RETURN_WATER_LEVEL_SIZE]= // Level 0 ~ 15
{	
{0x01, 0x04, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x93}, // 0 cm
{0x01, 0x04, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x5D, 0x53}, // 1 cm
{0x01, 0x04, 0x06, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x19, 0x53}, // 2 cm
{0x01, 0x04, 0x06, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x24, 0x93}, // 3 cm
{0x01, 0x04, 0x06, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x91, 0x53}, // 4 cm
{0x01, 0x04, 0x06, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0xAC, 0x93}, // 5 cm
{0x01, 0x04, 0x06, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x93}, // 6 cm
{0x01, 0x04, 0x06, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0xD5, 0x53}, // 7 cm
{0x01, 0x04, 0x06, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x81, 0x52}, // 8 cm
{0x01, 0x04, 0x06, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0xBC, 0x92}, // 9 cm
{0x01, 0x04, 0x06, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x92}, // 10 cm
{0x01, 0x04, 0x06, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00, 0xC5, 0x52}, // 11 cm
{0x01, 0x04, 0x06, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x70, 0x92}, // 12 cm
{0x01, 0x04, 0x06, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x4D, 0x52}, // 13 cm
{0x01, 0x04, 0x06, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x09, 0x52}, // 14 cm
{0x01, 0x04, 0x06, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x34, 0x92}, // 15 cm
};

uchar ReturnWaterLevel;

//#endif


uchar MeshCmdSizeRx,MeshCmdSizeTx;
//
// Server Node receive modbus command to save in order to send it to modbus device
void ModBusCmdToDevice()
{TraceProc();
    MeshCmdSizeRx = MeshNodeInfo.BuffSize;
    SendDataToTxBuff(MeshNodeInfo.pBuff,MeshNodeInfo.BuffSize);   // copy data to tx buffer
    SetTaskStage(ModbusToMeshServerProc,MM_MESH_RX_OK);
}

Result MeshServerSendModbusCmd(void)
{
    Result ret_code;
    ret_code = Cmd_ms_server_send_column_status(
                SENSOR_ELEMENT, MeshNodeInfo.ClientAddr, MeshNodeInfo.AppKey, NO_FLAGS,
                MeshNodeInfo.PropertyID, UsartGetRxCounter(), UsartGetBuff(USART_ID_RX))->result;
    
    ShowResult("Cmd_ms_server_send_column_status", ret_code);

   return ret_code;
    
}


// for Server node
//Mesh Rx => USART Tx => Waiting Tx Response => USART Rx => Mesh Tx
void ModbusToMeshServerProc()
{//TraceProc();
    uchar size;
    PUCHAR p_rx_buff,p_tx_buff;
    if(GetNodeStatus(STATUS_CLIENT)) return;
    switch(CurrTaskStage())
        {
        case MM_PENDING: //Trace("Server: MM_PENDING");
             SetNodeStatus(STATUS_MODBUS_MESH_PENDING,ON);
            break;
        case MM_MESH_RX_OK: Trace("Server: MM_MESH_RX_OK"); // to check modbus cmd
            BtMeshCountTx = BtMeshCountRx = 0;

        #ifdef WATER_LEVEL_DEBUG //richard debug
            ToNextTaskStage(MM_MESH_TX);    
        #else
            ToNextTaskStage(MM_USART_TX);
        #endif
            SetEventTaskTimer(TIMER_ID_SYS_RESET,TIMER_NO_SIGNAL,TIMER_EVENT_REPEAT); // system reset
            break;            
        case MM_USART_TX: Trace("Server: MM_USART_TX");  // to modbus device
            ComPortSendCmd(MeshCmdSizeRx);
            MeshModbusWaiting = ON;
            ToNextTaskStage(MM_USART_TX_WAITING);
            //ToNextTaskStage(MM_USART_RX_WAITING);
            break;
        case MM_USART_TX_WAITING: Trace("Server: MM_MESH_TX_WAITING");
            if(UsartGetStatusTxIng() == FAIL)  ToTaskWatingStage(MM_USART_RX_WAITING);
            //ToNextTaskStage(MM_USART_RX_WAITING);
            break;            
        case MM_USART_RX_WAITING: Trace("Server: MM_MESH_RX_WAITING");            
            //ModbusToMeshFormat(pServerMeshBuffTx,ServerMeshSizeTx);
            if(CheckTaskTimeOut() == 0) {ToNextTaskStage(MM_WAITING_ERROR); TraceErr("Server Rx Time-out");}
            else if(UsartGetStatusRxEnd() == TRUE )  ToNextTaskStage(MM_MESH_TX);
            break;
        case MM_MESH_TX: Trace("Server: MM_MESH_TX");    // send data to client node
            //richard: debug
            
            p_rx_buff = UsartGetBuff(USART_ID_RX); p_tx_buff = UsartGetBuff(USART_ID_TX);
            if(p_tx_buff[1] == 0x03) 
                {Trace("ReturnModbusInfo 1");
                    memcpy(p_rx_buff,ReturnModbusInfo,RETURN_DEVICE_INFO_SIZE);CounterRx = RETURN_DEVICE_INFO_SIZE;
                }
#ifdef WATER_LEVEL_DEBUG
            
            else if(p_tx_buff[1] == 0x04)
                {Trace("ReturnWaterLevelTbl 1");

                 memcpy(p_rx_buff,&ReturnWaterLevelTbl[ReturnWaterLevel],RETURN_WATER_LEVEL_SIZE);CounterRx = RETURN_WATER_LEVEL_SIZE;
                 TraceDec1("ReturnWaterLevel",ReturnWaterLevel);
                 PrintDataByte("ReturnWaterLevel",(PUCHAR)&ReturnWaterLevelTbl[ReturnWaterLevel], RETURN_WATER_LEVEL_SIZE);
                 
                if(++ReturnWaterLevel >= (RETURN_MAX_LEVEL)) 
                   ReturnWaterLevel = 0;
                }
 #endif
            
            result = MeshServerSendModbusCmd();
            if(result != RESULT_OK)
                {TraceErr1("MeshServerSendModbusCmd 1",result);
                   Cmd_sys_reset(0); 
                }
            ToNextTaskStage(MM_ENDING);
            break;
        case MM_ENDING:Trace("Server: MM_ENDING\r\n\r\n");
           
            ToNextTaskStage(MM_PENDING);
            break;
        case MM_WAITING_ERROR: TraceErr("Server:MM_WAITING_ERROR\r\n");
           UsartSetStage(USART_STAGE_RX_CLEAN);
           ToNextTaskStage(MM_ENDING);
           break;
         
        default:  TraceErr1("ModbusToMeshServerProc 1",CurrTaskStage()); 
        break; 
    };    
            
}

typedef struct
{
    uchar   ModbusID;
    uint16  Counter;
    int16   Tempature;
    uint16  Humidity;
    uint32  TempRh;  //for tempature & humidity
    uint16  ModbusReg0;
    uint16  ModbusReg1;
    uint16  ModbusReg2;
    uint16  ModbusReg3;
}_DevSensor,*_PDevSensor;


#endif

