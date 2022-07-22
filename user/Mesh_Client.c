
#include "global.h"

#define CLIENT_NODE_DEBUG 1
#define STATE_CHANGE_DEBUG 0

#if CLIENT_NODE_DEBUG
#undef DEBUG_FUNCTION
#define DEBUG_FUNCTION dprint
#else
#define DEBUG_FUNCTION(...)
#endif
#include "debugprint.h"

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

#ifdef BTM_A308
#include "A308_Server.h"
#endif

uint32  ClientStatus;
uchar   RespServerNode;
uint16  ActServerAddr = PUBLISH_ADDRESS;
uint16  GetPropertyID = NODE_GET_ALL_SENSOR;

///////////////////////////////////////////////////////////////

_ClientInfo ClientInfo[SERVER_NODE_MAX+1];
PClientInfo pClientInfo=ClientInfo;
_PModbusCmdF4  pModbusCmd;
_ModbusToHost ModbusToHostCmd;


void ClientNodeInit()
{

    NodeRole = NR_CLIENT;
    UsartRxCount = 8;   //from Host cmd bytes
    Rs485Rx();
    ToNextStage(NODE_STAGE_INIT);
    UsartClientProc();
    ClientFromHostProc();

}

uint16 test1;

uint16 rx_ccount;
void ClientNodeTask()
{
    /*uchar rx_count = UsartGetRxCounter();
    if(rx_count >= 7 && GetNodeStatus(NS_USART_RX_EVENT) != TRUE)
        {Delay_ms(3);
         UsartClientProc(); UsartClientProc();
        }
    else if(GetNodeStatus(NS_USART_RX_EVENT) == TRUE) 
        {UsartClientProc();ClientFromHostProc();UsartClientProc();ClientFromHostProc(); }*/

    ClientGetNodeInfoProc();
    UsartClientProc();

    ClientFromHostProc();
#ifndef BTM_TRANSMITTER
    ClientSetNodeInfoProc();
#endif
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
            if(p_client_info->ServerID == node_addr ){ 
                ret_code = p_client_info; break;
                }
            p_client_info++;
        }
    
    if(ret_code != NULL) return ret_code;
    p_client_info = ClientInfo;

    for(loop=0; loop<SERVER_NODE_MAX; loop++)
        {
            if(p_client_info->ServerID == 0) {
                ret_code = p_client_info; break;
                }
            p_client_info++;
        }
    return ret_code;
}

PClientInfo FindNextServerInfo(uint8 class, PClientInfo pClient){
	if (pClient==0){
		//dprint("FindNextServerInfo.. Get Init Client\r\n");
		pClient=ClientInfo;
	}
	else{
		//dprint("FindNextServerInfo.. Args Client Index:%d\r\n",((uint8*)pClient-(uint8*)ClientInfo)/sizeof(_ClientInfo));
		pClient++;
	}
	int16 idx=((uint8*)pClient-(uint8*)ClientInfo)/sizeof(_ClientInfo);
	int16 end=sizeof(ClientInfo)/sizeof(_ClientInfo);
	dprint("FindNextServerInfo start:%d, end:%d, loc:0x%x, size:%d\r\n",idx,end,(uint8*)pClient-(uint8*)ClientInfo,sizeof(_ClientInfo));
	while(idx>=0 && idx<end){
		if (ClientInfo[idx].SensorInfo.Header.SensorClass==class){
			dprint(" Next ServerInfo index:%d\r\n",idx);
			return &ClientInfo[idx];
		}
		idx++;
	}
	return 0;
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
    TraceDec1("Total Node Num",ret_code);
    return ret_code;
}

#define TIMER_CLIENT_IVI_WAIT       5   //sec

//#define MODBUS_AIP_POWER_CMD        5

#define GET_MODBUS_FUN_CODE         prx_buff[1]
#define MODBUS_CTRL_REG             prx_buff[3]
#define A6D6_CTRL_POWER             prx_buff[4]
#define AIP_CTRL_POWER              prx_buff[5]

#define BTM_AIP_AUTO                prx_buff[4]
#define BTM_AIP_POWER               prx_buff[5]


uint16 SetProperityID=NODE_SET_AIP_POWER_OFF;
uint16 SetServerFunID=0;



