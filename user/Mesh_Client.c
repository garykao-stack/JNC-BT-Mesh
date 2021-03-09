
#include "global.h"

//richard Add
/* BG stack headers */
#include "sensor_client.h"
#include "mesh_event.h"
#include "bus_usart.h"
#include "bus_rs485.h"
#include "bus_I2C.h"
#include "jnc_cmd.h"
#include "com_port.h"
#include "ivi_features.h"
#include "mesh_sensor.h"
#include "Mesh_Node.h"
#include "Mesh_Client.h"

uint32  ClientStatus;
uchar   RespServerNode;
uint16  ActServerAddr = PUBLISH_ADDRESS;
uint16  PropertyID = NODE_GET_ALL_SENSOR;

///////////////////////////////////////////////////////////////

_ClientInfo ClientInfo[SERVER_NODE_MAX+1];
PClientInfo pClientInfo=ClientInfo;
_PModbusCmdF4  pModbusCmd;
_ModbusToHost ModbusToHostCmd;


void ClientNodeInit()
{TraceProc();

    NodeRole = NR_CLIENT;
    UsartRxCount = 8;   //from Host cmd bytes
    Rs485Rx();
    ToNextStage(NODE_STAGE_INIT);
    UsartClientProc(); ClientFromHostProc();

}

void ClientNodeTask1()
{
    UsartClientProc();  ClientFromHostProc();ClientSetNodeInfoProc(); 
    UsartClientProc();  ClientFromHostProc();ClientGetNodeInfoProc(); 
    UsartClientProc();  ClientFromHostProc();NodeIviUpdateProc(); 
}
uint16 test1;

uint16 rx_ccount;

void ClientNodeTask2()
{
    uchar rx_count = UsartGetRxCounter();
    if(rx_count >= 7 && GetNodeStatus(NS_USART_RX_EVENT) != TRUE)
        {
         //if(rx_ccount > 100) rx_ccount = 0;   TraceDec2("Rx W",rx_ccount++,rx_count);
         Delay_ms(3);
         UsartClientProc();  UsartClientProc(); UsartClientProc(); UsartClientProc();
         return;
        }

    if(GetNodeStatus(NS_USART_RX_EVENT) == TRUE)
        {
         UsartClientProc(); ClientFromHostProc();
         UsartClientProc(); ClientFromHostProc(); //ClientFromHostProc();
        }
            ClientSetNodeInfoProc();
            NodeIviUpdateProc(); 
            ClientGetNodeInfoProc();
   
}

void ClientNodeTask()
{
    uchar rx_count = UsartGetRxCounter();
    if(rx_count >= 7 && GetNodeStatus(NS_USART_RX_EVENT) != TRUE)
        {
         Delay_ms(3);
         UsartClientProc(); UsartClientProc();UsartClientProc();//UsartClientProc();UsartClientProc();UsartClientProc();
         return;
        }
    else if(GetNodeStatus(NS_USART_RX_EVENT) == TRUE) 
        {UsartClientProc();ClientFromHostProc();UsartClientProc();ClientFromHostProc();UsartClientProc();
         ClientFromHostProc();UsartClientProc();ClientFromHostProc();
        }
    ClientGetNodeInfoProc();
    ClientSetNodeInfoProc();
    NodeIviUpdateProc(); 
}
//
//
//
PClientInfo GetServerInfoPos(uint16 node_addr)
{
    PClientInfo ret_code=NULL;
    PClientInfo p_client_info;
    int16 loop;
    p_client_info = ClientInfo;


    for(loop=0; loop<SERVER_NODE_MAX; loop++)
        {
            if(p_client_info->ServerID == node_addr )
                { //TraceDec2("Get Node Addr 1",p_client_info->ServerID,loop);
                ret_code = p_client_info; break;
                }
            p_client_info++;
        }
    
    if(ret_code != NULL) return ret_code;
    p_client_info = ClientInfo;

    for(loop=0; loop<SERVER_NODE_MAX; loop++)
        {
            if(p_client_info->ServerID == 0)
                { //TraceDec2("Get Node Addr 2",p_client_info->ServerID,loop);
                ret_code = p_client_info; break;
                }
            p_client_info++;
        }
    return ret_code;
}


uint16 GetEventCount;

//
//
//
uchar GetServerNodeNum()
{
    uchar ret_code=0;
    int16 loop;
    PClientInfo p_node_info;
    p_node_info = ClientInfo;

    for(loop=0; loop<SERVER_NODE_MAX; loop++)
        {
            if(p_node_info->ServerID != 0 ) ret_code++;
            p_node_info++;
        }
    return ret_code;
}



