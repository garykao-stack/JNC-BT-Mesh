

#include "global.h"

#define SERVER_NODE_DEBUG 1
#define STATE_CHANGE_DEBUG 1

#if SERVER_NODE_DEBUG
	#undef DEBUG_FUNCTION
	#define DEBUG_FUNCTION dprint
#else
	#define DEBUG_FUNCTION(...)
#endif
#include "debugprint.h"

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

#include "BQ3200.h"
#include "mesh_sensor.h"
#include "PMSA003.h"
#include "SGPxx.h"

#if STATE_CHANGE_DEBUG
#define STATE_CHANGE_PRINT DEBUG_FUNCTION
#else
#define STATE_CHANGE_PRINT(...)
#endif
#include "Mesh_Node.h"
#include "Mesh_Client.h"
#include "Mesh_Server.h"

#ifdef BTM_A308
#include "A308_Server.h"
#endif

#define UART_CMD_TIME 18    // N*10 ms // uart 通訊耗時約 173 ms，ServerGetInfoProc() 內會阻塞 ->Timer 計時
#define I2C_CMD_TIME  6     // N*10 ms // i2c 通訊耗時約 60 ms，ServerGetInfoProc() 內會阻塞 ->Timer 計時

#define TO_CLIENT_BUFF_MAX  32
uchar   ToClientBuf[TO_CLIENT_BUFF_MAX];
uchar   ToClientLen;
uint16  GetDeviceInfoDelay=5;     //Nx10ms ==> 50ms
uint16  PreReadDelay=0;           //Nx10ms
uint16  syncTime = 0;             //Nx10ms server 週期大於 client 時的補償


_ServerInfo ServerInfo;
PServerInfo pServerInfo = &ServerInfo;
PNodeHeader pSensorHeader = &ServerInfo.SensorInfo.Header;
void (*parseMbsResponse)()=0;

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

const uchar CmdGetCo2_CDM7160[7] ={0xFE, 0x44, 0x00, 0x08, 0x02, 0x9F, 0x25};       //獲取CDM7160 CO2數據
const uchar CmdGetCo2_GetS8[8]   ={0xFE, 0x04, 0x00, 0x03, 0x00, 0x01, 0xD5, 0xC5}; //獲取S8 CO2數據
const uchar CmdGetPm25[7] ={0x42, 0x4d, 0xe2, 0x00, 0x00, 0x01, 0x71};  //取得攀藤PM2.5數據

/*轉傳指令*/
uchar TransData[USART_TX_BUFF_SIZE];
uint8_t TransArrayLength=0;

//
//
//
void ServerNodeInit()
{
    NodeRole = NR_SERVER;
    UsartRxCount = 7;   //from RS485 device bytes (default)
    ToNextStage(SNS_WAKE_UP);
    Rs485Tx();
    CheckRs485Device(15);
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
    if(GetNodeStatus(NS_FULL_POWER) == ON) dprint("\r\nFull Power\r\n");
    else dprint("\r\nBattery Power\r\n");
    
}

void ServerSetupNodeInit()
{
    ServerNodeInit();
}

void ServerSerialTransProc();


//
// For Server Node Task
//  10ms for one time
void ServerNodeTask()
{
	//UsartClientProc(); /*移到app.c*/
	if(GetNodeStatus(NS_SERVER_RS485_ENABLE))
		ServerSetNodeProc();

#ifdef  BT_MESH_G6  
    if(GetNodeStatus(NS_G6_READY)){
        G6ScheduleProc();
        G6CheckStatusProc();
    }
#endif    
#ifdef BTM_A308
    A308_ModbusAction();
#endif
//#ifdef BTM_TRANSMITTER
    if (pSensorHeader->SensorClass==SENSOR_CUSTOM_SERIAL) ServerSerialTransProc();
//#endif
    ServerGetInfoProc();
    NodeIviUpdateProc();
}

extern uint32 keepAliveBeforeSleepMs;