//
// return properity ID
//
uint16 GetProperityID()
{
    PUCHAR prx_buff = UsartGetBuff(USART_ID_RX);
    uint16 properity=NULL;
    uint16 modbus_reg;
    uint16 btm_cmd;
    modbus_reg = WordSwap(*((PUINT)&prx_buff[2]));
    Trace16_1(modbus_reg);
    if(GET_MODBUS_FUN_CODE == 0x06)
    {//for AIP
        if(modbus_reg == 0x0002)
        {
        if(AIP_CTRL_POWER == MODBUS_AIP_POWER_00) properity = NODE_SET_AIP_POWER_OFF;
        else if(AIP_CTRL_POWER == MODBUS_AIP_POWER_25) properity = NODE_SET_AIP_POWER_25;
        else if(AIP_CTRL_POWER == MODBUS_AIP_POWER_50) properity = NODE_SET_AIP_POWER_50;
        else if(AIP_CTRL_POWER == MODBUS_AIP_POWER_75) properity = NODE_SET_AIP_POWER_75;
        else if(AIP_CTRL_POWER == MODBUS_AIP_POWER_100) properity = NODE_SET_AIP_POWER_100;
        }
        else if(modbus_reg == 0x0001)
        {
        if(AIP_CTRL_POWER == MODBUS_AGB_POWER_00) properity = NODE_SET_AIP_POWER_OFF;
        else if(AIP_CTRL_POWER == MODBUS_AGB_POWER_25) properity = NODE_SET_AIP_POWER_25;
        else if(AIP_CTRL_POWER == MODBUS_AGB_POWER_50) properity = NODE_SET_AIP_POWER_50;
        else if(AIP_CTRL_POWER == MODBUS_AGB_POWER_75) properity = NODE_SET_AIP_POWER_75;
        else if(AIP_CTRL_POWER == MODBUS_AGB_POWER_100) properity = NODE_SET_AIP_POWER_100;         
        }
        else if(modbus_reg == 0xF000)
        {//for BTM Full Power
            if(AIP_CTRL_POWER == MODBUS_AGB_POWER_00) 
                { properity = NODE_GET_INFO_FULL_POWER_OFF;}
            else 
                { properity = NODE_GET_INFO_FULL_POWER_ON;}
        }else if(modbus_reg == 0x0003){//Trace16_2(BTM_AIP_AUTO,BTM_AIP_POWER);
            btm_cmd = WordSwap(*((PUINT16)(&BTM_AIP_AUTO)));  Trace16_1(btm_cmd);
            if(btm_cmd & G6_FC6_CLS_ALL_FILTER) properity = CLEAR_ALL_FILTER;
            else if(btm_cmd & G6_FC6_CLS_FILTER1) properity = CLEAR_FILTER1;
            else if(btm_cmd & G6_FC6_AUTO_RUN) {
                   properity =  AUTO_POWER_0+(btm_cmd&0x00FF); Trace16_1(properity);
            }
            else {
                  properity =  MENU_POWER_0+(btm_cmd&0x00FF); Trace16_1(properity);  
            }            
            
            //properity = NULL; //debug
        }
    }
    else if(GET_MODBUS_FUN_CODE == 0x05)
    {//for A6D6
           if(A6D6_CTRL_POWER == 0xFF){//Trace1("A6D6 Set ON", MODBUS_CTRL_REG);
             properity = (NODE_SET_DO1_ON+(uint16)MODBUS_CTRL_REG); 
            }
           else {//Trace1("A6D6 Set OFF", MODBUS_CTRL_REG);
             properity = (NODE_SET_DO1_OFF+(uint16)MODBUS_CTRL_REG); 
            }

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
                UsartPrintBuff(USART_ID_RX);
                prx_buff = UsartGetBuff(USART_ID_RX);
                SetServerFunID = (uint16)prx_buff[0]; // get id number
                SetProperityID = GetProperityID();
                if(SetProperityID == NODE_GET_INFO_FULL_POWER_ON){
                     gecko_external_signal(PB_SPEED_5SEC);ToNextStage(CNS_SET_INFO_OK); 
                    }
                 else if(SetProperityID == NODE_GET_INFO_FULL_POWER_OFF){
                     gecko_external_signal(PB_SPEED_NORMAL);ToNextStage(CNS_SET_INFO_OK); 
                    }
                else if(SetProperityID != NULL ) ToNextStage(CNS_SET_INFO_SEND);
                else  ToNextStage(CNS_SET_INFO_ERR);
                break;
                
            case CNS_SET_INFO_SEND: //Trace1("CNS_SET_INFO_SEND",SetProperityID);

                result = Cmd_ms_client_get(SENSOR_ELEMENT, SetServerFunID, IGNORED, 0xA5, SetProperityID)->result;
                if(result){ 
                     ToWaitingStage(CNS_SET_INFO_PREEAT,50);
                    }
                else 
                    ToNextStage(CNS_SET_INFO_OK);
                
                break;
            case CNS_SET_INFO_PREEAT: 
                if(!CheckWaitTimeOut()) break;
                result = Cmd_ms_client_get(SENSOR_ELEMENT, SetServerFunID, IGNORED, 0xA5, SetProperityID)->result;
                if(result){
                     ToNextStage(CNS_SET_INFO_ERR);
                    }
                else{
                     
                     ToNextStage(CNS_SET_INFO_OK);
                    }
                break;
            case CNS_SET_INFO_OK:  //Trace1("CNS_SET_INFO_OK",SetProperityID); 
                prx_buff = UsartGetBuff(USART_ID_RX);
                UsartTxSendCmd(prx_buff ,MODBUS_CMD_NUM);
                ToWaitingStage(CNS_SET_UPDATE_INFO,10);
                break;
                
            case CNS_SET_UPDATE_INFO:  
                if(CheckWaitTimeOut())  ToNextStage(CNS_SET_INFO_END);
                break;
                
            case CNS_SET_INFO_END:  //Trace1("CNS_SET_INFO_END",SetProperityID); 
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

void ClientGetInfoActionNow()
{
    PNodeStageInfo p_stage_info;
    p_stage_info = GetNodeStageInfo(CLIENT_GET_NODE_INFO_PROC);
    p_stage_info->Timer = WAIT_SEC(1);
}

//
// handle node response
//
//uchar tstmsg_data[]={9,4,0,8,6};
//int tstmsg_len=sizeof(tstmsg_data)/sizeof(uchar);
void ClientGetNodeInfoProc()
{
    pStageInfo = GetNodeStageInfo(CLIENT_GET_NODE_INFO_PROC);
    
    switch(ActiveStage())
        {
            case NODE_STAGE_INIT:
                //SetLedStatus(LED_STATUS_ON);
                GetInfoCycle = WAIT_SEC(TIMER_GET_INFO_SLEEPING);
                ToWaitingStage(CNS_GET_SEVER_INFO,WAIT_SEC(1));
                break;
            case CNS_PRE_IVI_UPDATE: 
                if(CheckWaitTimeOut()){
                     SetNodeStatus(NS_IVI_UPDATE,ON);   // enable ivi update
                     CountErr = 0;
                    }
                break;
            
            case CNS_IVI_UPDATE: 
                if(CheckWaitTimeOut() == TRUE){
                    if(CountErr++ > 3){
                        ToNextStage(CNS_WAIT_SET_INFO);   
                      }
                    else {// waiting again
                        ToWaitingStage(CNS_IVI_UPDATE,WAIT_SEC(TIMER_CLIENT_IVI_WAIT));
                    } 
                  }
               if(GetNodeStatus(NS_IVI_UPDATE) == OFF){//TraceOk("--- IVI Update Ok ---");
                   ToNextStage(CNS_WAIT_SET_INFO); 
                  }
                break;
            case CNS_PRE_SEVER_INFO: 
                SetLed(LED_SERVER,OFF); 
                SetNodeStatus(NS_GET_INFO_ACT,OFF);
                if(RespServerNode == 0) {ToWaitingStage(CNS_WAIT_SET_INFO,100);}
                else {
                	dprint("*** Wait %d(ms) for next time.\r\n",GetInfoCycle*10);
                	ToWaitingStage(CNS_WAIT_SET_INFO,GetInfoCycle);
                }
                break;
            case CNS_WAIT_SET_INFO: 
                if(GetNodeStatus(NS_LINKING) ==ON) break;
                if(CheckWaitTimeOut()){
                     ToNextStage(CNS_GET_SEVER_INFO);
                    }
                else if(GetNodeStatus(NS_SET_NODE_ACT) != TRUE)
                    ToNextStage(CNS_GET_SEVER_INFO);
                break;
            case CNS_GET_SEVER_INFO: 
                
                if(CountErr > 3) ToNextStage(CNS_GET_INFO_ERR); // err: go to sleeping


                if(CheckWaitTimeOut()){
                    //to get info from all server node
                    GetEventCount++;
                    SetLed(LED_SERVER,ON);
                    dprint("STEP: CNS_GET_SERVER_INFO\r\n");
                    result = Cmd_ms_client_get(SENSOR_ELEMENT, ActServerAddr, IGNORED, 0xA5, GetPropertyID)->result;
                    //result = Cmd_ms_client_get_column(SENSOR_ELEMENT, ActServerAddr, IGNORED, 0xA5, GetPropertyID,tstmsg_len,tstmsg_data)->result;

                    if(result){ //error
						TraceErr1("CNS_GET_SEVER_INFO 1",result);
						CountErr++;
						ToWaitingStage(CNS_GET_SEVER_INFO,WAIT_SEC(2));
                    }else{ //successed
                    	//TraceOk("Get Info");
						CountErr = 0; RespServerNode = 0;
						ToWaitingStage(CNS_WAIT_INFO,TIMER_CLI_WAIT_INFO);
                    }
                }else{
                    if(GetNodeStatus(NS_GET_INFO_ACT)){
                    	ToWaitingStage(CNS_WAIT_INFO,TIMER_CLI_WAIT_INFO);
                    }
                }
                    
                break;
            case CNS_WAIT_INFO:
                if(CheckWaitTimeOut()){
                	ClientCheckNodeStatus();
#ifdef BTM_A308
                	ToNextStage(CNS_GET_A308_INFO);
#else
                	ToNextStage(CNS_WAIT_INFO_OK);
#endif
                }
                break;
#ifdef BTM_A308
            case CNS_GET_A308_INFO:
            	if(!A308_Client_GetInfo()) {
            		dprint(" CNS_GET_A308_INFO > FINISHED\r\n");
            		ToNextStage(CNS_WAIT_INFO_OK);
            	}
            	break;
#endif
            case CNS_WAIT_INFO_OK:
                
                if(GetNodeStatus(NS_FULL_POWER))
#ifdef BTM_TRANSMITTER
                	GetInfoCycle=WAIT_SEC(10);
#else
                	GetInfoCycle = WAIT_SEC(TIMER_GET_INFO_FULL_POWER);
#endif
                else
#ifdef BTM_A308
                	GetInfoCycle = WAIT_SEC(TIMER_GET_INFO_SLEEPING+10);
#else
                	GetInfoCycle = WAIT_SEC(TIMER_GET_INFO_SLEEPING);
#endif
                ToNextStage(CNS_GET_INFO_END); 
                break;

            case CNS_GET_INFO_ERR:
                CountErr = 0;
                ToNextStage(CNS_PRE_SEVER_INFO); 
                break;
            case CNS_GET_INFO_END:
                ShowAllNodeInfo();
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

    for(loop=0; loop < SERVER_NODE_MAX; loop++){
		if(pNodeInfo->ServerID != 0){
			if(pNodeInfo->Count == 0){
				dprint("\r\n***** Node No Response *****ID:%d\r\n",pNodeInfo->ServerID);
				memset(pNodeInfo,0,sizeof(_ClientInfo));
				pNodeInfo->Status |= SERVER_NO_RESPONSE;
			}else{
				if(pNodeInfo->Count) pNodeInfo->Count--;
				if((pNodeInfo->SensorInfo.Header.Status & SERVER_FULL_POWER) == 0)
				  power_flag = OFF;
			}
		}
        pNodeInfo++;
    }
    
    if(GetNodeStatus(NS_FORCE_FULL_POWER) == ON) {
        SetNodeStatus(NS_FULL_POWER,ON);SetLed(LED_GREEN,ON);
    }else{
        if(power_flag == OFF){
            SetNodeStatus(NS_FULL_POWER,OFF);SetLed(LED_GREEN,OFF);
        }else{
            SetNodeStatus(NS_FULL_POWER,ON);SetLed(LED_GREEN,ON);
        }
    }
    
}

//
//
//
bool SendModbusToHost()
{
    bool ret_code=TRUE;
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
    if(UsartGetRxCounter() < MODBUS_CMD_NUM || CheckModbusCrc((PUCHAR)pModbusCmd,MODBUS_CMD_NUM) == FALSE) 
        {
        ret_code=MODBUS_CMD_ERROR;
        }
    else
        {
            if(pModbusCmd->FunCode == 0x01 || pModbusCmd->FunCode == 0x04 || pModbusCmd->FunCode == 0x03)
            ret_code=MODBUS_GET_INFO;
            else if(pModbusCmd->FunCode == 0x06 || pModbusCmd->FunCode == 0x05)
            ret_code=MODBUS_SET_INFO;
        }
    return ret_code;
}


uint16_t trans_wait_ms=0;
int8_t trans_retry=0;
uint8 trans_seq=0;
uint8 rec_flag=0;
#define TIMER_REDUCE_BY_10MS(t) do{if(t>0)t=t-((t>=10)?10:t);}while(0)
#define TIMER_IS_TIMEROUT(t) (t==0)
/*uint16_t cntTick=0;
uint32_t cntSecond=0;*/
void ClientTimer_10ms(){
	TIMER_REDUCE_BY_10MS(trans_wait_ms);
	/*if(++cntTick>100){
		cntTick=0;
		cntSecond++;
		dprint("Time Second:%d\r\n",cntSecond);
	}*/

}

bool ClientModbusResponse();

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
            ToNextStage(CHS_CHECK_RX_EVENT);
            break;
        case CHS_CHECK_RX_EVENT: 
        	if(GetNodeStatus(NS_USART_RX_EVENT) == TRUE)
#ifdef BTM_TRANSMITTER
        	{
        		ToNextStage(CHS_SEND_TO_CLIENT);
        		trans_seq++;
        		rec_flag=0;
        		trans_retry=0;
        	}
#elif defined(BTM_A308)
			{
				if(ClientModbusResponse())
					ToNextStage(CHS_WAIT_TX_FINISHED);
				else
					ToNextStage(CHS_MODBUS_ERROR);
			}
#else
            	ToNextStage(CHS_CHECK_MODBUS); //ToNextStage(CHS_PREPARE_DATA);   //default
#endif
            break;

        case CHS_WAIT_TX_FINISHED:

        	if(UsartGetStatus(USART_TX_END)){
        		UsartResetRxTx(USART_ID_RX);
				SetNodeStatus(NS_USART_RX_EVENT,OFF);
				ToNextStage(CHS_CHECK_RX_EVENT);
        	}

        	break;
        case CHS_CHECK_MODBUS: 
            if(CheckModbusCmd() == MODBUS_GET_INFO) ToNextStage(CHS_PREPARE_DATA);
            else if(CheckModbusCmd() == MODBUS_SET_INFO) ToNextStage(CHS_SERVER_SET);
            else ToNextStage(CHS_MODBUS_ERROR);

            break;
        case CHS_SEND_TO_CLIENT:

        	if(TIMER_IS_TIMEROUT(trans_wait_ms)){
        		*(UsartGetBuff(USART_ID_RX)-1)=trans_seq;
				result = Cmd_ms_client_get_column(SENSOR_ELEMENT, ActServerAddr, IGNORED, 0xA5, CUSTOM_SERIAL_DATA,UsartGetRxCounter()+1,UsartGetBuff(USART_ID_RX)-1)->result;
				if (!result || ++trans_retry>=5){ /*發送完成 or 重試超過n次*/
					UsartResetRxTx(USART_ID_RX);
					SetNodeStatus(NS_USART_RX_EVENT,OFF);
					ToNextStage(CHS_CHECK_RX_EVENT);
				}else {
					trans_wait_ms=100; /*100ms後重試*/
					dprint("command transfer error! try:%d\r\n",trans_retry);
				}
			}
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
            
        default:  break;
        };
    
}



void ClientSeriesEvent(msg_ms_client_series_status_evt *pEvent){
	uint8_t *property_data = NULL;
	uint8_t property_len=0;
	PUCHAR p_tx_buff = UsartGetBuff(USART_ID_TX);
	uint8 seq;
	uint8 size;
	uint8 flag;
	uint8 loc=0;
	uint8 part=0;
	uint8 part_len=0;
	PUCHAR data;
	//dprint("Event Client(0x%02x), elem:%d, flag:%d, len:%d\r\n",pEvent->client_address,pEvent->elem_index,pEvent->flags,pEvent->sensor_data.len);
	//dprint("*** Client Series Event> property:0x%04X\r\n",pEvent->property_id);
	switch(pEvent->property_id){
#if BTM_A308
	case NODE_GET_A308_TABLE:
		A308ClientSeriesEvent(pEvent);
		break;
#endif
	case CUSTOM_SERIAL_DATA:
		property_len=pEvent->sensor_data.len;
		property_data=pEvent->sensor_data.data;
		seq=property_data[0];
		size=property_data[1];
		flag=property_data[2];
		data=property_data+3;
		if(trans_seq!=seq) return;


		if(rec_flag==0){
			part=0;
			loc=0;
			while(loc<size){
				rec_flag|=0x1<<part;
				part++;
				loc+=50;
			}
			dprint("initial rec_flag:0x%x\r\n",rec_flag);
		}

		part=0;
		loc=0;
		while(loc<size){
			if((1<<part)==flag) break;
			part++;
			loc+=50;
		}

		part_len=size-loc;
		if (part_len>50) part_len=50;
		memcpy(p_tx_buff+loc,data,part_len);
		rec_flag&= ~flag;
		dprint("receive msg. rec_flag:0x%x, flag:0x%x, loc:%d, part_len:%d\r\n",rec_flag,flag, loc, part_len);
		if(!rec_flag){
			IFDPRINT(
				dprint("req_seq:%d, rsp_seq:%d, size:%d>",trans_seq,property_data[0],size);
				for(uint8_t _idx=0;_idx<size;_idx++)dprint(" %02X",p_tx_buff[_idx]);
				dprint("\r\n");
			)
			UsartTxSendCmd(p_tx_buff,size);
			return;
		}

		/*IFDPRINT(
			dprint("req_seq:%d, rsp_seq:%d, len:%d(0x%x)>",trans_seq,property_data[0],property_len,property_len);
			for(uint8_t _idx=0;_idx<(property_len-1);_idx++)dprint(" %02X",property_data[_idx+1]);
			dprint("\r\n");
		)
		if (trans_seq==property_data[0])
			UsartTxSendCmd(property_data+1,property_len-1);*/
		break;
	}
}

void ClientColumnEvent(msg_ms_client_column_status_evt *pEvent){


}


void ClientPropertyEvent(msg_ms_client_status_evt *pEvent)
{
    uint8_t *p_sensor_data = pEvent->sensor_data.data;
    uint8_t data_len = pEvent->sensor_data.len;
    uint8_t pos = 0;
    mesh_device_properties_t property_id;
    //PClientNodeInfo p_node_info;
    if((pClientInfo = GetServerInfoPos(pEvent->server_address))== NULL) 
           {return;}
    TraceDec1("Node ID-2", pEvent->server_address);
    dprint("Event Client(0x%02x) from:%d, elem:%d, flag:%d, len:%d\r\n",pEvent->client_address,pEvent->server_address,pEvent->elem_index,pEvent->flags,data_len);

    //if(pEvent->server_address == 3) TraceDec1(" 3 = ",RTCC_CounterGet());
    //else if(pEvent->server_address == 51) TraceDec1(" 4 = ",RTCC_CounterGet());
    
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
        	   //dprint("first 3 bytes: %02X %02X %02X\r\n",p_sensor_data[pos],p_sensor_data[pos+1],p_sensor_data[pos+2]);
               property_id = (mesh_device_properties_t)(p_sensor_data[pos] + (p_sensor_data[pos + 1] << 8));
               //Trace16_1(property_id);
               uint8_t property_len = p_sensor_data[pos + PROPERTY_ID_SIZE];
               uint8_t *property_data = NULL;
               if(property_len && (data_len - pos > PROPERTY_HEADER_SIZE))
                   {property_data = &p_sensor_data[pos + PROPERTY_HEADER_SIZE];}
               dprint("> property id: %d\r\n",property_id);

               switch(property_id)
                {
                    case AUTO_POWER_0:
                    case AUTO_POWER_1:
                    case AUTO_POWER_2:
                    case AUTO_POWER_3:
                    case AUTO_POWER_4:
                    case MENU_POWER_0:
                    case MENU_POWER_1:
                    case MENU_POWER_2:
                    case MENU_POWER_3:
                    case MENU_POWER_4:
                    case CLEAR_FILTER1:
                    case CLEAR_ALL_FILTER:
                    case NODE_GET_ALL_SENSOR:
                    case NODE_GET_INFO_FULL_POWER_ON: 
                    case NODE_GET_INFO_FULL_POWER_OFF:
                        memcpy(&(pClientInfo->SensorInfo),property_data,property_len);
                        dprint("** Received Info Class:%d, Battery Power:%d\r\n",pClientInfo->SensorInfo.Header.SensorClass,pClientInfo->SensorInfo.Header.BatteryPower);

                        GetPropertyID = NODE_GET_ALL_SENSOR;  // recover data
                        break;
                    case CUSTOM_SERIAL_DATA:
                    	IFDPRINT(
							dprint("req_flag:%d, rsp_flag:%d, len:%d(0x%x)>",trans_seq,property_data[0],property_len,property_len);
							for(uint8_t _idx=0;_idx<(property_len-1);_idx++)dprint(" %02X",property_data[_idx+1]);
							dprint("\r\n");
						)
						if (trans_seq==property_data[0])
							UsartTxSendCmd(property_data+1,property_len-1);
                      break;
                    default: break;
                };
               pos += PROPERTY_HEADER_SIZE + property_len;
            }
           else 
            pos = data_len;
               
        };

   
    if(COUNT_NODE_DETECTED > 50) pClientInfo->Count = 50;
    else if(COUNT_NODE_DETECTED < 4) pClientInfo->Count = 4;
    else pClientInfo->Count = COUNT_NODE_DETECTED;

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
    pModbusCmd = (_PModbusCmdF4)UsartGetBuff(USART_ID_RX);
    if((pClientInfo = GetServerInfoPos((uint16)(pModbusCmd->ModbusID)))== NULL) 
           {return ret_code;}
