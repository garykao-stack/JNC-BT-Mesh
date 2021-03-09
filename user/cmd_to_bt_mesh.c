
#if 0

#include "global.h"

//richard Add
/* BG stack headers */
#include "sensor_server.h"
#include "mesh_event.h"
#include "device_bus.h"
#include "bus_usart.h"

#include "bus_rs485.h"
#include "bus_I2C.h"
#include "jnc_cmd.h"
#include "mod_bus.h"
#include "com_port.h"
#include "ivi_features.h"
#include "sensor_client.h"
#include "sensor_server.h"
#include "modbus_to_mesh.h"


#include "cmd_to_bt_mesh.h"

#define MESH_NODE_NUM       50
_ClientModbusRegs ClientModbusRegs[MESH_NODE_NUM + 1];

_PClientModbusRegs pClientModbusRegs;
uchar   CurrMeshNodeNum;



void BtMeshReset();
//
// if node has abnormal condition, then system reset.
//


void CmdToBtMeshInit()
{
    TraceProc();
    
    pClientModbusRegs = ClientModbusRegs;
    CurrMeshNodeNum = 0;    
    //SetEventTaskTimer(TD_CHECK_DEV_NODE,TIMER_CKECK_DEV_INFO,TIMER_EVENT_REPEAT);
    //GetWaitingTickValue();
    if(GetMeshNodeStatus(STATUS_CLIENT))
    {
        ClientModbusCmdInit();
        Trace("CmdToBtMeshInit 2");
        //SetEventTaskTimer(TD_GET_SENSOR_INFO,1000,TIMER_EVENT_ONCE);
    }
    else
        {
        ServerModbusCmdInit();
        Trace("CmdToBtMeshInit 3");
        }

    if(pMeshNodeData->Status & NODE_TEMP_HUM) SetMeshNodeStatus(STATUS_TEMP_HUM,ON);
    
}

void ClientModbusCmdInit()
{ TraceProc();

    SetEventTaskTimer(TD_GET_SENSOR_INFO, TIMER_CLIENT_GET_SENSOR_INFO, TIMER_EVENT_ONCE);
    memset(pClientModbusRegs, 0, sizeof(ClientModbusRegs));

    return; //richard debug
    
}

//
//
//
uchar CheckNodeActionStatus()
{//TraceProc();
    uchar loop;
    uchar node_num=0;
    _PClientModbusRegs pModbusRegs = ClientModbusRegs;
    
    for(loop=0; loop<MESH_NODE_NUM; loop++)
        {
           if(pModbusRegs->MeshNodeAddr !=0 && pModbusRegs->DevInfoCount > 0)
            {
                node_num++;
                pModbusRegs->DevInfoCount--;
                //TraceDec2("CheckNodeActionStatus 2",pModbusRegs->MeshNodeAddr,pModbusRegs->DevInfoCount);
            }
           pModbusRegs++;
        }
    
    return node_num;
}

void MeshNodeToReset(uint32 timer)
{
    //TraceProc();
    //return; //debug
    SetEventTaskTimer(TD_SYS_RESET, timer, TIMER_EVENT_ONCE); // system reset
}


#define TIMER_GET_SENSOR_DATA_ENDING          2500 //2800ms

//
// Receive data from server node
//
void ClientUpdateModbusRegs(uint16 server_adrr, PUCHAR pbuff, uchar len)
{
    uchar SendDataIndex, MeshNodeID;
    PUINT16 p_dest;
    SetMeshNodeStatus(STATUS_GET_SENSOR_ENDING, ON);
    SetEventTaskTimer(TD_GET_SENSOR_ENDING, TIMER_GET_SENSOR_DATA_ENDING, TIMER_EVENT_ONCE);
    ResetEventCounter(TD_NO_EVENT);

    MeshNodeID = *pbuff;
    SendDataIndex = *(pbuff + 1);

    // PrintDataByte("Receive Data",pbuff,len);

    TraceDec2("", MeshNodeID, SendDataIndex);

    AllNodeEventNum++;
    _PClientModbusRegs p_modbus_regs = ClientGetNodePos(MeshNodeID);

    if(p_modbus_regs == NULL)
    {
        TraceErr("Buff Full Save Modbus Regs");  return;
    }

    //update receive data
    p_modbus_regs->MeshNodeAddr = MeshNodeID;

    //Trace2("Address Test", (PUINT16)(&(p_modbus_regs->DevModbusRegs)), SendDataIndex);
    
    p_dest = (PUINT16)(&(p_modbus_regs->DevModbusRegs)) + SendDataIndex * 6;

    //Trace1("p_dest", p_dest);
    
    
    pbuff += 2; // point reegister value
    len -= 2;
    memcpy(p_dest, pbuff, len);
    
    p_modbus_regs->DevInfoCount = DEV_INFO_COUNT;   //update device info time out
    
    SetLedToggle(LED_GET_INFO); LedOnClient();

    //PrintDataByte("Client Buff",(PUCHAR)p_modbus_regs,sizeof(_ClientModbusRegs));
}




uint16 ModbusRegResetCount;
uchar AllNodeEventNum;


