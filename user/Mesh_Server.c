
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
const uchar AipPowerCtrlCmd[5][8]=
{
    {0x01, 0x06, 0x00, 0x02, 0x00, 0x00, 0x28, 0x0A},   // 00%
    {0x01, 0x06, 0x00, 0x02, 0x00, 0x19, 0xE9, 0xC0},   // 25%
    {0x01, 0x06, 0x00, 0x02, 0x00, 0x32, 0xA9, 0xDF},   // 50%
    {0x01, 0x06, 0x00, 0x02, 0x00, 0x4B, 0x68, 0x3D},   // 75%
    {0x01, 0x06, 0x00, 0x02, 0x00, 0x64, 0x29, 0xE1},   // 100%
};


const uchar AgbPowerCtrlCmd[5][8]=
{
    {0x01, 0x06, 0x00, 0x01, 0x00, 0x00, 0xD8, 0x0A},   // 00%
    {0x01, 0x06, 0x00, 0x01, 0x00, 0x01, 0x19, 0xCA},   // 25%
    {0x01, 0x06, 0x00, 0x01, 0x00, 0x02, 0x59, 0xCB},   // 50%
    {0x01, 0x06, 0x00, 0x01, 0x00, 0x03, 0x98, 0x0B},   // 75%
    {0x01, 0x06, 0x00, 0x01, 0x00, 0x04, 0xD9, 0xC9},   // 100%
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

const uchar PT485Temp[MODBUS_CMD_NUM]={0x01, 0x04, 0x00, 0x00, 0x00, 0x01, 0x31, 0xCA}; //to get value of temperature

const uchar AipPower[MODBUS_CMD_NUM]={0x01, 0x04, 0x00, 0x04, 0x00, 0x01,0x70, 0x0B};
const uchar AipPowerStatus[MODBUS_CMD_NUM]={0x01, 0x04, 0x00, 0x01, 0x00, 0x01, 0x60, 0x0A};

// For BT Mesh Modbus Reg
const uchar CmdBtA308M_1X[MODBUS_CMD_NUM] ={0x01, 0x03, 0x06, 0x00, 0x00, 0x0B, 0x04, 0x85};
const uchar CmdBtA308M_1Y[MODBUS_CMD_NUM] ={0x01, 0x03, 0x06, 0x0C, 0x00, 0x0B, 0xC4, 0x86};
const uchar CmdBtA308M_1Z[MODBUS_CMD_NUM] ={0x01, 0x03, 0x06, 0x18, 0x00, 0x0B, 0x84, 0x82};

const uchar CmdBtA308M_2X[MODBUS_CMD_NUM] ={0x01, 0x03, 0x06, 0x26, 0x00, 0x04, 0xA5, 0x4A};
const uchar CmdBtA308M_2Y[MODBUS_CMD_NUM] ={0x01, 0x03, 0x06, 0x32, 0x00, 0x04, 0xE5, 0x4E};
const uchar CmdBtA308M_2Z[MODBUS_CMD_NUM] ={0x01, 0x03, 0x06, 0x3E, 0x00, 0x04, 0x25, 0x4D};
const uchar CmdA308MTemp[MODBUS_CMD_NUM] ={0x01, 0x03, 0x06, 0x24, 0x00, 0x02, 0x84, 0x88}; 

const uchar CmdWaterLevel[MODBUS_CMD_NUM]={0x01, 0x04, 0x00, 0x00, 0x00, 0x02, 0x71, 0xCB};

const uchar CmdGetSdCo2[MODBUS_CMD_NUM] ={0x01, 0x04, 0x03, 0x00, 0x00, 0x01, 0x31, 0x8E};
const uchar CmdGetSdPm25[MODBUS_CMD_NUM]={0x01, 0x04, 0x03, 0x01, 0x00, 0x01, 0x60, 0x4E};
const uchar CmdGetSdTemp[MODBUS_CMD_NUM]={0x01, 0x04, 0x03, 0x02, 0x00, 0x01, 0x90, 0x4E};
const uchar CmdGetSdRh[MODBUS_CMD_NUM]  ={0x01, 0x04, 0x03, 0x03, 0x00, 0x01, 0xC1, 0x8E};
const uchar CmdGetSdAll[MODBUS_CMD_NUM]  ={0x01, 0x04, 0x03, 0x00, 0x00, 0x04, 0xF1, 0x8D};

const uchar CmdDoRealValue[MODBUS_CMD_NUM] ={0x01, 0x04, 0x00, 0x00, 0x00, 0x01, 0x31, 0xCA};
const uchar CmdDoRealOffset[MODBUS_CMD_NUM]={0x01, 0x04, 0x00, 0x02, 0x00, 0x01, 0x90, 0x0A};
const uchar CmdDoTempValue[MODBUS_CMD_NUM] ={0x01, 0x04, 0x00, 0x0A, 0x00, 0x01, 0x11, 0xC8};
const uchar CmdDoTempOffset[MODBUS_CMD_NUM]={0x01, 0x04, 0x00, 0x0B, 0x00, 0x01, 0x40, 0x08};

const uchar CmdA6D6_AI[MODBUS_CMD_NUM] ={0x01, 0x04, 0x00, 0x00, 0x00, 0x08, 0xF1, 0xCC};
const uchar CmdA6D6_DI[MODBUS_CMD_NUM] ={0x01, 0x02, 0x00, 0x00, 0x00, 0x08, 0x79, 0xCC};
const uchar CmdA6D6_DO[MODBUS_CMD_NUM] ={0x01, 0x01, 0x00, 0x00, 0x00, 0x08, 0x3D, 0xCC};

const uchar Fun3Addr00[MODBUS_CMD_NUM] ={0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A};
const uchar Fun3Addr01[MODBUS_CMD_NUM] ={0x01, 0x03, 0x00, 0x01, 0x00, 0x01, 0xD5, 0xCA};
const uchar Fun3Addr02[MODBUS_CMD_NUM] ={0x01, 0x03, 0x00, 0x02, 0x00, 0x01, 0x25, 0xCA};
const uchar Fun3Addr03[MODBUS_CMD_NUM] ={0x01, 0x03, 0x00, 0x03, 0x00, 0x01, 0x74, 0x0A};
const uchar Fun3Addr0A[MODBUS_CMD_NUM] ={0x01, 0x03, 0x00, 0x0A, 0x00, 0x01, 0xA4, 0x08};
const uchar CmdPzemValue[MODBUS_CMD_NUM] ={0x01, 0x04, 0x00, 0x00, 0x00, 0x0A, 0x70, 0x0D};


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
    ServerSetPowerStatus();
}