#ifndef OEM_SENSOR
    if(pModbusCmd->FunCode == 0x03 && pModbusCmd->StartAddr == 0x0000 && pModbusCmd->TotalReg == 0x0300)
        sensor_class = OTHER_MODBUS_CMD;
    else
#endif        
        sensor_class = pClientInfo->SensorInfo.Header.SensorClass;

    switch(sensor_class)
        {
            case SENSOR_SI7021:     ret_code = ClientSkynet(&pClientInfo->SensorInfo.Si7021Info);
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
            case SENSOR_ULTRA_SOUND: ret_code = ClientUltraSound(&pClientInfo->SensorInfo.UltraSound);
                break;
            case SENSOR_DO_485:     ret_code = ClientJncDo485(&pClientInfo->SensorInfo.JncDo485);
                break;
            case SENSOR_A6D6:       ret_code = ClientA6D6(&pClientInfo->SensorInfo.A6D6);
                break;             
            case SENSOR_RELAY:      ret_code = ClientRelay(&pClientInfo->SensorInfo.RelayNode);
                break;
            case SENSOR_IAQS:       ret_code = ClientIAQS(&pClientInfo->SensorInfo.IaqsInfo);
                break;
            case SENSOR_CW9:        ret_code = ClientCW9(&pClientInfo->SensorInfo.Cw9Info);
                break;
            case SENSOR_SKYNET_CO2: ret_code = ClientSkynetCo2(&pClientInfo->SensorInfo.SkynetCo2);
                break;
            case SENSOR_PZEM:       ret_code = ClientPzem(&pClientInfo->SensorInfo.Pzem);
                break;                   
            case SENSOR_OEM:        ret_code = ClientOemSensor(&pClientInfo->SensorInfo.OemSensor);
                break;
            case SENSOR_BTM_G6:     ret_code = ClientBtmG6(&pClientInfo->SensorInfo.BtmG6);
                break;
            case SENSOR_VELOCITY:   ret_code = ClientVelocity(&pClientInfo->SensorInfo.Velocity);
                break;                                 
            case OTHER_MODBUS_CMD:  ret_code = ClientOtherModbusCmd();
                break;
            default:  ret_code=FALSE;
                break;
        }

    if(ret_code == TRUE){
         ModbusAddCrc();
        }
    else TraceErr("ClientPrepareToHost 2");

    return ret_code;

}