//uint16    GetInfoCycle=WAIT_SEC(TIMER_GET_INFO_SLEEPING);


#define TIMER_CLIENT_IVI_WAIT       5   //sec

#define MODBUS_AIP_POWER_00         0x00
#define MODBUS_AIP_POWER_25         0x19
#define MODBUS_AIP_POWER_50         0x32
#define MODBUS_AIP_POWER_75         0x4B
#define MODBUS_AIP_POWER_100        0x64

#define MODBUS_308M_NEW_BIAS        0x00
#define MODBUS_RESET_XBIAS          0x00
#define MODBUS_RESET_YBIAS          0x01
#define MODBUS_RESET_ZBIAS          0x02



#define MODBUS_AIP_POWER_CMD        5
#define AIP_CTRL_POWER              prx_buff[5]
#define A308M_CTRL_BIAS             prx_buff[5]

#define MODBUS_CTRL_REG             prx_buff[3]
uint16 SetProperityID=NODE_SET_AIP_POWER_OFF;
uint16 SetServerFunID=0;

//
// return properity ID
//
uint16 GetProperityID()
{
    PUCHAR prx_buff = UsartGetBuff(USART_ID_RX);
    uint16 properity=NULL;

    if(MODBUS_CTRL_REG == 0x02)
      {
        if(AIP_CTRL_POWER == MODBUS_AIP_POWER_00) properity = NODE_SET_AIP_POWER_OFF;
        else if(AIP_CTRL_POWER == MODBUS_AIP_POWER_25) properity = NODE_SET_AIP_POWER_25;
        else if(AIP_CTRL_POWER == MODBUS_AIP_POWER_50) properity = NODE_SET_AIP_POWER_50;
        else if(AIP_CTRL_POWER == MODBUS_AIP_POWER_75) properity = NODE_SET_AIP_POWER_75;
        else if(AIP_CTRL_POWER == MODBUS_AIP_POWER_100) properity = NODE_SET_AIP_POWER_100;
      }
    else if(MODBUS_CTRL_REG == 0x20)
      {
       // if(A308M_CTRL_BIAS == MODBUS_308M_NEW_BIAS) properity = NODE_SET_WRITE_NEW_BIAS;
        
      }
    else if(MODBUS_CTRL_REG == 0xC)
    
    return properity;
}





//
// to set device info
//
void ClientSetNodeInfoProc()
{
    pStageInfo = GetNodeStageInfo(CLIENT_SET_NODE_INFO_PROC);
    PUCHAR prx_buff;
    
    switch(ActiveStage())
        {
            case CNS_SET_INFO_INIT:
                //SetLedStatus(LED_STATUS_ON);
                ToNextStage(CNS_SET_WAITING);
                break;
            case CNS_SET_WAITING:   //Trace("CNS_SET_WAITING");                
                if(GetNodeStatus(NS_SET_NODE_ACT) == TRUE)  ToNextStage(CNS_SET_INFO_PRE);
                break;
                
            case CNS_SET_INFO_PRE: 
                PrintRx(CNS_SET_INFO_PRE);
                prx_buff = UsartGetBuff(USART_ID_RX);
                SetServerFunID = (uint16)prx_buff[0]; // get id number
                SetProperityID = GetProperityID();
                if(SetProperityID != NULL ) ToNextStage(CNS_SET_INFO_SEND);
                else ToNextStage(CNS_SET_INFO_ERR);
                break;
                
            case CNS_SET_INFO_SEND: 
                result = Cmd_ms_client_get(SENSOR_ELEMENT, SetServerFunID, IGNORED, 0xA5, SetProperityID)->result;
                if(result)
                    { 
                     ToWaitingStage(CNS_SET_INFO_PREEAT,50);
                    }
                else 
                    ToNextStage(CNS_SET_INFO_OK);
                break;
            case CNS_SET_INFO_PREEAT:
                if(!CheckWaitTimeOut()) break;
                result = Cmd_ms_client_get(SENSOR_ELEMENT, SetServerFunID, IGNORED, 0xA5, SetProperityID)->result;
                if(result)
                    { 
                     ToNextStage(CNS_SET_INFO_ERR);
                    }
                else 
                    ToNextStage(CNS_SET_INFO_OK);
                break;
            case CNS_SET_INFO_OK:
                prx_buff = UsartGetBuff(USART_ID_RX);
                UsartTxSendCmd(prx_buff ,MODBUS_CMD_NUM);
                ToWaitingStage(CNS_SET_UPDATE_INFO,10);
                break;
                
            case CNS_SET_UPDATE_INFO: 
                if(CheckWaitTimeOut())  ToNextStage(CNS_SET_INFO_END);
                break;
                
            case CNS_SET_INFO_END:
                SetNodeStatus(NS_SET_NODE_ACT,OFF);
                UsartResetRxTx(USART_ID_RX);
                ToNextStage(CNS_SET_WAITING); 
                break;
                
            case CNS_SET_INFO_ERR:
                ToNextStage(CNS_SET_INFO_END);
                break;
            default: break;
        }
}




