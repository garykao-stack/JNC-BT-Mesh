
#include "global.h"

//richard Add
/* BG stack headers */
#include "sensor_server.h"
#include "mesh_event.h"
#include "bus_usart.h"
#include "bus_rs485.h"
#include "bus_I2C.h"
#include "jnc_cmd.h"
#include "com_port.h"
#include "ivi_features.h"

#include "people_count_sensor.h"
#include "mesh_sensor.h"
#include "Mesh_Node.h"
#include "Mesh_Client.h"
#include "Mesh_Server.h"

#define TO_CLIENT_BUFF_MAX  32
uchar   ToClientBuf[TO_CLIENT_BUFF_MAX];
uchar   ToClientLen;
uint16  GetDeviceInfoDelay=5;     //Nx10ms ==> 50ms


_ServerInfo ServerInfo;
PServerInfo pServerInfo = &ServerInfo;
PNodeHeader pSensorHeader = &ServerInfo.SensorInfo.Header;

//
//
//
void ServerNodeInit()
{
    NodeRole = NR_SERVER;
    UsartRxCount = 7;   //from RS485 device bytes (default)
    ToNextStage(SNS_WAKE_UP);
    Rs485Tx();
    CheckRs485Device();
    if(CheckPowerStatus() == POWER_USB) 
      {
        SetNodeStatus(NS_FULL_POWER,ON);   // start to get sensor info 
        pSensorHeader->Status |= SERVER_FULL_POWER;
      }

    if(pSensorHeader->Status & SERVER_FULL_POWER) Trace("Full Power");
    else Trace("Battery Power"); 
}

void ServerSetupNodeInit()
{
 Rs485Tx();
}

//
// For Server Node Task
//  10ms for one time
void ServerNodeTask()
{
  if(GetNodeStatus(NS_SERVER_RS485_ENABLE))
    {
     ServerSetNodeProc();
    }
  ServerGetInfoProc(); 
}


#define TIMER_WAIT_SEND_INFO        WAIT_MS(TIMER_SERVER_SENS_INFO) 
#define TIMER_WAIT_SLEEPING         WAIT_SEC(3)     //WAIT_MS(pMeshNodeData->MeshNodeID*3)
#define TIMER_GET_SENSOR_INFO       WAIT_MS(300)


const uchar AipPowerCtrlCmd[5][8]=
{
    {0x01, 0x06, 0x00, 0x02, 0x00, 0x00, 0x28, 0x0A},   // 00%
    {0x01, 0x06, 0x00, 0x02, 0x00, 0x19, 0xE9, 0xC0},   // 25%
    {0x01, 0x06, 0x00, 0x02, 0x00, 0x32, 0xA9, 0xDF},   // 50%
    {0x01, 0x06, 0x00, 0x02, 0x00, 0x4B, 0x68, 0x3D},   // 75%
    {0x01, 0x06, 0x00, 0x02, 0x00, 0x64, 0x29, 0xE1},   // 100%
};

const uchar A308MCtrlCmd[4][8]=
{
    {0x01, 0x06, 0x00, 0x20, 0x00, 0x00, 0x88, 0x00},   // Write New bias
    {0x01, 0x06, 0x00, 0x0C, 0x00, 0x00, 0x49, 0xC9},   // Reset X bias
    {0x01, 0x06, 0x00, 0x0C, 0x00, 0x01, 0x88, 0x09},   // Reset y bias
    {0x01, 0x06, 0x00, 0x0C, 0x00, 0x02, 0xC8, 0x08},   // Reset z bias
};


const uchar A6D6CtrlCmdOn[6][8]=
{
    {0x01, 0x05, 0x00, 0x00, 0xFF, 0x00, 0x8C, 0x3A}, //D1 ON
    {0x01, 0x05, 0x00, 0x01, 0xFF, 0x00, 0xDD, 0xFA}, //D12 ON
    {0x01, 0x05, 0x00, 0x02, 0xFF, 0x00, 0x2D, 0xFA}, //D13 ON
    {0x01, 0x05, 0x00, 0x03, 0xFF, 0x00, 0x7C, 0x3A}, //D14 ON
    {0x01, 0x05, 0x00, 0x04, 0xFF, 0x00, 0xCD, 0xFB}, //D15 ON
    {0x01, 0x05, 0x00, 0x05, 0xFF, 0x00, 0x9C, 0x3B}, //D16 ON
};
                                                    
const uchar A6D6CtrlCmdOff[6][8]=
{
    {0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0xCD, 0xCA}, //D1 OFF
    {0x01, 0x05, 0x00, 0x01, 0x00, 0x00, 0x9C, 0x0A}, //D2 OFF
    {0x01, 0x05, 0x00, 0x02, 0x00, 0x00, 0x6C, 0x0A}, //D3 OFF
    {0x01, 0x05, 0x00, 0x03, 0x00, 0x00, 0x3D, 0xCA}, //D4 OFF
    {0x01, 0x05, 0x00, 0x04, 0x00, 0x00, 0x8C, 0x0B}, //D5 OFF
    {0x01, 0x05, 0x00, 0x05, 0x00, 0x00, 0xDD, 0xCB}, //D6 OFF
};

void SystemPower(uchar status);


Bool AIPSetCtrl(uint16 property_id)
{
    bool ret_code=TRUE;
    uint16 power_status;
    const uchar *p_modbus_cmd=NULL;

    if(property_id >= NODE_SET_AIP_POWER_OFF && property_id <= NODE_SET_AIP_POWER_100)
        {
         switch(property_id)
            {
             case NODE_SET_AIP_POWER_OFF: 
                p_modbus_cmd = AIP_POWER_CTRL_CMD(AIP_POWER_CTRL_00);
                power_status = MODBUS_AIP_POWER_00;
                break;
             case NODE_SET_AIP_POWER_25: 
                p_modbus_cmd = AIP_POWER_CTRL_CMD(AIP_POWER_CTRL_25);
                power_status = MODBUS_AIP_POWER_25;
                break;
             case NODE_SET_AIP_POWER_50: 
                p_modbus_cmd = AIP_POWER_CTRL_CMD(AIP_POWER_CTRL_50);
                power_status = MODBUS_AIP_POWER_50;
                break;
             case NODE_SET_AIP_POWER_75:
                p_modbus_cmd = AIP_POWER_CTRL_CMD(AIP_POWER_CTRL_75);
                power_status = MODBUS_AIP_POWER_75;
                break;
             case NODE_SET_AIP_POWER_100: 
                p_modbus_cmd = AIP_POWER_CTRL_CMD(AIP_POWER_CTRL_100);
                power_status = MODBUS_AIP_POWER_100;
                break;                
             default: ret_code=FALSE; TraceErr1("AIPSetCtrl 1 property_id",property_id); 
                break;
            };
            
        }
    else {ret_code=FALSE;TraceErr1("AIPSetCtrl 2 property_id = ",property_id);}

    if(p_modbus_cmd != NULL) 
      {
       if(ServerSendModbusCmd((PUCHAR)p_modbus_cmd,MODBUS_CMD_NUM) == TRUE)
            ServerInfo.SensorInfo.AipInfo.AipPowerStatus = power_status;
      }
    else ret_code=FALSE;
    return ret_code;
}

