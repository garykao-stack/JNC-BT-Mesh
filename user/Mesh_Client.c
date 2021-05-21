
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
    //UsartClientProc(); ClientFromHostProc();
    UsartClientProc();  ClientFromHostProc();ClientSetNodeInfoProc(); 
    UsartClientProc();  ClientFromHostProc();ClientGetNodeInfoProc(); 
    UsartClientProc();  ClientFromHostProc();NodeIviUpdateProc(); 
}
uint16 test1;

uint16 rx_ccount;

void ClientNodeTask2()
{
    uchar rx_count = UsartGetRxCounter();
    //SetLedToggle(LED_BLUE);
    //if(UsartGetStatusRxIng() == TRUE)
    //if((rx_count > 1) && (rx_count <= 8))
    if(rx_count >= 7 && GetNodeStatus(NS_USART_RX_EVENT) != TRUE)
        {
         //if(rx_ccount > 100) rx_ccount = 0;   TraceDec2("Rx W",rx_ccount++,rx_count);
         Delay_ms(3);
         UsartClientProc();  UsartClientProc(); UsartClientProc(); UsartClientProc();
         return;
        }

    if(GetNodeStatus(NS_USART_RX_EVENT) == TRUE)
        {//Trace("NS_USART_RX_EVENT ON");
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
         UsartClientProc(); UsartClientProc();
         //return;
        }
    else if(GetNodeStatus(NS_USART_RX_EVENT) == TRUE) 
        {UsartClientProc();ClientFromHostProc();UsartClientProc();ClientFromHostProc(); }
    
    ClientGetNodeInfoProc(); UsartClientProc(); ClientFromHostProc(); 
    ClientSetNodeInfoProc(); //UsartClientProc(); ClientFromHostProc(); 
    NodeIviUpdateProc(); //UsartClientProc(); ClientFromHostProc(); 
    
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
    Printf("Total Server Node = %02d, Get Data Times = %ld\r\n",ret_code,GetEventCount);
   // TraceDec1("Total Server Node", ret_code,GetEventCount);
    return ret_code;
}



//uint16    GetInfoCycle=WAIT_SEC(TIMER_GET_INFO_SLEEPING);


#define TIMER_CLIENT_IVI_WAIT       5   //sec




//#define MODBUS_AIP_POWER_CMD        5

#define GET_MODBUS_FUN_CODE         prx_buff[1]

#define MODBUS_CTRL_REG             prx_buff[3]
#define A6D6_CTRL_POWER             prx_buff[4]
#define AIP_CTRL_POWER              prx_buff[5]

uint16 SetProperityID=NODE_SET_AIP_POWER_OFF;
uint16 SetServerFunID=0;