//
// handle node response
//
void ClientGetNodeInfoProc()
{
    pStageInfo = GetNodeStageInfo(CLIENT_GET_NODE_INFO_PROC);
    
    switch(ActiveStage())
        {
            case NODE_STAGE_INIT:
                //SetLedStatus(LED_STATUS_ON);
                ToWaitingStage(CNS_GET_SEVER_INFO,WAIT_SEC(1));
                break;
            case CNS_PRE_IVI_UPDATE: //Trace("CNS_PRE_IVI_UPDATE 1");
                if(CheckWaitTimeOut())
                    {
                     SetNodeStatus(NS_IVI_UPDATE,ON);   // enable ivi update
                     CountErr = 0;
                     ToWaitingStage(CNS_IVI_UPDATE,WAIT_SEC(TIMER_CLIENT_IVI_WAIT));
                    }
                break;
            
            case CNS_IVI_UPDATE: //Trace("CNS_IVI_UPDATE 1");
                if(CheckWaitTimeOut() == TRUE) 
                  {
                    if(CountErr++ > 3)
                      {
                        ToNextStage(CNS_WAIT_SET_INFO);   
                      }
                    else {ToWaitingStage(CNS_IVI_UPDATE,WAIT_SEC(TIMER_CLIENT_IVI_WAIT));} // waiting again
                  }
               if(GetNodeStatus(NS_IVI_UPDATE) == OFF)
                  {
                   ToNextStage(CNS_WAIT_SET_INFO); 
                  }
                break;
            case CNS_PRE_SEVER_INFO:
                SetLed(LED_SERVER,OFF); 
                SetNodeStatus(NS_GET_INFO_ACT,OFF);
                if(MeshCheckSeqNum())
                    {
                     ShowCurrRemSeq();
                     ToWaitingStage(CNS_WAIT_SET_INFO,GetInfoCycle);
                    } 
                else{
                     ToWaitingStage(CNS_PRE_IVI_UPDATE,GetInfoCycle);                   
                    }
                break;
            case CNS_WAIT_SET_INFO: 
                if(CheckWaitTimeOut())ToNextStage(CNS_GET_SEVER_INFO);
                else if(GetNodeStatus(NS_SET_NODE_ACT) != TRUE)  ToNextStage(CNS_GET_SEVER_INFO);
                
                break;
            case CNS_GET_SEVER_INFO: //TraceDec1("CNS_GET_SEVER_INFO",pStageInfo->Timer);
                if(CountErr > 3) ToNextStage(CNS_GET_INFO_ERR); // err: go to sleeping

                if(CheckWaitTimeOut()) 
                //if(CheckWaitTimeOut() && UsartGetStatusRxIng() == OFF) 
               // if(CheckWaitTimeOut() && ((UsartGetRxCounter() == 0) || (UsartGetRxCounter() ==8)) )
                  {//to get info from all server node
                    GetEventCount++;
                    SetLed(LED_SERVER,ON);
                    result = Cmd_ms_client_get(SENSOR_ELEMENT, ActServerAddr, IGNORED, 0xA5, PropertyID)->result;
                    if(result)
                        {
                          CountErr++;
                          ToWaitingStage(CNS_GET_SEVER_INFO,WAIT_SEC(2));  
                        }
                    else{
                          CountErr = 0; RespServerNode = 0;
                          ToWaitingStage(CNS_WAIT_INFO,TIMER_CLI_WAIT_INFO);
                        }
                  }
                 else
                 {
                    if(GetNodeStatus(NS_GET_INFO_ACT))
                        { 
                          ToWaitingStage(CNS_WAIT_INFO,TIMER_CLI_WAIT_INFO);
                        }
                 }
                    
                break;
            case CNS_WAIT_INFO:
                if(CheckWaitTimeOut()) ToNextStage(CNS_WAIT_INFO_OK); 
                break;
            case CNS_WAIT_INFO_OK:
                ClientCheckNodeStatus();
                
                if(GetNodeStatus(NS_FULL_POWER)) GetInfoCycle = WAIT_SEC(TIMER_GET_INFO_FULL_POWER);
                else GetInfoCycle = WAIT_SEC(TIMER_GET_INFO_SLEEPING);
                    
                ToNextStage(CNS_GET_INFO_END); 
                break;
            case CNS_GET_INFO_ERR:
                CountErr = 0;
                ToNextStage(CNS_PRE_SEVER_INFO); 
                break;
            case CNS_GET_INFO_END:
                TraceDec1("**** Return Node Num *****", RespServerNode); 
                GetServerNodeNum();
                
                ToNextStage(CNS_PRE_SEVER_INFO); 
                break;
            default: break;
        };
}