//
// Get sensor data from server node
//
void ClientGetSensorDataProc()
{
    //if(!GetMeshNodeStatus(STATUS_CLIENT)) return;
#ifdef SERVER_AUTO_PUBLISH
    return ;
#endif

    switch(CurrTaskStage())
    {
        case SGS_TASK_PENDING: 
            if(GetMeshNodeStatus(STATUS_GET_SENSOR_INFO) == ON)
            {//TraceDec1("Clean Modbus Reg",ModbusRegResetCount);                
                SetMeshNodeStatus(STATUS_GET_SENSOR_INFO, OFF);
                if(MeshCheckSeqNum() == FALSE) {ToNextTaskStage(SGS_ENDING);break;}
                /*
                if(ModbusRegResetCount++ > MODBUS_REG_CLEAN_NUM)
                {Trace("Clean Modbus Reg");
                    ClientModbusCmdInit();
                    ModbusRegResetCount = 0;
                    Trace("SGS_CLEAN_MODBUS_REGS");
                }
                */
                
               // SetMeshNodeStatus(STATUS_GET_SENSOR_ENDING,ON);
               // Rs485Tx();UsartResetRxTx(USART_ID_RX); // RS-485 RX disable
               // UsartIrq(USART_ID_RX,OFF);
                
                ToNextTaskStage(SGS_SEND_CMD);
            }
            break;
        
        case SGS_SEND_CMD:  
            //if(UsartGetStatus(USART_RX_END) == OFF)
            if(UsartGetStatus(USART_TX_ING) == OFF && UsartCounterTx == 0 )
             // if(UsartCounterTx == 0)
                {Trace("\r\n**** Client: SGS_SEND_CMD *****");
                ClientGetServerReg(); LedOffClient();
            
                ToNextTaskStage(SGS_RX_WAITING);
                }
           // else 
             //   Trace("Send Cmd Again");
        
            break;
        case SGS_RX_WAITING: 
            if(GetMeshNodeStatus(STATUS_GET_SENSOR_ENDING) == OFF)
            {
                Trace("STATUS_GET_SENSOR_ENDING == OFF 2 *****");
                SetMeshNodeStatus(STATUS_GET_SENSOR_ENDING, OFF);
                ToNextTaskStage(SGS_ENDING);
            }
            break;
        case SGS_ENDING: 
            ClientShowAllNodes();  ShowCurrRemSeq(); ShowTimerRTC();
            SetMeshNodeStatus(STATUS_GET_SENSOR_INFO, OFF);
            SetEventTaskTimer(TD_GET_SENSOR_INFO, TIMER_CLIENT_GET_SENSOR_INFO, TIMER_EVENT_ONCE);
            LedOffGetReg(); 
            LedOnClient();
            //Rs485Rx();  // RS-485 RX enable
            //UsartResetRxTx(USART_ID_RX); // RS-485 RX disable
            //UsartIrq(USART_ID_RX,ON);
            
            ToNextTaskStage(SGS_TASK_PENDING);
            break;
        case SGS_CLEAN_MODBUS_REGS:
            if(ModbusRegResetCount++ > 50)
            {
                Trace("SGS_CLEAN_MODBUS_REGS");
                ClientModbusCmdInit();
                ModbusRegResetCount = 0;
            }

            ToNextTaskStage(SGS_TASK_PENDING);
            break;
        case SGS_ERR_RX:
            TraceErr("Client: SGS_ERR_RX");
            //while(1); //debug
            gecko_bgapi_class_mesh_sensor_client_init();
            ToNextTaskStage(SGS_ENDING);
            break;

    };
}








//
// send data to host
//
void ClientSendDataToHostProc()
{

    switch(CurrTaskStage())
    {
        case CLIENT_HOST_PENDING: 
            if(UsartGetStatusRxEnd() == TRUE)
            {
               // TraceDec1("Host: Rx Ending",UsartGetRxCounter());
                if(UsartGetRxCounter() == 8)
                {
                   ToNextTaskStage(CLIENT_HOST_PREPARE);
                }
                else{TraceErr1(" Rx Error",UsartGetRxCounter());
                    PrintDataByte("Rx Data",UsartGetBuff(USART_ID_RX), UsartGetRxCounter());
                    UsartResetRxTx(USART_ID_RX);
                    }
            }
            break;
        case CLIENT_HOST_PREPARE:
            if(ClientHostPrepare() != TRUE)
                ToNextTaskStage(CLIENT_HOST_ENDING);
            else
                ToNextTaskStage(CLIENT_HOST_SEND_DATA);

            break;
        case CLIENT_HOST_SEND_DATA: 
            ClientHostSendData();   // send data to host
            ToNextTaskStage(CLIENT_HOST_ENDING);
            break;

        case CLIENT_HOST_ENDING:  
            //TaskActive();
            UsartResetRxTx(USART_ID_RX);
            ToNextTaskStage(CLIENT_HOST_PENDING);

            break;

        default:TraceErr("Host:ClientSendDataToHostProc");
    };
}



//
// get node position in the array(ModbusRegs)
//
_PClientModbusRegs ClientGetNodePos(uchar node_id)
{

    uchar loop;
    _PClientModbusRegs p_modbus_regs = NULL;
    for(loop = 0; loop < MESH_NODE_NUM; loop++)
    {
        if(ClientModbusRegs[loop].MeshNodeAddr == node_id || ClientModbusRegs[loop].MeshNodeAddr == 0)
        {
            p_modbus_regs = &ClientModbusRegs[loop];
            break;
        }
    }
    return p_modbus_regs;
}

void ClientShowAllNodes()
{
    uchar loop;
    for(loop = 0; loop < MESH_NODE_NUM; loop++)
    {
        if(ClientModbusRegs[loop].MeshNodeAddr != 0)
        {
            Printf("Node ID %d 0x%X \r\n", ClientModbusRegs[loop].MeshNodeAddr, ClientModbusRegs[loop].MeshNodeAddr);

        }
        else
        {
            TraceDec1("All Mesh Node", loop);
            break;
        }
    }
}

uchar CmdErrCount;
#define ERR_CMD_COUNT       10

//
// get device reg data
//
Result ClientGetServerReg()
{TraceProc();
    Result ret_code;
    AllNodeEventNum = 0;
    SetMeshNodeStatus(STATUS_GET_SENSOR_ENDING, ON);
    SetEventTaskTimer(TD_GET_SENSOR_ENDING, TIMER_GET_SENSOR_DATA_ENDING+4000, TIMER_EVENT_ONCE);
    ShowCurrRemSeq();


    //ret_code = Cmd_ms_client_get(SENSOR_ELEMENT, PUBLISH_ADDRESS, IGNORED, NO_FLAGS, MODBUS_GET_REGS_VALUE)->result;

    ret_code = Cmd_ms_client_get_setting(SENSOR_ELEMENT, PUBLISH_ADDRESS, IGNORED, NO_FLAGS, MODBUS_GET_REGS_VALUE,
                                         0x1234)->result;

    if(ret_code)
    {
        TraceErr1("Cmd_ms_client_get_setting", ret_code);
    }

    //SetLedToggle(LED_CLIENT); //Delay_ms(5);
    //LedClientSendCmd();
    //LedOnClient();
    //ShowTimerRTC();

    return ret_code;
}