uint32_t cd_sleep_ms=0;
uint32 IgnoreBroadcastCmdMs=0;
uint32 ServerResponseTestMs=0;
uint8 trans_retry_ms=0;
#define TIMER_REDUCE_BY_10MS(t) do{if(t>0)t=t-((t>=10)?10:t);}while(0)
#define TIMER_IS_TIMEROUT(t) (t==0)
void ServerTimer_10ms(){
	TIMER_REDUCE_BY_10MS(cd_sleep_ms);
	TIMER_REDUCE_BY_10MS(IgnoreBroadcastCmdMs);
	TIMER_REDUCE_BY_10MS(ServerResponseTestMs);
	TIMER_REDUCE_BY_10MS(trans_retry_ms);
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
             default: ret_code=FALSE; 
                break;
            };
            
        }
    else {ret_code=FALSE;}

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
             default: ret_code=FALSE; 
                break;
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
    Bool ret_code=TRUE;
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
        }
    else if((property_id >= NODE_SET_DO1_OFF) && (property_id <= NODE_SET_DO6_OFF))
        {
         p_modbus_cmd = (PUCHAR)&A6D6CtrlCmdOff[property_id-NODE_SET_DO1_OFF][0];
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
    uint16 property_id;
     
    
    switch(ActiveStage())
        {
            case SNS_SET_INFO_INIT: 
                ToNextStage(SNS_SET_WAITING);
                break;
            case SNS_SET_WAITING:   
                if(GetNodeStatus(NS_SET_NODE_ACT) == TRUE)  {ToNextStage(SNS_SET_INFO_PRE);}
                break;
                
            case SNS_SET_INFO_PRE:  
                ret_code=FALSE;
                property_id = pNodeEventInfo->PropertyID;
                if(property_id >=BTM_G6_CMD_START && property_id <=BTM_G6_CMD_END ){
                    ret_code = BtmG6SetCtrl(property_id);ToWaitingStage(SNS_SET_WAITING_INFO,5);
                    }
                else if(pFunSensor == GetAipInfo){
                     ret_code = AIPSetCtrl(property_id); ToWaitingStage(SNS_SET_WAITING_INFO,5);
                    }
                else  if(pFunSensor == GetA6D6Info){
                     ret_code = A6D6MSetCtrl(property_id); ToWaitingStage(SNS_SET_WAITING_INFO,5);
                    }
                else  if(pFunSensor == GetAgbPower){
                     ret_code = AGBSetCtrl(property_id); ToWaitingStage(SNS_SET_WAITING_INFO,5);
                    }
                
                else{ToNextStage(SNS_SET_INFO_END);}
                
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



void ServerGetInfoActionNow()
{
    PNodeStageInfo p_stage_info;
    p_stage_info = GetNodeStageInfo(SERVER_GET_INFO_PROC);
    p_stage_info->Timer = WAIT_SEC(1);
}

#define CHECK_FORWARD_DATA  //if(GetNodeStatus(NS_TRANS_SERIAL_DATA)){ToNextStage(SNS_EVENT_WAITING); break;}
uint8 res_loc;
extern bool UsartTxIsBusy();
extern bool UsartRxIsBusy();
uint8 trans_retry_cnt=0;
uint16 trans_tx_timeout_ms=0;
uint8 trans_ignore_rx=0;

void ServerSerialTransProc(){

	if(UsartTxIsBusy() && !trans_tx_timeout_ms){ // tx timeout
		UsartResetRxTx(USART_ID_TX_RX);
	}

	if(GetNodeStatus(NS_TRANS_SERIAL_DATA)){


		if( (UsartRxIsBusy())	/*正在接收RX訊息，放棄轉送TX訊息*/
			||(UsartTxIsBusy())){	/*正在轉送上一次資料，放棄新TX訊息*/
			SetNodeStatus(NS_TRANS_SERIAL_DATA,OFF);
			trans_ignore_rx=1; /*丟棄下一次接收到的RX訊息*/
			return;
		}

		/*清除已接收內容，避免造成錯位*/
		UsartResetRxTx(USART_ID_RX);

		dprint("sendData to Serial(%d)\r\n",TransArrayLength);
		if (TransArrayLength){
			SetLedStatus(LED_STATUS_ACTIVE);
			SetNodeStatus(NS_USART_RX_EVENT,OFF);
			UsartResetRxTx(USART_ID_TX);
			UsartTxSendCmd(TransData,TransArrayLength);
			CountErr = 0;
			trans_retry_cnt=0;
			trans_retry_ms=0;
			trans_tx_timeout_ms=1000;
			trans_ignore_rx=0;
		}
		SetNodeStatus(NS_TRANS_SERIAL_DATA,OFF);
		res_loc=0;
	}

	if(GetNodeStatus(NS_USART_RX_EVENT) && !trans_retry_ms){ //received response, send back to client
		//transprint("receive serial response\r\n");
		if(trans_ignore_rx){ /*丟棄這次接收的內容，因為Master已發出新的指令，舊回應已不需要*/
			trans_ignore_rx=0;
			result=0;
		}else{
			result=SendRxToClient(
					((uint16)TransData[2])<<8 | TransData[3],
					((uint16)TransData[4])<<8 | TransData[5],
					RxBuff,
					res_loc,
					CounterRx);//(CounterRx-res_loc)>50?50:(CounterRx-res_loc));
			//transprint("SendRxToClient loc:%d, len:%d, result:%s\r\n",((uint16)TransData[2])<<8 | TransData[3],CounterRx,result?"Successed":"Failed !!!!!");
			IFDPRINT(
				if(res_loc==0){
					//dprint("SendRxToClient: \r\n" );
					transprint("SendRxToClient loc:%d, len:%d, result:%s\r\n> ",((uint16)TransData[2])<<8 | TransData[3],CounterRx,result?"Successed":"Failed !!!!!");
					for(int i=0;i<CounterRx;i++) dprint(" %02X",RxBuff[i]);
					dprint("\r\n");
				}
			);
		}

		if (!result && trans_retry_cnt<6){//fail
			trans_retry_cnt++;
			trans_retry_ms=50;
		}else{ //succeed or try too many time
			res_loc+=50;
			CountErr=0;
			if (res_loc>=CounterRx){
				SetNodeStatus(NS_USART_RX_EVENT,OFF);
				UsartResetRxTx(USART_ID_TX_RX);
			}else{
				trans_retry_ms=100;
			}
		}

	}


}

//
// for sensor report
//
void ServerGetInfoProc()
{
    pStageInfo = GetNodeStageInfo(SERVER_GET_INFO_PROC);
    uint16 result;
    switch(ActiveStage())
        {
            case NODE_STAGE_INIT:
                GetInfoCycle = WAIT_SEC(TIMER_GET_INFO_SLEEPING);
                ToNextStage(SNS_PRE_WAITING);   //default
                break;
            case SNS_PRE_WAITING: 
            	if(!CheckWaitTimeOut()) return;
            	CountErr = 0;
            	SetNodeStatus(NS_SEND_INFO_ACK,OFF);
#if BTM_A308
            	A308_ResetModbusCmd();
            	ToWaitingStage(SNS_A308_FETCH_INFO,WAIT_MS(A308_Fetch_Timeout_Ms()+100));
            	break;
            case SNS_A308_FETCH_INFO:
            	if(CheckWaitTimeOut()||A308_Connected()){
            		dprint("A308 Connect:%d\r\n",A308_Connected());
            		//A308_ResetModbusCmd();
            		//A308_StopModbusAction();
            		ToWaitingStage(SNS_GET_INFO,0);
            	}
#else
                const uint32 DelayCompensation = ReInitSkynetSensor();
                const uint32 wait_10ms = GetDeviceInfoDelay + PreReadDelay - DelayCompensation;
                ToWaitingStage(SNS_GET_INFO, wait_10ms);
            	dprint("* wait_10ms %d(ms)\r\n",wait_10ms*10);
            	dprint("*** wait for %d(ms), pre read %d(ms)\r\n",(GetDeviceInfoDelay+PreReadDelay)*10,+PreReadDelay*10);
#endif
                break;

            case SNS_GET_INFO: 
            	CHECK_FORWARD_DATA;

                // 若收到 GET_ALL_SENSOR 但還沒讀 sensor，跳過等待時間趕快讀。
            	if(GetNodeStatus(NS_GET_INFO_ACT) && ActiveWaiting() > 2) {
                    static int cnt = 0;
                    syncTime = ActiveWaiting() + 20;    // 超時 + 200ms 緩衝
                    dprint("Force read early! count: %d, %d(ms)\r\n", ++cnt, ActiveWaiting() * 10);
                    SetWaiting(2);
                }

                if((GetNodeStatus(NS_SYS_NO_WAITING) == ON) || CheckWaitTimeOut() == TRUE){
                    //Trace("SNS_GET_INFO 1");
                	/*if(UsartIsBusy()) {
                		dprint("uart is busy\r\n");
                		return;
                	}*/
                	transprint("GET_INFO: pFunSensor:%d\r\n",(int)pFunSensor);
                    dprint("GET_INFO: pFunSensor:%d\r\n",(int)pFunSensor);
                    if((pFunSensor!= NULL) && (GetSensorInfo() == TRUE) ){
                    	if(GetNodeStatus(NS_USART_WAIT_RESP)){ /*非阻塞式Modbus處理，進入SNS_WAIT_GET_INFO中等待1秒，若未收到回應直接進入休眠模式*/
                    		SetNodeStatus(NS_USART_WAIT_RESP,OFF);
                    		ToWaitingStage(SNS_WAIT_GET_INFO,WAIT_MS(1000));
                    		return;
                    	}else if(pFunSensor==GetRelay /*&& !pMeshNodeData->RelayEnabled*/){ /*不論是否開啟Relay功能，都要嘗試讀取設備*/
							CheckRs485Device(3);
							if(pFunSensor!=GetRelay) break; /*初次讀取到Sensor種類，保持SNS_GET_INFO階段以讀取數值*/
						}
						ToNextStage(SNS_EVENT_WAITING);
                    }/*else if(pFunSensor== NULL||pFunSensor==GetRelay){
                	CheckRs485Device(3);
                	//if(!CheckRs485Device(3) || pFunSensor==NULL) ToWaitingStage(SNS_PRE_SLEEPING,TIMER_WAIT_SLEEPING);
                    }*/else{
                	//Trace1("Get Sensor Stop: 1",pFunSensor);
                        ToWaitingStage(SNS_PRE_SLEEPING,TIMER_WAIT_SLEEPING);
                   }
                } else if (CheckPreReadTimer(false, 0, 0)) {
                  // Start pre read
                  dprint("pre read: ");
                  pFunSensor();
                }
                break;
            case SNS_WAIT_GET_INFO:

            	if(GetNodeStatus(NS_USART_RX_EVENT)){
            		if(!UsartGetStatus(USART_RX_CRC_ERROR)){//succeed
            			if(parseMbsResponse) parseMbsResponse();
            			ToWaitingStage(SNS_EVENT_WAITING,0);
            		}else{//error
            			ToWaitingStage(SNS_PRE_SLEEPING,TIMER_WAIT_SLEEPING);
            		}
            		SetNodeStatus(NS_USART_RX_EVENT,OFF);
            		UsartResetRxTx(USART_ID_TX_RX);
            	}else if(CheckWaitTimeOut()){ //timeout
            		ToWaitingStage(SNS_PRE_SLEEPING,TIMER_WAIT_SLEEPING);
            	}
            	break;
            case SNS_EVENT_WAITING:

            	if(GetNodeStatus(NS_GET_INFO_ACT)){
            		SetNodeStatus(NS_A308_GET_FINISHED,OFF);
                    SetLedStatus(LED_STATUS_ACTIVE);
                    dprint("Step: NS_GET_INFO_ACT, Sleep:%d\r\n",GetNodeStatus(NS_SLEEPING));


                    if(GetNodeStatus(NS_SLEEPING))
                      {ToWaitingStage(SNS_PRE_SLEEPING,TIMER_WAIT_SLEEPING);} // go to sleeping from host                        
                    else{
                    	if(pFunSensor==GetRelay )GetSensorInfo(); /*發送資訊前先讀取電池電量*/
                    	ToWaitingStage(SNS_PRE_SEND_INFO,TIMER_WAIT_SEND_INFO);
                    	CountErr = 0;
#ifdef BTM_A308
                    	cd_sleep_ms=30*15*1000;
#else
                    	if(pSensorHeader->SensorClass==SENSOR_CUSTOM_SERIAL)
                    		cd_sleep_ms=pAdjValue->RS485TransmitterData.Rs485ServerDelayBeforeSleep*1000L;
                    	else if(pMeshNodeData->RelayEnabled)
                    		cd_sleep_ms=4000; /*Relay模式下，等待指定時間後再進入休眠 ，避免發送出資訊後沒辦法中繼其他設備的通訊*/
                    	else
                    		cd_sleep_ms=0;
                    	keepAliveBeforeSleepMs=cd_sleep_ms;

#endif
                        //TraceDec1("TIMER_WAIT_SEND_INFO",TIMER_WAIT_SEND_INFO);
                      }
                }
#ifdef BTM_A308
            	else if(GetNodeStatus(NS_A308_GET_INFO)){
                	ToNextStage(SNS_SEND_A308_INFO);
                }
#endif
                break;
            case SNS_PRE_SEND_INFO: 
                //waiting to send info
            	CHECK_FORWARD_DATA;
                if(CheckWaitTimeOut())  { ToNextStage(SNS_SEND_INFO); }
                break;                
            case SNS_SEND_INFO: 


                if(CountErr > 5) {
                	ToWaitingStage(SNS_PRE_SLEEPING,TIMER_WAIT_SLEEPING);
                	dprint("SNS_SEND_INFO Error!. go to Sleep\r\n");
                } // err: go to sleeping
            
                dprint("SNS_SEND_INFO ServerInfo.NodeInfoSize:%d\r\n",ServerInfo.NodeInfoSize);
                if(ServerInfo.NodeInfoSize>0) result=SendInfoToClient();
                /*else{
                	result=TRUE;
                }*/

                if(!result) // succeed. go to send sensor info
                {


                	SetLedStatus(LED_SEND_NODE_INFO_ERROR_OFF);
					SetNodeStatus(NS_GET_INFO_ACT,OFF);

					if(GetNodeStatus(NS_A308_GET_INFO)){ToNextStage(SNS_SEND_A308_INFO);}
#ifdef BTM_A308
                     //else if(pMeshNodeData->SensorClass==SENSOR_A308M)	ToWaitingStage(SNS_WAIT_A308_CMD,WAIT_SEC(30*15));
                     //else ToWaitingStage(SNS_WAIT_A308_CMD,WAIT_SEC(30*15));
                     else ToWaitingStage(SNS_WAIT_SEND_INFO_ACK,WAIT_SEC(2));
#else
                     else{
                    	 /*	1. Relay啟動時，發送完設備資訊後不直接進入休眠，讓後端的Server有機會把資料傳完
                    	  	2. 啟用透傳模式時，發送完設備資訊後亦不直接進入休眠，空出指定時間處理透傳指令            	  */
                    	 if(pMeshNodeData->RelayEnabled || pSensorHeader->SensorClass==SENSOR_CUSTOM_SERIAL){
                    		 ToWaitingStage(SNS_PRE_SLEEPING,WAIT_MS(cd_sleep_ms));
                    	 }else
                    		 ToWaitingStage(SNS_PRE_SLEEPING,TIMER_WAIT_SLEEPING);
                    	 dprint("keep alive before sleep:%d(ms)\r\n",pStageInfo->Timer*10);
                     }
#endif

                }else{ //failed. waiting to sleeping
                	if(result==0xc03){ //limit_reached
                		/*不知道達到什麼極限，需要重新組網*/
                		SetLedStatus(LED_SEND_NODE_INFO_ERROR_ON);
                		SetNodeStatus(NS_IVI_UPDATE,ON);
                	}
                	dprint("SNS_SEND_INFO failed. send again(try:%d)\r\n",CountErr);
                     CountErr++;
                     /* 發送失敗，等待指定時間後重試
                      * Client等待4秒，故每800ms重試一次，共5次
                      * ※原設定為Client等待2秒，Server每2秒重試共5次，此設定不合理*/
                     ToWaitingStage(SNS_PRE_SEND_INFO,WAIT_MS(800)); // send info again WAIT_SEC(2)
                }
                break;
#ifdef BTM_A308
            case SNS_WAIT_SEND_INFO_ACK:

            	if (GetNodeStatus(NS_SEND_INFO_ACK)){	// received ACK, wait for A308_GET_INFO
            		ToWaitingStage(SNS_WAIT_A308_CMD,WAIT_MS(cd_sleep_ms));
            	}else if(GetNodeStatus(NS_A308_GET_INFO)){ // received A308_GET_INFO, start to send A308 data
					ToNextStage(SNS_SEND_A308_INFO);
            	}else if(CheckWaitTimeOut()||GetNodeStatus(NS_GET_INFO_ACT)){ // Timeout or get another NS_GET_INFO_ACT, send Node info again.
            		dprint("\r\nNot Get Send Info Ack. Send again\r\n");
            		ToNextStage(SNS_SEND_INFO);
            		CountErr=0;
            	}

            	break;
            case SNS_SEND_A308_INFO:
            	CHECK_FORWARD_DATA;
            	result=A308_SendToClient();
            	if(result==0){ //proccess has been finished. move to next step.
            		SetNodeStatus(NS_A308_GET_INFO,OFF);
            		//ToWaitingStage(SNS_PRE_SLEEPING,TIMER_WAIT_SLEEPING+WAIT_SEC(10)); /*多預留一點時間，提供Client接收不完整時重試*/
            		dprint("relay mode:%d, cd_sleep_ms:%d\r\n",pMeshNodeData->RelayEnabled,cd_sleep_ms); /*啟用Relay功能時，發送完訊息不要直接進入休眠，保持一段時間保持Relay作用*/
            		ToWaitingStage(SNS_WAIT_A308_CMD,TIMER_WAIT_SLEEPING+(pMeshNodeData->RelayEnabled?cd_sleep_ms/10:WAIT_SEC(30))); /*多預留一點時間，提供Client接收不完整時重試*/
            	}
            	break;
            case SNS_WAIT_A308_CMD:
            	CHECK_FORWARD_DATA;
            	if(GetNodeStatus(NS_GET_INFO_ACT)){ //received another GET_INFO_ACE command, send Node info again.
            		ToNextStage(SNS_EVENT_WAITING);
            	}else if(GetNodeStatus(NS_A308_GET_INFO)){	//received A308_GET_INFO, start to send A308 data
					ToNextStage(SNS_SEND_A308_INFO);
				}else if(CheckWaitTimeOut()||GetNodeStatus(NS_A308_GET_FINISHED)){ //wait too
					SetNodeStatus(NS_A308_GET_FINISHED,OFF);
					ToWaitingStage(SNS_PRE_SLEEPING,TIMER_WAIT_SLEEPING);
				}

            	break;
#endif

            case SNS_PRE_SLEEPING: 
            	CHECK_FORWARD_DATA;
            	if(GetNodeStatus(NS_GET_INFO_ACT)){
            		ToWaitingStage(SNS_PRE_WAITING,WAIT_MS(0));
            	}else if(GetNodeStatus(NS_A308_GET_INFO)){
					ToNextStage(SNS_SEND_A308_INFO);
				}else if(CheckWaitTimeOut()){
                     if(GetNodeStatus(NS_FULL_POWER) == ON || GetNodeStatus(NS_FORCE_FULL_POWER) == ON || IgnoreBroadcastCmdMs||ServerResponseTestMs){ /*在App設定模式下不進入休眠*/
                    	 dprint("*** Full Power Mode ***\r\n");
                    	 SetLedStatus(LED_STATUS_SLEEP); ToNextStage(SNS_WAKE_UP);
                     }else{
                    	 dprint("*** Lowe Power Mode ***\r\n");
                    	 SetNodeSleeping(ON);  ToNextStage(SNS_SLEEPING);
                     }
                }
                    
                break;
            case SNS_SLEEPING: 
                // waiting wake up
                if(!GetNodeStatus(NS_SLEEPING)) 			ToNextStage(SNS_WAKE_UP);
                else if(GetNodeStatus(NS_A308_GET_INFO))	ToNextStage(SNS_SEND_A308_INFO);

                break;
            case SNS_WAKE_UP: 
#if defined(BTM_A308) && A308_SLEEP_MODE
            	ToWaitingStage(SNS_PRE_WAITING,WAIT_MS(2000));
#else
            	ToWaitingStage(SNS_PRE_WAITING,WAIT_MS(0));
                //ToNextStage(SNS_PRE_WAITING);
#endif
                break;
            default:  break;
        };
}



//
// Get sensor info and update
//
bool GetSensorInfo()
{
  bool ret_code = FALSE;
  pSensorHeader->BatteryPower = GetBatteryPower();
    
   // Trace8_1(pSensorHeader->Status);
  if(pNodeEventInfo->PropertyID == NODE_GET_BTM_INFO){
    ret_code = GetBtmMeshInfo();
  }else if(pFunSensor){
    ret_code = pFunSensor();
    if(GetCustomSerial!=pFunSensor && !GetNodeStatus(NS_USART_WAIT_RESP))UsartResetRxTx(USART_ID_TX_RX);
  }

  return ret_code;
}

extern  uchar PowerKeyCount;
uint16 G6StatusTest=0x8003;

const uchar PowerKeyMapSD[7]={3,0,1,1,2,1,2};



Bool BtmG6SetCtrl(uint16 property_id)
{
    bool ret_code=TRUE;

    G6SetActStatus(G6_STATUS_CHANGE,ON);
    if(property_id >= AUTO_POWER_0 && property_id <= AUTO_POWER_4 ){
          G6SetAutoRun(ON); property_id &=0x00FF;
          if(pMeshNodeData->G6HostPPercent != (uchar)property_id){//Trace("Save Data 1");
            pMeshNodeData->G6HostPPercent = (uchar)property_id;
            WriteMeshNodeData();
          }
        }
    else if(property_id >= MENU_POWER_0 && property_id <= MENU_POWER_4 ){   
        G6SetAutoRun(OFF); property_id &=0x00FF;
        if(pMeshNodeData->G6HostPPercent != (uchar)property_id){//Trace("Save Data 2");
            pMeshNodeData->G6HostPPercent = (uchar)property_id;
            PowerKeyCount = PowerKeyMapSD[pMeshNodeData->G6HostPPercent];
            WriteMeshNodeData();
          }
        }
    else if(property_id == CLEAR_FILTER1){
            G6SetActStatus(G6S_CLEAR_FILERT1,ON); G6SetActStatus(G6S_CLEAR_ALL_FILTER,OFF);
        }
    else if(property_id == CLEAR_ALL_FILTER){
            G6SetActStatus(G6S_CLEAR_ALL_FILTER,ON); //CLEAR_LED_ALL_FILTER();
        }
    else {
        ret_code=TRUE;}
    return ret_code;
}    

extern uint16   G6ActStatus;
extern PG6Schedule pActSchedule;

//
//for BTM G6
//
#ifdef BT_MESH_G6
bool GetBtmG6Info()
{
    bool ret_code = TRUE;
    uint16 properity;
    uint16 btm_g6_status;
    PBtmG6 p_sensor = &ServerInfo.SensorInfo.BtmG6;
    SetNodeInfoSize(_BtmG6);
    SetNodeClass(SENSOR_BTM_G6);
    properity = pNodeEventInfo->PropertyID;
    btm_g6_status = 0;
    if(properity == NODE_GET_ALL_SENSOR || (properity >= BTM_G6_CMD_START && properity <= BTM_G6_CMD_END)){//Trace1("G6 All Property 1",properity);
         if(G6GetActStatus(G6S_DOOR_OPEN)== TRUE){btm_g6_status |= G6_FC3_DOOR_OPEN;}
         if(G6GetActStatus(G6S_WARING_FILTER1)){btm_g6_status |= G6_FC3_WARNING_FILTER1;}
         if(G6GetActStatus(G6S_WARING_FILTER2)){btm_g6_status |= G6_FC3_WARNING_FILTER2;}
         if(G6GetActStatus(G6S_AUTO)) {btm_g6_status |= G6_FC6_AUTO_RUN;}
        Trace16_2(btm_g6_status,G6ActStatus);
         p_sensor->Status.G6CurrStatus = btm_g6_status;// btm_g6_status;

         if(btm_g6_status & G6_FC6_AUTO_RUN){//Trace("G6 Auto Run ON");
             if(G6GetActStatus(G6S_SCHEDULE_ACTIVE)){ //Trace("G6 schedule ON");
                p_sensor->Status.PPercent = pActSchedule->PowerPercent;
                }
             else{//Trace("G6 schedule OFF");
                /*if(pMeshNodeData->G6HostPPercent == MENUAL_KEY_AUTO)
                   p_sensor->Status.PPercent = 0;
                else*/
                    p_sensor->Status.PPercent = pMeshNodeData->G6HostPPercent;
                }
         }else{
             p_sensor->Status.PPercent = pMeshNodeData->G6HostPPercent;
         }
         p_sensor->TimeFilter1 =pMeshNodeData->FilterAllTime1;
         p_sensor->TimeFilter2 =pMeshNodeData->FilterAllTime2;
         Trace16Ptr_3(p_sensor,Status.G6CurrStatus,Status.PPercent,Status.G6Status);
        }
    else if(properity >= BTM_G6_CMD_START && properity <= BTM_G6_CMD_END){ 
        pNodeEventInfo->PropertyID = NODE_GET_ALL_SENSOR;
        SetNodeStatus(NS_GET_INFO_ACT,ON);
    }
    ret_code = TRUE; 
    return ret_code;
}
#else
bool GetBtmG6Info()
{
    bool ret_code = TRUE;
    return ret_code;
}

#endif




//
// for Property ID = NODE_GET_BTM_INFO for App
//
bool GetBtmMeshInfo()
{
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
 p_sensor->RebootForRs485IdelSecnods=pMeshNodeData->RebootForRs485IdelSecnods;
 p_sensor->RebootMinutes=pMeshNodeData->RebootMinutes;
 return ret_code; 
}

uint16 ResponseBtmMeshInfo(){
	_ServerInfo info=ServerInfo;

	 PBtMeshInfo p_sensor = &info.SensorInfo.BtmMeshInfo;
	 info.NodeInfoSize=sizeof(_NodeHeader) + sizeof(_BtMeshInfo);
	 info.ProperityID = NODE_GET_ALL_SENSOR_GEN2;

	 info.SensorInfo.Header.BatteryPower = GetBatteryPower();
	 if(GetNodeStatus(NS_FULL_POWER)) info.SensorInfo.Header.Status|= SERVER_FULL_POWER;
	 else		 info.SensorInfo.Header.Status&= ~SERVER_FULL_POWER;


	 memcpy(p_sensor->ModelName,MODEL_NAME,6);
	 p_sensor->Version = FW_VER;
	 p_sensor->TempGain = (int16)((pAdjValue->TempGain)*MESH_INFO_SCALING);
	 p_sensor->TempOffset = (int16)((pAdjValue->TempOffset)*MESH_INFO_SCALING);

	 p_sensor->RhGain = (int16)((pAdjValue->HumGain)*MESH_INFO_SCALING);
	 p_sensor->RhOffset = (int16)((pAdjValue->HumOffset)*MESH_INFO_SCALING);

	 p_sensor->WorkingTime = pMeshNodeData->WorkingTimer; //60;
	 p_sensor->BtmClass = pMeshNodeData->SensorClass; //0x01;
	 p_sensor->RebootForRs485IdelSecnods=pMeshNodeData->RebootForRs485IdelSecnods;
	 p_sensor->RebootMinutes=pMeshNodeData->RebootMinutes;
	 return Cmd_ms_server_send_status(SENSOR_ELEMENT, pNodeEventInfo->ClientAddr,pNodeEventInfo->AppkeyIndex,
	 	                                             NO_FLAGS, info.NodeInfoSize+3, (PUCHAR)&info)->result;
}

/**
 * @brief 若從睡眠模式起來，部分 sensor 需重新初始化並等待 sensor 穩定。
 * 因通訊會阻塞 ->Timer 計時，應預先扣除通訊延遲。
 * @return 通訊延遲補償 (N*10 ms)
 */
uint32 ReInitSkynetSensor() {
    if(GetNodeStatus(NS_FULL_POWER))
        return 0;

    const int init_times = 5; // 先假設初始化 5 次，之後該改成直接計算實際耗時
    uint16 cmd_time = 0; // 單次通訊耗時。通訊會阻塞 ->Timer 計時，用此值預先扣除
    
    switch (pMeshNodeData->SensorClass) {
        case SENSOR_SKYNET_CO2:
            CheckCDMCo2();                // init co2
            GetDeviceInfoDelay = 1500;    // Wait total 35s for stable.
            PreReadDelay = 2000;
            cmd_time = UART_CMD_TIME;
            break;
        case SENSOR_SKYNET_PM25:
            CheckCDMPm25();               // init pm2.5
            GetDeviceInfoDelay = 2000;    // Wait total 30s for stable.
            PreReadDelay = 1000;
            cmd_time = UART_CMD_TIME;
            break;
        case SENSOR_SKYNET_TVOC:
            CheckCDMTvoc(true);           // init tvoc
            GetDeviceInfoDelay = 1500;    // Wait 15s for initial.
            PreReadDelay = 1000;          // Pre read 10s of data.
            cmd_time = I2C_CMD_TIME;
            SGPxx_ResetTvocMax();
            break;
        default:
            PreReadDelay = 0;
            break;
    }
    
    uint32 DelayCompensation = (init_times + (PreReadDelay / 100)) * cmd_time;  // 延遲補償，扣除初始化耗時、預扣通訊耗時
    if(DelayCompensation > PreReadDelay)    // 以防萬一，不應該發生
        DelayCompensation = PreReadDelay;
    if (PreReadDelay > 0)
        CheckPreReadTimer(true, PreReadDelay - DelayCompensation, cmd_time); // init timer
    
    return DelayCompensation;
}

Bool AdjTempRh(int16* p_temp,uint16 *p_humidity)
{
  *p_temp = (int16)(((float)*p_temp)*(pAdjValue->TempGain) + (pAdjValue->TempOffset)*10);
  *p_humidity = (uint16)(((float)*p_humidity)*(pAdjValue->HumGain) + (pAdjValue->HumOffset)*10);
  return TRUE;
}

bool AdjCo2(uint16 *p_co2)
{
  *p_co2 = (uint16)(((float)*p_co2)*(pAdjValue->TempGain) + (pAdjValue->TempOffset));
  return true;
}

bool AdjPm25(float *p_pm25, float *p_pm10)
{
  *p_pm25 = (((float)*p_pm25)*(pAdjValue->TempGain) + (pAdjValue->TempOffset));
  *p_pm10 = (((float)*p_pm10)*(pAdjValue->HumGain) + (pAdjValue->HumOffset));
  return true;
}

bool AdjTvoc(uint16 *p_tvoc)
{
  *p_tvoc = (uint16)(((float)*p_tvoc)*(pAdjValue->TempGain) + (pAdjValue->TempOffset));
  return true;
}


//
//
//
bool GetSkynetInfo()
{
    bool ret_code = TRUE;
    int16 temp;
    uint16 Humidity;
    PSi7021Info p_sensor = &ServerInfo.SensorInfo.Si7021Info;
    SetNodeInfoSize(_Si7021Info);
    SetNodeClass(SENSOR_SI7021);
    if(GetTempAndRH(&temp,&Humidity) != VALUE_IS_NOT_KNOWN){//TraceDec2("Si7021=> 1", temp,Humidity);
        AdjTempRh(&temp,&Humidity); 
        p_sensor->Tempature = temp;
        p_sensor->Humidity  = Humidity;
        p_sensor->fTempature = (float)p_sensor->Tempature / 10;
        p_sensor->fHumidity = (float)p_sensor->Humidity / 10;
        dprint("====> temp: %d, hum:%d\r\n",p_sensor->Tempature,p_sensor->Humidity);
    }
    else {ret_code = FALSE;}
    ret_code = TRUE; 
    return ret_code;
}


//
//
//
bool Get_CO2(uint16_t *value) {
//   uchar device_name[6]={0xFE, 0x64, 0x0F, 0x00, 0x75, 0xE3};  //關閉CDM7160長期自動校正
  uchar device_name[8]={0xFE, 0x06, 0x00, 0x1F, 0x00, 0x00, 0xAC ,0x03};  //關閉S8自動校正
  
  bool ret_code = false;

  PUCHAR p_buff = UsartGetBuff(USART_ID_RX);
//   if(ServerSendModbusCmd((PUCHAR)CmdGetCo2_CDM7160,sizeof(CmdGetCo2_CDM7160)) == TRUE){
  if(ServerSendModbusCmd((PUCHAR)CmdGetCo2_GetS8,sizeof(CmdGetCo2_GetS8)) == TRUE){
      *value = WordSwap(*((PUINT16)&p_buff[3]));
      ret_code = TRUE;
  }else{
      ret_code = FALSE;
  }
  UsartResetRxTx(USART_ID_TX_RX);

  return ret_code;
}
bool GetSkynetCo2Info()
{
    bool ret_code = TRUE;
    uint16 co2 = 0;
    PSkynetCo2 p_sensor = &ServerInfo.SensorInfo.SkynetCo2;
    GetSkynetInfo();
    SetNodeInfoSize(_SkynetCo2);
    SetNodeClass(SENSOR_SKYNET_CO2);
    //GetDeviceInfoDelay=6000;
    PUCHAR p_buff = UsartGetBuff(USART_ID_RX);
    if(Get_CO2(&co2)){
        AdjCo2(&co2);
        p_sensor->Co2 = co2; 
        p_sensor->fCo2 = p_sensor->Co2;
        dprint("====> CO2: %d\r\n",p_sensor->Co2);
    }else{
        ret_code = FALSE;
        dprint("====> CO2: Read Faild.\r\n");
    }
    ret_code = TRUE;   
    return ret_code;
}

// --- PM2.5 PMSA003 ---
bool ServerSendPM25Cmd(PUCHAR cmd,uchar len)
{
    bool ret_code = TRUE;
    UsartTxSendCmd(cmd,len); 
    Delay_ms(10); Rs485Rx(); Delay_ms(150); Rs485Tx(); // must to check crc error
    ret_code = CheckPM25Valid(UsartGetBuff(USART_ID_RX), UsartGetRxCounter());

    if(!ret_code) TraceErr1("ServerSendPM25Cmd 1",UsartGetRxCounter());
    
    return ret_code;
}

bool GetSkynetPm25Info() {
    bool ret_code = TRUE;
    PSkynetPm25 p_sensor = &ServerInfo.SensorInfo.SkynetPm25;
    SetNodeInfoSize(_SkynetPm25);
    SetNodeClass(SENSOR_SKYNET_PM25);
    // GetDeviceInfoDelay=6000;
    PUCHAR p_buff = UsartGetBuff(USART_ID_RX);
    float pm25 = 0.0;
    float pm10 = 0.0;
    if (ServerSendPM25Cmd((PUCHAR)CmdGetPm25, 7) == TRUE) {
        CalculatePMSA003Value((uint8_t*)UsartGetBuff(USART_ID_RX), UsartGetRxCounter());
        pm25 = GetPMSA003Value_PM25();
        pm10 = GetPMSA003Value_PM10();
        AdjPm25(&pm25, &pm10);
        p_sensor->fPM25 = pm25;
        p_sensor->PM25 = p_sensor->fPM25 * 10;
        p_sensor->fPM10 = pm10;
        dprint("====> PM25: %.1f, PM10: %.1f\r\n", p_sensor->fPM25, p_sensor->fPM10);
    } else {
        ret_code = FALSE;
        dprint("====> PM25: Read Faild.\r\n");
    }
    return ret_code;
}

bool GetSkynetTvocInfo()
{
    bool ret_code = TRUE;
    uint16 tvoc;
    PSkynetTvoc p_sensor = &ServerInfo.SensorInfo.SkynetTvoc;
    SetNodeInfoSize(_SkynetTvoc);
    SetNodeClass(SENSOR_SKYNET_TVOC);
    if(GetNodeStatus(NS_FULL_POWER)) {
        ret_code = SGPxx_GetTvoc(&tvoc);
    } else {
        // 若從睡眠模式起來，取 pre read 的最大值
        ret_code = SGPxx_GetTvocMax(&tvoc);
    }

    if (ret_code) {
        AdjTvoc(&tvoc);
        p_sensor->TVOC = tvoc;
        p_sensor->fTVOC = (float)p_sensor->TVOC / 1000;
        dprint("====> TVOC: %d\r\n", p_sensor->TVOC);
    } else {
        dprint("====> TVOC: Read Faild.\r\n");
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
{
#ifdef BTM_A308
	return A308_Connected();
#else
    bool ret_code = TRUE;
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
         
        }else{ret_code = FALSE; return ret_code;}
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
        }
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
    ret_code = TRUE; 
    return ret_code;
#endif
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
    else {ret_code = FALSE;}
    ret_code = TRUE; 
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
      }
    return ret_code;
}



const uchar CmdGetUltraSound[MODBUS_CMD_NUM]={0x01, 0x04, 0x00, 0x00, 0x00, 0x02, 0x71, 0xCB};
const uchar CmdGetUltraSoundOther[MODBUS_CMD_NUM]={0x01, 0x04, 0x00, 0x1D, 0x00, 0x05, 0xA0, 0x0F};


//
//
//
bool GetUltraSoundInfo()
{
	bool ret_code = TRUE;
	int16 temp;
	uint16 Humidity;

	PUltraSoundInfo p_sensor = &ServerInfo.SensorInfo.UltraSound;
	SetNodeInfoSize(_UltraSoundInfo);
	SetNodeClass(SENSOR_ULTRA_SOUND);
	PUCHAR p_buff = UsartGetBuff(USART_ID_RX);
	if(ServerSendModbusCmd((PUCHAR)CmdGetUltraSound,MODBUS_CMD_NUM) == TRUE){
		p_sensor->WaterLevel = WordSwap(*((PUINT16)&p_buff[3]));
		p_sensor->OilLevel = WordSwap(*((PUINT16)&p_buff[5]));
	  }else {ret_code = FALSE;}
	UsartResetRxTx(USART_ID_TX_RX);
	if(ServerSendModbusCmd((PUCHAR)CmdGetUltraSoundOther,MODBUS_CMD_NUM) == TRUE){
		p_sensor->BatteryVol = WordSwap(*((PUINT16)&p_buff[3]));
		p_sensor->InputVol = WordSwap(*((PUINT16)&p_buff[5]));
		p_sensor->OutputVol = WordSwap(*((PUINT16)&p_buff[7]));
		p_sensor->ChargeCurr = WordSwap(*((PUINT16)&p_buff[9]));
		p_sensor->InputCurr = WordSwap(*((PUINT16)&p_buff[11]));
	  }
	dprint("===\r\nUltra Sound Info:\r\n");
	dprint("WaterLevel:%d\r\nOleLevel:%d\r\nBattery:%d\r\nInputVol:%d\r\nOutputVol:%d\r\nChargCurr:%d\r\nInputCurr:%d\r\n===\r\n",
			p_sensor->WaterLevel,p_sensor->OilLevel,p_sensor->BatteryVol,p_sensor->InputVol,p_sensor->OutputVol,p_sensor->ChargeCurr,p_sensor->InputCurr);

	if(GetTempAndRH(&temp,&Humidity) != VALUE_IS_NOT_KNOWN){ //TraceDec2("GetUltraSoundInfo Temp & RH=> 1", temp,Humidity);
		AdjTempRh(&temp,&Humidity);
		p_sensor->Tempature = temp;
		p_sensor->Humidity  = Humidity;
	  }
	ret_code = TRUE;

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
else {ret_code = FALSE;}
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
ret_code = TRUE; 

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
  {
    p_sensor->AiValue1 = WordSwap(*p_value++); p_sensor->AiValue2 = WordSwap(*p_value++);    
    p_sensor->AiValue3 = WordSwap(*p_value++); p_sensor->AiValue4 = WordSwap(*p_value++);
    p_sensor->AiValue5 = WordSwap(*p_value++); p_sensor->AiValue6 = WordSwap(*p_value++);
    p_sensor->AiValue7 = WordSwap(*p_value++); p_sensor->AiValue8 = WordSwap(*p_value++);
  }
else {ret_code = FALSE;}
UsartResetRxTx(USART_ID_TX_RX);

if(ServerSendModbusCmd((PUCHAR)CmdA6D6_DI,MODBUS_CMD_NUM) == TRUE)
  {
    p_sensor->Di_Status = A6D6_DI_VALUE;
  }
UsartResetRxTx(USART_ID_TX_RX);

if(ServerSendModbusCmd((PUCHAR)CmdA6D6_DO,MODBUS_CMD_NUM) == TRUE)
  {
    p_sensor->DO_Status = A6D6_DO_VALUE;
  }

UsartResetRxTx(USART_ID_TX_RX);
ret_code = TRUE; 

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
  }else {ret_code = FALSE;}
  ret_code = TRUE; 
return ret_code;
}