//
//
//
void ClientCheckNodeStatus()
{
    PClientInfo pNodeInfo=ClientInfo;
    uchar power_flag=ON;
    uchar loop;

    for(loop=0; loop < SERVER_NODE_MAX; loop++)
       {
        if(pNodeInfo->ServerID != 0)
          {
            if(pNodeInfo->Count == 0)
            {TraceErr1("Node No Response",pNodeInfo->ServerID);
             memset(pNodeInfo,0,sizeof(_ClientInfo));
             pNodeInfo->Status |= SERVER_NO_RESPONSE;
            } 
            else 
            {
              if(pNodeInfo->Count) pNodeInfo->Count--;
              if((pNodeInfo->SensorInfo.Header.Status & SERVER_FULL_POWER) == 0)
                {
                 power_flag = OFF;
                // Trace2("Power Sleeping", pNodeInfo->ServerID,pNodeInfo->SensorInfo.Header.Status );
                }
              //Trace2("Power status 1", pNodeInfo->ServerID,pNodeInfo->SensorInfo.Header.Status );
            }
                
          }  
        pNodeInfo++;
       }
    
    if(power_flag == OFF) 
        {Trace("NS_FULL_POWER: OFF");
        SetNodeStatus(NS_FULL_POWER,OFF);SetLed(LED_GREEN,OFF);}
    else 
        {Trace("NS_FULL_POWER: ON");
        SetNodeStatus(NS_FULL_POWER,ON);SetLed(LED_GREEN,ON);}
}

//
//
//
bool SendModbusToHost()
{//TraceProc();
    bool ret_code=TRUE;
    //PrintDataByte("SendModbusToHost", (PUCHAR)&ModbusToHostCmd,ModbusToHostCmd.ByteNum+5);
    UsartTxSendCmd((PUCHAR)&ModbusToHostCmd, ModbusToHostCmd.ByteNum+5); 
    return  ret_code;

}


#define MODBUS_GET_INFO         0
#define MODBUS_SET_INFO         1
#define MODBUS_CMD_ERROR        (-1)
//
// Check modbus command from host which is Get or Set command
//
uint16 CheckModbusCmd()
{
    uint16 ret_code=MODBUS_GET_INFO;
    pModbusCmd= (_PModbusCmdF4)UsartGetBuff(USART_ID_RX);
    // 1. check Rx byts 2. check crc error 3. check cmd content
    if(UsartGetRxCounter() < MODBUS_CMD_NUM || CheckModbusCrc((PUCHAR)pModbusCmd,MODBUS_CMD_NUM) == FALSE) 
        {TraceErr("Modbus Cmd: ");PrintDataByte("Modbus Cmd", (PUCHAR)pModbusCmd, UsartGetRxCounter());
        ret_code=MODBUS_CMD_ERROR;
        }
    else
        {
            if(pModbusCmd->FunCode == 0x01 || pModbusCmd->FunCode == 0x04 || pModbusCmd->FunCode == 0x03)
            ret_code=MODBUS_GET_INFO;
            else if(pModbusCmd->FunCode == 0x06)
            ret_code=MODBUS_SET_INFO;
            else Trace("CheckModbusCmd");
        }
    return ret_code;
}