//
//
//
Bool A308MSetCtrl(uint16 property_id)
{
    bool ret_code=TRUE;
    const uchar *p_modbus_cmd=NULL;

    if(property_id >= NODE_SET_WRITE_NEW_BIAS && property_id <= NODE_SET_RESET_BIAS_Z)
        {
         switch(property_id)
            {
             case NODE_SET_WRITE_NEW_BIAS:
                p_modbus_cmd = A308M_CTRL_CMD(A308M_CMD_WRITE_NEW_BIAS);
                break;
             case NODE_SET_RESET_BIAS_X:
                p_modbus_cmd = A308M_CTRL_CMD(A308M_CMD_RESET_XBIAS);
                break;
             case NODE_SET_RESET_BIAS_Y:
                p_modbus_cmd = A308M_CTRL_CMD(A308M_CMD_RESET_YBIAS);                
                break;
             case NODE_SET_RESET_BIAS_Z:
                p_modbus_cmd = A308M_CTRL_CMD(A308M_CMD_RESET_ZBIAS);                
                break;
             default: ret_code=FALSE; TraceErr1("A308MSetCtrl 1 property_id",property_id); 
                break;
            };
        }
    else {ret_code=FALSE;TraceErr1("A308MSetCtrl 2 property_id = ",property_id);}

    if(p_modbus_cmd != NULL) 
      {
       if(ServerSendModbusCmd((PUCHAR)p_modbus_cmd,MODBUS_CMD_NUM) == TRUE)
        {TraceOk("A308MSetCtrl");}
      }
    else ret_code=FALSE;
    return ret_code;
}



//
//
//
Bool A6D6MSetCtrl(uint16 property_id)
{
    bool ret_code=TRUE;
    const uchar *p_modbus_cmd=NULL;

    if((property_id >= NODE_SET_DO1_ON) && (property_id <= NODE_SET_DO6_ON))
        {
         p_modbus_cmd = (PUCHAR)&A6D6CtrlCmdOn[property_id-NODE_SET_DO1_ON][0];
        }
    else if((property_id >= NODE_SET_DO1_OFF) && (property_id <= NODE_SET_DO6_OFF))
        {
         p_modbus_cmd = (PUCHAR)&A6D6CtrlCmdOff[property_id-NODE_SET_DO1_OFF][0];
        }

    if(p_modbus_cmd != NULL) 
      {
       if(ServerSendModbusCmd((PUCHAR)p_modbus_cmd,MODBUS_CMD_NUM) == TRUE)
        { }
      }
    else ret_code=FALSE;
    return ret_code;
}






void ServerSetNodeProc()
{
    pStageInfo = GetNodeStageInfo(SERVER_SET_NODE_PROC);
    const uchar *p_modbus_cmd;
    bool ret_code=TRUE;
    uint16 temp,property_id;
     
    
    switch(ActiveStage())
        {
            case SNS_SET_INFO_INIT: 
              //  SetLedStatus(LED_STATUS_ON);
                ToNextStage(SNS_SET_WAITING);
                break;
            case SNS_SET_WAITING:  
                if(GetNodeStatus(NS_SET_NODE_ACT) == TRUE)  {ToNextStage(SNS_SET_INFO_PRE);}
                break;
                
            case SNS_SET_INFO_PRE: 
                ret_code=FALSE;
                property_id = pNodeEventInfo->PropertyID;

                if(pFunSensor == GetAipInfo)
                    {
                     ret_code = AIPSetCtrl(property_id); ToWaitingStage(SNS_SET_WAITING_INFO,5);
                    }
                else  if(pFunSensor == GetA6D6Info)
                    {
                     ret_code = A6D6MSetCtrl(property_id); ToWaitingStage(SNS_SET_WAITING_INFO,5);
                    }
                else{TraceErr("ServerSetNodeProc 1");ToNextStage(SNS_SET_INFO_END);}

                if(ret_code == FALSE) {ToWaitingStage(SNS_SET_WAITING_INFO,1);}
                break;               
            case SNS_SET_WAITING_INFO: 
                if(CheckWaitTimeOut()) ToNextStage(SNS_SET_INFO_OK);
                break;
                
            case SNS_SET_INFO_OK:  
                ToNextStage(SNS_SET_UPDATE_INFO);
                break;
                
            case SNS_SET_UPDATE_INFO:
                
                ToNextStage(SNS_SET_INFO_END);
                break;
                
            case SNS_SET_INFO_END: 
                SetNodeStatus(NS_SET_NODE_ACT,OFF);
                SetNodeStatus(NS_GET_INFO_ACT,ON);  // Mesh get event active
                UsartResetRxTx(USART_ID_TX_RX);   
                ToNextStage(SNS_SET_WAITING);
                break;
            default:  break;
        }
    
}