//
// return properity ID
//
uint16 GetProperityID()
{
    PUCHAR prx_buff = UsartGetBuff(USART_ID_RX);
    uint16 properity=NULL;
    if(GET_MODBUS_FUN_CODE == 0x06)
    {//for AIP
        if(MODBUS_CTRL_REG == 0x02)
        {//Trace("AIP Control");
        if(AIP_CTRL_POWER == MODBUS_AIP_POWER_00) properity = NODE_SET_AIP_POWER_OFF;
        else if(AIP_CTRL_POWER == MODBUS_AIP_POWER_25) properity = NODE_SET_AIP_POWER_25;
        else if(AIP_CTRL_POWER == MODBUS_AIP_POWER_50) properity = NODE_SET_AIP_POWER_50;
        else if(AIP_CTRL_POWER == MODBUS_AIP_POWER_75) properity = NODE_SET_AIP_POWER_75;
        else if(AIP_CTRL_POWER == MODBUS_AIP_POWER_100) properity = NODE_SET_AIP_POWER_100;
        }
        else TraceErr1("Fun Code 5 GetProperityID",MODBUS_CTRL_REG);
    }
    else if(GET_MODBUS_FUN_CODE == 0x05)
    {//for A6D6
           if(A6D6_CTRL_POWER == 0xFF) 
            {//Trace1("A6D6 Set ON", MODBUS_CTRL_REG);
             properity = (NODE_SET_DO1_ON+(uint16)MODBUS_CTRL_REG); 
            }
           else 
            {//Trace1("A6D6 Set OFF", MODBUS_CTRL_REG);
             properity = (NODE_SET_DO1_OFF+(uint16)MODBUS_CTRL_REG); 
            }

           Trace16_1(properity);
    }
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
            case CNS_SET_WAITING:         
                if(GetNodeStatus(NS_SET_NODE_ACT) == TRUE)  ToNextStage(CNS_SET_INFO_PRE);
                break;
                
            case CNS_SET_INFO_PRE:  
                //PrintRx(CNS_SET_INFO_PRE);
                prx_buff = UsartGetBuff(USART_ID_RX);
                SetServerFunID = (uint16)prx_buff[0]; // get id number
                SetProperityID = GetProperityID();
                if(SetProperityID != NULL ) ToNextStage(CNS_SET_INFO_SEND);
                else ToNextStage(CNS_SET_INFO_ERR);
                break;
                
            case CNS_SET_INFO_SEND:
               
                result = Cmd_ms_client_get(SENSOR_ELEMENT, SetServerFunID, IGNORED, 0xA5, SetProperityID)->result;
                if(result)
                    { TraceErr("CNS_SET_INFO_SEND");
                     ToWaitingStage(CNS_SET_INFO_PREEAT,50);
                    }
                else 
                    ToNextStage(CNS_SET_INFO_OK);
                break;
            case CNS_SET_INFO_PREEAT:
                if(!CheckWaitTimeOut()) break;
                result = Cmd_ms_client_get(SENSOR_ELEMENT, SetServerFunID, IGNORED, 0xA5, SetProperityID)->result;
                if(result)
                    { TraceErr("CNS_SET_INFO_PREEAT");
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
                
            case CNS_SET_INFO_ERR:  Trace("CNS_SET_INFO_ERR");
                ToNextStage(CNS_SET_INFO_END);
                break;
            default: TraceErr1("ClientSetNodeInfoProc",ActiveStage()); break;
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
            case CNS_PRE_IVI_UPDATE: 
                if(CheckWaitTimeOut())
                    {//Trace("CNS_PRE_IVI_UPDATE 2");
                     SetNodeStatus(NS_IVI_UPDATE,ON);   // enable ivi update
                     CountErr = 0;
                     ToWaitingStage(CNS_IVI_UPDATE,WAIT_SEC(TIMER_CLIENT_IVI_WAIT));
                    }
                break;
            
            case CNS_IVI_UPDATE: 
                if(CheckWaitTimeOut() == TRUE) 
                  {TraceErr1("--- IVI time-out 1---",CountErr);
                    if(CountErr++ > 3)
                      {TraceErr1("--- IVI time-out to Get info 2---",CountErr);
                        ToNextStage(CNS_WAIT_SET_INFO);   
                      }
                    else {ToWaitingStage(CNS_IVI_UPDATE,WAIT_SEC(TIMER_CLIENT_IVI_WAIT));} // waiting again
                  }
               if(GetNodeStatus(NS_IVI_UPDATE) == OFF)
                  {TraceOk("--- IVI Update Ok ---");
                   ToNextStage(CNS_WAIT_SET_INFO); 
                  }
                break;
            case CNS_PRE_SEVER_INFO: 
                SetLed(LED_SERVER,OFF); 
                SetNodeStatus(NS_GET_INFO_ACT,OFF);
                if(MeshCheckSeqNum())
                    {//Trace(" SEQ Upata OFF"); 
                     ShowCurrRemSeq();
                    if(RespServerNode == 0) {ToWaitingStage(CNS_WAIT_SET_INFO,100);}
                    else {ToWaitingStage(CNS_WAIT_SET_INFO,GetInfoCycle);}
                    } 
                else{//Trace(" SEQ Updata ON");
                     ToWaitingStage(CNS_PRE_IVI_UPDATE,GetInfoCycle);                   
                    } 
                break;
            case CNS_WAIT_SET_INFO:
                if(CheckWaitTimeOut())ToNextStage(CNS_GET_SEVER_INFO);
                else if(GetNodeStatus(NS_SET_NODE_ACT) != TRUE)  ToNextStage(CNS_GET_SEVER_INFO);
                
                break;
            case CNS_GET_SEVER_INFO: 
                if(CountErr > 3) ToNextStage(CNS_GET_INFO_ERR); // err: go to sleeping

                if(CheckWaitTimeOut()) 
                  {//to get info from all server node
                    GetEventCount++;
                    SetLed(LED_SERVER,ON);
                    result = Cmd_ms_client_get(SENSOR_ELEMENT, ActServerAddr, IGNORED, 0xA5, PropertyID)->result;
                    if(result)
                        {
                          CountErr++;
                          ToWaitingStage(CNS_GET_SEVER_INFO,WAIT_SEC(2));  
                        }
                    else{//TraceOk("Get Info");
                          CountErr = 0; RespServerNode = 0;
                          ToWaitingStage(CNS_WAIT_INFO,TIMER_CLI_WAIT_INFO);
                        }
                  }
                 else
                 {
                    if(GetNodeStatus(NS_GET_INFO_ACT))
                        { //Trace("Event Action 1");
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

                GetServerNodeNum();
                
                ToNextStage(CNS_PRE_SEVER_INFO); 
                break;
            default: TraceErr1("ClientGetNodeInfoProc",ActiveStage()); break;
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
            {//TraceErr1("Node No Response",pNodeInfo->ServerID);
             memset(pNodeInfo,0,sizeof(_ClientInfo));
             pNodeInfo->Status |= SERVER_NO_RESPONSE;
            } 
            else 
            {
              if(pNodeInfo->Count) pNodeInfo->Count--;
              if((pNodeInfo->SensorInfo.Header.Status & SERVER_FULL_POWER) == 0)
                {
                 power_flag = OFF;
                }
            }
                
          }  
        pNodeInfo++;
       }
    
    if(power_flag == OFF) 
        {//Trace("NS_FULL_POWER: OFF");
        SetNodeStatus(NS_FULL_POWER,OFF);SetLed(LED_GREEN,OFF);}
    else 
        {//Trace("NS_FULL_POWER: ON");
        SetNodeStatus(NS_FULL_POWER,ON);SetLed(LED_GREEN,ON);}
}

//
//
//
bool SendModbusToHost()
{
    bool ret_code=TRUE;
   // PrintDataByte("SendModbusToHost", (PUCHAR)&ModbusToHostCmd,ModbusToHostCmd.ByteNum+5);
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
            else if(pModbusCmd->FunCode == 0x06 || pModbusCmd->FunCode == 0x05)
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
        case CHS_CHECK_RX_EVENT: //Trace(("CHS_CHECK_RX_EVENT");
            if(GetNodeStatus(NS_USART_RX_EVENT) == TRUE) 
                ToNextStage(CHS_CHECK_MODBUS); //ToNextStage(CHS_PREPARE_DATA);   //default
            break;
        
        case CHS_CHECK_MODBUS: 
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
            
        default: //TraceErr1("ClientFromHostProc",ActiveStage()); 
        break;
        };
    
}