uchar SendDataDelay;
//
// Handle Client, Host message
//
void ClientFromHostProc()
{
    pStageInfo = GetNodeStageInfo(CLIENT_TO_HOST_STAGE_PROC);
    
    switch(ActiveStage())
        {
        case CHS_STAGE_INIT: 
            ToNextStage(CHS_CHECK_RX_EVENT);   //default
            break;
        case CHS_CHECK_RX_EVENT: //Trace("CHS_CHECK_RX_EVENT");
            if(GetNodeStatus(NS_USART_RX_EVENT) == TRUE) 
                ToNextStage(CHS_CHECK_MODBUS); //ToNextStage(CHS_PREPARE_DATA);   //default
            break;
        
        case CHS_CHECK_MODBUS: // check host cmd whether set or get cmd
            if(CheckModbusCmd() == MODBUS_GET_INFO) ToNextStage(CHS_PREPARE_DATA);
            else if(CheckModbusCmd() == MODBUS_SET_INFO) ToNextStage(CHS_SERVER_SET);
            else ToNextStage(CHS_MODBUS_ERROR);
            break;
        
        case CHS_PREPARE_DATA: 
            if(ClientPrepareToHost() == TRUE)
                {SendDataDelay = 1;ToNextStage(CHS_SEND_DATA_DELAY);}
            else{
                ToNextStage(CHS_MODBUS_ERROR);
                }
            break;
        case CHS_SEND_DATA_DELAY: 
            if(SendDataDelay == 0)   ToNextStage(CHS_SEND_DATA);
            else SendDataDelay--;
            break;
        case CHS_SEND_DATA: 
            SendModbusToHost();
            UsartResetRxTx(USART_ID_RX);
            ToNextStage(CHS_SEND_DATA_END);   //default
            break;
        case CHS_SERVER_SET: 
            SetNodeStatus(NS_SET_NODE_ACT,ON);
            ToNextStage(CHS_SEND_DATA_END);   //default
            break;
        case CHS_SEND_DATA_END: 
            SetNodeStatus(NS_USART_RX_EVENT,OFF);
            ToNextStage(CHS_CHECK_RX_EVENT);   //default
            break;
        case CHS_MODBUS_ERROR: 
            UsartResetRxTx(USART_ID_RX);
            ToNextStage(CHS_SEND_DATA_END);   //default
            break;
            
        default:break;
        };
    
}


void ClientPropertyEvent(msg_ms_client_status_evt *pEvent)
{//TraceProc();
    uint8_t *p_sensor_data = pEvent->sensor_data.data;
    uint8_t data_len = pEvent->sensor_data.len;
    uint8_t pos = 0;
    mesh_device_properties_t property_id;
    if((pClientInfo = GetServerInfoPos(pEvent->server_address))== NULL) 
           {return;}
    //Reset Timer
    GetNodeStageInfo(CLIENT_GET_NODE_INFO_PROC)->Timer = TIMER_CLI_WAIT_INFO;
    RespServerNode++;
    pClientInfo->ServerID = pEvent->server_address;
    pClientInfo->Status &= !SERVER_NO_RESPONSE;
    
    pNodeEventInfo->ElemIndex   = pEvent->appkey_index;
    pNodeEventInfo->ClientAddr  = pEvent->client_address;
    pNodeEventInfo->ServerAddr  = pEvent->server_address;
    pNodeEventInfo->AppkeyIndex = pEvent->appkey_index;
    pNodeEventInfo->Flags       = pEvent->flags;
    while(pos < data_len)
        {
           //TraceDec2("data_len" , data_len, pos);
           if(data_len - pos > PROPERTY_ID_SIZE)
            {
               property_id = (mesh_device_properties_t)(p_sensor_data[pos] + (p_sensor_data[pos + 1] << 8));
               //Trace16_1(property_id);
               uint8_t property_len = p_sensor_data[pos + PROPERTY_ID_SIZE];
               uint8_t *property_data = NULL;
               if(property_len && (data_len - pos > PROPERTY_HEADER_SIZE))
                   {property_data = &p_sensor_data[pos + PROPERTY_HEADER_SIZE];}
               switch(property_id)
                {
                    case NODE_GET_ALL_SENSOR: //Trace("NODE_GET_ALL_SENSOR");
                        memcpy(&(pClientInfo->SensorInfo),property_data,property_len);
                        break;
                    default: 
                };
               pos += PROPERTY_HEADER_SIZE + property_len;
            }
           else 
            pos = data_len;
               
        };
    pClientInfo->Count = COUNT_NODE_DETECTED;

}


void PropertyLcd()
{
#ifdef SILICON_EVA_BOARD

    uchar loop;    
    char tmp[21];
    PClientInfo p_node_info = ClientInfo;
    PSi7021Info pSi7021Info = &(p_node_info->SensorInfo.Si7021Info);

    for(loop=0; loop<7; loop++)
       {
        snprintf(tmp, 21, "Adr%2x %3u %3u ", p_node_info->ServerID, p_node_info->SensorInfo.Header.BatteryPower,
                        p_node_info->SensorInfo.Header.BatteryPower);
        
        p_node_info++;
        DI_Print(tmp, 3+loop);
       }
#endif
    
}