//
// for sensor report
//
void ServerGetInfoProc()
{
    pStageInfo = GetNodeStageInfo(SERVER_GET_INFO_PROC);
    switch(ActiveStage())
        {
            case NODE_STAGE_INIT:
                ToNextStage(SNS_PRE_WAITING);   //default
                break;
            case SNS_PRE_WAITING: 
                ToWaitingStage(SNS_GET_INFO,GetDeviceInfoDelay); TraceDec1("GetDeviceInfoDelay",GetDeviceInfoDelay);
                CountErr = 0;
                break;
            case SNS_GET_INFO: 
                if(CheckWaitTimeOut() == TRUE) 
                    {
                    if((pFunSensor!= NULL) && (GetSensorInfo() == TRUE) )
                        {ToNextStage(SNS_EVENT_WAITING);}
                    else 
                        {//Trace("Relay Only 2");
                         ToWaitingStage(SNS_PRE_SLEEPING,TIMER_WAIT_SLEEPING);
                        }
                    /*
                    else{TraceErr1("SNS_GET_INFO",CountErr);
                        if(CountErr++ > 10) {ToNextStage(SNS_EVENT_WAITING);CountErr=0;}
                        }
                        */
                    }
                
                break;
            case SNS_EVENT_WAITING: 
                if(GetNodeStatus(NS_GET_INFO_ACT))
                  {
                    SetLedStatus(LED_STATUS_ACTIVE);
                    if(GetNodeStatus(NS_SLEEPING))
                      {ToWaitingStage(SNS_PRE_SLEEPING,TIMER_WAIT_SLEEPING);} // go to sleeping from host                        
                    else
                      {ToWaitingStage(SNS_PRE_SEND_INFO,TIMER_WAIT_SEND_INFO); CountErr = 0;
                        //TraceDec1("TIMER_WAIT_SEND_INFO",TIMER_WAIT_SEND_INFO);
                      }
                  }
                break;
            case SNS_PRE_SEND_INFO: 
                //waiting to send info
                if(CheckWaitTimeOut())  { ToNextStage(SNS_SEND_INFO); }
                break;                
            case SNS_SEND_INFO: 
                if(CountErr > 5) {ToWaitingStage(SNS_PRE_SLEEPING,TIMER_WAIT_SLEEPING);} // err: go to sleeping
            
                if(SendInfoToClient()) // to send sensor info
                    {
                     SetNodeStatus(NS_GET_INFO_ACT,OFF);
                     ToWaitingStage(SNS_PRE_SLEEPING,TIMER_WAIT_SLEEPING);
                    }   // waiting to sleeping
                else{//TraceErr("SNS_SEND_INFO 2");
                     CountErr++; ToWaitingStage(SNS_PRE_SEND_INFO,WAIT_SEC(2)); // send info again
                    }                    
                break;
            case SNS_PRE_SLEEPING: 

                if(CheckWaitTimeOut())
                    {
                     if(GetNodeStatus(NS_FULL_POWER) == ON)
                        {SetLedStatus(LED_STATUS_SLEEP); ToNextStage(SNS_WAKE_UP);}
                     else
                        {SetNodeSleeping(ON);  ToNextStage(SNS_SLEEPING);}
                    }
                    
                break;
            case SNS_SLEEPING: 
                // waiting wake up
                if(!GetNodeStatus(NS_SLEEPING)) ToNextStage(SNS_WAKE_UP);
                break;
            case SNS_WAKE_UP: 
                ToNextStage(SNS_PRE_WAITING);
                break;
            default: TraceErr1("ServerGetInfoProc",ActiveStage()); break;
        };
}



const uchar PT485Temp[MODBUS_CMD_NUM]={0x01, 0x04, 0x00, 0x00, 0x00, 0x01, 0x31, 0xCA}; //to get value of temperature
//
// Get sensor info and update
//
bool GetSensorInfo()
{
    bool ret_code = FALSE;
    pSensorHeader->BatteryPower = GetBatteryPower(); 
    if(pFunSensor) ret_code = pFunSensor();
    UsartResetRxTx(USART_ID_TX_RX);
    return ret_code;
}


#define SetNodeInfoSize(info)   ServerInfo.NodeInfoSize = sizeof(_NodeHeader) + sizeof(info)
#define SetNodeClass(class)     pSensorHeader->SensorClass = class

//
//
//
bool GetSi7021Info()
{
    bool ret_code = TRUE;
    int16 temp;
    uint16 Humidity;
    PSi7021Info p_sensor = &ServerInfo.SensorInfo.Si7021Info;
    SetNodeInfoSize(_Si7021Info);
    SetNodeClass(SENSOR_SI7021);
    if(GetTempAndRH(&temp,&Humidity) != VALUE_IS_NOT_KNOWN)
        {
         p_sensor->Tempature = temp;
         p_sensor->Humidity = Humidity;
        }
    return ret_code;
}


//
//
//
bool GetPT485Info()
{
    bool ret_code = TRUE;    
    PPT485Info p_sensor = &ServerInfo.SensorInfo.PT485;
    SetNodeInfoSize(_PT485Info);
    SetNodeClass(SENSOR_PT485);
    
    PUCHAR p_buff = UsartGetBuff(USART_ID_RX);
    ServerSendModbusCmd((PUCHAR)PT485Temp,MODBUS_CMD_NUM); 
    p_sensor->Tempature = WordSwap(*((PUINT16)&p_buff[3]));
    return ret_code;
}

#define AIP_POWER_VALUE_OFF         200
const uchar AipPower[MODBUS_CMD_NUM]={0x01, 0x04, 0x00, 0x04, 0x00, 0x01,0x70, 0x0B};
const uchar AipPowerStatus[MODBUS_CMD_NUM]={0x01, 0x04, 0x00, 0x01, 0x00, 0x01, 0x60, 0x0A};
//const uchar AipPowerStatus[MODBUS_CMD_NUM]={0x01, 0x04, 0x00, 0x02, 0x00, 0x01, 0x90, 0x0A };

//
//
//
bool GetAipInfo()
{
    bool ret_code = TRUE;
    PAIPInfo p_sensor = &ServerInfo.SensorInfo.AipInfo;
    SetNodeInfoSize(_AIPInfo);
    SetNodeClass(SENSOR_AIP);

    PUCHAR p_buff = UsartGetBuff(USART_ID_RX);
    ServerSendModbusCmd((PUCHAR)AipPower,MODBUS_CMD_NUM); 
    p_sensor->AipPower = WordSwap(*((PUINT16)&p_buff[3]));
    UsartResetRxTx(USART_ID_TX_RX);
    ServerSendModbusCmd((PUCHAR)AipPowerStatus,MODBUS_CMD_NUM); 
    p_sensor->AipPowerStatus = WordSwap(*((PUINT16)&p_buff[3]));
    
    return ret_code;
}



