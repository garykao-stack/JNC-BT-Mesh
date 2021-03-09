
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
#include "Mesh_Server.h"

#define TO_CLIENT_BUFF_MAX  32
uchar   ToClientBuf[TO_CLIENT_BUFF_MAX];
uchar   ToClientLen;


_ServerInfo ServerInfo;
PServerInfo pServerInfo = &ServerInfo;
PNodeHeader pSensorHeader = &ServerInfo.SensorInfo.Header;

//
//
//
void ServerNodeInit()
{TraceProc();
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
    
  //  temp = GetTempature(); TraceDec1("Tempature", temp);
 //   temp = GetHumidity(); TraceDec1("GetHumidity", temp);    
}

void ServerSetupNodeInit()
{TraceProc();
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


void SystemPower(uchar status);


Bool AIPSetCtrl(uint16 property_id)
{
    bool ret_code=TRUE;
    const uchar *p_modbus_cmd=NULL;

    if(property_id >= NODE_SET_AIP_POWER_OFF && property_id <= NODE_SET_AIP_POWER_100)
        {
         switch(property_id)
            {
             case NODE_SET_AIP_POWER_OFF: //Trace("AIP Power 00%");
                p_modbus_cmd = AIP_POWER_CTRL_CMD(AIP_POWER_CTRL_00);
                break;
             case NODE_SET_AIP_POWER_25: //Trace("AIP Power 25%");
                p_modbus_cmd = AIP_POWER_CTRL_CMD(AIP_POWER_CTRL_25);
                break;
             case NODE_SET_AIP_POWER_50: //Trace("AIP Power 50%");
                p_modbus_cmd = AIP_POWER_CTRL_CMD(AIP_POWER_CTRL_50);                
                break;
             case NODE_SET_AIP_POWER_75: //Trace("AIP Power 75%");
                p_modbus_cmd = AIP_POWER_CTRL_CMD(AIP_POWER_CTRL_75);                
                break;
             case NODE_SET_AIP_POWER_100: //Trace("AIP Power 100%");
                p_modbus_cmd = AIP_POWER_CTRL_CMD(AIP_POWER_CTRL_100);                
                break;                
             default: ret_code=FALSE; TraceErr1("AIPSetCtrl 1 property_id",property_id); 
                break;
            };
            
        }
    else {ret_code=FALSE;TraceErr1("AIPSetCtrl 2 property_id = ",property_id);}

    if(p_modbus_cmd != NULL) 
      {
       if(ServerSendModbusCmd((PUCHAR)p_modbus_cmd,MODBUS_CMD_NUM) == TRUE)
            ServerInfo.SensorInfo.AipInfo.AipPowerStatus = property_id;
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
            ServerInfo.SensorInfo.AipInfo.AipPowerStatus = property_id;
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
            case SNS_SET_INFO_INIT: Trace("SNS_SET_INFO_INIT");
              //  SetLedStatus(LED_STATUS_ON);
                ToNextStage(SNS_SET_WAITING);
                break;
            case SNS_SET_WAITING:   //Trace("SNS_SET_WAITING");
                if(GetNodeStatus(NS_SET_NODE_ACT) == TRUE)  
                    {ToNextStage(SNS_SET_INFO_PRE);}
                break;
                
            case SNS_SET_INFO_PRE:  Trace("SNS_SET_INFO_PRE");
                property_id = pNodeEventInfo->PropertyID;
                ret_code=FALSE;
                if((ret_code = AIPSetCtrl(property_id)) == TRUE)
                    {Trace1("AIP Set OK",property_id);}
                else if((ret_code = A308MSetCtrl(property_id)) == TRUE)
                    {Trace1("A308M Set OK",property_id);}
                else TraceErr1("property_id 1",property_id);
                
                if(ret_code == TRUE) 
                    {ToWaitingStage(SNS_SET_WAITING_INFO,3);}
                else 
                    {ToWaitingStage(SNS_SET_WAITING_INFO,1);}
                break;               
            case SNS_SET_WAITING_INFO:  //Trace("SNS_SET_WAITING_INFO");
                if(CheckWaitTimeOut()) ToNextStage(SNS_SET_INFO_OK);
                break;
                
            case SNS_SET_INFO_OK:   Trace("SNS_SET_INFO_OK");
                ToNextStage(SNS_SET_UPDATE_INFO);
                break;
                
            case SNS_SET_UPDATE_INFO:  Trace("SNS_SET_UPDATE_INFO");
                
                ToNextStage(SNS_SET_INFO_END);
                break;
                
            case SNS_SET_INFO_END:  Trace("SNS_SET_INFO_END");
                SetNodeStatus(NS_SET_NODE_ACT,OFF);
                SetNodeStatus(NS_GET_INFO_ACT,ON);  // Mesh get event active
                UsartResetRxTx(USART_ID_TX_RX);   
                ToNextStage(SNS_SET_WAITING);
                break;
            default: TraceErr1("ClientSetNodeInfoProc",ActiveStage()); break;
        }
    
}





//
// for sensor report
//
void ServerGetInfoProc()
{//TraceProc();
    pStageInfo = GetNodeStageInfo(SERVER_GET_INFO_PROC);
    switch(ActiveStage())
        {
            case NODE_STAGE_INIT:
                ToNextStage(SNS_PRE_WAITING);   //default
                break;
            case SNS_PRE_WAITING: //Trace("SNS_PRE_WAITING");
                ToWaitingStage(SNS_GET_INFO,5); CountErr = 0;
                break;
            case SNS_GET_INFO: //Trace("SNS_GET_INFO");
                if(GetSensorInfo() == TRUE) 
                    ToNextStage(SNS_EVENT_WAITING);
                else{
                     if(CountErr++ > 10) {ToNextStage(SNS_EVENT_WAITING);CountErr=0;}
                    }
                
                break;
            case SNS_EVENT_WAITING: //TraceDec1("SNS_EVENT_WAITING",pStageInfo->Timer);
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
            case SNS_PRE_SEND_INFO: //waiting to send info depend on Node ID
                //waiting to send info
                if(CheckWaitTimeOut())  { ToNextStage(SNS_SEND_INFO); }
                break;                
            case SNS_SEND_INFO: //Trace("SNS_SEND_INFO 1");
                if(CountErr > 5) {ToWaitingStage(SNS_PRE_SLEEPING,TIMER_WAIT_SLEEPING);} // err: go to sleeping
            
                if(SendInfoToClient()) // to send sensor info
                    {
                     SetNodeStatus(NS_GET_INFO_ACT,OFF);
                     ToWaitingStage(SNS_PRE_SLEEPING,TIMER_WAIT_SLEEPING);
                    }   // waiting to sleeping
                else{TraceErr("SNS_SEND_INFO 2");
                     CountErr++; ToWaitingStage(SNS_PRE_SEND_INFO,WAIT_SEC(2)); // send info again
                    }                    
                break;
            case SNS_PRE_SLEEPING: //TraceDec1("SNS_PRE_SLEEPING",pStageInfo->Timer); //waiting 3sec

                if(CheckWaitTimeOut())
                    {
                     if(GetNodeStatus(NS_FULL_POWER) == ON)
                        {SetLedStatus(LED_STATUS_SLEEP); ToNextStage(SNS_WAKE_UP);}
                     else
                        {SetNodeSleeping(ON);  ToNextStage(SNS_SLEEPING);}
                    }
                    
                break;
            case SNS_SLEEPING: //Trace("SNS_SLEEPING");
                // waiting wake up
                if(!GetNodeStatus(NS_SLEEPING)) ToNextStage(SNS_WAKE_UP);
                break;
            case SNS_WAKE_UP: Trace("SNS_WAKE_UP");
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
{//TraceProc();
    bool ret_code = FALSE;
    pSensorHeader->BatteryPower = GetBatteryPower(); 
    
    Trace8_1(pSensorHeader->Status);
    ret_code = pFunSensor();
    UsartResetRxTx(USART_ID_TX_RX);
    return ret_code;
}


#define SetNodeInfoSize(info)   ServerInfo.NodeInfoSize = sizeof(_NodeHeader) + sizeof(info)
#define SetNodeClass(class)     pSensorHeader->SensorClass = class

//
//
//
bool GetSi7021Info()
{//TraceProc();
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
    else TraceErr("GetSi7021Info");
    TraceDec3("Si7021 ==> 3", p_sensor->Tempature,p_sensor->Humidity,ServerInfo.NodeInfoSize);
    return ret_code;
}


//
//
//
bool GetPT485Info()
{//TraceProc();
    bool ret_code = TRUE;    
    PPT485Info p_sensor = &ServerInfo.SensorInfo.PT485;
    SetNodeInfoSize(_PT485Info);
    SetNodeClass(SENSOR_PT485);
    
    PUCHAR p_buff = UsartGetBuff(USART_ID_RX);
    ServerSendModbusCmd((PUCHAR)PT485Temp,MODBUS_CMD_NUM); 
    p_sensor->Tempature = WordSwap(*((PUINT16)&p_buff[3]));
    TraceDec2("PT485 ==> ", p_sensor->Tempature,ServerInfo.NodeInfoSize);
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
{//TraceProc();
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
    
    TraceDec2("AIP ==> ",p_sensor->AipPower,p_sensor->AipPowerStatus);
    return ret_code;
}



const uchar A308MCmdXrms[MODBUS_CMD_NUM]  ={0x01, 0x03, 0x00, 0x04, 0x00, 0x02, 0x85, 0xCA};
const uchar A308MCmdXspeed[MODBUS_CMD_NUM]={0x01, 0x03, 0x00, 0x12, 0x00, 0x02, 0x64, 0x0E};
const uchar A308MCmdYrms[MODBUS_CMD_NUM]  ={0x01, 0x03, 0x00, 0x18, 0x00, 0x02, 0x44, 0x0C};
const uchar A308MCmdYspeed[MODBUS_CMD_NUM]={0x01, 0x03, 0x00, 0x26, 0x00, 0x02, 0x25, 0xC0};
const uchar A308MCmdZrms[MODBUS_CMD_NUM]  ={0x01, 0x03, 0x00, 0x2C, 0x00, 0x02, 0x05, 0xC2};
const uchar A308MCmdZspeed[MODBUS_CMD_NUM]={0x01, 0x03, 0x00, 0x3A, 0x00, 0x02, 0xE4, 0x06};
const uchar A308MCmdXtemp[MODBUS_CMD_NUM] ={0x01, 0x03, 0x00, 0x3C, 0x00, 0x02, 0x04, 0x07};  //for tempature

const uchar A308MCmd_XFFT_Fre_Str[MODBUS_CMD_NUM]={0x01, 0x03, 0x01, 0x00, 0x00, 0x04, 0x45, 0xF5};
const uchar A308MCmd_YFFT_Fre_Str[MODBUS_CMD_NUM]={0x01, 0x03, 0x02, 0x00, 0x00, 0x04, 0x45, 0xB1};
const uchar A308MCmd_ZFFT_Fre_Str[MODBUS_CMD_NUM]={0x01, 0x03, 0x03, 0x00, 0x00, 0x04, 0x44, 0x4D};



#define SCALE_VALUE     10
#define A308M_RxData()    (&p_buff[3])
#define A308M_Value()   (*((float*)A308M_RxData()))
//
//
//
bool GetA308mInfo()
{//TraceProc();
    bool ret_code = TRUE;
    float sensor_info;
    PUCHAR p_buff;
    PA308mInfo p_sensor = &ServerInfo.SensorInfo.A308mInfo;
    SetNodeInfoSize(_A308mInfo);
    SetNodeClass(SENSOR_A308M);
    
    
    p_buff = UsartGetBuff(USART_ID_RX);
    if(ServerSendModbusCmd((PUCHAR)A308MCmdXrms,MODBUS_CMD_NUM) == TRUE)
        {DWordSwap(A308M_RxData()); sensor_info = A308M_Value();  p_sensor->Xrms = (uint16)sensor_info;}
    //PrintDataByte("A308MCmdXrms", A308M_RxData(), 4);
    UsartResetRxTx(USART_ID_TX_RX); //Printf("XRMS   ==> %04d\r\n",p_sensor->Xrms); 
    

    if(ServerSendModbusCmd((PUCHAR)A308MCmdXspeed,MODBUS_CMD_NUM) == TRUE)
        {DWordSwap(A308M_RxData()); sensor_info = A308M_Value();  p_sensor->XSpeed = (uint16)(sensor_info*SCALE_VALUE);}
    UsartResetRxTx(USART_ID_TX_RX); 
    
    if(ServerSendModbusCmd((PUCHAR)A308MCmdYrms,MODBUS_CMD_NUM) == TRUE)
        {DWordSwap(A308M_RxData()); sensor_info = A308M_Value();  p_sensor->Yrms = (uint16)sensor_info;}
    UsartResetRxTx(USART_ID_TX_RX);  
    
    if(ServerSendModbusCmd((PUCHAR)A308MCmdYspeed,MODBUS_CMD_NUM) == TRUE)
        {DWordSwap(A308M_RxData()); sensor_info = A308M_Value(); p_sensor->YSpeed = (uint16)(sensor_info*SCALE_VALUE);}
    UsartResetRxTx(USART_ID_TX_RX); 

    if(ServerSendModbusCmd((PUCHAR)A308MCmdZrms,MODBUS_CMD_NUM) == TRUE)
        {DWordSwap(A308M_RxData()); sensor_info = A308M_Value();  p_sensor->Zrms = (uint16)sensor_info;}
    UsartResetRxTx(USART_ID_TX_RX); 
    
    if(ServerSendModbusCmd((PUCHAR)A308MCmdZspeed,MODBUS_CMD_NUM) == TRUE)
        {DWordSwap(A308M_RxData()); sensor_info = A308M_Value();  p_sensor->ZSpeed = (uint16)(sensor_info*SCALE_VALUE);}
    UsartResetRxTx(USART_ID_TX_RX); 

    if(ServerSendModbusCmd((PUCHAR)A308MCmdXtemp,MODBUS_CMD_NUM) == TRUE)
        {DWordSwap(A308M_RxData()); sensor_info = A308M_Value(); p_sensor->Tempature = (int16)(sensor_info*SCALE_VALUE);}
    UsartResetRxTx(USART_ID_TX_RX);
    
    if(ServerSendModbusCmd((PUCHAR)A308MCmd_XFFT_Fre_Str,MODBUS_CMD_NUM) == TRUE)
        {//PrintDataByte("A308MCmdXFre", A308M_RxData(), 8);
        DWordSwap(A308M_RxData()); 
        sensor_info = A308M_Value(); p_sensor->XFFT_Fre = (uint16)(sensor_info*SCALE_VALUE);
        memcpy(A308M_RxData(),(&p_buff[7]),4);
        DWordSwap(A308M_RxData());
        sensor_info = A308M_Value(); p_sensor->XFFT_Str = (uint16)(sensor_info);
        }
    TraceDec2("GetA308mInfo 1",p_sensor->XFFT_Fre, p_sensor->XFFT_Str);
    UsartResetRxTx(USART_ID_TX_RX);

    if(ServerSendModbusCmd((PUCHAR)A308MCmd_YFFT_Fre_Str,MODBUS_CMD_NUM) == TRUE)
        {//PrintDataByte("A308MCmdYFre", A308M_RxData(), 8);
        DWordSwap(A308M_RxData()); 
        sensor_info = A308M_Value(); p_sensor->YFFT_Fre = (uint16)(sensor_info*SCALE_VALUE);
        memcpy(A308M_RxData(),(&p_buff[7]),4);
        DWordSwap(A308M_RxData());
        sensor_info = A308M_Value(); p_sensor->YFFT_Str = (uint16)(sensor_info);
        }
    TraceDec2("GetA308mInfo 2",p_sensor->YFFT_Fre, p_sensor->YFFT_Str);
    UsartResetRxTx(USART_ID_TX_RX);    

    if(ServerSendModbusCmd((PUCHAR)A308MCmd_ZFFT_Fre_Str,MODBUS_CMD_NUM) == TRUE)
        {//PrintDataByte("A308MCmdZFre", A308M_RxData(), 8);
        DWordSwap(A308M_RxData()); 
        sensor_info = A308M_Value(); p_sensor->ZFFT_Fre = (uint16)(sensor_info*SCALE_VALUE);
        memcpy(A308M_RxData(),(&p_buff[7]),4);
        DWordSwap(A308M_RxData());
        sensor_info = A308M_Value(); p_sensor->ZFFT_Str = (uint16)(sensor_info);
        }
    TraceDec2("GetA308mInfo 3",p_sensor->ZFFT_Fre, p_sensor->ZFFT_Str);
    UsartResetRxTx(USART_ID_TX_RX); 

 
    TraceDec2("A308M ==>1 ", p_sensor->Xrms,p_sensor->XSpeed);
    TraceDec2("A308M ==>2 ", p_sensor->Yrms,p_sensor->YSpeed);
    TraceDec2("A308M ==>3 ", p_sensor->Zrms,p_sensor->ZSpeed);
    TraceDec2("A308M ==>4 ", p_sensor->Tempature,p_sensor->XFFT_Fre);

    return ret_code;
}




const uchar CmdWaterLevel[MODBUS_CMD_NUM]={0x01, 0x04, 0x00, 0x00, 0x00, 0x02, 0x71, 0xCB}; //to get value of temperature

//
//
//
bool GetWaterLevelInfo()
{//TraceProc();
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
    else PrintDataByte("WaterLevel Err", p_buff, 8);
  
    
    TraceDec2("Water Level ==> ", p_sensor->WaterLevel,p_sensor->OilLevel);
    
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
{//TraceProc();
    bool ret_code = TRUE; 
    
    PSdInfo p_sensor = &ServerInfo.SensorInfo.SdInfo;
    SetNodeInfoSize(_SdInfo);
    SetNodeClass(SENSOR_JNC_SD);
    PUCHAR p_buff = UsartGetBuff(USART_ID_RX);
    
    if(ServerSendModbusCmd((PUCHAR)CmdGetSdAll,MODBUS_CMD_NUM) == TRUE)
      {
        p_sensor->CO2 = WordSwap(*((PUINT16)&p_buff[3]));
        p_sensor->PM25 = WordSwap(*((PUINT16)&p_buff[5]));
        p_sensor->Tempature = WordSwap(*((PUINT16)&p_buff[7]));
        p_sensor->Humidity = WordSwap(*((PUINT16)&p_buff[9]));
        TraceDec2("JNC-SD ==> ", p_sensor->CO2,p_sensor->PM25);
      }
    else PrintDataByte("JNC SD Info Err", p_buff, 12);
    
    return ret_code;
}


const uchar CmdGetIAQSAll[MODBUS_CMD_NUM]  ={0x01, 0x04, 0x00, 0x00, 0x00, 0x09, 0x30, 0x0C}; //to get Co2, Pm2.5, Temp and RH value

//
//
//
bool GetIaqsInfo()
{//TraceProc();
    bool ret_code = TRUE;
    PUINT16 p_sensor_info;
    PIaqsInfo p_sensor = &ServerInfo.SensorInfo.IaqsInfo;
    SetNodeInfoSize(_IaqsInfo);
    SetNodeClass(SENSOR_IAQS);
    PUCHAR p_buff = UsartGetBuff(USART_ID_RX);
    
    if(ServerSendModbusCmd((PUCHAR)CmdGetIAQSAll,MODBUS_CMD_NUM) == TRUE)
      {
        p_sensor_info = (PUINT16)&p_buff[3];
        p_sensor->Tempature = WordSwap(*p_sensor_info); p_sensor_info++;
        p_sensor->Humidity = WordSwap(*p_sensor_info); p_sensor_info++;
        p_sensor->CO2 = WordSwap(*p_sensor_info); p_sensor_info++;
        p_sensor->PM25 = WordSwap(*p_sensor_info); p_sensor_info++;
        p_sensor->HCHO = WordSwap(*p_sensor_info); p_sensor_info++;
        p_sensor->CO = WordSwap(*p_sensor_info); p_sensor_info++;
        p_sensor->TVOC = WordSwap(*p_sensor_info); p_sensor_info++;
        p_sensor->O3 = WordSwap(*p_sensor_info); p_sensor_info++;
        p_sensor->PM10 = WordSwap(*p_sensor_info); p_sensor_info++;
        TraceDec4("IAQS ==> ", p_sensor->CO2,p_sensor->PM25,p_sensor->HCHO,p_sensor->PM10);
      }
    else PrintDataByte("GetIaqsInfo Err", p_buff, 23);
    
    return ret_code;
}

const uchar CmdGetUltraSound[MODBUS_CMD_NUM]  ={0x01, 0x04, 0x03, 0x00, 0x00, 0x04, 0xF1, 0x8D}; //to get Co2, Pm2.5, Temp and RH value

//
//
//
bool GetUltraSoundInfo()
{//TraceProc();
    bool ret_code = TRUE;
    PUINT16 p_sensor_info;
    PUltraSoundInfo p_sensor = &ServerInfo.SensorInfo.UltraSound;
    SetNodeInfoSize(_IaqsInfo);
    SetNodeClass(SENSOR_ULTRA_SOUND);
    PUCHAR p_buff = UsartGetBuff(USART_ID_RX);
    
    if(ServerSendModbusCmd((PUCHAR)CmdGetUltraSound,MODBUS_CMD_NUM) == TRUE)
      {
        p_sensor_info = (PUINT16)&p_buff[3];
        p_sensor->Distance = WordSwap(*p_sensor_info); p_sensor_info++;
        p_sensor->SetDistance = WordSwap(*p_sensor_info); p_sensor_info++;
        p_sensor->Distance100 = WordSwap(*p_sensor_info); p_sensor_info++;
        p_sensor->Distance200 = WordSwap(*p_sensor_info); p_sensor_info++;
        p_sensor->Distance300 = WordSwap(*p_sensor_info); p_sensor_info++;
        p_sensor->Distance400 = WordSwap(*p_sensor_info); p_sensor_info++;
        p_sensor->Distance500 = WordSwap(*p_sensor_info); p_sensor_info++;
        p_sensor->Distance600 = WordSwap(*p_sensor_info); p_sensor_info++;
        p_sensor->Distance700 = WordSwap(*p_sensor_info); p_sensor_info++;
        p_sensor->Distance800 = WordSwap(*p_sensor_info); p_sensor_info++;
        TraceDec4("UltraSound ==> ", p_sensor->Distance,p_sensor->SetDistance,p_sensor->Distance100,p_sensor->Distance200);
      }
    else PrintDataByte("GetIaqsInfo Err", p_buff, 23);
    
    return ret_code;
}

//
//Send sensor information to client
//
bool SendInfoToClient()
{//TraceProc();
    bool ret_code=FALSE;
    uchar   send_size;
    if(ServerInfo.NodeInfoSize > 0)
      {
        send_size = ServerInfo.NodeInfoSize+3;
        ServerInfo.ProperityID = NODE_GET_ALL_SENSOR;
        //PrintDataByte("SendInfoToClient", (PUCHAR)&ServerInfo, send_size);
        
        result = Cmd_ms_server_send_status(SENSOR_ELEMENT, pNodeEventInfo->ClientAddr,pNodeEventInfo->AppkeyIndex,
                                            NO_FLAGS, send_size, (PUCHAR)&ServerInfo)->result;
      if(result) {ret_code = FALSE; TraceErr1("SendInfoToClient 1", result);}
      else ret_code=TRUE;
        
      }
    else TraceErr1("SendInfoToClient 2",ServerInfo.NodeInfoSize);
    
    return ret_code;

}

//
//
//
void EvtGetRequestProc(PCmdPacket pCmdEvent)
{//TraceProc();
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
    else
    {//
        TraceErr1("EvtGetRequestProc",pNodeEventInfo->PropertyID);
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
{//TraceProc();
    bool ret_code = TRUE;
    UsartTxSendCmd(modbus_cmd,len); 
    Delay_ms(10); Rs485Rx();
    Delay_ms(150); Rs485Tx(); // must to check crc error
    ret_code = CheckModbusCrc(UsartGetBuff(USART_ID_RX), UsartGetRxCounter());

    if(!ret_code) TraceErr1("ServerSendModbusCmd ",UsartGetRxCounter());
    
    return ret_code;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




//result = Cmd_ms_client_set_setting(SENSOR_ELEMENT, pCmd->ModbusID, IGNORED, 0x00, MODBUS_GET_REGS_VALUE,
//                                   6, 4, (uint8 *)&setting_data)->result;


//
// for Set setting event
//
void EvtSetSettingRequestProc(PCmdPacket pCmdEvent)
{//TraceProc();
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
{//TraceProc();
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
{//TraceProc();
    uchar power = POWER_USB;

    if(GetBatteryPower() > 0) power = POWER_BATTERY;

    return power;
}