//
//
//
void ModbusAddCrc()
{
    PUINT16  p_modbus_crc;
    uint16  modbus_crc_bytes,modbus_crc;
    modbus_crc = ModbusRtu_CRC16((PUCHAR)&ModbusToHostCmd,ModbusToHostCmd.ByteNum+3);
    p_modbus_crc = (PUINT16)(((PUCHAR)&(ModbusToHostCmd.ByteNum)) + ModbusToHostCmd.ByteNum+1);
    *p_modbus_crc = modbus_crc;

}



const char BtmModelName[6]=MODEL_NAME;

//
//
//
bool ClientOtherModbusCmd()
{
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
bool ClientSkynet(PSi7021Info p_info)
{
    bool ret_code=TRUE;
    uint16 start_addr,total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr); // Trace16_1(start_addr);
    total_reg = WordSwap(pModbusCmd->TotalReg);    // Trace16_1(total_reg);

    if(start_addr == SI7021_TEMP){
        ModbusToHostCmd.Data[0] =  WordSwap(p_info->Tempature);
      }
    else if(start_addr == SI7021_RH ){
        ModbusToHostCmd.Data[0] =  WordSwap(p_info->Humidity);
      }
    else if(start_addr == BATTERY_POWER ){
        ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower));
      }
    else{ 
        ret_code=FALSE;
        }


    return ret_code;
}