void ClientPropertyEvent(msg_ms_client_status_evt *pEvent)
{
    uint8_t *p_sensor_data = pEvent->sensor_data.data;
    uint8_t data_len = pEvent->sensor_data.len;
    uint8_t pos = 0;
    mesh_device_properties_t property_id;
    //PClientNodeInfo p_node_info;
    if((pClientInfo = GetServerInfoPos(pEvent->server_address))== NULL) 
           {TraceErr("ClientPropertyEvent 1");return;}
    TraceDec1("Node ID", pEvent->server_address);
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
                    case NODE_GET_ALL_SENSOR: //Trace(("NODE_GET_ALL_SENSOR");
                        memcpy(&(pClientInfo->SensorInfo),property_data,property_len);
                        break;
                    default: //TraceErr1("ClientPropertyEvent 5",property_id);
                };
               pos += PROPERTY_HEADER_SIZE + property_len;
            }
           else 
            pos = data_len;
               
        };
    pClientInfo->Count = COUNT_NODE_DETECTED;

}


void PropertyLcd()
{// Tools
#ifdef SILICON_EVA_BOARD

    uchar loop;    
    char tmp[21];
    PClientInfo p_node_info;
    PSi7021Info pSi7021Info;
    PSdInfo     pSdInfo;

    for(loop=0; loop<8; loop++)
       {
        p_node_info = GetServerInfoPos(loop+3);
        if(p_node_info)
            {
            pSi7021Info = &(p_node_info->SensorInfo.Si7021Info);
            snprintf(tmp, 21, "Adr%2d %3u %2.1f %2.1f", p_node_info->ServerID, p_node_info->SensorInfo.Header.BatteryPower,
                          ((float)pSi7021Info->Tempature)/10,((float)pSi7021Info->Humidity)/10);
            }
        
        /*
        snprintf(tmp, 21, "Adr%2x %3u %3u ", p_node_info->ServerID, p_node_info->SensorInfo.Header.BatteryPower,
                        p_node_info->SensorInfo.Header.BatteryPower);
        */
        p_node_info++;
        DI_Print(tmp, 1+loop);
       }

    if((p_node_info = GetServerInfoPos(18)) != NULL)
        {
         pSdInfo = &(p_node_info->SensorInfo.SdInfo);
         
         snprintf(tmp, 21, "Adr%2d %4d %2.1f %2.1f", p_node_info->ServerID, pSdInfo->CO2,\
                          ((float)pSdInfo->Tempature)/10,((float)pSdInfo->Humidity)/10);    
         DI_Print(tmp, 1+loop);
         
         snprintf(tmp, 21, "Adr%2d %3u %4d %2.1f", p_node_info->ServerID, p_node_info->SensorInfo.Header.BatteryPower,\
                          pSdInfo->CO2,((float)pSdInfo->PM25)/10);
         DI_Print(tmp, 2+loop);
        }
    
#endif
    
}