bool GetRelay()
{
	bool ret_code = TRUE;
	PRelayNode p_sensor = &ServerInfo.SensorInfo.RelayNode;
	uint16 loop;

	SetNodeInfoSize(_RelayNode);
	SetNodeClass(SENSOR_RELAY);

	p_sensor->Status++;;


	return ret_code;
}

#define OEM_SHOW_RESULT(addr,val,result) dprint("Read %s OEM Addr:%d, val:%d\r\n",result?"":"!!! Error", addr,val);
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
	if(ServerSendModbusCmd((PUCHAR)Fun3Addr00,MODBUS_CMD_NUM) == TRUE){
		p_sensor->Addr00 = WordSwap(*((PUINT16)&p_buff[3]));
	  }
	else {ret_code = FALSE;}
	OEM_SHOW_RESULT(0,p_sensor->Addr00,ret_code);

	UsartResetRxTx(USART_ID_TX_RX);

	if(ServerSendModbusCmd((PUCHAR)Fun3Addr01,MODBUS_CMD_NUM) == TRUE){
		p_sensor->Addr01 = WordSwap(*((PUINT16)&p_buff[3]));
	  }
	else {ret_code = FALSE;}
	UsartResetRxTx(USART_ID_TX_RX);
	OEM_SHOW_RESULT(1,p_sensor->Addr01,ret_code);

	if(ServerSendModbusCmd((PUCHAR)Fun3Addr02,MODBUS_CMD_NUM) == TRUE){
		p_sensor->Addr02 = WordSwap(*((PUINT16)&p_buff[3]));
	  }
	else {ret_code = FALSE;}
	UsartResetRxTx(USART_ID_TX_RX);
	OEM_SHOW_RESULT(2,p_sensor->Addr02,ret_code);

	if(ServerSendModbusCmd((PUCHAR)Fun3Addr03,MODBUS_CMD_NUM) == TRUE){
		p_sensor->Addr03 = WordSwap(*((PUINT16)&p_buff[3]));
	  }
	else {ret_code = FALSE;}
	UsartResetRxTx(USART_ID_TX_RX);
	OEM_SHOW_RESULT(3,p_sensor->Addr03,ret_code);

	if(ServerSendModbusCmd((PUCHAR)Fun3Addr0A,MODBUS_CMD_NUM) == TRUE){
		p_sensor->Addr0A = WordSwap(*((PUINT16)&p_buff[3]));
	  }
	else {ret_code = FALSE;}
	UsartResetRxTx(USART_ID_TX_RX);
	OEM_SHOW_RESULT(0x0a,p_sensor->Addr0A,ret_code);

	ret_code = TRUE; //

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
if(ServerSendModbusCmd((PUCHAR)CmdGetIaqsCw9Info,MODBUS_CMD_NUM) == TRUE){//PrintData("GetIaqsInfo", p_value, IAQS_DATA_LEN);
    for(loop=0; loop<IAQS_DATA_LEN; loop++) 
        *p_sensor_value++ = WordSwap(*p_value++);
  }else {ret_code = FALSE;}
     