//
//
void ServerSetPowerStatus()
{
    if(CheckPowerStatus() == POWER_USB){
        SetNodeStatus(NS_FULL_POWER,ON);   // start to get sensor info 
      }
    else{
       SetNodeStatus(NS_FULL_POWER,OFF);   // start to get sensor info
      }
    if(GetNodeStatus(NS_FULL_POWER) == ON) Trace("Full Power");
    else Trace("Battery Power");
    
}

void ServerSetupNodeInit()
{
    ServerNodeInit();
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
             default: ret_code=FALSE;  break;
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


uint16 AgbPowerStatus=MODBUS_AGB_POWER_00;

Bool AGBSetCtrl(uint16 property_id)
{
    bool ret_code=TRUE;
    uint16 power_status;
    const uchar *p_modbus_cmd=NULL;

    if(property_id >= NODE_SET_AIP_POWER_OFF && property_id <= NODE_SET_AIP_POWER_100)
        {
         switch(property_id)
            {
             case NODE_SET_AIP_POWER_OFF: 
                p_modbus_cmd = AGB_POWER_CTRL_CMD(AIP_POWER_CTRL_00);
                power_status = MODBUS_AGB_POWER_00;
                break;
             case NODE_SET_AIP_POWER_25: 
                p_modbus_cmd = AGB_POWER_CTRL_CMD(AIP_POWER_CTRL_25);
                power_status = MODBUS_AGB_POWER_25;
                break;
             case NODE_SET_AIP_POWER_50: 
                p_modbus_cmd = AGB_POWER_CTRL_CMD(AIP_POWER_CTRL_50);
                power_status = MODBUS_AGB_POWER_50;
                break;
             case NODE_SET_AIP_POWER_75: 
                p_modbus_cmd = AGB_POWER_CTRL_CMD(AIP_POWER_CTRL_75);
                power_status = MODBUS_AGB_POWER_75;
                break;
             case NODE_SET_AIP_POWER_100: 
                p_modbus_cmd = AGB_POWER_CTRL_CMD(AIP_POWER_CTRL_100);
                power_status = MODBUS_AGB_POWER_100;
                break;                
             default: ret_code=FALSE;  break;
            };
            
        }
    else {ret_code=FALSE;}

    if(p_modbus_cmd != NULL) 
      {
       if(ServerSendModbusCmd((PUCHAR)p_modbus_cmd,MODBUS_CMD_NUM) == TRUE)
            ServerInfo.SensorInfo.AgbPower.PowerStatus = power_status;
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
             default: ret_code=FALSE; 
                break;
            };
        }
    else {ret_code=FALSE;}

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
         Trace16_1(property_id); PrintDataByte("A6D6MSetCtrl 1", (PUCHAR)p_modbus_cmd, 8);
        }
    else if((property_id >= NODE_SET_DO1_OFF) && (property_id <= NODE_SET_DO6_OFF))
        {
         p_modbus_cmd = (PUCHAR)&A6D6CtrlCmdOff[property_id-NODE_SET_DO1_OFF][0];
         Trace16_1(property_id); PrintDataByte("A6D6MSetCtrl 2", (PUCHAR)p_modbus_cmd, 8);
        }
    else {TraceErr1("A6D6MSetCtrl 3 property_id = ",property_id);}

    if(p_modbus_cmd != NULL) 
      {
       if(ServerSendModbusCmd((PUCHAR)p_modbus_cmd,MODBUS_CMD_NUM) == TRUE)
        {TraceOk("A6D6MSetCtrl");}
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
            case SNS_SET_WAITING:   
                if(GetNodeStatus(NS_SET_NODE_ACT) == TRUE)  {ToNextStage(SNS_SET_INFO_PRE);}
                break;
                
            case SNS_SET_INFO_PRE:  
                ret_code=FALSE;
                property_id = pNodeEventInfo->PropertyID;

                if(pFunSensor == GetAipInfo)
                    {Trace("Set AIP");
                     ret_code = AIPSetCtrl(property_id); ToWaitingStage(SNS_SET_WAITING_INFO,5);
                    }
                else  if(pFunSensor == GetA6D6Info)
                    {Trace("Set A6D6");
                     ret_code = A6D6MSetCtrl(property_id); ToWaitingStage(SNS_SET_WAITING_INFO,5);
                    }
                else  if(pFunSensor == GetAgbPower)
                    {Trace("Set AGB Power");
                     ret_code = AGBSetCtrl(property_id); ToWaitingStage(SNS_SET_WAITING_INFO,5);
                    }
                
                else{ToNextStage(SNS_SET_INFO_END);}
                
                if(ret_code == FALSE) {ToWaitingStage(SNS_SET_WAITING_INFO,1);}
                break;               
            case SNS_SET_WAITING_INFO:  
                if(CheckWaitTimeOut()) ToNextStage(SNS_SET_INFO_OK);
                break;
                
            case SNS_SET_INFO_OK:   Trace("SNS_SET_INFO_OK");
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
            default: TraceErr1("ClientSetNodeInfoProc",ActiveStage()); break;
        }
    
}