//
//
//
bool ClientPrepareToHost()
{
    bool    ret_code=FALSE;
    uint8   sensor_class;
    PUINT16  p_modbus_crc;
    uint16  modbus_crc_bytes,modbus_crc;
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
//            case SENSOR_IAQS:     ret_code = ClientIAQS(&pClientInfo->SensorInfo.IaqsInfo);
//                break;           
            case SENSOR_ULTRA_SOUND: ret_code = ClientUltraSound(&pClientInfo->SensorInfo.UltraSound);
                break;
            case SENSOR_DO_485:     ret_code = ClientJncDo485(&pClientInfo->SensorInfo.JncDo485);
                break;
            case SENSOR_A6D6:       ret_code = ClientA6D6(&pClientInfo->SensorInfo.A6D6);
                break;             
            case SENSOR_PZEM:       ret_code = ClientPzem(&pClientInfo->SensorInfo.Pzem);
                break;   
            case SENSOR_RELAY:      ret_code = ClientRelay(&pClientInfo->SensorInfo.RelayNode);
                break;             
            case OTHER_MODBUS_CMD:  ret_code = ClientOtherModbusCmd();
                break;
            default:           
                ret_code=FALSE;
                break;
        }

    if(ret_code == TRUE)
        {
         modbus_crc = ModbusRtu_CRC16((PUCHAR)&ModbusToHostCmd,ModbusToHostCmd.ByteNum+3);
         p_modbus_crc = (PUINT16)(((PUCHAR)&(ModbusToHostCmd.ByteNum)) + ModbusToHostCmd.ByteNum+1);
         *p_modbus_crc = modbus_crc;
         
        }
    else TraceErr("ClientPrepareToHost 2");

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
{
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
    else{ //TraceErr1("ClientSi7021",total_reg);
        ret_code=FALSE;
        }


    return ret_code;
}

//
//
//
bool ClientPT485(PPT485Info p_info)
{
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
    
    else{ //TraceErr1("ClientPT485",total_reg);
        ret_code=FALSE;
        }


    return ret_code;
}

//
//
//
bool ClientAIP(PAIPInfo p_info)
{
    bool ret_code=TRUE;
    uint16 start_addr,total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr); 
    total_reg = WordSwap(pModbusCmd->TotalReg);   

    if(start_addr == AIP_POWER)
      {
        ModbusToHostCmd.Data[0] =  WordSwap(p_info->AipPower);
      }
    else if(start_addr == AIP_POWER_STATUS)
      {
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->AipPowerStatus); 
      }
    else if(start_addr == AIP_POWER_STATUS_VALUE)
      {

       if(p_info->AipPowerStatus == 0)      ModbusToHostCmd.Data[0] = 0x0000;  //0%
       else if(p_info->AipPowerStatus == 1) ModbusToHostCmd.Data[0] = 0x1900;   //25%
       else if(p_info->AipPowerStatus == 2) ModbusToHostCmd.Data[0] = 0x3200;   //50%
       else if(p_info->AipPowerStatus == 3) ModbusToHostCmd.Data[0] = 0x4B00;   //75%
       else if(p_info->AipPowerStatus == 4) ModbusToHostCmd.Data[0] = 0x6400;   //100%
       else TraceErr("AIP Power Value");
      }
    
    else if(start_addr == BATTERY_POWER )
      {
        ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower));
      }
    else{ 
        ret_code=FALSE;
        }


    return ret_code;
}

#ifdef JNC_A308M