ret_code = TRUE; 

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
if(ServerSendModbusCmd((PUCHAR)CmdGetIaqsCw9Info,MODBUS_CMD_NUM) == TRUE){//PrintData("GetCw9Info 1", p_value, CW9_DATA_LEN);
    for(loop=0; loop<CW9_DATA_LEN; loop++){ 
        *p_sensor_value++ = WordSwap(*p_value++); 
    }
  }
else {ret_code = FALSE;}
ret_code = TRUE; 

return ret_code;
}


const uchar RawVelocityCmd01[MODBUS_CMD_NUM] ={0x01,0x03,0x00,0x09,0x00,0x01,0x54,0x08};
const uchar RawVelocityCmd02[MODBUS_CMD_NUM] ={0x01,0x03,0x00,0x0A,0x00,0x01,0xA4,0x08};

const uchar VelocityCmd01[MODBUS_CMD_NUM] ={0x01,0x03,0x04,0x01,0x00,0x01,0xD4,0xFA};
const uchar VelocityCmd02[MODBUS_CMD_NUM] ={0x01,0x03,0x04,0x02,0x00,0x01,0x24,0xFA};

const uchar VelocityTempCmd01[MODBUS_CMD_NUM] ={0x01,0x03,0x04,0x15,0x00,0x01,0x94,0xFE};
const uchar VelocityTempCmd02[MODBUS_CMD_NUM] ={0x01,0x03,0x04,0x16,0x00,0x01,0x64,0xFE};