#define TIMER_GET_REG_UPDATE    TIMER_15SEC
//
//for write reg data
//
bool ClientSetServerReg(_PModbusCmd pCmd)
{
    TraceProc();
    bool ret_code = TRUE;
    uint32 setting_data = MAKEDWORD(pCmd->RegNum, pCmd->Register);

    //result = Cmd_ms_client_set_setting(SENSOR_ELEMENT, PUBLISH_ADDRESS, IGNORED, 0x02,MODBUS_GET_REGS_VALUE,
    result = Cmd_ms_client_set_setting(SENSOR_ELEMENT, pCmd->ModbusID, IGNORED, 0x00, MODBUS_GET_REGS_VALUE,
                                       6, 4, (uint8 *)&setting_data)->result;
    //(uint16)pCmd->FunCode,4,(uint8*)&setting_data)->result;
    if(result){TraceErr("Cmd_ms_client_set_setting");
         Trace16_2(result, pCmd->ModbusID);
        }
    else
        TraceOk("Cmd_ms_client_set_setting");

    if(UsartTxSendCmd((PUCHAR)pCmd, 8) == FALSE)
    {
        Delay_ms(100); UsartTxSendCmd((PUCHAR)pCmd, 8);
    }

    Trace("To Get Setup register status");
    // for Modbus Setting Function to update register status
#if !UPDATE_REAL_TIME
    SetEventTaskTimer(TD_GET_SENSOR_INFO, TIMER_GET_REG_UPDATE, TIMER_EVENT_ONCE);
#endif
    return ret_code;
}


//uchar ModbusToHostBuff[8]={0x0F,0x01,0x02,0x03,0x04,0x05,0x06,0x07};
//uchar ModbusToHostBuff[8];
uint test_counter;
_ModbusToHostPack ModbusToHostPack;

#define MODBUS_FC1          1
#define MODBUS_FC4          4
#define MODBUS_FC5          5
#define MODBUS_FC6          6



//
// To Get Modbus Register value to buffer
//
bool GetModbusValue(uchar modbus_id, uint16 modbus_reg, uchar reg_num, PUINT16 pbuff)
{
    //TraceProc();
    bool ret_code = TRUE;
    _PClientModbusRegs p_modbus_regs;
    PUINT16 pRegs = NULL;
    p_modbus_regs = ClientGetNodePos(modbus_id);
    // if(p_modbus_regs == NULL || p_modbus_regs->MeshNodeAddr == 0)
    //if(p_modbus_regs->MeshNodeAddr == 0)
    if(p_modbus_regs->MeshNodeAddr == 0 || p_modbus_regs->DevInfoCount == 0)
    {
        TraceErr1("Modbus Mesh Node ID ", modbus_id);
        ret_code = FALSE;
    }
    else
    {
        if(modbus_reg >= 0x00 && modbus_reg <= 0x0C)
        {
                pRegs = (PUINT16) & (p_modbus_regs->DevModbusRegs[0]) + modbus_reg;
        }
        else if(modbus_reg >= 0x02FE && modbus_reg <= 0x0315)
        {
            pRegs = (PUINT16)&(p_modbus_regs->DevModbusRegs[1]) + (modbus_reg - 0x02FE);
        }

        if(pRegs != NULL)
            memcpy(pbuff, pRegs, reg_num * 2);
        else
        {
            ret_code = FALSE;
            TraceErr("pRegs == NULL");
        }
    }

    return ret_code;
}

#define BT_DEVICE_INFO_SIZE     13

const uchar ModbudDeviceIDCmd[8]={0x01, 0x03, 0x00, 0x00, 0x00, 0x04, 0x44, 0x09};
//for version: BT Mesh 0.99
uchar BtDeviceInfo[BT_DEVICE_INFO_SIZE]={0x01, 0x03, 0x08, 0x42, 0x54, 0x4D, 0x45, 0x53, 0x48, 0x00, 0x99, 0x96, 0xD1};

bool SendDeviceModelInfo(_PModbusCmd p_modbus_cmd)
{TraceProc();
    bool ret_code=false;
    uint16 modbus_crc;

    BtDeviceInfo[0]=p_modbus_cmd->ModbusID;
    BtDeviceInfo[1]=0x03;
    modbus_crc = ModbusRtu_CRC16(BtDeviceInfo,11);
    *((PUINT16)&BtDeviceInfo[11]) = modbus_crc;
    if(UsartTxSendCmd((PUCHAR)BtDeviceInfo, BT_DEVICE_INFO_SIZE) == FALSE)
         {
             Delay_ms(20); UsartTxSendCmd((PUCHAR)BtDeviceInfo, BT_DEVICE_INFO_SIZE);
         }
         ret_code = true;
    return ret_code;
}



uchar ClientToHostDataNum;
//
// Prepare Modbus Data to Host
// depend on modbus send data to host for Func4,6,3
bool ClientHostPrepare()
{
    //TraceProc();
    bool ret_code = TRUE;
    _PModbusCmd p_modbus_cmd;
    _PClientModbusRegs p_modbus_regs;
    uchar modbus_id;
    uint16 reg_addr, reg_num, cmd_crc16;
    p_modbus_cmd = (_PModbusCmd)UsartGetBuff(USART_ID_RX);    // get Rx buffer

    //if(p_modbus_cmd->FunCode != 0x04)
    if(p_modbus_cmd->FunCode == 0x06)
    {
        ret_code = ClientSetServerReg(p_modbus_cmd); // Modbus Fun 6
        return FALSE; //ret_code;
    }
    
    if(p_modbus_cmd->FunCode == 0x03)
     {
        ret_code = SendDeviceModelInfo(p_modbus_cmd);
        return FALSE; //ret_code;
        
     }
    modbus_id = p_modbus_cmd->ModbusID;
    reg_addr = WordSwap(p_modbus_cmd->Register);
    reg_num = WordSwap(p_modbus_cmd->RegNum);

    //Trace16_3(modbus_id, reg_addr, reg_num); //Host debug message

    if(reg_num > TO_HOST_REGS_MAX) 
        {TraceErr1("ClientHostPrepare 1",reg_num);
        PrintDataByte("ClientHostPrepare 1", (PUCHAR)p_modbus_cmd, TO_HOST_REGS_MAX);
        return FALSE;    // return error
        }

    memset(&ModbusToHostPack, 0, sizeof(ModbusToHostPack));

    ModbusToHostPack.ModbusID = p_modbus_cmd->ModbusID;
    ModbusToHostPack.FunCode = p_modbus_cmd->FunCode;
    ModbusToHostPack.ByteNum = reg_num * 2;

    ClientToHostDataNum = 0;

    if(GetModbusValue(modbus_id, reg_addr, reg_num, ModbusToHostPack.Data) == TRUE)
    {
        cmd_crc16 = ModbusRtu_CRC16((PUCHAR)&ModbusToHostPack, ModbusToHostPack.ByteNum + 3);
        *(PUINT16)(&ModbusToHostPack.Data[reg_num]) = cmd_crc16;

        ClientToHostDataNum = ModbusToHostPack.ByteNum + 5;
        if(ClientToHostDataNum >= MODBUS_RET_NUM) 
            {TraceErr1("ClientToHostDataNum", ClientToHostDataNum);
            ClientToHostDataNum = 0; ret_code = FALSE;
            }
        //TraceDec1("ClientToHostDataNum", ClientToHostDataNum);
    }
    else
        ret_code = FALSE;


    // for send data to PC 4 sets 8 bytes

    return ret_code;
}