//
//
//
bool ClientPrepareToHost()
{//TraceProc();
    bool    ret_code=FALSE;
    uint8   sensor_class;
    pModbusCmd = (_PModbusCmdF4)UsartGetBuff(USART_ID_RX);
    
    //check host request
    if((pClientInfo = GetServerInfoPos((uint16)(pModbusCmd->ModbusID)))== NULL) 
           {return ret_code;}
    if(pModbusCmd->FunCode == 0x03 && pModbusCmd->StartAddr == 0x0000 && pModbusCmd->TotalReg == 0x0300)
        sensor_class = OTHER_MODBUS_CMD;
    else
        sensor_class = pClientInfo->SensorInfo.Header.SensorClass;
    switch(sensor_class)
        {
            case SENSOR_SI7021:     ret_code = ClientSi7021(&pClientInfo->SensorInfo.Si7021Info);
                break;
            case SENSOR_PT485:      ret_code = ClientPT485(&pClientInfo->SensorInfo.PT485);
                break;
            case SENSOR_AIP:        ret_code = ClientAIP(&pClientInfo->SensorInfo.AipInfo);
                break;
            case SENSOR_A308M:      ret_code = ClientA308m(&pClientInfo->SensorInfo.A308mInfo);
                break;
           case SENSOR_WATER_LEVEL: ret_code = ClientWaterLevel(&pClientInfo->SensorInfo.WaterLevelInfo);
                break;
            case SENSOR_JNC_SD:     ret_code = ClientJncSd(&pClientInfo->SensorInfo.SdInfo);
                break; 
/*                
            case SENSOR_IAQS:     ret_code = ClientIAQS(&pClientInfo->SensorInfo.IaqsInfo);
                break;           
            case SENSOR_ULTRA_SOUND:     ret_code = ClientUltraSound(&pClientInfo->SensorInfo.UltraSound);
                break;
*/                
            case OTHER_MODBUS_CMD:  ret_code = ClientOtherModbusCmd();
                break;
            default: 
                ret_code=FALSE;
                break;
        }

    if(ret_code == TRUE)  
        *(PUINT16)(&ModbusToHostCmd.Data[(ModbusToHostCmd.ByteNum)/2]) = ModbusRtu_CRC16((PUCHAR)&ModbusToHostCmd,ModbusToHostCmd.ByteNum+3);

    return ret_code;

}

const char BtmModelName[6]="BTM001";

//
//
//
bool ClientOtherModbusCmd()
{TraceProc();
    bool ret_code=TRUE;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 6;
    memcpy(ModbusToHostCmd.Data,BtmModelName,6);

    return ret_code;
}


//
//
//
bool ClientSi7021(PSi7021Info p_info)
{//TraceProc();
    bool ret_code=TRUE;
    uint16 start_addr,total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr); // Trace16_1(start_addr);
    total_reg = WordSwap(pModbusCmd->TotalReg);    // Trace16_1(total_reg);

    if(start_addr == SI7021_TEMP)
      {//TraceDec1("Temp",p_info->Tempature);
        ModbusToHostCmd.Data[0] =  WordSwap(p_info->Tempature);
      }
    else if(start_addr == SI7021_RH )
      {//TraceDec1("Humidity",p_info->Humidity);
        ModbusToHostCmd.Data[0] =  WordSwap(p_info->Humidity);
      }
    else if(start_addr == BATTERY_POWER )
      {
        ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower));
      }
    else{ TraceErr1("ClientSi7021",total_reg);
        ret_code=FALSE;
        }


    return ret_code;
}

//
//
//
bool ClientPT485(PPT485Info p_info)
{//TraceProc();
    bool ret_code=TRUE;
    uint16 start_addr,total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr); 
    total_reg = WordSwap(pModbusCmd->TotalReg);    

    if(start_addr == PT485_TEMP)
      {//TraceDec1("PT485 Temp",p_info->Tempature);
        ModbusToHostCmd.Data[0] =  WordSwap(p_info->Tempature);
      }
    else if(start_addr == BATTERY_POWER )
      {
        ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower));
      }
    
    else{ TraceErr1("ClientPT485",total_reg);
        ret_code=FALSE;
        }


    return ret_code;
}