//
//
//
bool ClientA308m(PA308mInfo p_info)
{TraceProc();
    bool ret_code=TRUE;
    uint16 start_addr,total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr); 
    total_reg = WordSwap(pModbusCmd->TotalReg);

    switch(start_addr)
        {
            case BT_RMS_X:
            case RMS_X:     ModbusToHostCmd.Data[0] = WordSwap(p_info->RmsX);
                break;
            case BT_SKEWNESS_X:
            case SKEWNESS_X: ModbusToHostCmd.Data[0] = WordSwap(p_info->SkewnessX);
                break;
            case BT_KURTOSIS_X:
            case KURTOSIS_X: ModbusToHostCmd.Data[0] = WordSwap(p_info->KurtosisX);
                break;
            case BT_FREQUENCY_X:
            case FREQUENCY_X: ModbusToHostCmd.Data[0] = WordSwap(p_info->FrequencyX);
                break;
            case BT_SPEED_X:
            case SPEED_X: ModbusToHostCmd.Data[0] = WordSwap(p_info->SpeedX);
                break;
            case BT_STRENGTH_X:
            case STRENGTH_X: ModbusToHostCmd.Data[0] = WordSwap(p_info->StengthX);
                break;

            case BT_RMS_Y:
            case RMS_Y:     ModbusToHostCmd.Data[0] = WordSwap(p_info->RmsY);
                break;
            case BT_SKEWNESS_Y:
            case SKEWNESS_Y: ModbusToHostCmd.Data[0] = WordSwap(p_info->SkewnessY);
                break;

            case BT_KURTOSIS_Y:
            case KURTOSIS_Y: ModbusToHostCmd.Data[0] = WordSwap(p_info->KurtosisY);
                break;
            case BT_FREQUENCY_Y:
            case FREQUENCY_Y: ModbusToHostCmd.Data[0] = WordSwap(p_info->FrequencyY);
                break;
            case BT_SPEED_Y:
            case SPEED_Y: ModbusToHostCmd.Data[0] = WordSwap(p_info->SpeedY);
                break;
            case BT_STRENGTH_Y:
            case STRENGTH_Y: ModbusToHostCmd.Data[0] = WordSwap(p_info->StengthY);
                break;


            case BT_RMS_Z:
            case RMS_Z:     ModbusToHostCmd.Data[0] = WordSwap(p_info->RmsZ);
                break;
            case BT_SKEWNESS_Z:
            case SKEWNESS_Z: ModbusToHostCmd.Data[0] = WordSwap(p_info->SkewnessZ);
                break;
            case BT_KURTOSIS_Z:
            case KURTOSIS_Z: ModbusToHostCmd.Data[0] = WordSwap(p_info->KurtosisZ);
                break;
            case BT_FREQUENCY_Z:
            case FREQUENCY_Z: ModbusToHostCmd.Data[0] = WordSwap(p_info->FrequencyZ);
                break;
            case BT_SPEED_Z:
            case SPEED_Z: ModbusToHostCmd.Data[0] = WordSwap(p_info->SpeedZ);
                break;
            case BT_STRENGTH_Z:
            case STRENGTH_Z: ModbusToHostCmd.Data[0] = WordSwap(p_info->StengthZ);
                break;


            case BT_A308M_TEMP:
            case A308M_TEMP:    ModbusToHostCmd.Data[0] = WordSwap(p_info->Tempature);
                break;
            case BATTERY_POWER: ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower));
                break;
            default: ret_code=FALSE; Trace1("ClientA308m:start_addr",start_addr);break;
        };

    return ret_code;
}

#else
//
//
//
bool ClientA308m(PA308mInfo p_info)
{
    bool ret_code=TRUE; 
    uint16 start_addr,total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr); 
    total_reg = WordSwap(pModbusCmd->TotalReg);

    switch(start_addr)
        {
            case RMS_X:    ModbusToHostCmd.Data[0] = WordSwap(p_info->RmsX);
                break;
            case SPEED_X:  ModbusToHostCmd.Data[0] = WordSwap(p_info->SpeedX);
                break;
            case RMS_Y:    ModbusToHostCmd.Data[0] = WordSwap(p_info->RmsY);
                break;
            case SPEED_Y:  ModbusToHostCmd.Data[0] = WordSwap(p_info->SpeedY);
                break;
            case RMS_Z:    ModbusToHostCmd.Data[0] = WordSwap(p_info->RmsZ);
                break;
            case SPEED_Z:  ModbusToHostCmd.Data[0] = WordSwap(p_info->SpeedZ);
                break;
            case A308M_TEMP:    ModbusToHostCmd.Data[0] = WordSwap(p_info->Tempature);
                break;
            case FREQUENCY_X:   ModbusToHostCmd.Data[0] = WordSwap(p_info->FrequencyX);
                break;
            case STRENGTH_X:    ModbusToHostCmd.Data[0] = WordSwap(p_info->StengthX);
                break;
            case FREQUENCY_Y:   ModbusToHostCmd.Data[0] = WordSwap(p_info->FrequencyY);
                break;
            case STRENGTH_Y:    ModbusToHostCmd.Data[0] = WordSwap(p_info->StengthY);
                break;
            case FREQUENCY_Z:   ModbusToHostCmd.Data[0] = WordSwap(p_info->FrequencyZ);
                break;
            case STRENGTH_Z:    ModbusToHostCmd.Data[0] = WordSwap(p_info->StengthZ);
                break;
            case BATTERY_POWER: ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower));
                break;
            default: ret_code=FALSE; Trace1("ClientA308m:start_addr",start_addr);break;
        };

    return ret_code;
}

#endif

//
//
//
bool ClientWaterLevel(PWaterLevelInfo p_info)
{
    bool ret_code=TRUE;
    uint16 start_addr,total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr); 
    total_reg = WordSwap(pModbusCmd->TotalReg);

    
    if(start_addr == POSITION_WATER)
      {//TraceDec1("Water Position",p_info->WaterLevel);
        ModbusToHostCmd.Data[0] =  WordSwap(p_info->WaterLevel);
      }
    else if(start_addr == POSITION_OIL)
      {//Trace(("AIP Power Status");
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->OilLevel); 
      }
    else if(start_addr == BATTERY_POWER )
      {
        ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower));
      }
    
    else{//TraceErr1("ClientWaterLevel 1",total_reg);  UsartPrintBuff(USART_ID_RX);
        ret_code=FALSE;
        }


    return ret_code;
}