bool ClientHostSendData()
{
    //TraceProc();
    // ClientHostPrepare();

    //UsartTxSendCmd((PUCHAR)&ModbusToHostPack,sizeof(ModbusToHostPack));
    // PrintDataByte("ClientHostSendData",(PUCHAR)&ModbusToHostPack, ClientToHostDataNum);
    UsartTxSendCmd((PUCHAR)&ModbusToHostPack, ClientToHostDataNum);

       
}


////////////////////////////////////////////////////////////////////////////////////////
//
//              For Server Node process
//
////////////////////////////////////////////////////////////////////////////////////////





/*
const uchar ModbusCmdTbl[MODBUS_CMD_MAX][MODBUS_CMD_SIZE]=
{
    MODBUS_FC4_GET_REG0_8,
    MODBUS_FC4_GET_REG60,
    MODBUS_FC1_GET_REG1,
    MODBUS_FC1_GET_REG7
};
*/
const uchar ModbusCmdTbl[MODBUS_CMD_MAX][MODBUS_CMD_SIZE] =
{
    MOBUS_GET_REGS_0,   // 0x000 ~ 0x00C
    MOBUS_GET_REGS_1,   // 0x2FE ~ 0x309
    //MOBUS_GET_REGS_2,   // 0x30A ~ 0x315
};

uint16 DevModbusRegs[MODBUS_CMD_MAX][REGS_ALL_NUM_MAX];


//_ModbusRegs DevModbusRegs;

_SensorRegsData SensorRegsData;
uchar ModbusCmdIndex;
//uchar GetSensorStage;
uint16 SgsTaskTimer;



#define CMD_INDEX_FC4_REG0_8        0
#define CMD_INDEX_FC4_REG60         1
#define CMD_INDEX_FC1_REG1          2
#define CMD_INDEX_FC1_REG7          3

#define CMD_FC4_REGS_0              0
#define CMD_FC4_REGS_1              1
#define CMD_FC4_REGS_2              2


void ServerModbusCmdInit()
{

    memset(&SensorRegsData, 0, sizeof(SensorRegsData));
    SetMeshNodeStatus(STATUS_SERVER_MODBUS_TIME_OUT, ON);
    SetEventTaskTimer(TD_GET_SENSOR_INFO, TIMER_SERVER_GET_SENSOR_INFO, TIMER_EVENT_ONCE);
    Rs485Tx();
}

//
// Update regster value to buff
//
void ServerUpdateModbusRegs(uchar cmd_index)
{//TraceProc();
    PUCHAR pRxBuff = UsartGetBuff(USART_ID_RX);
    //UsartShowDataRx();
    if(*(pRxBuff + 1) != 0x04)
    {
        PrintDataByte("Fun6 Response", pRxBuff, 8);
        return;
    }
    pRxBuff += 3;
    switch(cmd_index)
    {
        case CMD_FC4_REGS_0://PrintData("CMD_FC4_REGS_0",(PUINT16)pRxBuff,REGS_ALL_NUM_MAX);
            memcpy(&DevModbusRegs[CMD_FC4_REGS_0], pRxBuff, REGS_ALL_NUM_MAX * 2);
            break;
        case CMD_FC4_REGS_1://PrintData("CMD_FC4_REGS_1",(PUINT16)pRxBuff,REGS_ALL_NUM_MAX);
            memcpy(&DevModbusRegs[CMD_FC4_REGS_1], pRxBuff, REGS_ALL_NUM_MAX * 2);
            break;
        case CMD_FC4_REGS_2://PrintData("CMD_FC4_REGS_2",(PUINT16)pRxBuff,REGS_ALL_NUM_MAX);
            memcpy(&DevModbusRegs[CMD_FC4_REGS_2], pRxBuff, REGS_ALL_NUM_MAX * 2);
            break;

    };
}


#ifdef SIMULATION_MODBUS_REGS

const SimModbusRegs Fc4Regs_0 = { 0x01, 0x04, 0x18, {0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C}};

//const SimModbusRegs Fc4Regs_1 = { 0x01, 0x04, 0x18, {0x0101, 0x010D, 0x0103, 0x0104, 0x0105, 0x0106, 0x0107, 0x0108, 0x0109, 0x010A, 0x010B, 0x010C}};
const SimModbusRegs Fc4Regs_1 = { 0x01, 0x04, 0x18, {0x0101, 0x0000, 0x0103, 0x0104, 0x0105, 0x0106, 0x0107, 0x0108, 0x0109, 0x010A, 0x010B, 0x010C}};


const SimModbusRegs Fc4Regs_2 = { 0x01, 0x04, 0x18, {0x0201, 0x0202, 0x0203, 0x0204, 0x0205, 0x0206, 0x0207, 0x0208, 0x0209, 0x020A, 0x020B, 0x020C}};



uint16 SimCounter;