//
//
//
bool ClientSkynetCo2(PSkynetCo2 p_info)
{
    bool ret_code=TRUE;
    uint16 start_addr,total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr); 
    total_reg = WordSwap(pModbusCmd->TotalReg);    

    if(start_addr == SI7021_TEMP){
        ModbusToHostCmd.Data[0] =  WordSwap(p_info->Tempature);
      }
    else if(start_addr == SI7021_RH ){
        ModbusToHostCmd.Data[0] =  WordSwap(p_info->Humidity);
      }
    else if(start_addr == SENSOR_CO2 ){
        ModbusToHostCmd.Data[0] =  WordSwap(p_info->Co2);
      }
    else if(start_addr == BATTERY_POWER ){
        ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower));
      }
    else{
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
      {
        ModbusToHostCmd.Data[0] =  WordSwap(p_info->Tempature);
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
            case BT_RMS_X:
            case RMS_X:     ModbusToHostCmd.Data[0] = WordSwap(p_info->RmsX);break;
            case BT_SKEWNESS_X:
            case SKEWNESS_X: ModbusToHostCmd.Data[0] = WordSwap(p_info->SkewnessX); break;
            case BT_KURTOSIS_X:
            case KURTOSIS_X: ModbusToHostCmd.Data[0] = WordSwap(p_info->KurtosisX); break;
            case BT_FREQUENCY_X:
            case FREQUENCY_X: ModbusToHostCmd.Data[0] = WordSwap(p_info->FrequencyX); break;
            case BT_SPEED_X:
            case SPEED_X: ModbusToHostCmd.Data[0] = WordSwap(p_info->SpeedX); break;
            case BT_STRENGTH_X:
            case STRENGTH_X: ModbusToHostCmd.Data[0] = WordSwap(p_info->StengthX); break;
            case BT_RMS_Y:
            case RMS_Y:     ModbusToHostCmd.Data[0] = WordSwap(p_info->RmsY); break;
            case BT_SKEWNESS_Y:
            case SKEWNESS_Y: ModbusToHostCmd.Data[0] = WordSwap(p_info->SkewnessY); break;
            case BT_KURTOSIS_Y:
            case KURTOSIS_Y: ModbusToHostCmd.Data[0] = WordSwap(p_info->KurtosisY); break;
            case BT_FREQUENCY_Y:
            case FREQUENCY_Y: ModbusToHostCmd.Data[0] = WordSwap(p_info->FrequencyY); break;
            case BT_SPEED_Y:
            case SPEED_Y: ModbusToHostCmd.Data[0] = WordSwap(p_info->SpeedY); break;
            case BT_STRENGTH_Y:
            case STRENGTH_Y: ModbusToHostCmd.Data[0] = WordSwap(p_info->StengthY); break;
            case BT_RMS_Z:
            case RMS_Z:     ModbusToHostCmd.Data[0] = WordSwap(p_info->RmsZ); break;
            case BT_SKEWNESS_Z:
            case SKEWNESS_Z: ModbusToHostCmd.Data[0] = WordSwap(p_info->SkewnessZ); break;
            case BT_KURTOSIS_Z:
            case KURTOSIS_Z: ModbusToHostCmd.Data[0] = WordSwap(p_info->KurtosisZ); break;
            case BT_FREQUENCY_Z:
            case FREQUENCY_Z: ModbusToHostCmd.Data[0] = WordSwap(p_info->FrequencyZ); break;
            case BT_SPEED_Z:
            case SPEED_Z: ModbusToHostCmd.Data[0] = WordSwap(p_info->SpeedZ);break;
            case BT_STRENGTH_Z:
            case STRENGTH_Z: ModbusToHostCmd.Data[0] = WordSwap(p_info->StengthZ);break;
            case BT_A308M_TEMP:
            case A308M_TEMP:    ModbusToHostCmd.Data[0] = WordSwap(p_info->Tempature);  break;
            case BATTERY_POWER: ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower));
                break;
        };

    return ret_code;
}

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

    
    if(start_addr == POSITION_WATER) {
        ModbusToHostCmd.Data[0] =  WordSwap(p_info->WaterLevel);
        if(total_reg>1) {
            ModbusToHostCmd.Data[1] = WordSwap(p_info->OilLevel);
            ModbusToHostCmd.ByteNum=4;}
      }
    else if(start_addr == POSITION_OIL){
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->OilLevel); 
      }
    else if(start_addr == BATTERY_POWER ){
        ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower));
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
{
    bool ret_code=TRUE;
    uint16 start_addr,total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr); 
    total_reg = WordSwap(pModbusCmd->TotalReg);   

    if(start_addr == JNC_SD_CO2){
        ModbusToHostCmd.Data[0] =  WordSwap(p_info->CO2);
      }
    else if(start_addr == JNC_SD_PM25){
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->PM25); 
      }
    else if(start_addr == JNC_SD_TEMP){
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->Tempature); 
      }
    else if(start_addr == JNC_SD_RH){
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->Humidity); 
      }
    else if(start_addr == BATTERY_POWER ){
        ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower));
      }
    
    else{ 
        ret_code=FALSE;
        }
    return ret_code;
}