//
//for Velocity
//
bool GetVelocityInfo()
{
    bool ret_code = TRUE;
    PUINT16 p_value,p_sensor_value;
    uint16 loop;
    float   temp_value;
    PUINT16   p_uint;
    
    PVelocity p_sensor = &ServerInfo.SensorInfo.Velocity;
    SetNodeInfoSize(_Velocity);
    SetNodeClass(SENSOR_VELOCITY);


    //to get Raw Velocity
    PUCHAR p_buff = UsartGetBuff(USART_ID_RX);

    p_uint = (PUINT16)&temp_value;
    if(ServerSendModbusCmd((PUCHAR)RawVelocityCmd01,MODBUS_CMD_NUM) == TRUE){
        *p_uint = WordSwap(*((PUINT16)&p_buff[3])); 
      }
    else{
        return FALSE;}
    UsartResetRxTx(USART_ID_TX_RX);

    p_uint++;
    if(ServerSendModbusCmd((PUCHAR)RawVelocityCmd02,MODBUS_CMD_NUM) == TRUE){
        *p_uint = WordSwap(*((PUINT16)&p_buff[3])); 
        p_sensor->RawFlowVelocity = temp_value;
      }
    UsartResetRxTx(USART_ID_TX_RX);

    //to get Raw Velocity
    
    p_buff = UsartGetBuff(USART_ID_RX);
    p_uint = (PUINT16)&temp_value;
    if(ServerSendModbusCmd((PUCHAR)VelocityCmd01,MODBUS_CMD_NUM) == TRUE){
        *p_uint = WordSwap(*((PUINT16)&p_buff[3])); 
      }
    UsartResetRxTx(USART_ID_TX_RX);

    p_uint++;
    if(ServerSendModbusCmd((PUCHAR)VelocityCmd02,MODBUS_CMD_NUM) == TRUE){
        *p_uint = WordSwap(*((PUINT16)&p_buff[3])); 
        p_sensor->FlowVelocity = temp_value;
      }
    UsartResetRxTx(USART_ID_TX_RX);

    //to get Temperature
    p_buff = UsartGetBuff(USART_ID_RX);
    p_uint = (PUINT16)&temp_value;
    if(ServerSendModbusCmd((PUCHAR)VelocityTempCmd01,MODBUS_CMD_NUM) == TRUE){
        *p_uint = WordSwap(*((PUINT16)&p_buff[3])); 
      }
    UsartResetRxTx(USART_ID_TX_RX);

    p_uint++;
    if(ServerSendModbusCmd((PUCHAR)VelocityTempCmd02,MODBUS_CMD_NUM) == TRUE){
        *p_uint = WordSwap(*((PUINT16)&p_buff[3])); 
        p_sensor->Tempature = temp_value;
      }
    UsartResetRxTx(USART_ID_TX_RX);

    if(p_sensor->RawFlowVelocity < 0) p_sensor->RawFlowVelocity = 0;
    if(p_sensor->RawFlowVelocity < 0) p_sensor->RawFlowVelocity = 0;
    ret_code = TRUE; 
    return ret_code;
}

void ParseJYGD15Info(){
	//dprint("ParseJYGD15Info()...\r\n");
	_JYGD15Info *p_sensor = &ServerInfo.SensorInfo.JYGD15Info;
	PUCHAR rx=UsartGetBuff(USART_ID_RX);
	int len=rx[2];
	if(CounterRx<3 || CounterRx<(5+len/2)) { dprint("receive data is too short. len:%d\r\n",CounterRx);return;} /*長度不足*/
	if(rx[1]&0x80) {dprint("receive error code\r\n");return;}/*錯誤碼*/
	for(int i=0;i<14;i++){
		p_sensor->values[i]=(((uint16)rx[3+i*2])<<8) | rx[4+i*2];
		dprint(">%d ",p_sensor->values[i]);
	}
	dprint("\r\n");
}


bool GetJYGD15Info(){
	//dprint("GetJYGD15Info()...\r\n");
	SetNodeInfoSize(_JYGD15Info);
	SetNodeClass(SENSOR_JYGD15);
	MbsSetReadRegCmd(1,3,0,14);
	SetNodeStatus(NS_USART_WAIT_RESP,ON);
	parseMbsResponse=ParseJYGD15Info;
	return TRUE;
}



bool GetCustomSerial(){
	SetNodeInfoSize(_BtMeshInfo);
	SetNodeClass(SENSOR_CUSTOM_SERIAL);
	/*uchar res[]={5,4,0,8};
	memcpy(RxBuff,res,4);
	CounterRx=4;*/

	/*if(TransArrayLength&&!UsartIsBusy()){
		return UsartTxSendCmd(TransData,TransArrayLength);
	}else{
		return FALSE;
	}*/
	return TRUE;
}




//
//Send sensor information to client
//
uint16 SendInfoToClient()
{
    //bool ret_code=FALSE;
	uint16 result;
    uchar send_size;
    if(ServerInfo.NodeInfoSize > 0){
        if(GetNodeStatus(NS_FULL_POWER) == ON)
           pSensorHeader->Status |= SERVER_FULL_POWER;
        else
           pSensorHeader->Status &= ~SERVER_FULL_POWER; 
        
        send_size = ServerInfo.NodeInfoSize+3;
        //ServerInfo.ProperityID = NODE_GET_ALL_SENSOR;
        //ServerInfo.ProperityID = pNodeEventInfo->PropertyID;
        ServerInfo.ProperityID = NODE_GET_ALL_SENSOR_GEN2;
        if(ServerResponseTestMs){
        	result = Cmd_ms_server_send_column_status(SENSOR_ELEMENT, pNodeEventInfo->ClientAddr,pNodeEventInfo->AppkeyIndex,
        	                                            NO_FLAGS,NODE_GET_ALL_SENSOR_GEN2, send_size, (PUCHAR)&ServerInfo)->result;
        }else{
        	result = Cmd_ms_server_send_status(SENSOR_ELEMENT, pNodeEventInfo->ClientAddr,pNodeEventInfo->AppkeyIndex,
                                            NO_FLAGS, send_size, (PUCHAR)&ServerInfo)->result;
        }
      if(result) {
    	dprint("!!!Cmd_ms_server_send_status, dest addr:%d, len:%d, ERROR:0x%x\r\n",pNodeEventInfo->ClientAddr,send_size,result);
        //ret_code = FALSE;
      }
      //else ret_code=TRUE;
    }
    else{
    	//TraceErr1("SendInfoToClient 2",ServerInfo.NodeInfoSize);
    	dprint("SendInfoToClient Error NodeInfoSize(%d)<=0\r\n",ServerInfo.NodeInfoSize);
    	//return TRUE;
    	return  0;
    }
    //return ret_code;
    return result;
}

uint16 Server_ResponseInfo(){

	NodeInfo info;
	//uint16 result;
	info.SensorClass= ServerInfo.SensorInfo.Header.SensorClass;
	info.BatteryPower=ServerInfo.SensorInfo.Header.BatteryPower;
	info.Status=ServerInfo.SensorInfo.Header.Status;
	info.Version=FW_VER;
	memcpy(info.ModelName,MODEL_NAME,6);
	info.ProtocolGen=BTM_PROTOCOL_GEN;
	info.DeviceNameChartCount=sizeof(DEVICE_NAME)-1;
	if(info.DeviceNameChartCount>sizeof(info.DeviceName)) info.DeviceNameChartCount=sizeof(info.DeviceName);
	memcpy(info.DeviceName,DEVICE_NAME,info.DeviceNameChartCount);

	return Cmd_ms_server_send_column_status(SENSOR_ELEMENT, pNodeEventInfo->ClientAddr, IGNORED, 0xA5, NODE_GET_BTM_INFO,sizeof(info),(uint8*)&info)->result;
	//return Cmd_ms_server_send_status(SENSOR_ELEMENT, pNodeEventInfo->ClientAddr,pNodeEventInfo->AppkeyIndex, NO_FLAGS, sizeof(info), (uint8*)&info)->result;
}

uint16 Server_ResponseSetting(uint8 *in,uint8 len){

   uint16 result;
   uint8 data[sizeof(_BtAppData)+2];
   _BtAppData *set=(void*)(data+2);
   *((uint16*)data)=NODE_SENSOR_SETUP_GET;
   set->TempGain = (int16)(pAdjValue->TempGain*MESH_INFO_SCALING);
   set->TempOffset = (int16)(pAdjValue->TempOffset*MESH_INFO_SCALING);
   set->RhGain = (int16)(pAdjValue->HumGain*MESH_INFO_SCALING);
   set->RhOffset = (int16)(pAdjValue->HumOffset*MESH_INFO_SCALING);
   set->WorkingTimer = pMeshNodeData->WorkingTimer;
   set->BtmClass = SensorClassChange(pMeshNodeData->SensorClass,CLASS_TO_UTILITY);
   set->BaudrateIndex=pMeshNodeData->BaudRate;
   set->Rs485ClientBuffTimeoutMs=pAdjValue->RS485TransmitterData.Rs485ClientBuffTimeoutMs;
   set->Rs485ServerDelayBeforeSleep=pAdjValue->RS485TransmitterData.Rs485ServerDelayBeforeSleep;
   set->ProtocolGen=BTM_PROTOCOL_GEN;
   set->RebootForRs485IdelSecnods=pMeshNodeData->RebootForRs485IdelSecnods;
   set->RebootMinutes=pMeshNodeData->RebootMinutes;

   if (len>=1){/*停止回應廣播指令 in[0] 秒*/
	   IgnoreBroadcastCmdMs=in[0]*1000;
	   dprint("Response Setting: Ignore Broadcast Request for %.1f sec\r\n",IgnoreBroadcastCmdMs/1000.0);
   }


   result= Cmd_ms_server_send_column_status(SENSOR_ELEMENT, pNodeEventInfo->ClientAddr, pNodeEventInfo->AppkeyIndex, 0xA5, NODE_SENSOR_SETUP_GET,sizeof(_BtAppData)+2, data)->result;
   dprint("*** Server_ResponseSetting result:0x%X\r\n",result);

   return result;
}

void DebugShowSetting(){
    dprint("> TempGain:%f\r\n",pAdjValue->TempGain);
    dprint("> TempOffset:%f\r\n",pAdjValue->TempOffset);
    dprint("> HumGain:%f\r\n",pAdjValue->HumGain);
    dprint("> HumOffset:%f\r\n",pAdjValue->HumOffset);
    dprint("> WorkingTimer:%d\r\n",pMeshNodeData->WorkingTimer);
    dprint("> SensorClass:%d\r\n",pMeshNodeData->SensorClass);
    dprint("> Baudrate:%d(%d)\r\n",pMeshNodeData->BaudRate,IndexToBaudrate(pMeshNodeData->BaudRate));
}

//if (len>=((uint8*)&(set->BaudrateIndex)-(uint8*)set))pMeshNodeData->BaudRate=set->BaudrateIndex;
/*避免被舊版App誤設. ※舊版_BtAppData長度較短*/
#define NODE_DATA_UPDATE(dest,src,exp) do{if (len>=((uint8*)&(src)-(uint8*)set)){dest=src;if(dest!=src){exp;}}}while(0)

extern void UsartBaudrateChange();
uint16 Server_ReceiveSetting(uint8 *data,uint8 len){
	_BtAppData *set=(void*)(data);
	uint16 result;
	uint16 code=ACK_OK;
	uint8 req_reboot=0;
    pAdjValue->TempGain = (float)(set->TempGain)/MESH_INFO_SCALING;
    pAdjValue->TempOffset = (float)(set->TempOffset)/MESH_INFO_SCALING;
    pAdjValue->HumGain = (float)(set->RhGain)/MESH_INFO_SCALING;
    pAdjValue->HumOffset = (float)(set->RhOffset)/MESH_INFO_SCALING;
    pMeshNodeData->WorkingTimer = set->WorkingTimer;
    pMeshNodeData->SensorClass = SensorClassChange(set->BtmClass,CLASS_TO_BTM);
    NODE_DATA_UPDATE(pMeshNodeData->BaudRate,set->BaudrateIndex,req_reboot=1);
    NODE_DATA_UPDATE(pAdjValue->RS485TransmitterData.Rs485ClientBuffTimeoutMs,set->Rs485ClientBuffTimeoutMs,);
    NODE_DATA_UPDATE(pAdjValue->RS485TransmitterData.Rs485ServerDelayBeforeSleep,set->Rs485ServerDelayBeforeSleep,);
    NODE_DATA_UPDATE(pMeshNodeData->RebootForRs485IdelSecnods,set->RebootForRs485IdelSecnods,);
    NODE_DATA_UPDATE(pMeshNodeData->RebootMinutes,set->RebootMinutes,);

    WriteNodeData();
    dprint("*** Server_ReceiveSetting\r\n");
    DebugShowSetting();

    IgnoreBroadcastCmdMs=0;

    result= Cmd_ms_server_send_column_status(SENSOR_ELEMENT, pNodeEventInfo->ClientAddr, pNodeEventInfo->AppkeyIndex, 0xA5, NODE_SENSOR_SETUP_SET,sizeof(code), (uint8*)&code )->result;
    dprint("*** result:0x%X\r\n",result);

    if(req_reboot)Cmd_sys_reset(0); //reboot
    return result;
};




//#ifdef BTM_TRANSMITTER
typedef struct __attribute__((__packed__)){
  //uint16 ProperityID;
  uint8 Seq;
  uint8 Size;
  uint8 Flag;
  uint16 Location;
  uint8 RegCount;
  uint8 Data[50];
} _ServerSerialRsp;