void ServerGetInfoActionNow()
{
    PNodeStageInfo p_stage_info;
    p_stage_info = GetNodeStageInfo(SERVER_GET_INFO_PROC);
    p_stage_info->Timer = WAIT_SEC(1);
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
                GetInfoCycle = WAIT_SEC(TIMER_GET_INFO_SLEEPING);
                ToNextStage(SNS_PRE_WAITING);   //default
                break;
            case SNS_PRE_WAITING: 
                ToWaitingStage(SNS_GET_INFO,GetDeviceInfoDelay); 
                CountErr = 0;
                break;
            case SNS_GET_INFO: 
                if(CheckWaitTimeOut() == TRUE) 
                    {
                    if((pFunSensor!= NULL) && (GetSensorInfo() == TRUE) )
                        {ToNextStage(SNS_EVENT_WAITING);}
                    else 
                        {
                         ToWaitingStage(SNS_PRE_SLEEPING,TIMER_WAIT_SLEEPING);
                        }
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
            
                if(SendInfoToClient()) 
                    {
                     SetNodeStatus(NS_GET_INFO_ACT,OFF);
                     ToWaitingStage(SNS_PRE_SLEEPING,TIMER_WAIT_SLEEPING);
                    }   // waiting to sleeping
                else{
                     CountErr++; ToWaitingStage(SNS_PRE_SEND_INFO,WAIT_SEC(2)); // send info again
                    }                    
                break;
            case SNS_PRE_SLEEPING: 

                if(CheckWaitTimeOut())
                    {
                     //if(GetNodeStatus(NS_FULL_POWER) == ON)
                     if(GetNodeStatus(NS_FULL_POWER) == ON || GetNodeStatus(NS_FORCE_FULL_POWER) == ON)
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



//
// Get sensor info and update
//
bool GetSensorInfo()
{
    bool ret_code = FALSE;
    pSensorHeader->BatteryPower = GetBatteryPower(); 
    if(pNodeEventInfo->PropertyID == NODE_GET_BTM_INFO)
        {
         ret_code = GetBtmMeshInfo();
        }
    else if(pFunSensor) 
        {
         ret_code = pFunSensor(); 
         UsartResetRxTx(USART_ID_TX_RX);
        }
    return ret_code;
}





//
// for Property ID = NODE_GET_BTM_INFO
//
bool GetBtmMeshInfo()
{TraceProc();
 bool ret_code = TRUE;

 PBtMeshInfo p_sensor = &ServerInfo.SensorInfo.BtmMeshInfo;
 SetNodeInfoSize(_BtMeshInfo);
 SetNodeClass(SENSOR_BTM_MESH_INFO);


 memcpy(p_sensor->ModelName,MODEL_NAME,6);
 p_sensor->Version = FW_VER;
 p_sensor->TempGain = (int16)((pAdjValue->TempGain)*MESH_INFO_SCALING); 
 p_sensor->TempOffset = (int16)((pAdjValue->TempOffset)*MESH_INFO_SCALING);
 
 p_sensor->RhGain = (int16)((pAdjValue->HumGain)*MESH_INFO_SCALING); 
 p_sensor->RhOffset = (int16)((pAdjValue->HumOffset)*MESH_INFO_SCALING); 

 p_sensor->WorkingTime = pMeshNodeData->WorkingTimer; //60;
 p_sensor->BtmClass = pMeshNodeData->SensorClass; //0x01;

 return ret_code; 
}



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
    if(GetTempAndRH(&temp,&Humidity) != VALUE_IS_NOT_KNOWN){
         p_sensor->Tempature = (int16)(((float)temp)*pAdjValue->TempGain + (pAdjValue->TempOffset));
         p_sensor->Humidity  = (uint16)(((float)Humidity)*pAdjValue->HumGain + (pAdjValue->HumOffset));
        }
    else TraceErr("GetSi7021Info");
    return ret_code;
}


const uchar CmdGetCo2[7] ={0xFE, 0x44, 0x00, 0x08, 0x02, 0x9F, 0x25};


//
//
//
bool GetSkynetCo2Info()
{//TraceProc();
    bool ret_code = TRUE;
    PUINT16 p_value,p_sensor_value;
    uint16 loop;
    PSkynetCo2 p_sensor = &ServerInfo.SensorInfo.SkynetCo2;
    GetSi7021Info();
    SetNodeInfoSize(_SkynetCo2);
    SetNodeClass(SENSOR_SKYNET_CO2);
    PUCHAR p_buff = UsartGetBuff(USART_ID_RX);
    if(ServerSendModbusCmd((PUCHAR)CmdGetCo2,7) == TRUE){
        p_sensor->Co2 = WordSwap(*((PUINT16)&p_buff[3])); 
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



#define SCALE_VALUE         10
#define SCALE_VALUE_100     100

#define A308M_RxData()    (&p_buff[3])
#define A308M_Value()   (*((float*)A308M_RxData()))

float sensor_info;


//
// For JNC Demo
//
bool GetA308mInfo()
{TraceProc();
    bool ret_code = TRUE;
    //float sensor_info;
    PUCHAR p_buff;
    PA308mInfo p_sensor = &ServerInfo.SensorInfo.A308mInfo;
    SetNodeInfoSize(_A308mInfo);
    SetNodeClass(SENSOR_A308M);
    p_buff = UsartGetBuff(USART_ID_RX);
    if(ServerSendModbusCmd((PUCHAR)CmdBtA308M_1X,MODBUS_CMD_NUM) == TRUE)
        {p_buff +=3; 
         DWordSwapN(p_buff,6);          
         p_buff +=4; // for Mean
         sensor_info = *((float *)p_buff); p_sensor->RmsX = (uint16)sensor_info; p_buff+=4;
         sensor_info = *((float *)p_buff); p_sensor->SkewnessX = (int16)(sensor_info*SCALE_VALUE); p_buff+=4;
         sensor_info = *((float *)p_buff); p_sensor->KurtosisX = (int16)(sensor_info*SCALE_VALUE); p_buff +=8; // to Speed
         sensor_info = *((float *)p_buff); p_sensor->SpeedX = (int16)(sensor_info*SCALE_VALUE);
        };
     UsartResetRxTx(USART_ID_TX_RX); 

     p_buff = UsartGetBuff(USART_ID_RX);
    if(ServerSendModbusCmd((PUCHAR)CmdBtA308M_2X,MODBUS_CMD_NUM) == TRUE)
        {p_buff +=3;          
         DWordSwapN(p_buff,2);          
         sensor_info = *((float *)p_buff); p_sensor->FrequencyX = (uint16)sensor_info; p_buff+=4;
         sensor_info = *((float *)p_buff); p_sensor->StengthX = (uint16)(sensor_info); p_buff+=4;
        };
     UsartResetRxTx(USART_ID_TX_RX); 

    p_buff = UsartGetBuff(USART_ID_RX);
    if(ServerSendModbusCmd((PUCHAR)CmdBtA308M_1Y,MODBUS_CMD_NUM) == TRUE)
        {p_buff +=3;          
         DWordSwapN(p_buff,6);          
         p_buff +=4; // for Mean
         sensor_info = *((float *)p_buff); p_sensor->RmsY = (uint16)sensor_info; p_buff+=4;
         sensor_info = *((float *)p_buff); p_sensor->SkewnessY = (int16)(sensor_info*SCALE_VALUE); p_buff+=4;
         sensor_info = *((float *)p_buff); p_sensor->KurtosisY = (int16)(sensor_info*SCALE_VALUE); p_buff +=8; // to Speed
         sensor_info = *((float *)p_buff); p_sensor->SpeedY = (int16)(sensor_info*SCALE_VALUE);         
        };
     UsartResetRxTx(USART_ID_TX_RX); 

     p_buff = UsartGetBuff(USART_ID_RX);
    if(ServerSendModbusCmd((PUCHAR)CmdBtA308M_2Y,MODBUS_CMD_NUM) == TRUE)
        {p_buff +=3; 
         DWordSwapN(p_buff,2);          
         sensor_info = *((float *)p_buff); p_sensor->FrequencyY = (uint16)sensor_info; p_buff+=4;
         sensor_info = *((float *)p_buff); p_sensor->StengthY = (uint16)(sensor_info); p_buff+=4;
        };
     UsartResetRxTx(USART_ID_TX_RX); 

    p_buff = UsartGetBuff(USART_ID_RX);
    if(ServerSendModbusCmd((PUCHAR)CmdBtA308M_1Z,MODBUS_CMD_NUM) == TRUE)
        {p_buff +=3;          
         DWordSwapN(p_buff,6);       
         p_buff +=4; // for Mean
         sensor_info = *((float *)p_buff); p_sensor->RmsZ = (uint16)sensor_info; p_buff+=4;
         sensor_info = *((float *)p_buff); p_sensor->SkewnessZ = (int16)(sensor_info*SCALE_VALUE); p_buff+=4;
         sensor_info = *((float *)p_buff); p_sensor->KurtosisZ = (int16)(sensor_info*SCALE_VALUE); p_buff +=8; // to Speed
         sensor_info = *((float *)p_buff); p_sensor->SpeedZ = (int16)(sensor_info*SCALE_VALUE);         
        };
     UsartResetRxTx(USART_ID_TX_RX); 

     p_buff = UsartGetBuff(USART_ID_RX);
    if(ServerSendModbusCmd((PUCHAR)CmdBtA308M_2Z,MODBUS_CMD_NUM) == TRUE)
        {p_buff +=3;          
         DWordSwapN(p_buff,2);          
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
    return ret_code;
}




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
    else PrintDataByte("WaterLevel Err", p_buff, 8);
  
    
    TraceDec2("Water Level ==> ", p_sensor->WaterLevel,p_sensor->OilLevel);
    
    return ret_code;
}


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
        TraceDec4("JNC-SD ==> ", CO2,PM25,Tempature,Humidity);
      }
    else PrintDataByte("JNC SD Info Err", p_buff, 12);
    
    return ret_code;
}


//const uchar CmdGetUltraSound[MODBUS_CMD_NUM]={0x01, 0x04, 0x00, 0x00, 0x00, 0x01, 0x31, 0xCA}; //to get value of temperature
const uchar CmdGetUltraSound[MODBUS_CMD_NUM]={0x01, 0x04, 0x00, 0x00, 0x00, 0x02, 0x71, 0xCB};
const uchar CmdGetUltraSoundOther[MODBUS_CMD_NUM]={0x01, 0x04, 0x00, 0x1D, 0x00, 0x05, 0xA0, 0x0F};


//
//
//
bool GetUltraSoundInfo()
{
bool ret_code = TRUE; 

PUltraSoundInfo p_sensor = &ServerInfo.SensorInfo.UltraSound;
SetNodeInfoSize(_UltraSoundInfo);
SetNodeClass(SENSOR_ULTRA_SOUND);
PUCHAR p_buff = UsartGetBuff(USART_ID_RX);
if(ServerSendModbusCmd((PUCHAR)CmdWaterLevel,MODBUS_CMD_NUM) == TRUE)
  {
    p_sensor->WaterLevel = WordSwap(*((PUINT16)&p_buff[3]));
    p_sensor->OilLevel = WordSwap(*((PUINT16)&p_buff[5]));
  }
else PrintDataByte("Ultra Sound Err", p_buff, 8);
UsartResetRxTx(USART_ID_TX_RX);
if(ServerSendModbusCmd((PUCHAR)CmdGetUltraSoundOther,MODBUS_CMD_NUM) == TRUE)
  {
    p_sensor->BatteryVol = WordSwap(*((PUINT16)&p_buff[3]));
    p_sensor->InputVol = WordSwap(*((PUINT16)&p_buff[5]));
    p_sensor->OutputVol = WordSwap(*((PUINT16)&p_buff[7]));
    p_sensor->ChargeCurr = WordSwap(*((PUINT16)&p_buff[9]));
    p_sensor->InputCurr = WordSwap(*((PUINT16)&p_buff[11]));
  }
else PrintDataByte("Ultra Sound Other Err", p_buff, 8);

PrintDataByte("GetUltraSoundInfo", (PUCHAR)p_sensor, sizeof(_UltraSoundInfo));


return ret_code;
}


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
else PrintDataByte("GetDo485 Err 1", p_buff, 8);
UsartResetRxTx(USART_ID_TX_RX);

if(ServerSendModbusCmd((PUCHAR)CmdDoRealOffset,MODBUS_CMD_NUM) == TRUE)
  {
    p_sensor->DoOffsetValue = WordSwap(*((PUINT16)&p_buff[3]));
  }
else PrintDataByte("GetDo485 Err 2", p_buff, 8);
UsartResetRxTx(USART_ID_TX_RX);

if(ServerSendModbusCmd((PUCHAR)CmdDoTempValue,MODBUS_CMD_NUM) == TRUE)
  {
    p_sensor->TempRealValue = WordSwap(*((PUINT16)&p_buff[3]));
  }
else PrintDataByte("GetDo485 Err 3", p_buff, 8);
UsartResetRxTx(USART_ID_TX_RX);

if(ServerSendModbusCmd((PUCHAR)CmdDoTempOffset,MODBUS_CMD_NUM) == TRUE)
  {
    p_sensor->TempOffsetValue = WordSwap(*((PUINT16)&p_buff[3]));
  }
else PrintDataByte("GetDo485 Err 4", p_buff, 8);
UsartResetRxTx(USART_ID_TX_RX);


TraceDec2("D0-485 1",p_sensor->DoRealValue ,p_sensor->DoOffsetValue);
TraceDec2("D0-485 2",p_sensor->TempRealValue, p_sensor->TempOffsetValue);

return ret_code;
}


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
  {//PrintDataByte("Get A6D6 Ai", &A6D6_AI_VALUE, 16);
    p_sensor->AiValue1 = WordSwap(*p_value++); p_sensor->AiValue2 = WordSwap(*p_value++);    
    p_sensor->AiValue3 = WordSwap(*p_value++); p_sensor->AiValue4 = WordSwap(*p_value++);
    p_sensor->AiValue5 = WordSwap(*p_value++); p_sensor->AiValue6 = WordSwap(*p_value++);
    p_sensor->AiValue7 = WordSwap(*p_value++); p_sensor->AiValue8 = WordSwap(*p_value++);

    //memcpy(&(p_sensor->AiValue1),&A6D6_AI_VALUE,16);
  }
else PrintDataByte("CmdA6D6_AI Err", p_buff, 8);
UsartResetRxTx(USART_ID_TX_RX);

if(ServerSendModbusCmd((PUCHAR)CmdA6D6_DI,MODBUS_CMD_NUM) == TRUE)
  {
    p_sensor->Di_Status = A6D6_DI_VALUE;
  }
else PrintDataByte("CmdA6D6_DI Err", p_buff, 8);
UsartResetRxTx(USART_ID_TX_RX);

if(ServerSendModbusCmd((PUCHAR)CmdA6D6_DO,MODBUS_CMD_NUM) == TRUE)
  {
    p_sensor->DO_Status = A6D6_DO_VALUE;
  }
else PrintDataByte("CmdA6D6_DO Err", p_buff, 8);
UsartResetRxTx(USART_ID_TX_RX);
PrintData("Get A6D6 Value", (PUINT16)(p_sensor), 9);

return ret_code;
}




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
else {PrintDataByte("GetPzem Err 1", p_buff, 8);ret_code = FALSE;}
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
//
//
bool GetOemSensor()
{
bool ret_code = TRUE;
POemSensor p_sensor = &ServerInfo.SensorInfo.OemSensor;
SetNodeInfoSize(_OemSensor);
SetNodeClass(SENSOR_OEM);

PUCHAR p_buff = UsartGetBuff(USART_ID_RX);
if(ServerSendModbusCmd((PUCHAR)Fun3Addr00,MODBUS_CMD_NUM) == TRUE)
  {
    p_sensor->Addr00 = WordSwap(*((PUINT16)&p_buff[3]));
  }
else PrintDataByte("GetOemSensor Err 1", p_buff, 8);
UsartResetRxTx(USART_ID_TX_RX);

if(ServerSendModbusCmd((PUCHAR)Fun3Addr01,MODBUS_CMD_NUM) == TRUE)
  {
    p_sensor->Addr01 = WordSwap(*((PUINT16)&p_buff[3]));
  }
else PrintDataByte("GetOemSensor Err 2", p_buff, 8);
UsartResetRxTx(USART_ID_TX_RX);

if(ServerSendModbusCmd((PUCHAR)Fun3Addr02,MODBUS_CMD_NUM) == TRUE)
  {
    p_sensor->Addr02 = WordSwap(*((PUINT16)&p_buff[3]));
  }
else PrintDataByte("GetOemSensor Err 3", p_buff, 8);
UsartResetRxTx(USART_ID_TX_RX);

if(ServerSendModbusCmd((PUCHAR)Fun3Addr03,MODBUS_CMD_NUM) == TRUE)
  {
    p_sensor->Addr03 = WordSwap(*((PUINT16)&p_buff[3]));
  }
else PrintDataByte("GetOemSensor Err 4", p_buff, 8);
UsartResetRxTx(USART_ID_TX_RX);

if(ServerSendModbusCmd((PUCHAR)Fun3Addr0A,MODBUS_CMD_NUM) == TRUE)
  {
    p_sensor->Addr0A = WordSwap(*((PUINT16)&p_buff[3]));
  }
else PrintDataByte("GetOemSensor Err A", p_buff, 8);
UsartResetRxTx(USART_ID_TX_RX);


TraceDec2("GetOemSensor 1",p_sensor->Addr00 ,p_sensor->Addr01);
TraceDec3("GetOemSensor 2",p_sensor->Addr02, p_sensor->Addr03,p_sensor->Addr0A);

return ret_code;
}