#ifdef JNC_A308M
// For BT Mesh Modbus Reg
const uchar CmdBtA308M_1X[MODBUS_CMD_NUM] ={0x01, 0x03, 0x06, 0x00, 0x00, 0x0B, 0x04, 0x85};// 0x600 to 0x60A
const uchar CmdBtA308M_1Y[MODBUS_CMD_NUM] ={0x01, 0x03, 0x06, 0x0C, 0x00, 0x0B, 0xC4, 0x86};//
const uchar CmdBtA308M_1Z[MODBUS_CMD_NUM] ={0x01, 0x03, 0x06, 0x18, 0x00, 0x0B, 0x84, 0x82};//

const uchar CmdBtA308M_2X[MODBUS_CMD_NUM] ={0x01, 0x03, 0x06, 0x26, 0x00, 0x04, 0xA5, 0x4A};// Frequency, Strength
const uchar CmdBtA308M_2Y[MODBUS_CMD_NUM] ={0x01, 0x03, 0x06, 0x32, 0x00, 0x04, 0xE5, 0x4E};//
const uchar CmdBtA308M_2Z[MODBUS_CMD_NUM] ={0x01, 0x03, 0x06, 0x3E, 0x00, 0x04, 0x25, 0x4D};//
const uchar CmdA308MTemp[MODBUS_CMD_NUM] ={0x01, 0x03, 0x06, 0x24, 0x00, 0x02, 0x84, 0x88};  //for tempature
#else
const uchar A308MCmdXrms[MODBUS_CMD_NUM]  ={0x01, 0x03, 0x00, 0x04, 0x00, 0x02, 0x85, 0xCA};
const uchar A308MCmdXspeed[MODBUS_CMD_NUM]={0x01, 0x03, 0x00, 0x12, 0x00, 0x02, 0x64, 0x0E};
const uchar A308MCmdYrms[MODBUS_CMD_NUM]  ={0x01, 0x03, 0x00, 0x18, 0x00, 0x02, 0x44, 0x0C};
const uchar A308MCmdYspeed[MODBUS_CMD_NUM]={0x01, 0x03, 0x00, 0x26, 0x00, 0x02, 0x25, 0xC0};
const uchar A308MCmdZrms[MODBUS_CMD_NUM]  ={0x01, 0x03, 0x00, 0x2C, 0x00, 0x02, 0x05, 0xC2};
const uchar A308MCmdZspeed[MODBUS_CMD_NUM]={0x01, 0x03, 0x00, 0x3A, 0x00, 0x02, 0xE4, 0x06};
const uchar A308MCmdTemp[MODBUS_CMD_NUM] ={0x01, 0x03, 0x00, 0x3C, 0x00, 0x02, 0x04, 0x07};  //for tempature

const uchar A308MCmd_XFFT_Fre_Str[MODBUS_CMD_NUM]={0x01, 0x03, 0x01, 0x00, 0x00, 0x04, 0x45, 0xF5};
const uchar A308MCmd_YFFT_Fre_Str[MODBUS_CMD_NUM]={0x01, 0x03, 0x02, 0x00, 0x00, 0x04, 0x45, 0xB1};
const uchar A308MCmd_ZFFT_Fre_Str[MODBUS_CMD_NUM]={0x01, 0x03, 0x03, 0x00, 0x00, 0x04, 0x44, 0x4D};

#endif




#define SCALE_VALUE     10
#define A308M_RxData()    (&p_buff[3])
#define A308M_Value()   (*((float*)A308M_RxData()))

#ifdef JNC_A308M
float sensor_info;