//
//
bool ClientUltraSound(PUltraSoundInfo p_info)
{
    bool ret_code=TRUE;
    uint16 start_addr,end_addr,*p_ai_value,*p_modbus_value,total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr); 
    total_reg = WordSwap(pModbusCmd->TotalReg);    
    end_addr = start_addr + total_reg -1;

    if(start_addr != SI7021_TEMP && start_addr != SI7021_RH)
        if(end_addr > UD_RH) {end_addr = UD_RH;total_reg = end_addr - start_addr +1;}

    

    if(start_addr == BATTERY_POWER ) {
        ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower));
      }
    else if(start_addr == POSITION_WATER){
        ModbusToHostCmd.Data[0] =  WordSwap(p_info->WaterLevel);
        if(total_reg>1) {
            ModbusToHostCmd.Data[1] = WordSwap(p_info->OilLevel);
            ModbusToHostCmd.ByteNum=4;
        }
      }
    else if(start_addr == POSITION_OIL){
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->OilLevel); 
      }
    else if(start_addr == SI7021_TEMP){
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->Tempature);
       if(total_reg > 1) {
            ModbusToHostCmd.Data[1] = WordSwap(p_info->Humidity);
            ModbusToHostCmd.ByteNum=4;
            }
       }
    else if(start_addr == SI7021_RH){
       ModbusToHostCmd.Data[0] =  WordSwap(p_info->Humidity); 
      }    
    else if(start_addr > UD_RH) {ret_code = FALSE;}
    else{
        p_ai_value = &(p_info->BatteryVol)+(start_addr-UD_BATTERY_VOL);  p_modbus_value = &ModbusToHostCmd.Data[0];
        ModbusToHostCmd.ByteNum = (end_addr - start_addr+1)*2;
        while(start_addr++ <= end_addr)
            {*p_modbus_value++ = WordSwap(*(p_ai_value++));}
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

    
    if(start_addr == DO485_REAL_VALUE){
        ModbusToHostCmd.Data[0] =  WordSwap(p_info->DoRealValue);
      }
    else if(start_addr == DO485_REAL_OFFSET){
      ModbusToHostCmd.Data[0] =  WordSwap(p_info->DoOffsetValue);
    }
    else if(start_addr == DO485_TEMP_VALUE) {
      ModbusToHostCmd.Data[0] =  WordSwap(p_info->TempRealValue);
    }
    else if(start_addr == DO485_TEMP_OFFSET){
      ModbusToHostCmd.Data[0] =  WordSwap(p_info->TempOffsetValue);
    }    
    else if(start_addr == BATTERY_POWER ){
        ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower));
      }
    
    else{ 
        ret_code=FALSE;
        }
    return ret_code;
}