void SetRxMdobusValue(uchar cmd_index)
{//TraceProc();
    uint16 modbus_Value;
    PUCHAR pRxBuff = UsartGetBuff(USART_ID_RX);
    CounterRx = sizeof(SimModbusRegs);
    if(SimCounter >= 5000) SimCounter = 0;

    //SimCounter = 2;
    modbus_Value = WordSwap(SimCounter);
    switch(cmd_index)
    {
        case CMD_FC4_REGS_0:
            SimCounter++;
            memcpy(pRxBuff, &Fc4Regs_0, sizeof(SimModbusRegs));
            *((PUINT16)(&pRxBuff[3])) = modbus_Value; //for debug 5 ==> CO2 value
            break;
        case CMD_FC4_REGS_1:
            memcpy(pRxBuff, &Fc4Regs_1, sizeof(SimModbusRegs));
            *((PUINT16)(&pRxBuff[7])) = modbus_Value; //for debug 5 ==> CO2 value
            break;
        case CMD_FC4_REGS_2:
            memcpy(pRxBuff, &Fc4Regs_2, sizeof(SimModbusRegs));
            *((PUINT16)(&pRxBuff[3])) = modbus_Value; //for debug 5 ==> O2 value
            break;

    };

}



#endif




uchar ModbusSetCmdF6[10];
uchar UsartTimeoutNum;
//
//update tempature & Humidity to Register
//
void BtTempHumToReg()
{TraceProc();
    uint16 Tempature, Humidity;
   if(GetMeshNodeStatus(STATUS_TEMP_HUM) == OFF) return;

   // update tempatur & Humidity from BT itself
    Tempature = GetTempature();
    Humidity = GetHumidity();

   // TraceDec2("BT Temp & Hum 1",Tempature, Humidity);
    
    //Printf("Adj TempGain = %f HumGain = %f\r\n",pAdjValue->TempGain,pAdjValue->HumGain);

    Tempature = (uint16)((float)(pAdjValue->TempGain)*Tempature + pAdjValue->TempOffset);
    Humidity = (uint16)((float)(pAdjValue->HumGain)*Humidity + pAdjValue->HumOffset);

   // TraceDec2("BT Temp & Hum 2",DevModbusRegs[1][4], DevModbusRegs[1][5]);
    
    DevModbusRegs[1][4] = Tempature;
    DevModbusRegs[1][5] = Humidity;

    //PrintData("DevModbusRegs", (PUINT16)DevModbusRegs, sizeof(DevModbusRegs)/2);
        
    
    TraceDec2("BT Temp & Hum 3",Tempature, Humidity);
}



//
// get sensor data to update database
//
void ServerGetSensorDataProc()
{
    if(GetMeshNodeStatus(STATUS_SERVER_TO_CLIENT) == ON) return;

    switch(CurrTaskStage())
    {
        case SGS_TASK_PENDING: //Trace("SGS_TASK_PENDING");
            if(GetMeshNodeStatus(STATUS_GET_SENSOR_INFO) == ON)
            {
                //memset(DevModbusRegs, 0, sizeof(DevModbusRegs)); // clean register value
                UsartResetRxTx(USART_ID_TX_RX);  Rs485Tx();
                //UsartSetStage(USART_STAGE_TX_CLEAN);
                ToNextTaskStage(SGS_SEND_CMD);
                
            }
            break;
        case SGS_SEND_CMD: //Trace("SGS_SEND_CMD to Device");

#ifdef SIMULATION_MODBUS_REGS
            SetRxMdobusValue(ModbusCmdIndex);
            SetCurrTaskTimeOut();
            //ToUserWaiting(SGS_RX_WAITING, 10);
            ToNextTaskStage(SGS_UPDTAE_REGS_VALUE);
#else
                if(GetMeshNodeStatus(STATUS_TEMP_HUM) == OFF)
                {TraceDec1("SGS_SEND_CMD: ModbusSetCmdF4",ModbusCmdIndex);
                    UsartTxSendCmd((PUCHAR)&ModbusCmdTbl[ModbusCmdIndex], MODBUS_CMD_SIZE);
                    ToUserWaiting(SGS_RX_WAITING, 20);
                }else
                {Trace("SGS_SEND_CMD: TEMP & Humidity");
                   BtTempHumToReg(); 
                   ToNextTaskStage(SGS_ENDING);
                }
            
#endif
            break;
        case SGS_RX_WAITING: //Trace("SGS_RX_WAITING");
            if(CheckTaskTimeOut() == 0)
            {
                ToNextTaskStage(SGS_RX_WAITING_TIME_OUT);
            }
            else if(UsartGetStatusRxEnd() == TRUE)
            {
                UsartTimeoutNum = 0; ToNextTaskStage(SGS_UPDTAE_REGS_VALUE);
            }
            break;
        case SGS_RX_WAITING_TIME_OUT: TraceErr1("SGS_RX_WAITING_TIME_OUT ModbusCmdIndex", ModbusCmdIndex);
            if(UsartTimeoutNum > (MODBUS_CMD_MAX+4))
              { TraceErr("RS-485 Link");
                SetMeshNodeStatus(STATUS_SERVER_MODBUS_TIME_OUT, ON);
              }
            else UsartTimeoutNum++;
            ToNextTaskStage(SGS_TO_NEXT_CMD);
            break;
        case SGS_UPDTAE_REGS_VALUE: //Trace("SGS_UPDTAE_REGS_VALUE");
            
            SetMeshNodeStatus(STATUS_SERVER_MODBUS_TIME_OUT, OFF);
            ServerUpdateModbusRegs(ModbusCmdIndex);
            ToNextTaskStage(SGS_TO_NEXT_CMD);
            break;
        case SGS_TO_NEXT_CMD: //Trace("SGS_TO_NEXT_CMD");
            if(++ModbusCmdIndex < MODBUS_CMD_MAX)
            {
                ToUserWaiting(SGS_TO_NEXT_CMD_DELAY,40);   // to next cmd
            }
            else
                ToNextTaskStage(SGS_ENDING);//to ending
            break;

        case SGS_TO_NEXT_CMD_DELAY:
            if(CheckTaskTimeOut() == 0)  ToNextTaskStage(SGS_SEND_CMD);   // to next cmd
            break;
            
        case SGS_ENDING: Trace("SGS_ENDING");
            //PrintDataByte("SGS_ENDING", (PUCHAR)&SensorRegsData, sizeof(SensorRegsData));
            DevG6sStatusToClient();
            ModbusCmdIndex = 0;
            SetMeshNodeStatus(STATUS_GET_SENSOR_INFO, OFF);
            SetEventTaskTimer(TD_GET_SENSOR_INFO, TIMER_SERVER_GET_SENSOR_INFO, TIMER_EVENT_ONCE);
            //BtTempHumToReg();
            ToNextTaskStage(SGS_TASK_PENDING);
            break;
/*
        case SGS_ERR_TX:   TraceErr("Tx");
            SetMeshNodeStatus(TD_GET_SENSOR_INFO, OFF);
            SetEventTaskTimer(TD_GET_SENSOR_INFO, TIMER_SERVER_GET_SENSOR_INFO, TIMER_EVENT_ONCE);
            ToNextTaskStage(SGS_TASK_PENDING);
            break;
        case SGS_ERR_RX:  TraceErr("Rx");
            SetMeshNodeStatus(TD_GET_SENSOR_INFO, OFF);
            SetEventTaskTimer(TD_GET_SENSOR_INFO, TIMER_SERVER_GET_SENSOR_INFO, TIMER_EVENT_ONCE);
            ToNextTaskStage(SGS_TASK_PENDING);
            break;
*/            
        default:TraceErr("ServerGetSensorData");   break;

    };

}