//
// For JNC Demo
//
bool GetA308mInfo()
{
    bool ret_code = TRUE;
    //float sensor_info;
    PUCHAR p_buff;
    PA308mInfo p_sensor = &ServerInfo.SensorInfo.A308mInfo;
    SetNodeInfoSize(_A308mInfo);
    SetNodeClass(SENSOR_A308M);
    p_buff = UsartGetBuff(USART_ID_RX);
    if(ServerSendModbusCmd((PUCHAR)CmdBtA308M_1X,MODBUS_CMD_NUM) == TRUE)
        {p_buff +=3; 
         //PrintDataByte("A308MCmdXrms 1", p_buff, 24);
         DWordSwapN(p_buff,6); 
         //PrintDataByte("A308MCmdXrms 2", p_buff, 24);
         p_buff +=4; // for Mean
         sensor_info = *((float *)p_buff); p_sensor->RmsX = (uint16)sensor_info; p_buff+=4;
         sensor_info = *((float *)p_buff); p_sensor->SkewnessX = (int16)(sensor_info*SCALE_VALUE); p_buff+=4;
         sensor_info = *((float *)p_buff); p_sensor->KurtosisX = (int16)(sensor_info*SCALE_VALUE); p_buff +=8; // to Speed
         sensor_info = *((float *)p_buff); p_sensor->SpeedX = (int16)(sensor_info);         
        };
     UsartResetRxTx(USART_ID_TX_RX); 

     p_buff = UsartGetBuff(USART_ID_RX);
    if(ServerSendModbusCmd((PUCHAR)CmdBtA308M_2X,MODBUS_CMD_NUM) == TRUE)
        {p_buff +=3; 
         //PrintDataByte("A308MCmdXrms 3", p_buff, 8);
         DWordSwapN(p_buff,2); 
         //PrintDataByte("A308MCmdXrms 4", p_buff, 8);
         sensor_info = *((float *)p_buff); p_sensor->FrequencyX = (uint16)sensor_info; p_buff+=4;
         sensor_info = *((float *)p_buff); p_sensor->StengthX = (uint16)(sensor_info); p_buff+=4;
        };
     UsartResetRxTx(USART_ID_TX_RX); 

    p_buff = UsartGetBuff(USART_ID_RX);
    if(ServerSendModbusCmd((PUCHAR)CmdBtA308M_1Y,MODBUS_CMD_NUM) == TRUE)
        {p_buff +=3; 
         //PrintDataByte("A308MCmdXrms 1", p_buff, 24);
         DWordSwapN(p_buff,6); 
         //PrintDataByte("A308MCmdXrms 2", p_buff, 24);
         p_buff +=4; // for Mean
         sensor_info = *((float *)p_buff); p_sensor->RmsY = (uint16)sensor_info; p_buff+=4;
         sensor_info = *((float *)p_buff); p_sensor->SkewnessY = (int16)(sensor_info*SCALE_VALUE); p_buff+=4;
         sensor_info = *((float *)p_buff); p_sensor->KurtosisY = (int16)(sensor_info*SCALE_VALUE); p_buff +=8; // to Speed
         sensor_info = *((float *)p_buff); p_sensor->SpeedY = (int16)(sensor_info);         
        };
     UsartResetRxTx(USART_ID_TX_RX); 

     p_buff = UsartGetBuff(USART_ID_RX);
    if(ServerSendModbusCmd((PUCHAR)CmdBtA308M_2Y,MODBUS_CMD_NUM) == TRUE)
        {p_buff +=3; 
         //PrintDataByte("A308MCmdXrms 3", p_buff, 8);
         DWordSwapN(p_buff,2); 
         //PrintDataByte("A308MCmdXrms 4", p_buff, 8);
         sensor_info = *((float *)p_buff); p_sensor->FrequencyY = (uint16)sensor_info; p_buff+=4;
         sensor_info = *((float *)p_buff); p_sensor->StengthY = (uint16)(sensor_info); p_buff+=4;
        };
     UsartResetRxTx(USART_ID_TX_RX); 

    p_buff = UsartGetBuff(USART_ID_RX);
    if(ServerSendModbusCmd((PUCHAR)CmdBtA308M_1Z,MODBUS_CMD_NUM) == TRUE)
        {p_buff +=3; 
         //PrintDataByte("A308MCmdXrms 1", p_buff, 24);
         DWordSwapN(p_buff,6); 
         //PrintDataByte("A308MCmdXrms 2", p_buff, 24);
         p_buff +=4; // for Mean
         sensor_info = *((float *)p_buff); p_sensor->RmsZ = (uint16)sensor_info; p_buff+=4;
         sensor_info = *((float *)p_buff); p_sensor->SkewnessZ = (int16)(sensor_info*SCALE_VALUE); p_buff+=4;
         sensor_info = *((float *)p_buff); p_sensor->KurtosisZ = (int16)(sensor_info*SCALE_VALUE); p_buff +=8; // to Speed
         sensor_info = *((float *)p_buff); p_sensor->SpeedZ = (int16)(sensor_info);         
        };
     UsartResetRxTx(USART_ID_TX_RX); 

     p_buff = UsartGetBuff(USART_ID_RX);
    if(ServerSendModbusCmd((PUCHAR)CmdBtA308M_2Z,MODBUS_CMD_NUM) == TRUE)
        {p_buff +=3; 
         //PrintDataByte("A308MCmdXrms 3", p_buff, 8);
         DWordSwapN(p_buff,2); 
         //PrintDataByte("A308MCmdXrms 4", p_buff, 8);
         sensor_info = *((float *)p_buff); p_sensor->FrequencyZ = (uint16)sensor_info; p_buff+=4;
         sensor_info = *((float *)p_buff); p_sensor->StengthZ = (uint16)(sensor_info); p_buff+=4;
        };
     UsartResetRxTx(USART_ID_TX_RX); 

    p_buff = UsartGetBuff(USART_ID_RX);
    if(ServerSendModbusCmd((PUCHAR)CmdA308MTemp,MODBUS_CMD_NUM) == TRUE)
        {p_buff +=3;
         DWordSwap(p_buff); 
        sensor_info = *((float *)p_buff); p_sensor->Tempature = (int16)(sensor_info*SCALE_VALUE);
        }
    UsartResetRxTx(USART_ID_TX_RX);

   Printf("A308M=>X RMS=%ld Skew=%ld Kurt=%ld Freq=%ld Speed=%ld Stength=%ld\r\n",\
          p_sensor->RmsX,p_sensor->SkewnessX,p_sensor->KurtosisX,p_sensor->FrequencyX,p_sensor->SpeedX,p_sensor->StengthX);
   
   Printf("A308M=>Y RMS=%ld Skew=%ld Kurt=%ld Freq=%ld Speed=%ld Stength=%ld\r\n",\
          p_sensor->RmsY,p_sensor->SkewnessY,p_sensor->KurtosisY,p_sensor->FrequencyY,p_sensor->SpeedY,p_sensor->StengthY);
    
   Printf("A308M=>Z RMS=%ld Skew=%ld Kurt=%ld Freq=%ld Speed=%ld Stength=%ld\r\n",\
          p_sensor->RmsZ,p_sensor->SkewnessZ,p_sensor->KurtosisZ,p_sensor->FrequencyZ,p_sensor->SpeedZ,p_sensor->StengthZ);

    TraceDec1("A308M => Temp ", p_sensor->Tempature);

    return ret_code;
}



#else