uint8 SeriesSeq;
bool SendRxToClient(uint16 reg_loc, uint16 reg_count, PUCHAR data,uint8_t offset,uint8_t count){
	_ServerSerialRsp ServerSerialRsp;
	uint16 len;
	uint16 result;
	ServerSerialRsp.Seq=SeriesSeq;
	ServerSerialRsp.Size=count;
	ServerSerialRsp.Flag=0x01<<(offset/50);
	ServerSerialRsp.Location=reg_loc;
	ServerSerialRsp.RegCount=reg_count;
	len=(count-offset)>50?50:(count-offset);
	memcpy(ServerSerialRsp.Data,data+offset,len);
	result=Cmd_ms_server_send_series_status(SENSOR_ELEMENT, 0/*pNodeEventInfo->ClientAddr*/,pNodeEventInfo->AppkeyIndex,
		  NO_FLAGS,CUSTOM_SERIAL_DATA, len+6, ((PUCHAR)&ServerSerialRsp))->result;
	dprint(" *** SendRxToClient Seq:%d(%d,%d), len:%d, result:%d\r\n",ServerSerialRsp.Seq,offset/50+1,count/50+1, len,result);

  return result?FALSE:TRUE;
}


//#endif

uint16 TransSourceAddress;
uint16 TransAppkeyIndex;
//
//
//
void EvtGetRequestProc(PCmdPacket pCmdEvent)
{
	uint16 result;
    msg_ms_server_get_request_evt *pEvent = &(pCmdEvent->data.evt_mesh_sensor_server_get_request);
    _NodeEventInfo tmpNodeEvtInfo=*pNodeEventInfo;
    pNodeEventInfo->ElemIndex   = pEvent->appkey_index;
    pNodeEventInfo->ClientAddr  = pEvent->client_address;
    pNodeEventInfo->ServerAddr  = pEvent->server_address;
    pNodeEventInfo->AppkeyIndex = pEvent->appkey_index;
    pNodeEventInfo->Flags       = pEvent->flags;
    pNodeEventInfo->PropertyID  = pEvent->property_id;
    uint8array* ext=0;

    if(IgnoreBroadcastCmdMs && pEvent->server_address==0xC000){
    	dprint("Ignore Broadcate Request from:%d for %.1f Sec\r\n",pEvent->client_address,IgnoreBroadcastCmdMs/1000.0);
    	return;
    }


    if (BGLIB_MSG_ID(pCmdEvent->header)==Evt_ms_server_get_column_req){
		ext=&pCmdEvent->data.evt_mesh_sensor_server_get_column_request.column_ids;

		IFDPRINT( // show received modbus data
		  dprint("btm Request(%d->%d) evt:%08X, property:0x%x\r\n",pEvent->client_address,pEvent->server_address,pCmdEvent->header,pEvent->property_id);
		  if (ext){
			  dprint("ext cmd (flag:%d)>",ext->data[0]);
			  for (int i=0;i<(ext->len-1);i++)dprint(" %02x",ext->data[i+1]);
			  dprint("\r\n");
		  }
		);

		if(pEvent->property_id==CUSTOM_SERIAL_DATA){
			if(ext && ext->len>1 && ext->len<=USART_TX_BUFF_SIZE){
				SeriesSeq=ext->data[0];
				TransArrayLength=ext->len-1;
				memcpy(TransData,ext->data+1,TransArrayLength);

				SetNodeStatus(NS_TRANS_SERIAL_DATA,ON);
				//printf("Set Node Sataus: NS_TRANS_SERIAL_DATA\r\n");
				dprint("Set Node Sataus: NS_TRANS_SERIAL_DATA\r\n");
				return;
			}else{
				dprint("request uart data len is over(%d/%d)\r\n",ext->len,USART_TX_BUFF_SIZE);
			}
		}/*else if(pEvent->property_id==NODE_GET_BTM_INFO){
			Server_ResponseInfo();
			return;
		}*/else if(pEvent->property_id==NODE_SENSOR_SETUP_GET){
			Server_ResponseSetting(ext->data,ext->len);
			*pNodeEventInfo=tmpNodeEvtInfo;
			return;
		}else if(pEvent->property_id==NODE_SENSOR_SETUP_SET){
			Server_ReceiveSetting(ext->data,ext->len);
			*pNodeEventInfo=tmpNodeEvtInfo;
			return;
		}else if(pEvent->property_id == NODE_GET_ALL_SENSOR_GEN2){
			SetNodeStatus(NS_GET_INFO_ACT,ON);
			if (ext->len>=2) ServerResponseTestMs=(ext->data[0]+ext->data[1]*0x100)*1000;
			*pNodeEventInfo=tmpNodeEvtInfo;
			return;
		}
#ifdef BTM_A308
		else if(pEvent->property_id==NODE_GET_A308_TABLE){
			dprint("request A308 Table flag:0x%08X\r\n",*(uint32*)ext->data);
			A308_GetInfo_Set_Flag(*(uint32*)ext->data);
		}
#endif

    }

    //Trace16_1(pEvent->property_id);
    if(pEvent->property_id == PROP_NODE_INFO){
    	dprint("*** Receive PROP_NODE_INFO:");
    	result=Server_ResponseInfo();
    	dprint("%s, code:0x%X\r\n===\r\n",result?"failed":"successed",result);
    	*pNodeEventInfo=tmpNodeEvtInfo;
    }else if(pEvent->property_id == PROP_SERVER_ACK){
    	dprint("*** receive Server Ack\r\n");
    	SetNodeStatus(NS_SEND_INFO_ACK,ON);
    }
    else if(pEvent->property_id == NODE_GET_ALL_SENSOR){
    	dprint("receive cmd: GET_ALL_SENSOR\r\n");
         SetNodeStatus(NS_GET_INFO_ACT,ON);  // Mesh get event active
         if(pEvent->server_address!=0xC000){ /*非廣播指令，直接回應(for APP)*/
        	 SendInfoToClient();
        	 *pNodeEventInfo=tmpNodeEvtInfo;
         }
     }
    else if(pEvent->property_id ==NODE_GET_A308_TABLE){
    	 SetNodeStatus(NS_A308_GET_INFO,ON);
     }
    else if(pEvent->property_id == NODE_GET_INFO_FULL_POWER_ON){
        SetNodeStatus(NS_GET_INFO_ACT,ON);
        SetForceFullPowerTime(ON);
        SetNodeStatus(NS_FULL_POWER,ON);   // start to get sensor info 
        pSensorHeader->Status |= SERVER_FULL_POWER;
         
     }
    else if(pEvent->property_id == NODE_GET_INFO_FULL_POWER_OFF){ 
         SetNodeStatus(NS_GET_INFO_ACT,ON);
         SetForceFullPowerTime(OFF);
     }    
    else if(pEvent->property_id >= NODE_SET_AIP_POWER_OFF && pEvent->property_id <= NODE_SET_AIP_POWER_100){
         SetNodeStatus(NS_SET_NODE_ACT,ON);  // Mesh set event active
     }    
    else if(pEvent->property_id >= NODE_SET_DO1_ON && pEvent->property_id <= NODE_SET_DO6_OFF){
         SetNodeStatus(NS_SET_NODE_ACT,ON);  // Mesh set event active
    }    
    else if(pEvent->property_id == NODE_GET_BTM_INFO){
    	ResponseBtmMeshInfo();
    	*pNodeEventInfo=tmpNodeEvtInfo;
         //SetNodeStatus(NS_GET_INFO_ACT,ON);  // Mesh set event active
    }    
    else if(pEvent->property_id >= BTM_G6_CMD_START && pEvent->property_id <= BTM_G6_CMD_END){//Trace("G6-Setup");
             SetNodeStatus(NS_SET_NODE_ACT,ON);  // Mesh set event active
        }
    else if(pEvent->property_id==NODE_A308_GET_FINISHED){
    	SetNodeStatus(NS_A308_GET_FINISHED,ON);
    	dprint("\r\nA308 Receive Get info fhinised notify.\r\n");
    }
    else{
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

//
//
//
uint16  SensorClassChange(uint16 class,uint8 status)
{
    if(status == CLASS_TO_UTILITY){ // change to Utility or App sensor class
        switch(class){
        case SENSOR_PZEM: return 2;
        case SENSOR_OEM: return 3;
        case SENSOR_AGB_POWER: return 4;
        case SENSOR_VELOCITY: return 5;
        case SENSOR_JYGD15: return 6;
        case SENSOR_CUSTOM_SERIAL: return 7;
        default: return 1;
        }
        //TraceDec1("To Utility",ret_code);
    }
    else{// change to BTM Class
        switch(class){
        case 2: return SENSOR_PZEM;
        case 3: return SENSOR_OEM;
        case 4: return SENSOR_AGB_POWER;
        case 5: return SENSOR_VELOCITY;
        case 6: return SENSOR_JYGD15;
        case 7: return SENSOR_CUSTOM_SERIAL;
        default :return 1;
        }
		/*if(class == 2) ret_code = SENSOR_PZEM;
		else if(class == 3) ret_code = SENSOR_OEM;
		else if(class == 4) ret_code = SENSOR_AGB_POWER;
		else if(class == 5) ret_code = SENSOR_VELOCITY;
		else ret_code = 1;*/
     //TraceDec1("To BTM",ret_code);
    }
}


//
// for Set setting event
//
void EvtSetSettingRequestProc(PCmdPacket pCmdEvent)
{// for Android App
    msg_ms_setup_server_set_setting_request_evt *pEvent = &(pCmdEvent->data.evt_mesh_sensor_setup_server_set_setting_request);
    uint16 ret_ack=ACK_OK;
    uint16 setting_data;
    uint16 setting_id=-1;
    PBtAppData set;
    uint len=pEvent->raw_value.len;

    Trace16_1(pEvent->property_id);
    set = (PBtAppData)(&pEvent->raw_value.data);
    Trace16_4(set->TempGain,set->TempOffset,set->RhGain,set->RhOffset);
    if(pEvent->property_id == NODE_SENSOR_SETUP_GET)
        {
         setting_id = pEvent->setting_id;
         setting_data = *(PUINT16)set; Trace16_1(setting_data);
        }
    else { return;}
    
    switch(setting_id)
        {
            case ALL_SETTING_ID:
              //  flag_write_data = ON;
                pAdjValue->TempGain = (float)(set->TempGain)/MESH_INFO_SCALING;
                pAdjValue->TempOffset = (float)(set->TempOffset)/MESH_INFO_SCALING;
                pAdjValue->HumGain = (float)(set->RhGain)/MESH_INFO_SCALING;
                pAdjValue->HumOffset = (float)(set->RhOffset)/MESH_INFO_SCALING;
                pMeshNodeData->WorkingTimer = set->WorkingTimer;
                pMeshNodeData->SensorClass = SensorClassChange(set->BtmClass,CLASS_TO_BTM);
                NODE_DATA_UPDATE(pMeshNodeData->BaudRate,set->BaudrateIndex,);
                NODE_DATA_UPDATE(pAdjValue->RS485TransmitterData.Rs485ClientBuffTimeoutMs,set->Rs485ClientBuffTimeoutMs,);
                NODE_DATA_UPDATE(pAdjValue->RS485TransmitterData.Rs485ServerDelayBeforeSleep,set->Rs485ServerDelayBeforeSleep,);
                NODE_DATA_UPDATE(pMeshNodeData->RebootForRs485IdelSecnods,set->RebootForRs485IdelSecnods,);
                NODE_DATA_UPDATE(pMeshNodeData->RebootMinutes,set->RebootMinutes,);
                break;
  
            default: ret_ack=ACK_ERROR; 
        };

    if(ret_ack == ACK_OK){ 
        WriteNodeData();         
        } 
    else{ ret_ack = ACK_ERROR;}

    Cmd_ms_setup_server_send_setting_status(SENSOR_ELEMENT,pEvent->client_address, pEvent->appkey_index, NO_FLAGS,
                                            pEvent->property_id, pEvent->setting_id,sizeof(ret_ack), (uchar *)&ret_ack);
    if(setting_id==ALL_SETTING_ID) SetNodeStatus(NS_REBOOT,ON);//Cmd_sys_reset(0); //reboot
    
}

//
// for Get setting event
//
void EvtSetGettingRequestProc(PCmdPacket pCmdEvent)
{//for Android App
	uint16 result;
    uint16 setting_id=-1;
    _BtAppData BtSetupData;

    uchar flag_write_data=OFF;
    msg_ms_setup_server_get_setting_request_evt *pEvent = &(pCmdEvent->data.evt_mesh_sensor_setup_server_get_setting_request);
    if(pEvent->property_id == NODE_SENSOR_SETUP_GET)
        {setting_id = pEvent->setting_id;         
        }
    else { return;}

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
                   BtSetupData.BtmClass = SensorClassChange(pMeshNodeData->SensorClass,CLASS_TO_UTILITY);
                   BtSetupData.BaudrateIndex=pMeshNodeData->BaudRate;
                   BtSetupData.Rs485ClientBuffTimeoutMs=pAdjValue->RS485TransmitterData.Rs485ClientBuffTimeoutMs;
                   BtSetupData.Rs485ServerDelayBeforeSleep=pAdjValue->RS485TransmitterData.Rs485ServerDelayBeforeSleep;
                   BtSetupData.ProtocolGen=BTM_PROTOCOL_GEN;
                   BtSetupData.RebootForRs485IdelSecnods=pMeshNodeData->RebootForRs485IdelSecnods;
                   BtSetupData.RebootMinutes=pMeshNodeData->RebootMinutes;
                   break;
           };
    result=Cmd_ms_setup_server_send_setting_status(SENSOR_ELEMENT,pEvent->client_address, pEvent->appkey_index, NO_FLAGS,
                                            pEvent->property_id, pEvent->setting_id,sizeof(_BtAppData), (uchar *)&BtSetupData)->result;
    dprint("\r\n*** Get Setting Setting. result:0x%X***\r\n",result);
}


//
// return power status
//
uchar CheckPowerStatus()
{
    uchar power = POWER_USB;
    
#ifdef  BT_MESH_G6
    return POWER_USB;
#endif

    if(GetBatteryPower() > 0) power = POWER_BATTERY;

    return power;
}





void BtMeshSetupInit()
{
   UsartRxCount = 8; 
}


void BtMeshSetupTask()
{
    //UsartClientProc(); /*移到app.c*/
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
extern const char BtmModelName[6]; //BTM001

//
//
//
void MeshNodeSetupProc()
{
    pStageInfo = GetNodeStageInfo(MESH_NODE_SETUP_PROC);
    switch(ActiveStage())
        {
            
            case NSS_NODE_STAGE_INIT: 
                 UsartResetRxTx(USART_ID_RX); SetNodeStatus(NS_USART_RX_EVENT,OFF);
                 pModbusCmd= (_PModbusCmdF4)UsartGetBuff(USART_ID_RX);
                 Rs485Rx();
                 ToNextStage(NSS_RX_INFO_WAITING);   
                break;
                
            case NSS_RX_INFO_WAITING: 
                 if(GetNodeStatus(NS_USART_RX_EVENT) == TRUE){
                     //SetNodeStatus(NS_USART_RX_EVENT,OFF);
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
                if(pModbusCmd->FunCode == 0x03){// To Get information
                    if(MeshNodeGetInfoProc() == TRUE) ToNextStage(NSS_SEND_INFO);
                    else ToNextStage(NSS_SETUP_ERROR);
                }
                else if(pModbusCmd->FunCode == 0x06) {
                    if(ModbusSetupFC6() == TRUE) ToNextStage(NSS_SETUP_ACK);
                    else ToNextStage(NSS_SETUP_ERROR);
                }
                else {ToNextStage(NSS_SETUP_ERROR);}
                break;
            case NSS_SEND_INFO: 
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
            
            case NSS_SETUP_ERROR: 
                ToNextStage(NSS_NODE_STAGE_INIT);
                break;
        };
    
    
}

//
//
//
bool ModbusSetupFC6()
{   
   bool ret_code=TRUE;    
   if(pMeshNodeData->SensorClass == SENSOR_BTM_G6)
    ret_code = G6SetupInfo();
   else
    ret_code = MeshNodeSetInfoProc();
   return ret_code;
     
}




//
// G6 FC6
//
bool G6SetupInfo()
{
    bool ret_code=TRUE;    
    _PModbusCmdF6 p_cmd_fc6;
    uint16 start_addr, value;
    p_cmd_fc6 = (_PModbusCmdF6)pModbusCmd;
    start_addr = WordSwap(p_cmd_fc6->StartAddr);
    value = WordSwap(p_cmd_fc6->Value);
    if(start_addr == G6_ADDR_BEHAVIOR){
       G6SetBehavior(start_addr,value);
    }
    else if(start_addr >= POWER_PERCENT_SEG0 && start_addr <= POWER_PERCENT_SEG4 )
        G6SetSegPowerPercent(start_addr,value);
    else if(start_addr >= RTC_ADDR_YEAR && start_addr <= RTC_ADDR_WEEK )
        G6SetInitRtc(start_addr,value);
    else if(start_addr >= SEG1_ON_OFF && start_addr <= SEG5_ON_OFF )
        G6SetSegOnOff( start_addr, value);
    else if(start_addr >= SEG1_POWER && start_addr <= SEG5_POWER )
        G6SetSegPower( start_addr, value);
    else if(start_addr >= SEG1_WEEK && start_addr <= SEG5_WEEK )
        G6SetSegWeek( start_addr, value);
    else if(start_addr >= SEG1_START_TIME && start_addr <= SEG5_START_TIME )
        G6SetSegStartTime( start_addr, value);
    else if(start_addr >= SEG1_END_TIME && start_addr <= SEG5_END_TIME )
        G6SetSegEndTime( start_addr, value);
    else if(start_addr >= RUNING_TIME_FILTER1 && start_addr <= RUNING_TIME_FILTER2 )
        G6SetTimeFilter( start_addr, value);
    else ret_code = FALSE;

    if(ret_code == TRUE) WriteAdjValue(); // save info
    
    return ret_code;
}

_DevDate    InitDevDate;


bool G6SetSegPowerPercent(uint16 addr,uint16 value)
{//TraceDec2("G6SetSegPowerPercent", addr,value);
    bool ret_code=TRUE;
    uint16 index;
    index = addr - POWER_PERCENT_SEG0;
    if(index < 5){
        pMeshNodeData->SegPPercent[index] =  value; 
        WriteMeshNodeData();
        }
    return ret_code;
}

//
// 1. Set Power Percent
// 2. Clear Filter Time
// 3. Set Filter Warning Time
//
bool G6SetBehavior(uint16 addr,uint16 value)
{
    BYTE speed,status;    
   speed = pMeshNodeData->G6HostPPercent;
   status = pMeshNodeData->G6Status;

if(addr == 3){
  
   if(value & G6_FC6_AUTO_RUN){
        G6SetAutoRun(ON);pMeshNodeData->G6HostPPercent = value&0x00FF;
    }else{
        G6SetAutoRun(OFF);pMeshNodeData->G6HostPPercent = value&0x00FF;
    }
   if((speed != pMeshNodeData->G6HostPPercent) | (status !=  pMeshNodeData->G6Status)){
        G6SetActStatus(G6_STATUS_CHANGE,ON);    // to triggle task
        WriteMeshNodeData(); // save change
       }
   if(value & G6_FC6_CLS_FILTER1){//Trace1(" G6_FC6_CLS_FILTER1",value);
        if(pMeshNodeData->FilterAllTime1)
            {pMeshNodeData->FilterAllTime1 = 0; WriteNodeData();}
    }
   if(value & G6_FC6_CLS_ALL_FILTER){//Trace1(" G6_FC6_CLS_ALL_FILTER",value);
        if(pMeshNodeData->FilterAllTime2)
            {pMeshNodeData->FilterAllTime2 = 0; WriteNodeData();}  
     }
 }
   
}

//
// Setup RTC Initial Data
//
bool G6SetInitRtc(uint16 addr,uint16 value)
{
    bool ret_code=TRUE;    
    PDevDate p_init_date=&InitDevDate;
    //TraceDec2("G6SetupInitDate 1",addr,value);
    if(addr == RTC_ADDR_YEAR) p_init_date->Date.Year = value;
    else if(addr == RTC_ADDR_MONTH) p_init_date->Date.Month = value;
    else if(addr == RTC_ADDR_DATE)  p_init_date->Date.Date = value;
    else if(addr == RTC_ADDR_HOUR)  p_init_date->Date.Hour = value;
    else if(addr == RTC_ADDR_MIN)   p_init_date->Date.Min = value;
    else if(addr == RTC_ADDR_SEC)   p_init_date->Date.Sec = value;
    else if(addr == RTC_ADDR_WEEK) p_init_date->Date.Week = value;
    else {ret_code=FALSE;}

    if(p_init_date->Date.Year !=0 && p_init_date->Date.Month !=0 && p_init_date->Date.Date !=0){
       p_init_date->Date.Week = CalculateWeek(p_init_date->Date.Year,p_init_date->Date.Month,p_init_date->Date.Date);
       SetSysDate(p_init_date);
      }
    return ret_code;    
}

//
// 
//
bool G6SetSegOnOff(uint16 addr,uint16 value)
{
    bool ret_code=TRUE;
    //TraceDec2("G6SetSegOnOff 1",addr,value);
    switch(addr)
        {
            case SEG1_ON_OFF:
                if(value == ON) pAdjValue->G6Schedule[0].WeekPower |= G6_SCHEDULE_ON;
                else pAdjValue->G6Schedule[0].WeekPower &= ~G6_SCHEDULE_ON;
                break;
            case SEG2_ON_OFF:
                if(value == ON) pAdjValue->G6Schedule[1].WeekPower |= G6_SCHEDULE_ON;
                else pAdjValue->G6Schedule[1].WeekPower &= ~G6_SCHEDULE_ON;
                break;
            case SEG3_ON_OFF:
                if(value == ON) pAdjValue->G6Schedule[2].WeekPower |= G6_SCHEDULE_ON;
                else pAdjValue->G6Schedule[2].WeekPower &= ~G6_SCHEDULE_ON;
                break;
            case SEG4_ON_OFF:
                if(value == ON) pAdjValue->G6Schedule[3].WeekPower |= G6_SCHEDULE_ON;
                else pAdjValue->G6Schedule[3].WeekPower &= ~G6_SCHEDULE_ON;
                break;
            case SEG5_ON_OFF:
                if(value == ON) pAdjValue->G6Schedule[4].WeekPower |= G6_SCHEDULE_ON;
                else pAdjValue->G6Schedule[4].WeekPower &= ~G6_SCHEDULE_ON;
                break;
            default:  ret_code=FALSE; break;
        };
    
    return ret_code;    
}

//
// 
//
bool G6SetSegPower(uint16 addr,uint16 value)
{
      bool ret_code=TRUE;
      //TraceDec2("G6SetSegPower 1",addr,value);
      switch(addr)
          {
              case SEG1_POWER:
                  pAdjValue->G6Schedule[0].PowerPercent = value;break;
              case SEG2_POWER:
                  pAdjValue->G6Schedule[1].PowerPercent = value;break;
              case SEG3_POWER:
                  pAdjValue->G6Schedule[2].PowerPercent = value;break;
              case SEG4_POWER:
                  pAdjValue->G6Schedule[3].PowerPercent = value;break;
              case SEG5_POWER:
                  pAdjValue->G6Schedule[4].PowerPercent = value;break;
              default:  ret_code=FALSE; break;
          };
    return ret_code;          
}

//
// 
//
bool G6SetSegWeek(uint16 addr,uint16 value)
{
      bool ret_code=TRUE;
      uchar week_mask;
      week_mask = (uchar)value;
      switch(addr)
          {
              case SEG1_WEEK:
                  pAdjValue->G6Schedule[0].WeekPower |= week_mask;break;
              case SEG2_WEEK:
                  pAdjValue->G6Schedule[1].WeekPower |= week_mask;break;
              case SEG3_WEEK:
                  pAdjValue->G6Schedule[2].WeekPower |= week_mask;break;
              case SEG4_WEEK:
                  pAdjValue->G6Schedule[3].WeekPower |= week_mask;break;
              case SEG5_WEEK:
                  pAdjValue->G6Schedule[4].WeekPower |= week_mask;break;
              default:  ret_code=FALSE;  break;
          };
    return ret_code;          
}

//
// 
//
bool G6SetSegStartTime(uint16 addr,uint16 value)
{
      bool ret_code=TRUE;
      uchar week_mask;
      week_mask = (uchar)value;
      switch(addr)
          {
              case SEG1_START_TIME:
                  pAdjValue->G6Schedule[0].StartTime = value;break;
              case SEG2_START_TIME:
                  pAdjValue->G6Schedule[1].StartTime = value;break;
              case SEG3_START_TIME:
                  pAdjValue->G6Schedule[2].StartTime = value;break;
              case SEG4_START_TIME:
                  pAdjValue->G6Schedule[3].StartTime = value;break;
              case SEG5_START_TIME:
                  pAdjValue->G6Schedule[4].StartTime = value;break;
              default:  ret_code=FALSE;break;
          };
    return ret_code;          
}

//
// 
//
bool G6SetSegEndTime(uint16 addr,uint16 value)
{
    bool ret_code=TRUE;
    uchar week_mask;
    week_mask = (uchar)value;
    switch(addr)
        {
            case SEG1_END_TIME:
                pAdjValue->G6Schedule[0].EndTime = value;break;
            case SEG2_END_TIME:
                pAdjValue->G6Schedule[1].EndTime = value;break;
            case SEG3_END_TIME:
                pAdjValue->G6Schedule[2].EndTime = value;break;
            case SEG4_END_TIME:
                pAdjValue->G6Schedule[3].EndTime = value;break;
            case SEG5_END_TIME:
                pAdjValue->G6Schedule[4].EndTime = value;break;
            default:  ret_code=FALSE; break;
        };
    return ret_code;    
}

//
// 
//
bool G6SetTimeFilter(uint16 addr,uint16 value)
{
    bool ret_code=TRUE;
    uchar week_mask;
    week_mask = (uchar)value;
    switch(addr)
        {
            case RUNING_TIME_FILTER1:
                pMeshNodeData->FilterTime1 = value;break;
            case RUNING_TIME_FILTER2:
                pMeshNodeData->FilterTime2 = value;break;
            default:  ret_code=FALSE; break;
        };
    WriteMeshNodeData();   
    return ret_code;    
}


//
// FC3
//
bool G6GetInfo()
{
    bool ret_code=TRUE;
    

    return ret_code;
}



#define WORKING_TIME_MIN        5
#define WORKING_TIME_MAX        3600
//
//
// FC6
bool MeshNodeSetInfoProc()
{//
    bool ret_code=TRUE;
    _PModbusCmdF6 p_cmd_fc6;
    uint16 start_addr, value;
    p_cmd_fc6 = (_PModbusCmdF6)pModbusCmd;
    start_addr = WordSwap(p_cmd_fc6->StartAddr);
    value = WordSwap(p_cmd_fc6->Value);
    if(start_addr == 0x06 ){
         pAdjValue->TempGain = ((float)value)/(float)MESH_INFO_SCALING;
        }
    else if(start_addr == 0x07 ){
         pAdjValue->TempOffset = ((float)value)/(float)MESH_INFO_SCALING;
        }
    else if(start_addr == 0x08 ){
         pAdjValue->HumGain = ((float)value)/(float)MESH_INFO_SCALING;
        }
    else if(start_addr == 0x09 ){
         pAdjValue->HumOffset = ((float)value)/(float)MESH_INFO_SCALING;
        }
    else if(start_addr == 0x0A ){
         if(value >= WORKING_TIME_MIN && value <=WORKING_TIME_MAX){//TraceDec1("Set Working Time",value);
             pMeshNodeData->WorkingTimer = value;
            }
         else{ 
            ret_code=FALSE;
            }
        }
    else if(start_addr == 0x0B ){
             if( value == 1) value = SENSOR_SI7021;
             else if( value == 2) value = SENSOR_PZEM;
             else if( value == 3) value = SENSOR_OEM;
             else if( value == 4) value = SENSOR_AGB_POWER;
             else if( value == 5) value = SENSOR_VELOCITY;
             //else if( value == 6) value = SENSOR_BTM_G6;
             else ret_code=FALSE;
             if(ret_code == TRUE){ 
                pMeshNodeData->SensorClass = (uchar)value;
                }
        }
    else if(start_addr == 0x0F ){
         MeshNodeSetupReset(); 
        }
    else if(start_addr == 0x11){
         pMeshNodeData->BaudRate = value;
        }
    else ret_code=FALSE;

    if(ret_code == TRUE) WriteNodeData(); 

    return ret_code;
        
}


//
//
//FC3
bool MeshNodeGetInfoProc()
{//
    bool ret_code=TRUE;
    uint16 start_addr, total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr = WordSwap(pModbusCmd->StartAddr);
    total_reg = WordSwap(pModbusCmd->TotalReg);    
    if(start_addr >= 0x00 && total_reg == 3){
         ModbusToHostCmd.ByteNum = 6; memcpy(ModbusToHostCmd.Data,BtmModelName,6);
        }
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
        else if(pMeshNodeData->SensorClass == SENSOR_VELOCITY) ModbusToHostCmd.Data[0] = WordSwap(5);
        //else if(pMeshNodeData->SensorClass == SENSOR_BTM_G6) ModbusToHostCmd.Data[0] = WordSwap(6);
        else ModbusToHostCmd.Data[0] = WordSwap(1);
        }
    
    else if(start_addr == 0x11 && total_reg == 1){
         ModbusToHostCmd.Data[0] = WordSwap(pMeshNodeData->BaudRate);
        }
    else if((start_addr >= 0x38 && start_addr <= 0xDC) || (start_addr >= 0xF0 && start_addr <= 0xFB)){

            ret_code = GetG6InfoFC3(start_addr,total_reg);
        }
    else ret_code=FALSE;
    
    if(ret_code == TRUE) ModbusAddCrc(); 

    return ret_code;
        
}

Bool GetG6InfoFC3(uint16 start_addr,uint16 total_reg)
{
    Bool ret_code=TRUE;
    if(total_reg != 1) return FALSE;
    else if(start_addr >= RTC_ADDR_YEAR && start_addr <= RTC_ADDR_WEEK )
        G6FC3GetInitRtc(start_addr);
    else if(start_addr >= SEG1_ON_OFF && start_addr <= SEG5_ON_OFF )
        G6FC3GetSegOnOff( start_addr);
    else if(start_addr >= SEG1_POWER && start_addr <= SEG5_POWER )
        G6FC3GetSegPower(start_addr);
    else if(start_addr >= SEG1_WEEK && start_addr <= SEG5_WEEK )
        G6FC3GetSegWeek( start_addr);
    else if(start_addr >= SEG1_START_TIME && start_addr <= SEG5_START_TIME )
        G6FC3GetSegStartTime( start_addr);
    else if(start_addr >= SEG1_END_TIME && start_addr <= SEG5_END_TIME )
        G6FC3GetSegEndTime( start_addr);
    else if(start_addr >= POWER_PERCENT_SEG0 && start_addr <= POWER_PERCENT_SEG4 )
        G6FC3GetSegPowerPercent(start_addr);
    else if(start_addr >= RUNING_TIME_FILTER1 && start_addr <= RUNING_TIME_FILTER2 )
        G6FC3GetTimeFilter( start_addr);
    else ret_code = FALSE;

    return ret_code;
}


Bool G6FC3GetInitRtc(uint16 start_addr)
{//
    Bool ret_code = TRUE;
    GetSysDate();
    uchar rtc_data;
    
    switch(start_addr)
        {
            case RTC_ADDR_YEAR: rtc_data = pDevDate->Date.Year;  break;
            case RTC_ADDR_MONTH: rtc_data = pDevDate->Date.Month;break;
            case RTC_ADDR_DATE: rtc_data = pDevDate->Date.Date;  break;
            case RTC_ADDR_HOUR: rtc_data = pDevDate->Date.Hour;  break;
            case RTC_ADDR_MIN:  rtc_data = pDevDate->Date.Min;   break;
            case RTC_ADDR_SEC:  rtc_data = pDevDate->Date.Sec;   break;
            case RTC_ADDR_WEEK: rtc_data = pDevDate->Date.Week;  break;
            default:  ret_code=FALSE;  break;
        }; 
   if(ret_code) ModbusToHostCmd.Data[0] =  WordSwap(rtc_data);
   return ret_code;
            
   return ret_code;
}

Bool G6FC3GetSegOnOff(uint16 start_addr)
{//
  Bool ret_code=TRUE;
  uchar WeekPowerOnOff = OFF;
  switch(start_addr)
      {
          case SEG1_ON_OFF: if(pAdjValue->G6Schedule[0].WeekPower & G6_SCHEDULE_ON) WeekPowerOnOff = ON;
              break;
          case SEG2_ON_OFF: if(pAdjValue->G6Schedule[1].WeekPower & G6_SCHEDULE_ON) WeekPowerOnOff = ON;
              break;
          case SEG3_ON_OFF: if(pAdjValue->G6Schedule[2].WeekPower & G6_SCHEDULE_ON) WeekPowerOnOff = ON;
              break;
          case SEG4_ON_OFF: if(pAdjValue->G6Schedule[3].WeekPower & G6_SCHEDULE_ON) WeekPowerOnOff = ON;
              break;
          case SEG5_ON_OFF: if(pAdjValue->G6Schedule[4].WeekPower & G6_SCHEDULE_ON) WeekPowerOnOff = ON;
              break;
          default:  ret_code=FALSE;  break;
      };
  
  if(ret_code) ModbusToHostCmd.Data[0] = WordSwap(WeekPowerOnOff);
  return ret_code;

  return ret_code;
}

Bool G6FC3GetSegPower(uint16 start_addr)
{//
  Bool ret_code=TRUE;
  uchar PowerPercent;
  switch(start_addr)
      {
          case SEG1_POWER: PowerPercent =  pAdjValue->G6Schedule[0].PowerPercent; break;
          case SEG2_POWER: PowerPercent =  pAdjValue->G6Schedule[1].PowerPercent; break;
          case SEG3_POWER: PowerPercent =  pAdjValue->G6Schedule[2].PowerPercent; break;
          case SEG4_POWER: PowerPercent =  pAdjValue->G6Schedule[3].PowerPercent; break;
          case SEG5_POWER: PowerPercent =  pAdjValue->G6Schedule[4].PowerPercent; break;
          default:  ret_code=FALSE;  break;
      };
  if(ret_code) ModbusToHostCmd.Data[0] = WordSwap(PowerPercent);
  return ret_code;
}

Bool G6FC3GetSegWeek(uint16 start_addr)
{//
  Bool ret_code=TRUE;
  uchar WeekPower=0;
  switch(start_addr)
      {
          case SEG1_WEEK: WeekPower = pAdjValue->G6Schedule[0].WeekPower; break;
          case SEG2_WEEK: WeekPower = pAdjValue->G6Schedule[1].WeekPower; break;
          case SEG3_WEEK: WeekPower = pAdjValue->G6Schedule[2].WeekPower; break;
          case SEG4_WEEK: WeekPower = pAdjValue->G6Schedule[3].WeekPower; break;
          case SEG5_WEEK: WeekPower = pAdjValue->G6Schedule[4].WeekPower; break;
          default:  ret_code=FALSE;  break;
      };

  if(ret_code) ModbusToHostCmd.Data[0] = WordSwap(WeekPower);
  return ret_code;
}

Bool G6FC3GetSegStartTime(uint16 start_addr)
{//
  Bool ret_code=TRUE;  
  uint16 value;
  switch(start_addr)
      {
          case SEG1_START_TIME: value = pAdjValue->G6Schedule[0].StartTime;break;
          case SEG2_START_TIME: value = pAdjValue->G6Schedule[1].StartTime;break;
          case SEG3_START_TIME: value = pAdjValue->G6Schedule[2].StartTime;break;
          case SEG4_START_TIME: value = pAdjValue->G6Schedule[3].StartTime;break;
          case SEG5_START_TIME: value = pAdjValue->G6Schedule[4].StartTime;break;
          default:  ret_code=FALSE; break;
      };

  if(ret_code) ModbusToHostCmd.Data[0] = WordSwap(value);
  return ret_code;
}
Bool G6FC3GetSegEndTime(uint16 start_addr)
{//
  Bool ret_code=TRUE;
  uint16 value;
  switch(start_addr)
      {
          case SEG1_END_TIME: value = pAdjValue->G6Schedule[0].EndTime; break;
          case SEG2_END_TIME: value = pAdjValue->G6Schedule[1].EndTime; break;
          case SEG3_END_TIME: value = pAdjValue->G6Schedule[2].EndTime; break;
          case SEG4_END_TIME: value = pAdjValue->G6Schedule[3].EndTime; break;
          case SEG5_END_TIME: value = pAdjValue->G6Schedule[4].EndTime; break;
          default:  ret_code=FALSE; break;
      };
  if(ret_code) ModbusToHostCmd.Data[0] = WordSwap(value);
  return ret_code;
}
Bool G6FC3GetSegPowerPercent(uint16 start_addr)
{//
  Bool ret_code=TRUE;
  uchar value;
  switch(start_addr)
      {
          case POWER_PERCENT_SEG0: value = pMeshNodeData->SegPPercent[0]; break;
          case POWER_PERCENT_SEG1: value = pMeshNodeData->SegPPercent[1]; break;
          case POWER_PERCENT_SEG2: value = pMeshNodeData->SegPPercent[2]; break;
          case POWER_PERCENT_SEG3: value = pMeshNodeData->SegPPercent[3]; break;
          case POWER_PERCENT_SEG4: value = pMeshNodeData->SegPPercent[4]; break;
          default:  ret_code=FALSE; break;
      };
  if(ret_code) ModbusToHostCmd.Data[0] = WordSwap(value);
  return ret_code;

}

Bool G6FC3GetTimeFilter(uint16 start_addr)
{//
  Bool ret_code=TRUE;
  uint16 value;
  switch(start_addr)
      {
          case RUNING_TIME_FILTER1: value = pMeshNodeData->FilterTime1; break;
          case RUNING_TIME_FILTER2: value = pMeshNodeData->FilterTime2; break;
          default:  ret_code=FALSE; break;
      };

  if(ret_code) ModbusToHostCmd.Data[0] =  WordSwap(value);
  return ret_code;
}