#define SERVER_SEND_ALL_REGS      1   // send all regs to client
#define SERVER_SEND_FC4_REG0      2   // send all regs to client
#define SERVER_SEND_FC4_REG1      3   // send all regs to client

//extern uint16 MeshNodeID;
uint16 SimCount;

static uchar WaitingTick;

#define SEND_DATA_WAITING_MS_MAX            90
#define SEND_DATA_WAITING_MASK              0x003F


/*

#define WAITING_TICK_MAX    128
uchar WaitingTickTbl[WAITING_TICK_MAX];
uchar WaitingTickIndex;

void GetWaitingTickValue()
{

    struct gecko_msg_system_get_random_data_rsp_t *pRandomValue;
    uchar loop = 0;
    for(loop = 0; loop < (WAITING_TICK_MAX / 16); loop++)
    { //TraceDec1("GetWaitingTickValue",loop);
        Trace("GetWaitingTickValue 1");
        pRandomValue = Cmd_sys_get_random_data(16); // Unprovision it maybe crash
        Trace("GetWaitingTickValue 2");
        memcpy(&WaitingTickTbl[loop * 16], pRandomValue->data.data, 16);
    }

    //PrintDataByte("WaitingTickTbl", WaitingTickTbl,WAITING_TICK_MAX);
    WaitingTickIndex = 0;
}
*/

//
// return waiting timer
//
uchar GetSendTimerToWaiting()
{   uchar ret_code;

/*
    if(++WaitingTickIndex >= WAITING_TICK_MAX)
    {  Trace("WaitingTickIndex Reset");
        WaitingTickIndex = 0;
    }

    return WaitingTickTbl[WaitingTickIndex];
*/
    ret_code = pMeshNodeData->MeshNodeID*3;
//    TraceDec1("Waiting Timing", ret_code);
    return ret_code;
    
}

uchar SendDataIndex;

//
// Server node send data to client
//
void ServerToClientProc()
{
    bool ret_code;
    struct gecko_msg_system_get_random_data_rsp_t *pRandomValue;
    switch(CurrTaskStage())
    {
        case SGS_TASK_PENDING: //Trace("ToClient: SGS_TASK_PENDING");
            if(GetMeshNodeStatus(STATUS_SERVER_TO_CLIENT) == ON)
            {
                CmdErrCount = 0;
                SendDataIndex = SEND_DATA_INDEX_START;
                ToNextTaskStage(SGS_TX_RANDOM_WAITING);
                SetLedStatus(LED_STATUS_SERVER_TO_CLIENT);
                
            }
            break;

        case SGS_TX_RANDOM_WAITING: // Trace("SGS_TX_RANDOM_WAITING");
            //ToUserWaiting(SGS_TX_WAITING, WaitingTickTbl[WaitingTickIndex]);
            ToUserWaiting(SGS_TX_WAITING,GetSendTimerToWaiting());
            break;

        case SGS_TX_WAITING: //Trace("ToClient: SGS_TX_WAITING");
            if(CheckTaskTimeOut() == 0) ToNextTaskStage(SGS_SEND_CMD);
            break;
        case SGS_SEND_CMD:  Trace("ToClient: SGS_SEND_CMD");
            if(GetMeshNodeStatus(STATUS_SERVER_MODBUS_TIME_OUT) == ON)
            {// do not send data to client
                ToNextTaskStage(SGS_ENDING);  TraceErr("Modbus Data");
                break;
            }
            ret_code = ServerModbusRegsToClient(SendDataIndex);
            if(ret_code == FALSE)
            {
                CmdErrCount++; ToUserWaiting(SGS_TX_WAITING, 250); Trace1("Error 0x181",SendDataIndex);
                if(CmdErrCount > ERR_CMD_COUNT )  {ToNextTaskStage(SGS_ENDING); TraceErr("Server Send Fail!!!");}
                break;
            }
            
            if(++SendDataIndex > SEND_DATA_INDEX_ENDING)
                {ToUserWaiting(SGS_WAITING_ENDING, 100);}
            else
                {ToUserWaiting(SGS_TX_WAITING, 200);}
            break;
        case SGS_WAITING_ENDING:// TraceDec1("ToClient: SGS_WAITING_ENDING",pDeviceTask->TaskTimeOut);
            if(CheckTaskTimeOut() == 0)  ToNextTaskStage(SGS_ENDING);
            break;
        case SGS_ENDING: Trace("ToClient: SGS_ENDING");
            SetMeshNodeStatus(STATUS_SERVER_TO_CLIENT, OFF);
            ToNextTaskStage(SGS_TASK_PENDING);
            break;
        default:TraceErr("ServerToClientProc");
            break;

    };
}

//
// for  Sensor Server
//
void ServerGetReguset(PCmdPacket pCmdEvent)
{
    TraceProc();
    msg_ms_server_get_request_evt *pEvent = &(pCmdEvent->data.evt_mesh_sensor_server_get_request);
    Trace16_4(pEvent->client_address, pEvent->server_address, pEvent->appkey_index, pEvent->elem_index);
    pMeshNodeData->ElemIndex = pEvent->elem_index;
    pMeshNodeData->ClientAddr = pEvent->client_address;
    pMeshNodeData->ServerAddr = pEvent->server_address;
    pMeshNodeData->AppkeyIndex = pEvent->appkey_index;

    SetMeshNodeStatus(STATUS_SERVER_TO_CLIENT, ON);
     ResetEventCounter(TD_NO_EVENT);
}