//
//
//
bool GetA308mInfo()
{
    bool ret_code = TRUE;
    //float sensor_info;
    PUCHAR p_buff;
    PA308mInfo p_sensor = &ServerInfo.SensorInfo.A308mInfo;
    SetNodeInfoSize(_A308mInfo);
    SetNodeClass(SENSOR_A308M);
    
    
    p_buff = UsartGetBuff(USART_ID_RX);
    if(ServerSendModbusCmd((PUCHAR)A308MCmdXrms,MODBUS_CMD_NUM) == TRUE)
        {DWordSwap(A308M_RxData()); sensor_info = A308M_Value();  p_sensor->RmsX = (uint16)sensor_info;}
    //PrintDataByte("A308MCmdXrms", A308M_RxData(), 4);
    UsartResetRxTx(USART_ID_TX_RX); //Printf("XRMS   ==> %04d\r\n",p_sensor->RmsX); 
    

    if(ServerSendModbusCmd((PUCHAR)A308MCmdXspeed,MODBUS_CMD_NUM) == TRUE)
        {DWordSwap(A308M_RxData()); sensor_info = A308M_Value();  p_sensor->SpeedX = (uint16)(sensor_info*SCALE_VALUE);}
    UsartResetRxTx(USART_ID_TX_RX); 
    
    if(ServerSendModbusCmd((PUCHAR)A308MCmdYrms,MODBUS_CMD_NUM) == TRUE)
        {DWordSwap(A308M_RxData()); sensor_info = A308M_Value();  p_sensor->RmsY = (uint16)sensor_info;}
    UsartResetRxTx(USART_ID_TX_RX);  
    
    if(ServerSendModbusCmd((PUCHAR)A308MCmdYspeed,MODBUS_CMD_NUM) == TRUE)
        {DWordSwap(A308M_RxData()); sensor_info = A308M_Value(); p_sensor->SpeedY = (uint16)(sensor_info*SCALE_VALUE);}
    UsartResetRxTx(USART_ID_TX_RX); 

    if(ServerSendModbusCmd((PUCHAR)A308MCmdZrms,MODBUS_CMD_NUM) == TRUE)
        {DWordSwap(A308M_RxData()); sensor_info = A308M_Value();  p_sensor->RmsZ = (uint16)sensor_info;}
    UsartResetRxTx(USART_ID_TX_RX); 
    
    if(ServerSendModbusCmd((PUCHAR)A308MCmdZspeed,MODBUS_CMD_NUM) == TRUE)
        {DWordSwap(A308M_RxData()); sensor_info = A308M_Value();  p_sensor->SpeedZ = (uint16)(sensor_info*SCALE_VALUE);}
    UsartResetRxTx(USART_ID_TX_RX); 

    if(ServerSendModbusCmd((PUCHAR)A308MCmdTemp,MODBUS_CMD_NUM) == TRUE)
        {DWordSwap(A308M_RxData()); sensor_info = A308M_Value(); p_sensor->Tempature = (int16)(sensor_info*SCALE_VALUE);}
    UsartResetRxTx(USART_ID_TX_RX);
    
    if(ServerSendModbusCmd((PUCHAR)A308MCmd_XFFT_Fre_Str,MODBUS_CMD_NUM) == TRUE)
        {//PrintDataByte("A308MCmdXFre", A308M_RxData(), 8);
        DWordSwap(A308M_RxData()); 
        sensor_info = A308M_Value(); p_sensor->FrequencyX = (uint16)(sensor_info); //(uint16)(sensor_info*SCALE_VALUE);
        memcpy(A308M_RxData(),(&p_buff[7]),4);
        DWordSwap(A308M_RxData());
        sensor_info = A308M_Value(); p_sensor->StengthX = (uint16)(sensor_info);
        }
    UsartResetRxTx(USART_ID_TX_RX);

    if(ServerSendModbusCmd((PUCHAR)A308MCmd_YFFT_Fre_Str,MODBUS_CMD_NUM) == TRUE)
        {//PrintDataByte("A308MCmdYFre", A308M_RxData(), 8);
        DWordSwap(A308M_RxData()); 
        sensor_info = A308M_Value(); p_sensor->FrequencyY = (uint16)(sensor_info); //(uint16)(sensor_info*SCALE_VALUE);
        memcpy(A308M_RxData(),(&p_buff[7]),4);
        DWordSwap(A308M_RxData());
        sensor_info = A308M_Value(); p_sensor->StengthY = (uint16)(sensor_info);
        }
    UsartResetRxTx(USART_ID_TX_RX);    

    if(ServerSendModbusCmd((PUCHAR)A308MCmd_ZFFT_Fre_Str,MODBUS_CMD_NUM) == TRUE)
        {//PrintDataByte("A308MCmdZFre", A308M_RxData(), 8);
        DWordSwap(A308M_RxData()); 
        sensor_info = A308M_Value(); p_sensor->FrequencyZ = (uint16)(sensor_info); //(uint16)(sensor_info*SCALE_VALUE);
        memcpy(A308M_RxData(),(&p_buff[7]),4);
        DWordSwap(A308M_RxData());
        sensor_info = A308M_Value(); p_sensor->StengthZ = (uint16)(sensor_info);
        }
    UsartResetRxTx(USART_ID_TX_RX); 
    return ret_code;
}

#endif


const uchar CmdWaterLevel[MODBUS_CMD_NUM]={0x01, 0x04, 0x00, 0x00, 0x00, 0x02, 0x71, 0xCB}; //to get value of temperature

//
//
//
bool GetWaterLevelInfo()
{
    bool ret_code = TRUE; 
    
    PWaterLevelInfo p_sensor = &ServerInfo.SensorInfo.WaterLevelInfo;
    SetNodeInfoSize(_WaterLevelInfo);
    SetNodeClass(SENSOR_WATER_LEVEL);
    PUCHAR p_buff = UsartGetBuff(USART_ID_RX);

    if(ServerSendModbusCmd((PUCHAR)CmdWaterLevel,MODBUS_CMD_NUM) == TRUE)
      {
        p_sensor->WaterLevel = WordSwap(*((PUINT16)&p_buff[3]));
        p_sensor->OilLevel = WordSwap(*((PUINT16)&p_buff[5]));
      }
  
    
    return ret_code;
}

const uchar CmdGetSdCo2[MODBUS_CMD_NUM] ={0x01, 0x04, 0x03, 0x00, 0x00, 0x01, 0x31, 0x8E}; //
const uchar CmdGetSdPm25[MODBUS_CMD_NUM]={0x01, 0x04, 0x03, 0x01, 0x00, 0x01, 0x60, 0x4E}; //
const uchar CmdGetSdTemp[MODBUS_CMD_NUM]={0x01, 0x04, 0x03, 0x02, 0x00, 0x01, 0x90, 0x4E}; //to get value of temperature
const uchar CmdGetSdRh[MODBUS_CMD_NUM]  ={0x01, 0x04, 0x03, 0x03, 0x00, 0x01, 0xC1, 0x8E}; //to get value of RH
const uchar CmdGetSdAll[MODBUS_CMD_NUM]  ={0x01, 0x04, 0x03, 0x00, 0x00, 0x04, 0xF1, 0x8D}; //to get Co2, Pm2.5, Temp and RH value

//
//
//
bool GetJncSdInfo()
{
    bool ret_code = TRUE;
    uint16 CO2,PM25,Tempature,Humidity;
    
    PSdInfo p_sensor = &ServerInfo.SensorInfo.SdInfo;
    SetNodeInfoSize(_SdInfo);
    SetNodeClass(SENSOR_JNC_SD);
    PUCHAR p_buff = UsartGetBuff(USART_ID_RX);
    
    if(ServerSendModbusCmd((PUCHAR)CmdGetSdAll,MODBUS_CMD_NUM) == TRUE)
      {
        CO2  = p_sensor->CO2 = WordSwap(*((PUINT16)&p_buff[3]));
        PM25  = p_sensor->PM25 = WordSwap(*((PUINT16)&p_buff[5]));
        Tempature  = p_sensor->Tempature = WordSwap(*((PUINT16)&p_buff[7]));
        Humidity  = p_sensor->Humidity = WordSwap(*((PUINT16)&p_buff[9]));
      }
    
    return ret_code;
}