//
//
//
bool ClientAIP(PAIPInfo p_info)
{//TraceProc();
    bool ret_code=TRUE;
    uint16 start_addr,total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr); 
    total_reg = WordSwap(pModbusCmd->TotalReg);   

    if(start_addr == AIP_POWER)
      {//TraceDec1("AIP Power",p_info->AipPower);
        ModbusToHostCmd.Data[0] =  WordSwap(p_info->AipPower);
      }
    else if(start_addr == AIP_POWER_STATUS)
      {//Trace("AIP Power Status 1");
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->AipPowerStatus); 
      }
    else if(start_addr == AIP_POWER_STATUS_VALUE)
      {//TraceDec1("AIP Power Status value 2",p_info->AipPowerStatus);

       if(p_info->AipPowerStatus == 0)      ModbusToHostCmd.Data[0] = 0x0000;  //0%
       else if(p_info->AipPowerStatus == 1) ModbusToHostCmd.Data[0] = 0x1900;   //25%
       else if(p_info->AipPowerStatus == 2) ModbusToHostCmd.Data[0] = 0x3200;   //50%
       else if(p_info->AipPowerStatus == 3) ModbusToHostCmd.Data[0] = 0x4B00;   //75%
       else if(p_info->AipPowerStatus == 4) ModbusToHostCmd.Data[0] = 0x6400;   //100%
       else TraceErr("AIP Power Value");
       //PrintDataByte("AIP Power Value", (PUCHAR)&ModbusToHostCmd, 7);
      }
    
    else if(start_addr == BATTERY_POWER )
      {
        ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower));
      }
    else{ TraceErr1("ClientPT485",total_reg);
        ret_code=FALSE;
        }


    return ret_code;
}

//
//
//
bool ClientA308m(PA308mInfo p_info)
{//TraceProc();
    bool ret_code=TRUE;
    uint16 start_addr,total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr); 
    total_reg = WordSwap(pModbusCmd->TotalReg);

    switch(start_addr)
        {
            case A308M_XRMS:    ModbusToHostCmd.Data[0] = WordSwap(p_info->Xrms);
                break;
            case A308M_XSPEED:  ModbusToHostCmd.Data[0] = WordSwap(p_info->XSpeed);
                break;
            case A308M_YRMS:    ModbusToHostCmd.Data[0] = WordSwap(p_info->Yrms);
                break;
            case A308M_YSPEED:  ModbusToHostCmd.Data[0] = WordSwap(p_info->YSpeed);
                break;
            case A308M_ZRMS:    ModbusToHostCmd.Data[0] = WordSwap(p_info->Zrms);
                break;
            case A308M_ZSPEED:  ModbusToHostCmd.Data[0] = WordSwap(p_info->ZSpeed);
                break;
            case A308M_TEMP:    ModbusToHostCmd.Data[0] = WordSwap(p_info->Tempature);
                break;
            case A308M_XFFT_FRE:    ModbusToHostCmd.Data[0] = WordSwap(p_info->XFFT_Fre);
                break;
            case A308M_XFFT_STR:    ModbusToHostCmd.Data[0] = WordSwap(p_info->XFFT_Str);
                break;
            case A308M_YFFT_FRE:    ModbusToHostCmd.Data[0] = WordSwap(p_info->YFFT_Fre);
                break;
            case A308M_YFFT_STR:    ModbusToHostCmd.Data[0] = WordSwap(p_info->YFFT_Str);
                break;
            case A308M_ZFFT_FRE:    ModbusToHostCmd.Data[0] = WordSwap(p_info->ZFFT_Fre);
                break;
            case A308M_ZFFT_STR:    ModbusToHostCmd.Data[0] = WordSwap(p_info->ZFFT_Str);
                break;
            case BATTERY_POWER:     ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower));
                break;
            default: ret_code=FALSE; Trace1("ClientA308m:start_addr",start_addr);break;
        };

    return ret_code;
}

//
//
//
bool ClientWaterLevel(PWaterLevelInfo p_info)
{//TraceProc();
    bool ret_code=TRUE;
    uint16 start_addr,total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr); 
    total_reg = WordSwap(pModbusCmd->TotalReg);   

    if(start_addr == POSITION_WATER)
      {
        ModbusToHostCmd.Data[0] =  WordSwap(p_info->WaterLevel);
      }
    else if(start_addr == POSITION_OIL)
      {
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->OilLevel); 
      }
    else{ 
        ret_code=FALSE;
        }


    return ret_code;
}

//
//
//
bool ClientJncSd(PSdInfo p_info)
{//TraceProc();
    bool ret_code=TRUE;
    uint16 start_addr,total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr); 
    total_reg = WordSwap(pModbusCmd->TotalReg);   

    if(start_addr == JNC_SD_CO2)
      {
        ModbusToHostCmd.Data[0] =  WordSwap(p_info->CO2);
      }
    else if(start_addr == JNC_SD_PM25)
      {
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->PM25); 
      }
    else if(start_addr == JNC_SD_TEMP)
      {
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->Tempature); 
      }
    else if(start_addr == JNC_SD_RH)
      {//Trace(JNC_SD_RH");
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->Humidity); 
      }    
    else{ TraceErr1("ClientJncSd",total_reg);
        ret_code=FALSE;
        }
    return ret_code;
}