bool ClientOemSensor(POemSensor p_info)
{
    bool ret_code=TRUE;
    uint16 start_addr,end_addr,*p_ai_value,*p_modbus_value,total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr); 
    total_reg = WordSwap(pModbusCmd->TotalReg);

    if(ModbusToHostCmd.FunCode != 0x03 && start_addr != BATTERY_POWER) 
        { ret_code = FALSE; }
    end_addr = start_addr + total_reg -1;
    if(end_addr > 0x03) end_addr = 0x03;

    if(start_addr == BATTERY_POWER )  {ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower)); }
    else if(start_addr == OEM_ADDR_0A) { ModbusToHostCmd.Data[0] =  WordSwap(p_info->Addr0A);  }
    else if(start_addr > 0x03) {ret_code = FALSE;}
    else{
        p_ai_value = &(p_info->Addr00)+start_addr;  p_modbus_value = &ModbusToHostCmd.Data[0];
        //PrintData("OEM Sensor Data",p_ai_value,5);
        ModbusToHostCmd.ByteNum = (end_addr - start_addr+1)*2;
        while(start_addr++ <= end_addr)
            {*p_modbus_value++ = WordSwap(*(p_ai_value++));}
        }
        

    return ret_code;
}



bool ClientIAQS(PIaqsInfo p_info)
{
    bool ret_code=TRUE;
    uint16 start_addr,end_addr,*p_ai_value,*p_modbus_value,total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr); 
    total_reg = WordSwap(pModbusCmd->TotalReg);
    if(ModbusToHostCmd.FunCode != 0x04)  { return FALSE;}
    end_addr = start_addr + total_reg -1;
    if(end_addr > 0x340) end_addr = 0x340;

    if(start_addr == BATTERY_POWER )  
        {ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower)); }
    else if(start_addr>=0x330 && start_addr<=0x340 )
        {
        p_ai_value = &(p_info->Tempature)+(start_addr-0x330);  p_modbus_value = &ModbusToHostCmd.Data[0];
        ModbusToHostCmd.ByteNum = (end_addr - start_addr+1)*2;
        do{
            *p_modbus_value++ = WordSwap(*(p_ai_value++));             
          }while(++start_addr <= end_addr);
        }
    else{ret_code=FALSE;}
        

    return ret_code;

}

bool ClientCW9(PCw9Info p_info)
{
    bool ret_code=TRUE;
    uint16 start_addr,end_addr,*p_ai_value,*p_modbus_value,total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr); 
    total_reg = WordSwap(pModbusCmd->TotalReg);
    if(ModbusToHostCmd.FunCode != 0x04)  { return FALSE;}
    end_addr = start_addr + total_reg -1;
    if(end_addr > 0x340) end_addr = 0x340;

    if(start_addr == BATTERY_POWER )  
        {ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower)); }
    else if(start_addr>=0x330 && start_addr<=0x340 )
        {
        p_ai_value = &(p_info->Data01)+(start_addr-0x330);  p_modbus_value = &ModbusToHostCmd.Data[0];
        ModbusToHostCmd.ByteNum = (end_addr - start_addr+1)*2;
        do{// Trace16_2(start_addr, end_addr);
           *p_modbus_value++ = WordSwap(*(p_ai_value++));
          }while(++start_addr <= end_addr);
        }
    else{ret_code=FALSE;}
        
    return ret_code;

}




bool ClientA6D6(PA6D6 p_info)
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

    if(start_addr > 7) start_addr = 7;
    if(total_reg > 8) total_reg = 8;

    loop = total_reg;
    
    if(ModbusToHostCmd.FunCode == 0x04)
      {
        p_ai_value = &(p_info->AiValue1);
        p_modbus_value = &ModbusToHostCmd.Data[0];
        for(loop=start_addr; loop<8 && (total_reg--!=0)  ; loop++){
             *p_modbus_value = WordSwap(*(p_ai_value+loop));
             p_modbus_value++; ModbusToHostCmd.ByteNum += 2;
            }
      }
    else if(ModbusToHostCmd.FunCode == 0x02){   
        ModbusToHostCmd.ByteNum = 1;
        bit_mask = bit_mask << start_addr;
        for(loop=start_addr; loop<8 && (total_reg--!=0)  ; loop++)
            {
              if(p_info->Di_Status & bit_mask) {status_bit |= status_mask;}
              bit_mask<<=1; status_mask<<=1;
            }

         *((PUCHAR)(&(ModbusToHostCmd.Data[0]))) = status_bit;
      }
    else if(ModbusToHostCmd.FunCode == 0x01){
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
    //uchar  bit_mask=0x01,status_mask=0x01,status_bit=0x00; 
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
    else{ ret_code = FALSE;}
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
    start_addr  = WordSwap(pModbusCmd->StartAddr);
    total_reg = WordSwap(pModbusCmd->TotalReg);    
    Trace16_1(p_info->Status);
    if(start_addr == BATTERY_POWER ){
        ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower));
      }
    else{ret_code=FALSE;}
    return ret_code;
}

uint16 ClientGetRelayRegister(uint16 loc){
	if (loc==BATTERY_POWER) return (uint16)(pClientInfo->SensorInfo.Header.BatteryPower);
	return 0;
}



//
//
//
bool ClientBtmG6(PBtmG6 p_info)
{
    bool ret_code=TRUE;
    uint16 start_addr,total_reg;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr);
    total_reg = WordSwap(pModbusCmd->TotalReg);    
    if(start_addr == BATTERY_POWER ){
        ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower));
      }
    else if(start_addr == BTM_G6_STATUS ){
           if(p_info->Status.PPercent > 4) {
                p_info->Status.PPercent =4;
            }
           Trace16_1(p_info->Status.G6CurrStatus);
           ModbusToHostCmd.Data[0] = WordSwap(p_info->Status.G6CurrStatus); 
        }
    else if(start_addr == BTM_G6_TIME_FILTER1 ){Trace16_1(p_info->TimeFilter1);
           ModbusToHostCmd.Data[0] = WordSwap(p_info->TimeFilter1); 
        }
    else if(start_addr == BTM_G6_TIME_FILTER2 ){Trace16_1(p_info->TimeFilter2);
           ModbusToHostCmd.Data[0] = WordSwap(p_info->TimeFilter2); 
        }
    else{
        ret_code=FALSE;
      }
    return ret_code;
}