const uchar CmdGetUltraSound[MODBUS_CMD_NUM]={0x01, 0x04, 0x00, 0x00, 0x00, 0x01, 0x31, 0xCA}; //to get value of temperature


//
//
//
bool GetUltraSoundInfo()
{
bool ret_code = TRUE; 

PWaterLevelInfo p_sensor = &ServerInfo.SensorInfo.WaterLevelInfo;
SetNodeInfoSize(_WaterLevelInfo);
SetNodeClass(SENSOR_ULTRA_SOUND);
PUCHAR p_buff = UsartGetBuff(USART_ID_RX);
if(ServerSendModbusCmd((PUCHAR)CmdWaterLevel,MODBUS_CMD_NUM) == TRUE)
  {
    p_sensor->WaterLevel = WordSwap(*((PUINT16)&p_buff[3]));
  }

return ret_code;
}

const uchar CmdDoRealValue[MODBUS_CMD_NUM] ={0x01, 0x04, 0x00, 0x00, 0x00, 0x01, 0x31, 0xCA};
const uchar CmdDoRealOffset[MODBUS_CMD_NUM]={0x01, 0x04, 0x00, 0x02, 0x00, 0x01, 0x90, 0x0A};
const uchar CmdDoTempValue[MODBUS_CMD_NUM] ={0x01, 0x04, 0x00, 0x0A, 0x00, 0x01, 0x11, 0xC8};
const uchar CmdDoTempOffset[MODBUS_CMD_NUM]={0x01, 0x04, 0x00, 0x0B, 0x00, 0x01, 0x40, 0x08};

//
//
//
bool GetDo485()
{
bool ret_code = TRUE;
PJncDo485 p_sensor = &ServerInfo.SensorInfo.JncDo485;
SetNodeInfoSize(_JncDo485);
SetNodeClass(SENSOR_DO_485);
PUCHAR p_buff = UsartGetBuff(USART_ID_RX);
if(ServerSendModbusCmd((PUCHAR)CmdDoRealValue,MODBUS_CMD_NUM) == TRUE)
  {
    p_sensor->DoRealValue = WordSwap(*((PUINT16)&p_buff[3]));
  }
UsartResetRxTx(USART_ID_TX_RX);

if(ServerSendModbusCmd((PUCHAR)CmdDoRealOffset,MODBUS_CMD_NUM) == TRUE)
  {
    p_sensor->DoOffsetValue = WordSwap(*((PUINT16)&p_buff[3]));
  }
UsartResetRxTx(USART_ID_TX_RX);

if(ServerSendModbusCmd((PUCHAR)CmdDoTempValue,MODBUS_CMD_NUM) == TRUE)
  {
    p_sensor->TempRealValue = WordSwap(*((PUINT16)&p_buff[3]));
  }
UsartResetRxTx(USART_ID_TX_RX);

if(ServerSendModbusCmd((PUCHAR)CmdDoTempOffset,MODBUS_CMD_NUM) == TRUE)
  {
    p_sensor->TempOffsetValue = WordSwap(*((PUINT16)&p_buff[3]));
  }
UsartResetRxTx(USART_ID_TX_RX);

return ret_code;
}

const uchar CmdA6D6_AI[MODBUS_CMD_NUM] ={0x01, 0x04, 0x00, 0x00, 0x00, 0x08, 0xF1, 0xCC};
const uchar CmdA6D6_DI[MODBUS_CMD_NUM] ={0x01, 0x02, 0x00, 0x00, 0x00, 0x08, 0x79, 0xCC};
const uchar CmdA6D6_DO[MODBUS_CMD_NUM] ={0x01, 0x01, 0x00, 0x00, 0x00, 0x08, 0x3D, 0xCC};

#define A6D6_AI_VALUE   (p_buff[3])
#define A6D6_DI_VALUE   (p_buff[3])
#define A6D6_DO_VALUE   (p_buff[3])

//
//
//
bool GetA6D6Info()
{
bool ret_code = TRUE; 
PA6D6 p_sensor = &ServerInfo.SensorInfo.A6D6;
PUINT16 p_value;

SetNodeInfoSize(_A6D6);
SetNodeClass(SENSOR_A6D6);

PUCHAR p_buff = UsartGetBuff(USART_ID_RX); p_value = (PUINT16)(&p_buff[3]);
if(ServerSendModbusCmd((PUCHAR)CmdA6D6_AI,MODBUS_CMD_NUM) == TRUE)
  {
    p_sensor->AiValue1 = WordSwap(*p_value++); p_sensor->AiValue2 = WordSwap(*p_value++);    
    p_sensor->AiValue3 = WordSwap(*p_value++); p_sensor->AiValue4 = WordSwap(*p_value++);
    p_sensor->AiValue5 = WordSwap(*p_value++); p_sensor->AiValue6 = WordSwap(*p_value++);
    p_sensor->AiValue7 = WordSwap(*p_value++); p_sensor->AiValue8 = WordSwap(*p_value++);
  }
UsartResetRxTx(USART_ID_TX_RX);

if(ServerSendModbusCmd((PUCHAR)CmdA6D6_DI,MODBUS_CMD_NUM) == TRUE)
  {
    p_sensor->Di_Status = A6D6_DI_VALUE;
  }
UsartResetRxTx(USART_ID_TX_RX);

if(ServerSendModbusCmd((PUCHAR)CmdA6D6_DO,MODBUS_CMD_NUM) == TRUE)
  {//PrintDataByte("Get A6D6 DO", &A6D6_DO_VALUE, 1);
    p_sensor->DO_Status = A6D6_DO_VALUE;
  }
UsartResetRxTx(USART_ID_TX_RX);

return ret_code;
}


const uchar CmdPzemValue[MODBUS_CMD_NUM] ={0x01, 0x04, 0x00, 0x00, 0x00, 0x0A, 0x70, 0x0D};


bool GetPzem()
{
bool ret_code = TRUE;
PPzem p_sensor = &ServerInfo.SensorInfo.Pzem;
PUINT16 p_value,p_sensor_value;
uint16 loop;

SetNodeInfoSize(_Pzem);
SetNodeClass(SENSOR_PZEM);
PUCHAR p_buff = UsartGetBuff(USART_ID_RX); 
p_value = (PUINT16)(&p_buff[3]); p_sensor_value=&p_sensor->Voltage;
if(ServerSendModbusCmd((PUCHAR)CmdPzemValue,MODBUS_CMD_NUM) == TRUE)
  {
    for(loop=0; loop<PZEM_ITEM_SIZE; loop++) *p_sensor_value++ = WordSwap(*p_value++);
  }
else {ret_code = FALSE;}
return ret_code;
}