//
// for  Sensor Setup Server
//
void ServerGetRegusetToClient(PCmdPacket pCmdEvent)
{
    TraceProc();
    msg_ms_setup_server_get_setting_request_evt *pEvent = &(pCmdEvent->data.evt_mesh_sensor_setup_server_get_setting_request);
    //Trace16_4(pEvent->client_address, pEvent->server_address, pEvent->appkey_index, pEvent->elem_index);
    //Trace16_2(pEvent->property_id, pEvent->setting_id);
    pMeshNodeData->ElemIndex = pEvent->elem_index;
    pMeshNodeData->ClientAddr = pEvent->client_address;
    pMeshNodeData->ServerAddr = pEvent->server_address;
    pMeshNodeData->AppkeyIndex = pEvent->appkey_index;

    SetMeshNodeStatus(STATUS_SERVER_TO_CLIENT, ON);
    if(GetMeshNodeStatus(STATUS_IVI_UPDATE) == ON)
        {
        SetLedStatus(LED_STATUS_IVI_UPDATE_OFF);
        IviUpdateStatus(OFF);
        }
    ResetEventCounter(TD_NO_EVENT);
}


uint16 G6sStatus;

#define G6SPOWER_REG        DevModbusRegs[1][0]
#define G6STATUS_REG        DevModbusRegs[1][1]


//
//
//
void DevG6sStatusToClient()
{
#if UPDATE_REAL_TIME
TraceProc();

#ifdef SIMULATION_MODBUS_REGS
        if(G6sStatus == 0x19) DevModbusRegs[1][1] = 0x4B;
        else if(G6sStatus == 0x4B) DevModbusRegs[1][1] = 0x64;
        else if(G6sStatus == 0x64) DevModbusRegs[1][1] = 0x00;
        else if(G6sStatus == 0x00) DevModbusRegs[1][1] = 0x19;
#endif

    if(G6sStatus != DevModbusRegs[1][1])
        {Trace("***** Update Client ****");Trace16_2(G6sStatus, DevModbusRegs[1][1]);
            G6sStatus = DevModbusRegs[1][1];
            ServerModbusRegsToClient(SEND_DATA_INDEX_2);
        }
    else {Trace("***** Not Update ****");Trace16_2(G6sStatus, DevModbusRegs[1][1]);}
#endif
    return;
}



#define G6S_POWER_OFF       0x4D00
#define G6S_POWER_LOW       0x0802
#define G6S_POWER_MIDDLE    0xB702
#define G6S_POWER_HIGH      0xA705





//
// for  Sensor Setup Server
//
void ServerSetDevice(msg_ms_setup_server_set_setting_request_evt *pEvent)
{
    TraceProc();
    uchar   dev_status, loop;
    PUCHAR  p_rx_buff;
    //Trace16_4(pEvent->elem_index, pEvent->client_address, pEvent->server_address, pEvent->appkey_index);
    //Trace16_2(pEvent->property_id, pEvent->setting_id);
    pMeshNodeData->ElemIndex = pEvent->elem_index;
    pMeshNodeData->ClientAddr = pEvent->client_address;
    pMeshNodeData->ServerAddr = pEvent->server_address;
    pMeshNodeData->AppkeyIndex = pEvent->appkey_index;
    
    memset(ModbusSetCmdF6, 0, sizeof(ModbusSetCmdF6));
    ModbusSetCmdF6[0] = 1;
    ModbusSetCmdF6[1] = pEvent->setting_id; //Modbus Fun 6
    memcpy(&ModbusSetCmdF6[2], &pEvent->raw_value.data, pEvent->raw_value.len);
    *(PUINT16)(&ModbusSetCmdF6[6]) = ModbusRtu_CRC16((PUCHAR)&ModbusSetCmdF6, 6);

   // PrintDataByte("Fun6 Code",ModbusSetCmdF6, 8);
   
    p_rx_buff = UsartGetBuff(USART_ID_RX);
    dev_status = 0xFF;

#ifndef SIMULATION_MODBUS_REGS

    for(loop=0; loop < 10; loop++)
        {TraceDec1("Control 1",CounterRx);
           if(UsartGetStatusRxEnd() != TRUE) {Delay_ms(200);Trace("USART RX Busy");}
           UsartTxSendCmd((PUCHAR)&ModbusSetCmdF6, 8); Delay_ms(500);
           if(CounterRx == 8 && ModbusSetCmdF6[5] == p_rx_buff[5])
            {TraceOk("Dev Control 1"); PrintDataByte("RX Buff 1",p_rx_buff,CounterRx);
           
              goto ToClient;   
            }
           else {TraceErr("Dev Control 2"); goto ToReturn; }
        }

#endif

ToClient:
    
    dev_status = ModbusSetCmdF6[5];
    dev_status &=0x0C;

    if(dev_status == 0x00) {G6SPOWER_REG = G6S_POWER_OFF; Trace(" G6S Power OFF");}
    else if(dev_status == 0x04) {G6SPOWER_REG = G6S_POWER_LOW; Trace(" G6S Power LOW");}
    else if(dev_status == 0x08) {G6SPOWER_REG = G6S_POWER_MIDDLE;Trace(" G6S Power MIDDLE");}
    else if(dev_status == 0x0C) {G6SPOWER_REG = G6S_POWER_HIGH; Trace(" G6S Power HIGH");}
    else TraceErr("G6S Power");    
    G6STATUS_REG = *((PUINT16)&ModbusSetCmdF6[4]);   //update register

    SetMeshNodeStatus(STATUS_SERVER_SET_DEVICE, ON); // Set Modbus Fun 6

/*
    Delay_ms(pMeshNodeData->MeshNodeID*10);
    //TraceDec1("CRTL Delay", pMeshNodeData->MeshNodeID*10);
    if(ServerModbusRegsToClient(SEND_DATA_INDEX_2) == FALSE)
        {TraceErr("Return Control");
         //Delay_ms(2000); ServerModbusRegsToClient(SEND_DATA_INDEX_2);
        }
*/    

ToReturn:    
    
   SetEventTaskTimer(TD_GET_SENSOR_INFO, TIMER_10SEC, TIMER_EVENT_ONCE);
   // SetEventTaskTimer(TD_GET_SENSOR_INFO, TIMER_15SEC, TIMER_EVENT_ONCE);
   // SetEventTaskTimer(TD_GET_SENSOR_INFO, TIMER_5SEC, TIMER_EVENT_ONCE);
    PrintDataByte("SetDeviceCmd", (PUCHAR)ModbusSetCmdF6, sizeof(ModbusSetCmdF6));



    
    //PrintDataByte("ServerSetDevice", (PUCHAR)&pEvent->raw_value.data,pEvent->raw_value.len);


}