//
//
//
bool ClientJncSd(PSdInfo p_info)
{
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
      {
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->Humidity); 
      }
    else if(start_addr == BATTERY_POWER )
      {
        ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower));
      }
    
    else{ //TraceErr1("ClientJncSd",total_reg);
        ret_code=FALSE;
        }
    return ret_code;
}

//
//
bool ClientUltraSound(PUltraSoundInfo p_info)
{
    bool ret_code=TRUE;
    uint16 start_addr,total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr); 
    total_reg = WordSwap(pModbusCmd->TotalReg);

    
    if(start_addr == POSITION_WATER)
      {//TraceDec1("Water Position",p_info->WaterLevel);
        ModbusToHostCmd.Data[0] =  WordSwap(p_info->WaterLevel);
      }
    else if(start_addr == BATTERY_POWER )
      {
        ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower));
      }
    
    else{ //TraceErr1("ClientUltraSound 1",total_reg);  UsartPrintBuff(USART_ID_RX);
        ret_code=FALSE;
        }


    return ret_code;
}

bool ClientJncDo485(PJncDo485 p_info)
{
    bool ret_code=TRUE;
    uint16 start_addr,total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr); 
    total_reg = WordSwap(pModbusCmd->TotalReg);

    
    if(start_addr == DO485_REAL_VALUE)
      {//TraceDec1("Water Position",p_info->WaterLevel);
        ModbusToHostCmd.Data[0] =  WordSwap(p_info->DoRealValue);
      }
    else if(start_addr == DO485_REAL_OFFSET)
    {//TraceDec1("Water Position",p_info->WaterLevel);
      ModbusToHostCmd.Data[0] =  WordSwap(p_info->DoOffsetValue);
    }
    else if(start_addr == DO485_TEMP_VALUE)
    {//TraceDec1("Water Position",p_info->WaterLevel);
      ModbusToHostCmd.Data[0] =  WordSwap(p_info->TempRealValue);
    }
    else if(start_addr == DO485_TEMP_OFFSET)
    {//TraceDec1("Water Position",p_info->WaterLevel);
      ModbusToHostCmd.Data[0] =  WordSwap(p_info->TempOffsetValue);
    }    
    else if(start_addr == BATTERY_POWER )
      {
        ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower));
      }
    
    else{ //TraceErr1("ClientJncDo485 1",total_reg);  UsartPrintBuff(USART_ID_RX);
        ret_code=FALSE;
        }


    return ret_code;
}

bool ClientA6D6(PA6D6 p_info)
{TraceProc();
    bool ret_code=TRUE;
    uint16 start_addr,total_reg,*p_ai_value,*p_modbus_value;
    uint16 loop;
    uchar  bit_mask=0x01,status_mask=0x01,status_bit=0x00,di_status=0,do_status=0; 
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 0;
    start_addr  = WordSwap(pModbusCmd->StartAddr); 
    total_reg = WordSwap(pModbusCmd->TotalReg);

    if(start_addr > 7) start_addr = 7;
    if(total_reg > 8) total_reg = 8;

    //PrintDataByte("ClientA6D6", (PUCHAR)pModbusCmd, 8);
    loop = total_reg;
    
    if(ModbusToHostCmd.FunCode == 0x04)
      {
        p_ai_value = &(p_info->AiValue1);
        p_modbus_value = &ModbusToHostCmd.Data[0];
        for(loop=start_addr; loop<8 && (total_reg--!=0)  ; loop++)
            {
             *p_modbus_value = WordSwap(*(p_ai_value+loop));
             p_modbus_value++; ModbusToHostCmd.ByteNum += 2;
            }
        //TraceDec1("A6D6 Get Ai Value",*p_ai_value);
      }
    else if(ModbusToHostCmd.FunCode == 0x02)
      {//TraceDec2("A6D6 Get Di Value",start_addr,total_reg);        
        ModbusToHostCmd.ByteNum = 1;
        bit_mask = bit_mask << start_addr;
        for(loop=start_addr; loop<8 && (total_reg--!=0)  ; loop++)
            {
              if(p_info->Di_Status & bit_mask) {status_bit |= status_mask;}
              bit_mask<<=1; status_mask<<=1;
              //Trace8_3(loop,p_info->Di_Status,status_bit);
            }

         *((PUCHAR)(&(ModbusToHostCmd.Data[0]))) = status_bit;
      }
    else if(ModbusToHostCmd.FunCode == 0x01)
      {//TraceDec2("A6D6 Get DO Value",start_addr,total_reg);
        ModbusToHostCmd.ByteNum = 1;
        bit_mask = bit_mask << start_addr;
        for(loop=start_addr; loop<6 && (total_reg--!=0)  ; loop++)
            {
              if(p_info->DO_Status & bit_mask)  {status_bit |= status_mask;}
              bit_mask<<=1; status_mask<<=1;
              //Trace8_3(loop,p_info->Di_Status,status_bit);
            }
         *((PUCHAR)(&(ModbusToHostCmd.Data[0]))) = status_bit;
      }
    else{ret_code = FALSE;}
    return ret_code;
}