bool GetRelay()
{
bool ret_code = TRUE;
PRelayNode p_sensor = &ServerInfo.SensorInfo.RelayNode;
PUINT16 p_value,p_sensor_value;
uint16 loop;

SetNodeInfoSize(_RelayNode);
SetNodeClass(SENSOR_RELAY);

p_sensor->Status++;;


return ret_code;
}


//
//Send sensor information to client
//
bool SendInfoToClient()
{
    bool ret_code=FALSE;
    uchar   send_size;
    if(ServerInfo.NodeInfoSize > 0)
      {
        send_size = ServerInfo.NodeInfoSize+3;
        ServerInfo.ProperityID = NODE_GET_ALL_SENSOR;
        
        result = Cmd_ms_server_send_status(SENSOR_ELEMENT, pNodeEventInfo->ClientAddr,pNodeEventInfo->AppkeyIndex,
                                            NO_FLAGS, send_size, (PUCHAR)&ServerInfo)->result;
      if(result) {ret_code = FALSE; }
      else ret_code=TRUE;
        
      }
    else 
    
    return ret_code;

}

//
//
//
void EvtGetRequestProc(PCmdPacket pCmdEvent)
{
    msg_ms_server_get_request_evt *pEvent = &(pCmdEvent->data.evt_mesh_sensor_server_get_request);
    pNodeEventInfo->ElemIndex   = pEvent->appkey_index;
    pNodeEventInfo->ClientAddr  = pEvent->client_address;
    pNodeEventInfo->ServerAddr  = pEvent->server_address;
    pNodeEventInfo->AppkeyIndex = pEvent->appkey_index;
    pNodeEventInfo->Flags       = pEvent->flags;
    pNodeEventInfo->PropertyID  = pEvent->property_id;
    //Trace16_1(pEvent->property_id);
    if(pEvent->property_id == NODE_GET_ALL_SENSOR)    
     {
         SetNodeStatus(NS_GET_INFO_ACT,ON);  // Mesh get event active
     }
    else if(pEvent->property_id >= NODE_SET_AIP_POWER_OFF && pEvent->property_id <= NODE_SET_AIP_POWER_100)
    {
         SetNodeStatus(NS_SET_NODE_ACT,ON);  // Mesh set event active
    }
    
    else if(pEvent->property_id >= NODE_SET_DO1_ON && pEvent->property_id <= NODE_SET_DO6_OFF)
    {//Trace("Set A6D6");
         SetNodeStatus(NS_SET_NODE_ACT,ON);  // Mesh set event active
    }
    else
    {//
        ToClientBuf[0] = pEvent->property_id & 0xFF;
        ToClientBuf[1] = ((pEvent->property_id) >> 8) & 0xFF;
        ToClientBuf[3] = 0; // Length is 0 for unsupported property_id
        Cmd_ms_server_send_status(SENSOR_ELEMENT, pEvent->client_address, pEvent->appkey_index, NO_FLAGS, 3, ToClientBuf);
    }
        
}



//
// send modbus cmd, return info to Rx buffer
//
bool ServerSendModbusCmd(PUCHAR modbus_cmd,uchar len)
{
    bool ret_code = TRUE;
    UsartTxSendCmd(modbus_cmd,len); 
    Delay_ms(10); Rs485Rx();
    Delay_ms(150); Rs485Tx(); // must to check crc error
    ret_code = CheckModbusCrc(UsartGetBuff(USART_ID_RX), UsartGetRxCounter());

    if(!ret_code) TraceErr1("ServerSendModbusCmd 1",UsartGetRxCounter());
    
    return ret_code;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




//result = Cmd_ms_client_set_setting(SENSOR_ELEMENT, pCmd->ModbusID, IGNORED, 0x00, MODBUS_GET_REGS_VALUE,
//                                   6, 4, (uint8 *)&setting_data)->result;


//
// for Set setting event
//
void EvtSetSettingRequestProc(PCmdPacket pCmdEvent)
{
    msg_ms_setup_server_set_setting_request_evt *pEvent = &(pCmdEvent->data.evt_mesh_sensor_setup_server_set_setting_request);
    pNodeEventInfo->ElemIndex   = pEvent->appkey_index;
    pNodeEventInfo->ClientAddr  = pEvent->client_address;
    pNodeEventInfo->ServerAddr  = pEvent->server_address;
    pNodeEventInfo->AppkeyIndex = pEvent->appkey_index;
    pNodeEventInfo->Flags       = pEvent->flags;
    pNodeEventInfo->PropertyID  = pEvent->property_id;
    pNodeEventInfo->SettingID   = pEvent->setting_id;
    pNodeEventInfo->SensorData  = pEvent->raw_value;

    TraceDec3("EvtSetSettingRequestProc 1 ",pNodeEventInfo->ClientAddr ,pNodeEventInfo->PropertyID, pNodeEventInfo->SettingID);
    
}

//
// for Get setting event
//
void EvtSetGettingRequestProc(PCmdPacket pCmdEvent)
{
    msg_ms_setup_server_get_setting_request_evt *pEvent = &(pCmdEvent->data.evt_mesh_sensor_setup_server_get_setting_request);   
    pNodeEventInfo->ElemIndex   = pEvent->appkey_index;
    pNodeEventInfo->ClientAddr  = pEvent->client_address;
    pNodeEventInfo->ServerAddr  = pEvent->server_address;
    pNodeEventInfo->AppkeyIndex = pEvent->appkey_index;
    pNodeEventInfo->Flags       = pEvent->flags;
    pNodeEventInfo->PropertyID  = pEvent->property_id;
    pNodeEventInfo->SettingID   = pEvent->setting_id;
    TraceDec3("EvtSetGettingRequestProc 1",pNodeEventInfo->ClientAddr ,pNodeEventInfo->PropertyID, pNodeEventInfo->SettingID);
}


//
// return power status
//
uchar CheckPowerStatus()
{
    uchar power = POWER_USB;

    if(GetBatteryPower() > 0) power = POWER_BATTERY;

    return power;
}