//
//
//
bool ClientVelocity(PVelocity p_info)
{//
    bool ret_code=TRUE;
    uint16 start_addr,total_reg,temp_value;
    float *p_float, float_value;
    ModbusToHostCmd.ModbusID = pModbusCmd->ModbusID;
    ModbusToHostCmd.FunCode = pModbusCmd->FunCode;
    ModbusToHostCmd.ByteNum = 2;
    start_addr  = WordSwap(pModbusCmd->StartAddr);
    total_reg = WordSwap(pModbusCmd->TotalReg);    
    
    if(start_addr == BATTERY_POWER ){
        ModbusToHostCmd.Data[0] = WordSwap((uint16)(pClientInfo->SensorInfo.Header.BatteryPower));
      }
    else if(start_addr == FTM_RAW_VEL_1 ){        
        if(total_reg == 2){ 
            ModbusToHostCmd.Data[0]=WordSwap(*(((PUINT16)&(p_info->RawFlowVelocity))+1));
            ModbusToHostCmd.Data[1]=WordSwap(*((PUINT16)&(p_info->RawFlowVelocity)));
            ModbusToHostCmd.ByteNum=4;
        }else if(total_reg == 1){
            temp_value = *((PUINT16)&(p_info->RawFlowVelocity)+0);            
            ModbusToHostCmd.Data[0] = WordSwap(temp_value);
        }else ret_code=FALSE;
      }
    else if(start_addr == FTM_RAW_VEL_2){
            temp_value = *((PUINT16)(&(p_info->RawFlowVelocity)+1));   
            ModbusToHostCmd.Data[0] = WordSwap(temp_value);
        }
    else if(start_addr == FTM_VEL_1 ){
        if(total_reg == 2){ 
            ModbusToHostCmd.Data[0]=WordSwap(*(((PUINT16)&(p_info->FlowVelocity))+1));
            ModbusToHostCmd.Data[1]=WordSwap(*((PUINT16)&(p_info->FlowVelocity)));
            ModbusToHostCmd.ByteNum=4;
        }else if(total_reg == 1){
            temp_value = *((PUINT16)&(p_info->FlowVelocity)+0);            
            ModbusToHostCmd.Data[0] = WordSwap(temp_value);
        }else ret_code=FALSE;
      }
    else if(start_addr == FTM_VEL_2){
            temp_value = *(((PUINT16)&(p_info->FlowVelocity))+1);   
            ModbusToHostCmd.Data[0] = WordSwap(temp_value);
        }
    else if(start_addr == FTM_VEL_TEMP_1 ){
        if(total_reg == 2){ 
            ModbusToHostCmd.Data[0]=WordSwap(*(((PUINT16)&(p_info->Tempature))+1));
            ModbusToHostCmd.Data[1]=WordSwap(*((PUINT16)&(p_info->Tempature)));
            ModbusToHostCmd.ByteNum=4;
        }else if(total_reg == 1){
            temp_value = *((PUINT16)&(p_info->Tempature)+0);            
            ModbusToHostCmd.Data[0] = WordSwap(temp_value);
        }else ret_code=FALSE;
      }
    else if(start_addr == FTM_VEL_TEMP_2){
            temp_value = *(((PUINT16)&(p_info->Tempature))+1);   
            ModbusToHostCmd.Data[0] = WordSwap(temp_value);
        }        
    else{ 
        ret_code=FALSE;
      }
    return ret_code;
}




bool ClientModbusResponse(){
	uint16 (*getValueFunc)(uint16 loc);
	UCHAR *rx=UsartGetBuff(USART_ID_RX);
	UCHAR *tx=UsartGetBuff(USART_ID_TX);
	uint16 start=(((uint16)rx[2])<<8) | rx[3];
	uint16 len=(((uint16)rx[4])<<8) | rx[5];
	uint16 *values=(void*)&tx[3];
	if(len>127 )return MbsResponseError(rx[0],rx[1],0x02); /* location error*/


	if((pClientInfo = GetServerInfoPos(rx[0]))== NULL) return FALSE;
	switch(pClientInfo->SensorInfo.Header.SensorClass){
#ifdef BTM_A308
	case SENSOR_A308M:
		return A308_Client_Modbus_Response();
#endif
	case SENSOR_RELAY: getValueFunc=ClientGetRelayRegister; break;
	default:
		return FALSE;
	}

	tx[0]=rx[0];
	tx[1]=rx[1];
	tx[2]=len*2;

	for(uint16 i=0; i<len;i++)	values[i]=WordSwap(getValueFunc(start+i));

	return MbsSend(tx,len*2+3);
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
     uint8      sensor_class;
     PA308mInfo p_info_a308m;
     PSdInfo    p_info_jnc_sd;
     PA6D6      p_A6D6;

    sensor_class = p_info->SensorInfo.Header.SensorClass;
    Printf("\r\nID=%02d Class=%d Lose=%02d Power=%03d => ",
            p_info->ServerID,p_info->SensorInfo.Header.SensorClass, 
            p_info->Count,p_info->SensorInfo.Header.BatteryPower
          );  

     switch(sensor_class)
        {
            case SENSOR_SI7021: 
                Printf("BTM Temp = %03d, RH= %02d\r\n",p_info->SensorInfo.Si7021Info.Tempature ,p_info->SensorInfo.Si7021Info.Humidity);
                break;
            case SENSOR_PT485: 
                Printf("PT485 Temp= %d\r\n",p_info->SensorInfo.PT485.Tempature);
                break;
            case SENSOR_AIP: 
                Printf("AIP Power= %d Status=%X\r\n",p_info->SensorInfo.AipInfo.AipPower,p_info->SensorInfo.AipInfo.AipPowerStatus);
                break;
            case SENSOR_RELAY: 
                Printf("Relay: Power=%x\r\n",p_info->SensorInfo.Header.BatteryPower);
                break;            
            case SENSOR_AGB_POWER:
                Printf("PowerStatus = %d \r\n",p_info->SensorInfo.AgbPower.PowerStatus);
                break;
            case SENSOR_CW9:
                Printf("CW9 Data01 = %d \r\n",p_info->SensorInfo.Cw9Info.Data01);
                break;
            case SENSOR_BTM_G6:
                Printf("G6 Status = %04xh Filter1 = %04xh, Filter2 = %04xh, \r\n",
                        p_info->SensorInfo.BtmG6.Status.G6CurrStatus,p_info->SensorInfo.BtmG6.TimeFilter1,
                        p_info->SensorInfo.BtmG6.TimeFilter2);
                break;
        };
        
#endif    
    return;
}