//
//
//
bool ClientIAQS(PIaqsInfo p_info)
{//TraceProc();
    bool ret_code=TRUE;
    uint16 start_addr,total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr); 
    total_reg = WordSwap(pModbusCmd->TotalReg);   

    if(start_addr == FC04_REG_TEMP)
      {
        ModbusToHostCmd.Data[0] =  WordSwap(p_info->Tempature);
      }
    else if(start_addr == FC04_REG_RH)
      {
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->Humidity); 
      }
    else if(start_addr == FC04_REG_CO2)
      {
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->CO2); 
      }
    else if(start_addr == FC04_REG_PM25)
      {
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->PM25); 
      }
    else if(start_addr == FC04_REG_HCHO)
      {
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->HCHO); 
      } 
    else if(start_addr == FC04_REG_CO)
      {
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->CO); 
      } 
    else if(start_addr == FC04_REG_TVOC)
      {
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->TVOC); 
      } 
    else if(start_addr == FC04_REG_O3)
      {
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->O3); 
      } 
    else if(start_addr == FC04_REG_PM10)
      {//Trace(FC04_REG_PM10");
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->PM10); 
      }     
    
    else{ TraceErr1("ClientIAQS",total_reg);
        ret_code=FALSE;
        }
    return ret_code;
}


//
//
//
bool ClientUltraSound(PUltraSoundInfo p_info)
{//TraceProc();
    bool ret_code=TRUE;
    uint16 start_addr,total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr); 
    total_reg = WordSwap(pModbusCmd->TotalReg);   

    if(start_addr == FC04_DISTANCE)
      {
        ModbusToHostCmd.Data[0] =  WordSwap(p_info->Distance);
      }
    else if(start_addr == FC04_SET_DISTANCE)
      {
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->SetDistance); 
      }
    else if(start_addr == FC04_DISTANCE100)
      {
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->Distance100); 
      }
    else if(start_addr == FC04_DISTANCE200)
      {//Trace(FC04_REG_PM25");
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->Distance200); 
      }
    else if(start_addr == FC04_DISTANCE300)
      {
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->Distance300); 
      } 
    else if(start_addr == FC04_DISTANCE400)
      {
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->Distance400); 
      } 
    else if(start_addr == FC04_DISTANCE500)
      {
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->Distance500); 
      } 
    else if(start_addr == FC04_DISTANCE600)
      {
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->Distance600); 
      }     
    else if(start_addr == FC04_DISTANCE700)
      {
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->Distance700); 
      }     
    else if(start_addr == FC04_DISTANCE800)
      {
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->Distance800); 
      } 
    else{ TraceErr1("ClientUltraSound",total_reg);
        ret_code=FALSE;
        }
    return ret_code;
}


void ShowAllNodeInfo(void)
{
 #ifdef DEBUG_PRINT
    PClientInfo p_info=ClientInfo;
    uint16 loop;

    for(loop=0; loop<SERVER_NODE_MAX; loop++)
        {
            if(p_info->ServerID) ShowEventInfo(p_info);
            p_info++;
        }
  

 #endif
}

void ShowEventInfo(PClientInfo p_info)
{
#ifdef DEBUG_PRINT
     uint8   sensor_class;
     PSi7021Info    p_info_si7021;
     PPT485Info     p_info_pt485;     
     PAIPInfo       p_info_aip;
     PA308mInfo     p_info_a308m;
     PWaterLevelInfo p_info_water_level;
     PSdInfo p_info_jnc_sd;
     PIaqsInfo p_info_iaqs;


    sensor_class = p_info->SensorInfo.Header.SensorClass;

     switch(sensor_class)
        {
            case SENSOR_SI7021: p_info_si7021 = &(p_info->SensorInfo.Si7021Info);
                break;
            case SENSOR_PT485: p_info_pt485 = &(p_info->SensorInfo.PT485);
                break;
            case SENSOR_AIP: p_info_aip = &(p_info->SensorInfo.AipInfo);
                break;
            case SENSOR_A308M: p_info_a308m = &(p_info->SensorInfo.A308mInfo);
                break;
            case SENSOR_WATER_LEVEL: p_info_water_level = &(p_info->SensorInfo.WaterLevelInfo);
                break;
            case SENSOR_JNC_SD: p_info_jnc_sd = &(p_info->SensorInfo.SdInfo);
                break;  
            case SENSOR_IAQS: p_info_iaqs = &(p_info->SensorInfo.IaqsInfo);
                break;            
            
            default:
        };
        
#endif    
    return;
}