//
// Setup Modbus Device
//
void ServerSetupProc()
{//TraceProc();
 //   return;
    switch(CurrTaskStage())
    {
        case SERVER_SETUP_PENDING:  //Trace("SERVER_SETUP_PENDING");
            if(GetMeshNodeStatus(STATUS_SERVER_SET_DEVICE) == ON)
            {
               SetMeshNodeStatus(STATUS_SERVER_SET_DEVICE, OFF);  
               ToUserWaiting(SERVER_SETUP_TO_CLIENT,pMeshNodeData->MeshNodeID*5);  
            }
                            
            break;
        case SERVER_SETUP_TO_CLIENT:  //Trace("SERVER_SETUP_TO_CLIENT");
            if(CheckTaskTimeOut() == 0)
            {
                if(ServerModbusRegsToClient(SEND_DATA_INDEX_2) == FALSE)
                    {ToUserWaiting(SERVER_SETUP_TO_CLIENT,50);}
                else
                    {ToNextTaskStage(SERVER_SETUP_ENDING);}
            }
            break;
        case SERVER_SETUP_ENDING:  Trace("SERVER_SETUP_ENDING");
            ToNextTaskStage(SERVER_SETUP_PENDING);
            break;

            
        default: TraceErr("ServerSetupProc");  break;
    };

}



//
//
//
Bool ServerModbusRegsToClient(uchar status)
{
    TraceProc();
    Bool ret_code = TRUE;
    PUINT16 pRegs;

    if(pMeshNodeData->ClientAddr == 0) return TRUE;

    SensorRegsData.ProperityID = MODBUS_GET_REGS_VALUE;
    SensorRegsData.PacketSize =  sizeof(_ToClientRegs);
    //SensorRegsData.ToClientRegs.MeshNodeAddr = (uchar)MeshNodeID;
    SensorRegsData.ToClientRegs.MeshNodeAddr = pMeshNodeData->MeshNodeID;
    SensorRegsData.ToClientRegs.GetRegsStatus = status;

    switch(status)
    {
        case SEND_DATA_INDEX_0:
            Trace("SEND_DATA_INDEX_0:0x000 ~ 0x005");
            pRegs = &DevModbusRegs[0][0];
            break;
        case SEND_DATA_INDEX_1:
            Trace("SEND_DATA_INDEX_1:0x006 ~ 0x009");
            pRegs = &DevModbusRegs[0][6];
            break;
        case SEND_DATA_INDEX_2:
            Trace("SEND_DATA_INDEX_2:0x2FE ~ 0x303");
            pRegs = &DevModbusRegs[1][0];
            break;
        case SEND_DATA_INDEX_3:
            Trace("SEND_DATA_INDEX_3:0x304 ~ 0x309");
            pRegs = &DevModbusRegs[1][6];
            break;
        case SEND_DATA_INDEX_4:
            Trace("SEND_DATA_INDEX_4:0x30A ~ 0x30F");
            pRegs = &DevModbusRegs[2][0];
            break;
        case SEND_DATA_INDEX_5:
            Trace("SEND_DATA_INDEX_5:0x310 ~ 0x315");
            pRegs = &DevModbusRegs[2][6];
            break;
        default: TraceErr("ServerModbusRegsToClient");
            return FALSE;
    }
    memcpy(SensorRegsData.ToClientRegs.ModbusRegs, pRegs, To_CLIENT_REGS_NUM_MAX * 2);
    //PrintDataByte("To Client Data", (PUCHAR)&SensorRegsData, sizeof(SensorRegsData));
    ShowCurrRemSeq(); ShowTimerRTC();

    result = Cmd_ms_server_send_status(SENSOR_ELEMENT, pMeshNodeData->ClientAddr, pMeshNodeData->AppkeyIndex, NO_FLAGS,
                                       sizeof(SensorRegsData), (PUCHAR)&SensorRegsData)->result;
    if(result)
    {
        TraceErr1("SERVER_SEND_ALL_REGS 1", result);
        if(result == 0x182) { TraceErr("Device Out of memory");
            SetEventTaskTimer(TD_SYS_RESET, TIMER_5SEC, TIMER_EVENT_ONCE);
        }
        ret_code = FALSE;
    }
    else
    {
        TraceOk1("SERVER_SEND_ALL_REGS 2",status);
    }

    return ret_code;
}


//
//
//
bool ServerPubModbusRegs()
{
    TraceProc();

#ifndef SERVER_AUTO_PUBLISH
    return TRUE;
#endif

    /*
        SetLedToggle(LED0);
        SensorRegsData.ProperityID = MODBUS_GET_REGS_VALUE;
        SensorRegsData.PacketSize = sizeof(_ModbusRegs);// sizeof(SensorRegsData);
        SensorRegsData.ModbusRegs.MeshNodeAddr = pMeshNodeData->MeshNodeID;

    #ifdef SIMULATION_MODBUS_REGS
        //SensorRegsData.ModbusRegs.ModbusID++;
        SensorRegsData.ModbusRegs.RegFc4_0_8[0] = SimCount++;
    #endif
        ShowCurrRemSeq(); ShowTimerRTC();
        PrintDataByte("SERVER_SEND_ALL_REGS", (PUCHAR)&SensorRegsData, sizeof(SensorRegsData));
        result = Cmd_ms_server_send_status(SENSOR_ELEMENT, PUBLISH_ADDRESS, IGNORED, NO_FLAGS,
        //result = Cmd_ms_server_send_status(SENSOR_ELEMENT, 84, 0x01, NO_FLAGS,
                                  sizeof(SensorRegsData), (PUCHAR)&SensorRegsData)->result;
        if(result) Trace1("Error: SERVER_SEND_ALL_REGS",result); else  TraceOk("SERVER_SEND_ALL_REGS");
    */
    return TRUE;

}


#endif