bool ClientPzem(PPzem p_info)
{
    bool ret_code=TRUE;
    uint16 start_addr,total_reg,*p_ai_value,*p_modbus_value;
    uint16 loop;
    uchar  bit_mask=0x01,status_mask=0x01,status_bit=0x00; 
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 0;
    start_addr  = WordSwap(pModbusCmd->StartAddr); 
    total_reg = WordSwap(pModbusCmd->TotalReg);

    if(start_addr > PZEM_ITEM_SIZE-1) start_addr = PZEM_ITEM_SIZE-1;
    if(total_reg > PZEM_ITEM_SIZE) total_reg = PZEM_ITEM_SIZE;
    loop = total_reg;
    
    if(ModbusToHostCmd.FunCode == 0x04)
      {
        p_ai_value = &(p_info->Voltage);
        p_modbus_value = &ModbusToHostCmd.Data[0];
        for(loop=start_addr; loop<PZEM_ITEM_SIZE && (total_reg--!=0)  ; loop++)
            {
             *p_modbus_value = WordSwap(*(p_ai_value+loop));
             p_modbus_value++; ModbusToHostCmd.ByteNum += 2;
            }
      }
    else{TraceErr("ClientPzem"); ret_code = FALSE;}
    return ret_code;
}

//
//
//
bool ClientRelay(PRelayNode p_info)
{
    bool ret_code=TRUE;
    uint16 start_addr,total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr); // Trace16_1(start_addr);
    total_reg = WordSwap(pModbusCmd->TotalReg);    // Trace16_1(total_reg);
    Trace16_1(p_info->Status);
    if(start_addr == BATTERY_POWER )
      {
        ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower));
      }
    else
      { //TraceErr1("ClientRelay",total_reg);
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
     PUltraSoundInfo p_info_ultra_sound;
     PJncDo485 p_jnc_Do485;     
     PA6D6 p_A6D6;
     PPzem p_pzem;

    sensor_class = p_info->SensorInfo.Header.SensorClass;
    Printf("\r\nID=%02d Class=%d Lose=%02d Power=%03d => ",p_info->ServerID,p_info->SensorInfo.Header.SensorClass, 
                                                p_info->Count,p_info->SensorInfo.Header.BatteryPower);  

     switch(sensor_class)
        {
            case SENSOR_SI7021: p_info_si7021 = &(p_info->SensorInfo.Si7021Info);
                Printf("BTM Temp = %03d, RH= %02d\r\n",p_info_si7021->Tempature ,p_info_si7021->Humidity);
                break;
            case SENSOR_PT485: p_info_pt485 = &(p_info->SensorInfo.PT485);
                Printf("PT485 Temp= %d\r\n",p_info_pt485->Tempature);
                break;
            case SENSOR_AIP: p_info_aip = &(p_info->SensorInfo.AipInfo);
                Printf("AIP Power= %d Status=%X\r\n",p_info_aip->AipPower,p_info_aip->AipPowerStatus);
                break;
#ifdef JNC_A308M
            case SENSOR_A308M_JNC: p_info_a308m = &(p_info->SensorInfo.A308mInfo);
            /*
                Printf("\r\nA308_JNC Temp=%d XRMS=%d SkewX=%d\r\n",p_info_a308m->Tempature,p_info_a308m->RmsX,p_info_a308m->SkewnessX );
                Printf("YRMS=%d SkewY=%d ZRMS=%d SkewZ=%d \r\nFrequencyX=%d KurtX=%d\r\nFrequencyY=%d KurtY=%d\r\nFrequencyZ=%d KurtZ=%d\r\n\r\n",
                        p_info_a308m->RmsY,p_info_a308m->SkewnessY,p_info_a308m->RmsZ,p_info_a308m->SkewnessZ,
                        p_info_a308m->FrequencyX,p_info_a308m->KurtosisX,
                        p_info_a308m->FrequencyY,p_info_a308m->KurtosisY,
                        p_info_a308m->FrequencyZ,p_info_a308m->KurtosisZ
                        );
             */

                Printf("\r\nA308M=>X RMS=%ld Skew=%ld Kurt=%ld Freq=%ld Speed=%ld Stength=%ld\r\n",
                        p_info_a308m->RmsX,p_info_a308m->SkewnessX,p_info_a308m->KurtosisX,p_info_a308m->FrequencyX,
                        p_info_a308m->SpeedX,p_info_a308m->StengthX);
   
                Printf("A308M=>Y RMS=%ld Skew=%ld Kurt=%ld Freq=%ld Speed=%ld Stength=%ld\r\n",
                        p_info_a308m->RmsY,p_info_a308m->SkewnessY,p_info_a308m->KurtosisY,p_info_a308m->FrequencyY,
                        p_info_a308m->SpeedY,p_info_a308m->StengthY);
    
                Printf("A308M=>Z RMS=%ld Skew=%ld Kurt=%ld Freq=%ld Speed=%ld Stength=%ld\r\n",
                        p_info_a308m->RmsZ,p_info_a308m->SkewnessZ,p_info_a308m->KurtosisZ,p_info_a308m->FrequencyZ,
                        p_info_a308m->SpeedZ,p_info_a308m->StengthZ);                        
                break;

            
#else             
            case SENSOR_A308M: p_info_a308m = &(p_info->SensorInfo.A308mInfo);
                Printf("\r\nA308_1 Temp=%d XRMS=%d SpeedX=%d\r\n",p_info_a308m->Tempature,p_info_a308m->RmsX,p_info_a308m->SpeedX );
                Printf("      YRMS=%d SpeedY=%d ZRMS=%d SpeedZ=%d FrequencyX=%d StengthX=%d\r\n \
                        FrequencyY=%d StengthY=%d FrequencyZ=%d StengthZ=%d\r\n",
                        p_info_a308m->RmsY,p_info_a308m->SpeedY,p_info_a308m->RmsZ,p_info_a308m->SpeedZ,
                        p_info_a308m->FrequencyX,p_info_a308m->StengthX,
                        p_info_a308m->FrequencyY,p_info_a308m->StengthY,
                        p_info_a308m->FrequencyZ,p_info_a308m->StengthZ
                        );
                break;

#endif                
            case SENSOR_WATER_LEVEL: p_info_water_level = &(p_info->SensorInfo.WaterLevelInfo);
                Printf("Water = %dcm, Oil= %dcm\r\n",p_info_water_level->WaterLevel,p_info_water_level->OilLevel);
                break;
            case SENSOR_JNC_SD: p_info_jnc_sd = &(p_info->SensorInfo.SdInfo);
                Printf("CO2 = %d, PM2.5= %d Temp=%d RH=%d\r\n",p_info_jnc_sd->CO2,p_info_jnc_sd->PM25,p_info_jnc_sd->Tempature,p_info_jnc_sd->Humidity);
                break;  
            case SENSOR_IAQS: p_info_iaqs = &(p_info->SensorInfo.IaqsInfo);
                break;   
            case SENSOR_ULTRA_SOUND: p_info_ultra_sound = &(p_info->SensorInfo.UltraSound);
                Printf("UltraSound Water = %dcm, \r\n",p_info_ultra_sound->WaterLevel);
                break;            
            case SENSOR_DO_485: p_jnc_Do485 = &(p_info->SensorInfo.JncDo485);
                Printf("Do485: Real=%d Offset=%d Temp=%d Temp_Offset=%d  \r\n",p_jnc_Do485->DoRealValue,p_jnc_Do485->DoOffsetValue,p_jnc_Do485->TempRealValue,
                    p_jnc_Do485->TempOffsetValue);
                break;
            case SENSOR_A6D6: p_A6D6 = &(p_info->SensorInfo.A6D6);
                Printf("A6D6: AiValue1=%x AiValue2=%x AiValue3=%x AiValue4=%x  \r\n",p_A6D6->AiValue1,p_A6D6->AiValue2,p_A6D6->AiValue3,
                    p_A6D6->AiValue4);
                Printf("A6D6: AiValue5=%x AiValue6=%x AiValue7=%x AiValue8=%x  \r\n",p_A6D6->AiValue5,p_A6D6->AiValue6,p_A6D6->AiValue7,
                    p_A6D6->AiValue8);
                break;            
            case SENSOR_PZEM: p_pzem = &(p_info->SensorInfo.Pzem);
                Printf("\r\nPZEM: Vol=%04d CurLo=%x PowerLo=%x ElectLo=%x Freq=%x Warning=%x\r\n",p_pzem->Voltage,p_pzem->CurrentLo,p_pzem->PowerLo,
                    p_pzem->ElectLo,p_pzem->Frequecny,p_pzem->Warning);
                break;            
            case SENSOR_RELAY: 
                Printf("\r\Relay: Power=%x\r\n",p_info->SensorInfo.Header.BatteryPower);
               
                break;            
                
            default: TraceErr2("ShowEventInfo",sensor_class,p_info->ServerID);
        };
        
#endif    
    return;
}