//
//
//
bool GetAgbPower()
{
bool ret_code = TRUE;
PAgbPower p_sensor = &ServerInfo.SensorInfo.AgbPower;
SetNodeInfoSize(_AgbPower);
SetNodeClass(SENSOR_AGB_POWER);

//p_sensor->PowerStatus = AgbPowerStatus;

return ret_code;
}

const uchar CmdGetIaqsCw9Info[MODBUS_CMD_NUM]  ={0x01, 0x04, 0x03, 0x30, 0x00, 0x11, 0x30, 0x4D}; //to get Co2, Pm2.5, Temp and RH value

#define CW9_DATA_LEN        17
#define IAQS_DATA_LEN       9


//
//
//
bool GetIaqsInfo()
{
bool ret_code = TRUE;
PUINT16 p_value,p_sensor_value;
uint16 loop;

PIaqsInfo p_sensor = &ServerInfo.SensorInfo.IaqsInfo;
SetNodeInfoSize(_IaqsInfo);
SetNodeClass(SENSOR_IAQS);

PUCHAR p_buff = UsartGetBuff(USART_ID_RX); 
p_value = (PUINT16)(&p_buff[3]); p_sensor_value=(PUINT16)p_sensor;
if(ServerSendModbusCmd((PUCHAR)CmdGetIaqsCw9Info,MODBUS_CMD_NUM) == TRUE)
  {
    for(loop=0; loop<IAQS_DATA_LEN; loop++) 
        *p_sensor_value++ = WordSwap(*p_value++);    
  }
else {ret_code = FALSE;}

return ret_code;
}


//
//
//
bool GetCw9Info()
{
bool ret_code = TRUE;
PUINT16 p_value,p_sensor_value;
uint16 loop;

PCw9Info p_sensor = &ServerInfo.SensorInfo.Cw9Info;
SetNodeInfoSize(_Cw9Info);
SetNodeClass(SENSOR_CW9);

PUCHAR p_buff = UsartGetBuff(USART_ID_RX); 
p_value = (PUINT16)(&p_buff[3]); p_sensor_value=(PUINT16)p_sensor;
if(ServerSendModbusCmd((PUCHAR)CmdGetIaqsCw9Info,MODBUS_CMD_NUM) == TRUE)
  {
    for(loop=0; loop<CW9_DATA_LEN; loop++) 
       { *p_sensor_value++ = WordSwap(*p_value++); }
    
  }
else {ret_code = FALSE;}

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
        if(GetNodeStatus(NS_FULL_POWER) == ON)
           pSensorHeader->Status |= SERVER_FULL_POWER;
        else
           pSensorHeader->Status &= ~SERVER_FULL_POWER; 
        
        send_size = ServerInfo.NodeInfoSize+3;
        ServerInfo.ProperityID = pNodeEventInfo->PropertyID;
        result = Cmd_ms_server_send_status(SENSOR_ELEMENT, pNodeEventInfo->ClientAddr,pNodeEventInfo->AppkeyIndex,
                                            NO_FLAGS, send_size, (PUCHAR)&ServerInfo)->result;
      if(result) {ret_code = FALSE; TraceErr1("SendInfoToClient 1", result);}
      else ret_code=TRUE;

      //PrintDataByte("Send To Client",(BYTE*)&ServerInfo ,send_size); // for Debug
        
      }
    else TraceErr1("SendInfoToClient 2",ServerInfo.NodeInfoSize);
    
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
    if(pEvent->property_id == NODE_GET_ALL_SENSOR)    
     {
         SetNodeStatus(NS_GET_INFO_ACT,ON);  // Mesh get event active
     }
    else if(pEvent->property_id == NODE_GET_INFO_FULL_POWER_ON)    
     {  
        SetNodeStatus(NS_GET_INFO_ACT,ON);
        SetForceFullPowerTime(ON);
        SetNodeStatus(NS_FULL_POWER,ON);   // start to get sensor info 
        pSensorHeader->Status |= SERVER_FULL_POWER;
         
     }
    else if(pEvent->property_id == NODE_GET_INFO_FULL_POWER_OFF)    
     { 
         SetNodeStatus(NS_GET_INFO_ACT,ON);
         SetForceFullPowerTime(OFF);
     }    
    else if(pEvent->property_id >= NODE_SET_AIP_POWER_OFF && pEvent->property_id <= NODE_SET_AIP_POWER_100)
     {
         SetNodeStatus(NS_SET_NODE_ACT,ON);  // Mesh set event active
     }    
    else if(pEvent->property_id >= NODE_SET_DO1_ON && pEvent->property_id <= NODE_SET_DO6_OFF)
    {
         SetNodeStatus(NS_SET_NODE_ACT,ON);  // Mesh set event active
    }    
    else if(pEvent->property_id == NODE_GET_BTM_INFO)
    {
         SetNodeStatus(NS_GET_INFO_ACT,ON);  // Mesh set event active
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
{
    bool ret_code = TRUE;
    UsartTxSendCmd(modbus_cmd,len); 
    Delay_ms(10); Rs485Rx(); Delay_ms(150); Rs485Tx(); // must to check crc error
    ret_code = CheckModbusCrc(UsartGetBuff(USART_ID_RX), UsartGetRxCounter());

    if(!ret_code) TraceErr1("ServerSendModbusCmd 1",UsartGetRxCounter());
    
    return ret_code;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define ALL_SETTING_ID                  0x01
#define TEMP_GAIN_SETTING_ID            0x02
#define TEMP_OFFSET_SETTING_ID          0x03
#define RH_GAIN_SETTING_ID              0x04
#define RH_OFFSET_SETTING_ID            0x05
#define WORKING_TIME_SETTING_ID         0x06
#define SENSOR_CLASS_SETTING_ID         0x07

#define SET_FULL_POWER_ON               0x10
#define SET_FULL_POWER_OFF              0x11


typedef struct _BtAppData_
{
    int16   TempGain,TempOffset;    // Tempature Gain & Offset  
    int16   RhGain,RhOffset;        // RH Gain & Offset
    uint16  WorkingTimer;            // >5 and <3600 sec
    uint16  BtmClass;               //1 : for JNC Sensor(Auto Scan) 2 : PZEM 3 : Visual Sensor 4 : AGB Motor Control(恆達) 
}_BtAppData,*PBtAppData;


//
// for Set setting event
//
void EvtSetSettingRequestProc(PCmdPacket pCmdEvent)
{TraceProc();
    msg_ms_setup_server_set_setting_request_evt *pEvent = &(pCmdEvent->data.evt_mesh_sensor_setup_server_set_setting_request);
    uint16 ret_ack=ACK_OK;
    uint16 setting_data;
    uint16 setting_id=-1;
    PBtAppData p_bt_app_data;
    uchar flag_write_data=OFF;
    p_bt_app_data = (PBtAppData)(&pEvent->raw_value.data);
    if(pEvent->property_id == NODE_SENSOR_SETUP_SET)
        {
         setting_id = pEvent->setting_id;
         setting_data = *(PUINT16)p_bt_app_data; Trace16_1(setting_data);
        }
    else { return;}
    

    switch(setting_id)
        {
            case ALL_SETTING_ID:  
                flag_write_data = ON;
                pAdjValue->TempGain = (float)(p_bt_app_data->TempGain)/MESH_INFO_SCALING;
                pAdjValue->TempOffset = (float)(p_bt_app_data->TempOffset)/MESH_INFO_SCALING;
                pAdjValue->HumGain = (float)(p_bt_app_data->RhGain)/MESH_INFO_SCALING;
                pAdjValue->HumOffset = (float)(p_bt_app_data->RhOffset)/MESH_INFO_SCALING;
                pMeshNodeData->WorkingTimer = p_bt_app_data->WorkingTimer;
                pMeshNodeData->SensorClass = p_bt_app_data->BtmClass;
                break;
            default: ret_ack=ACK_ERROR;  
        };



    if(ret_ack == ACK_OK){ 
        WriteNodeData();         
        } 
    else{ ret_ack = ACK_ERROR; }

    Cmd_ms_setup_server_send_setting_status(SENSOR_ELEMENT,pEvent->client_address, pEvent->appkey_index, NO_FLAGS,
                                            pEvent->property_id, pEvent->setting_id,sizeof(ret_ack), (uchar *)&ret_ack);
    
}

//
// for Get setting event
//
void EvtSetGettingRequestProc(PCmdPacket pCmdEvent)
{TraceProc();
    uchar get_data[5]={0x01,0x02,0x03,0x04,0x05};
    
    uint16 ret_ack=ACK_OK;
    uint16 setting_id=-1;
    _BtAppData BtSetupData;
    uchar flag_write_data=OFF;
    msg_ms_setup_server_get_setting_request_evt *pEvent = &(pCmdEvent->data.evt_mesh_sensor_setup_server_get_setting_request);
    if(pEvent->property_id == NODE_SENSOR_SETUP_SET)
        {
         setting_id = pEvent->setting_id;         
        }
    else {return;}

    memset((PUCHAR)&BtSetupData,0,sizeof(_BtAppData));
    switch(setting_id)
           {
               case ALL_SETTING_ID:    
                   flag_write_data = ON;
                   BtSetupData.TempGain = (int16)(pAdjValue->TempGain*MESH_INFO_SCALING);
                   BtSetupData.TempOffset = (int16)(pAdjValue->TempOffset*MESH_INFO_SCALING);
                   BtSetupData.RhGain = (int16)(pAdjValue->HumGain*MESH_INFO_SCALING);
                   BtSetupData.RhOffset = (int16)(pAdjValue->HumOffset*MESH_INFO_SCALING);
                   BtSetupData.WorkingTimer = pMeshNodeData->WorkingTimer;
                   BtSetupData.BtmClass = pMeshNodeData->SensorClass ;
                   break;
               default: ret_ack=ACK_ERROR;  TraceErr1("Get BT Setup 1",setting_id);
           };
    
    
    Cmd_ms_setup_server_send_setting_status(SENSOR_ELEMENT,pEvent->client_address, pEvent->appkey_index, NO_FLAGS,
                                            pEvent->property_id, pEvent->setting_id,sizeof(_BtAppData), (uchar *)&BtSetupData);
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





void BtMeshSetupInit()
{TraceProc();
   UsartRxCount = 8; 
}


void BtMeshSetupTask()
{//TraceProc();
    UsartClientProc(); 
    MeshNodeSetupProc();
}




// Node Setup Stage ==> NSS_XXXX
#define NSS_NODE_STAGE_INIT     0
#define NSS_RX_INFO_WAITING     1    // waiting command of setup
#define NSS_SETUP_CHECK         2    // 
#define NSS_SETUP_PROCESS       3    // 
#define NSS_SEND_INFO           4    // 
#define NSS_SETUP_ACK           5    // 
#define NSS_SETUP_OK            6    // 
#define NSS_SETUP_OK_INIT       7    // 


#define NSS_SETUP_ERROR         10    // 


uint16 CheckModbusCmd();
extern _PModbusCmdF4  pModbusCmd;
extern _ModbusToHost ModbusToHostCmd;
extern const char BtmModelName[6]; 

//
//
//
void MeshNodeSetupProc()
{
    pStageInfo = GetNodeStageInfo(MESH_NODE_SETUP_PROC);
    switch(ActiveStage())
        {
            
            case NSS_NODE_STAGE_INIT: Trace("NSS_NODE_STAGE_INIT");
                 UsartResetRxTx(USART_ID_RX); SetNodeStatus(NS_USART_RX_EVENT,OFF);
                 pModbusCmd= (_PModbusCmdF4)UsartGetBuff(USART_ID_RX);
                 Rs485Rx();
                 ToNextStage(NSS_RX_INFO_WAITING);   
                break;
                
            case NSS_RX_INFO_WAITING: 
                 if(GetNodeStatus(NS_USART_RX_EVENT) == TRUE) 
                    {
                     
                     ToNextStage(NSS_SETUP_CHECK);
                    }
                break;
        
            case NSS_SETUP_CHECK: 
                if(UsartGetRxCounter() < MODBUS_CMD_NUM || CheckModbusCrc((PUCHAR)pModbusCmd,MODBUS_CMD_NUM) == FALSE) 
                    {
                    ToNextStage(NSS_SETUP_ERROR);
                    }
                else 
                    {ToNextStage(NSS_SETUP_PROCESS);}
                break;
                
            case NSS_SETUP_PROCESS: 
                

                if(pModbusCmd->FunCode == 0x03){
                    if(MeshNodeGetInfoProc() == TRUE) ToNextStage(NSS_SEND_INFO);
                    else ToNextStage(NSS_SETUP_ERROR);
                }
                else if(pModbusCmd->FunCode == 0x06) {
                    if(MeshNodeSetInfoProc() == TRUE) ToNextStage(NSS_SETUP_ACK);
                    else ToNextStage(NSS_SETUP_ERROR);
                }
                else {ToNextStage(NSS_SETUP_ERROR);}
                break;
            case NSS_SEND_INFO: //TraceDec1("NSS_SEND_INFO",ModbusToHostCmd.ByteNum+5);
                UsartTxSendCmd((PUCHAR)&ModbusToHostCmd,ModbusToHostCmd.ByteNum+5); Delay_ms(10);
                ToNextStage(NSS_SETUP_OK);
                
                break;
                
            case NSS_SETUP_ACK: 
                UsartTxSendCmd((PUCHAR)pModbusCmd,8); Delay_ms(10);
                ToNextStage(NSS_SETUP_OK);
                break;
                
            case NSS_SETUP_OK: 
                ToWaitingStage(NSS_SETUP_OK_INIT,WAIT_MS(30));
                break;
            case NSS_SETUP_OK_INIT:
                if(CheckWaitTimeOut())  { ToNextStage(NSS_NODE_STAGE_INIT); }
                break;
            
            case NSS_SETUP_ERROR: Trace("NSS_SETUP_ERROR");
                ToNextStage(NSS_NODE_STAGE_INIT);
                break;
           default: TraceErr1("MeshNodeSetupProc",ActiveStage()); break;
        };
    
    
}


#define WORKING_TIME_MIN        5
#define WORKING_TIME_MAX        3600


//
//
// FC6
bool MeshNodeSetInfoProc()
{//TraceProc();
    bool ret_code=TRUE;
    _PModbusCmdF6 p_cmd_fc6;
    uint16 start_addr, value;
    p_cmd_fc6 = (_PModbusCmdF6)pModbusCmd;
    start_addr = WordSwap(p_cmd_fc6->StartAddr);
    value = WordSwap(p_cmd_fc6->Value);
    if(start_addr == 0x06 ){Trace("Set Temp Gain");
         pAdjValue->TempGain = ((float)value)/(float)MESH_INFO_SCALING;
        }
    else if(start_addr == 0x07 ){Trace("Set Temp Offset");
         pAdjValue->TempOffset = ((float)value)/(float)MESH_INFO_SCALING;
        }
    else if(start_addr == 0x08 ){Trace("Set Humidity Gain");
         pAdjValue->HumGain = ((float)value)/(float)MESH_INFO_SCALING;
        }
    else if(start_addr == 0x09 ){Trace("Set Humidity Offset");
         pAdjValue->HumOffset = ((float)value)/(float)MESH_INFO_SCALING;
        }
    else if(start_addr == 0x0A ){
         if(value >= WORKING_TIME_MIN && value <=WORKING_TIME_MAX)
            {TraceDec1("Set Working Time",value);
             pMeshNodeData->WorkingTimer = value;
            }
         else{ TraceErr1("Set Working Time",value); 
            ret_code=FALSE;
            }
        }
    else if(start_addr == 0x0B ){Trace("Set BTM Class 1");
             if( value == 1) value = SENSOR_SI7021;
             else if( value == 2) value = SENSOR_PZEM;
             else if( value == 3) value = SENSOR_OEM;
             else if( value == 4) value = SENSOR_AGB_POWER;
             else ret_code=FALSE;
             if(ret_code == TRUE){ Trace("Set BTM Class 2");
                pMeshNodeData->SensorClass = (uchar)value;
                }
        }
    else if(start_addr == 0x0F ){Trace("Reset to Default");
         MeshNodeSetupReset(); 
        }
    else if(start_addr == 0x11){Trace("Set Baudrate"); // for Debug&Test
         pMeshNodeData->BaudRate = value;
        }
    else ret_code=FALSE;

    if(ret_code == TRUE) WriteNodeData(); else { TraceErr1("MeshNodeSetInfoProc",start_addr);}

    return ret_code;
        
}


//
//
//FC3
bool MeshNodeGetInfoProc()
{//TraceProc();
    bool ret_code=TRUE;
    uint16 start_addr, total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr = WordSwap(pModbusCmd->StartAddr);
    total_reg = WordSwap(pModbusCmd->TotalReg);
    //Trace16_1(start_addr);
    
    if(start_addr >= 0x00 && total_reg == 3){
         ModbusToHostCmd.ByteNum = 6; memcpy(ModbusToHostCmd.Data,BtmModelName,6);
        }
    /*
    else if(start_addr >= 0x00 && total_reg == 9){
         ModbusToHostCmd.ByteNum = 18; memcpy(ModbusToHostCmd.Data,BtmModelName,6);
         ModbusToHostCmd.Data[4] = WordSwap(FW_VER);
         ModbusToHostCmd.Data[6] = WordSwap((uint16)(pAdjValue->TempGain*MESH_INFO_SCALING));
         ModbusToHostCmd.Data[7] = WordSwap((uint16)(pAdjValue->TempOffset*MESH_INFO_SCALING));
         ModbusToHostCmd.Data[8] = WordSwap((uint16)(pAdjValue->HumGain*MESH_INFO_SCALING));
        }    
        */
    else if(start_addr == 0x04 && total_reg == 1){
         ModbusToHostCmd.Data[0] = WordSwap(FW_VER);
        }
    else if(start_addr == 0x06 && total_reg == 1){
         ModbusToHostCmd.Data[0] = WordSwap((uint16)(pAdjValue->TempGain*MESH_INFO_SCALING));
        }
    else if(start_addr == 0x07 && total_reg == 1){
         ModbusToHostCmd.Data[0] = WordSwap((uint16)(pAdjValue->TempOffset*MESH_INFO_SCALING));
        }
    else if(start_addr == 0x08 && total_reg == 1){
         ModbusToHostCmd.Data[0] =WordSwap((uint16)(pAdjValue->HumGain*MESH_INFO_SCALING));
        }
    else if(start_addr == 0x09 && total_reg == 1){
         ModbusToHostCmd.Data[0] = WordSwap((uint16)(pAdjValue->HumOffset*MESH_INFO_SCALING));
        } 
    else if(start_addr == 0x0A && total_reg == 1){
         ModbusToHostCmd.Data[0] = WordSwap(pMeshNodeData->WorkingTimer);
        }
    else if(start_addr == 0x0B && total_reg == 1){
        if(pMeshNodeData->SensorClass == SENSOR_PZEM) ModbusToHostCmd.Data[0] = WordSwap(2);
        else if(pMeshNodeData->SensorClass == SENSOR_OEM) ModbusToHostCmd.Data[0] = WordSwap(3);
        else if(pMeshNodeData->SensorClass == SENSOR_AGB_POWER) ModbusToHostCmd.Data[0] = WordSwap(4);
        else ModbusToHostCmd.Data[0] = WordSwap(1);//default
        
         //ModbusToHostCmd.Data[0] = WordSwap(pMeshNodeData->SensorClass);
        }
    
    else if(start_addr == 0x11 && total_reg == 1){
         ModbusToHostCmd.Data[0] = WordSwap(pMeshNodeData->BaudRate);
        }    
    else ret_code=FALSE;


    
    if(ret_code == TRUE) ModbusAddCrc(); else { TraceErr1("MeshNodeSetInfoProc",start_addr);}

    return ret_code;
        
}



