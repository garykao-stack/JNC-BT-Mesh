/*
 * A308.c
 *
 *  Created on: 2022年7月5日
 *      Author: user
 */

#include "global.h"
//#include "ble_comm.h"
//#include "cmd_rsp_evt.h"
//#include "ble_msg.h"
//#include "base_def.h"
#include "Mesh_Node.h"
#include "bus_usart.h"

#if BTM_A308
#include "A308_Server.h"

typedef struct {
	uint16 loc;
	uint16 len;
	uint16 offset;
} MbsInfo;

typedef struct{
	uint8 id;
	//uint8 index;
}IdMap;


#define A308_DEVICE_COUNT 14 //SERVER_NODE_MAX
#define A308_MEM_SIZE	(62+100*6+6+14)
#define A308_BTM_SIZE	50
#define A308_TOT_MBS_CMD	(sizeof(A308Info)/sizeof(MbsInfo))
#define A308_TOT_BTM_RSP	(A308_MEM_SIZE*2/A308_BTM_SIZE+1)
#define A308_MBS_TIMEOUT	300

MbsInfo A308Info[]={
		{0x000,	62,		0},
		{0x100,	100,	62},
		{0x164,	100,	62+100},
		{0x200, 100,	62+100*2},
		{0x264, 100,	62+100*3},
		{0x300, 100,	62+100*4},
		{0x364, 100,	62+100*5},
		{0x400, 6,		62+100*6},
		{0x40c, 14,		62+100*6+6}
};

uint8 A308TableBuff[A308_MEM_SIZE*2];
uint8 A308Table[A308_MEM_SIZE*2*A308_DEVICE_COUNT];
uint8 A308CmdIndex=0;
IdMap A308IdMap[A308_DEVICE_COUNT];
uint8 A308DataIsReceived=0;

extern _ServerInfo ServerInfo;
extern PNodeHeader pSensorHeader;


typedef enum{
	A308Step_Command=0,
	A308Step_WaitMbsRsp=1,
}A308_STEP;

A308_STEP A308Step=A308Step_Command;
extern bool UsartIsBusy();

int16 MbsUpdateSpan=-1;
int16 a308_client_receive_span_time=-1;

void A308_Initialize(){
	memcpy(A308IdMap,0,sizeof(A308IdMap));
	A308CmdIndex=0;
	MbsUpdateSpan=-1;
	a308_client_receive_span_time=-1;
	//for(uint16 i=0;i<(A308_MEM_SIZE*2);i++) A308Table[i]=i&0xff;
}

uint16 A308_Fetch_Timeout_Ms(){
	return  A308_TOT_MBS_CMD*(A308_MBS_TIMEOUT+300);
}



#define ToStep(step) A308Step=step
#define TIMER_REDUCE_BY_10MS(t) do{if(t>0)t=t-((t>=10)?10:t);}while(0)
#define TIMER_IS_TIMEOUT(t) (t==0)
uint16_t wait_rsp_ms=0;
uint16 a308_btm_wait_ms=0;
//uint8 a308_client_get_info_retry=0;
uint8 a308_server_mbs_cmd_retry=0;
uint16 a308_client_get_info_wait_ms=0;
uint8 flag_a308_init_mbs_cmd=0;
uint8 flag_a308_stop_mbs_cmd=0;
uint8 a308_update_seq=0;

void A308_TimeEvent(){
	TIMER_REDUCE_BY_10MS(wait_rsp_ms);
	TIMER_REDUCE_BY_10MS(a308_btm_wait_ms);
	TIMER_REDUCE_BY_10MS(a308_client_get_info_wait_ms);
	if(MbsUpdateSpan>=0) MbsUpdateSpan++;
	if(a308_client_receive_span_time>=0)a308_client_receive_span_time++;
}

void A308_ResetModbusCmd(){
	flag_a308_init_mbs_cmd=1;
	wait_rsp_ms=0;
	flag_a308_stop_mbs_cmd=0;
	a308_server_mbs_cmd_retry=0;
	UsartResetRxTx(USART_ID_TX_RX);
}

bool A308_Connected(){
	/* 回傳連線狀態 !!!!!!!!!!!!!!!!!!!!!!!!!!*/
	return A308DataIsReceived && flag_a308_init_mbs_cmd==0;
}

void A308_StopModbusAction(){
	flag_a308_stop_mbs_cmd=1;
}

void A308_ModbusAction(){
	MbsInfo *pm;
	pm=&A308Info[A308CmdIndex];
	if (flag_a308_stop_mbs_cmd) return;
	switch(A308Step){
	case A308Step_Command:
		if(!TIMER_IS_TIMEOUT(wait_rsp_ms)) break;
		if(flag_a308_init_mbs_cmd){
			flag_a308_init_mbs_cmd=0;
			A308CmdIndex=0;
			A308DataIsReceived=0;
			a308_server_mbs_cmd_retry=0;
		}
		if (A308CmdIndex==0)MbsUpdateSpan=0;
		dprint("cmd > loc:%d, len:%d\r\n", pm->loc,pm->len);
		MbsSetReadRegCmd(1,3,pm->loc,pm->len);
		wait_rsp_ms=A308_MBS_TIMEOUT;
		ToStep(A308Step_WaitMbsRsp);
		break;
	case A308Step_WaitMbsRsp:
		if(GetNodeStatus(NS_USART_RX_EVENT)){
			if(!UsartGetStatus(USART_RX_CRC_ERROR)){
				a308_server_mbs_cmd_retry=0;
				if (pm->offset+pm->len >A308_MEM_SIZE){
					dprint("!!!!! MBS Response is overrange !!!!!\r\n");
				}else
					memcpy((PUCHAR)A308Table+pm->offset*2,RxBuff+3,pm->len*2);
				A308CmdIndex=(A308CmdIndex+1)%A308_TOT_MBS_CMD;
				if (A308CmdIndex==0){
					a308_update_seq++;
					wait_rsp_ms=5000; /*5秒更新一次資料*/
					A308DataIsReceived=1;
					SetNodeInfoSize(_A308mInfo);
					SetNodeClass(SENSOR_A308M);
					dprint("Update A308 Info Span %d(ms)\r\n",MbsUpdateSpan*10);
					MbsUpdateSpan=-1;
					A308_StopModbusAction();
				}else
					wait_rsp_ms=0;
			}else{
				if(++a308_server_mbs_cmd_retry>3){
					A308_ResetModbusCmd();
					A308_StopModbusAction();
					ToStep(A308Step_Command);
					return;
				}
				wait_rsp_ms=1000;
			}
			SetNodeStatus(NS_USART_RX_EVENT,OFF);
			UsartResetRxTx(USART_ID_TX_RX);
			ToStep(A308Step_Command);
		}else if(TIMER_IS_TIMEOUT(wait_rsp_ms)){
			SetNodeStatus(NS_USART_RX_EVENT,OFF);
			UsartResetRxTx(USART_ID_TX_RX);
			A308DataIsReceived=0; /*未接收到回應，設定為斷線*/
			if(++a308_server_mbs_cmd_retry>3){
				A308_ResetModbusCmd();
				A308_StopModbusAction();
			}
			ToStep(A308Step_Command);
		}
		break;
	}

}

uint16 A308_ReadTable(PClientInfo pInfo,uint16 idx_table,uint16 loc){
	PUCHAR tab=A308Table+(idx_table*A308_MEM_SIZE*2);
	MbsInfo *tab_info;
	if (idx_table>=A308_DEVICE_COUNT) return  0;
	if (loc==0x3e) return ((uint16)pInfo->SensorInfo.Header.BatteryPower)<<8;
	for (int i=0;i<A308_TOT_MBS_CMD;i++){
		tab_info=&A308Info[i];
		if (loc>=tab_info->loc && loc<(tab_info->loc+tab_info->len)){
			return ((uint16*)tab)[tab_info->offset+(loc-tab_info->loc)];
		}
	}
	return  0;
}


int8 A308_FindTableIndex(uint8 id){
	for(int i=0;i<A308_DEVICE_COUNT;i++) if(A308IdMap[i].id==id) return i;
	for(int i=0;i<A308_DEVICE_COUNT;i++) if(A308IdMap[i].id==0) return i;
	return -1;
}

//extern _ClientInfo ClientInfo[];

bool A308_Client_Modbus_Response(){
	UCHAR *rx=UsartGetBuff(USART_ID_RX);
	UCHAR *tx=UsartGetBuff(USART_ID_TX);
	PClientInfo pInfo=GetServerInfoPos(rx[0]);

	uint8 fun=rx[1];
	uint16 loc=(((uint16)rx[2])<<8) | rx[3];
	uint16 len=(((uint16)rx[4])<<8) | rx[5];
	//uint8 idx=((uint8*)pInfo-(uint8*)ClientInfo)/sizeof(_ClientInfo);
	uint16 *values=(void*)&tx[3];

	int8 idxTab;


	if(!pInfo || pInfo->ServerID== 0) return FALSE;
	idxTab=A308_FindTableIndex(pInfo->ServerID);
	if (idxTab==-1 || A308IdMap[idxTab].id==0){
		dprint("!!! A308 Can't Get Table Index. ID:%d\r\n",pInfo->ServerID);
		return FALSE;
	}

	dprint("A308 Mbs Cmd > ID:%d(%d), index:%d, loc:%d(0x%X), len:%d\r\n", rx[0],pInfo->ServerID,idxTab,loc,loc,len);

	if(fun!=3){
		return MbsResponseError(rx[0],rx[1],0x01); /* function error */
	}else if(len>127 || idxTab>=A308_DEVICE_COUNT){
		return MbsResponseError(rx[0],rx[1],0x02); /* location error*/
	}

	tx[0]=rx[0];
	tx[1]=rx[1];
	tx[2]=len*2;
	for(int i=0;i<len;i++) values[i]=A308_ReadTable(pInfo,idxTab,loc+i);
	return MbsSend(tx,len*2+3);
}


int8 idxSendToClient=0;
typedef struct{
	uint16 seq;
	uint16 loc;
	uint16 len;
	uint8 data[A308_BTM_SIZE];
}A308_RSP;

uint16 A308_SendToClientPart(uint16 loc, uint16 len, uint8 *data){
	A308_RSP rsp;
	uint16 result;
	if (len>A308_BTM_SIZE) return 1;
	rsp.loc=loc;
	rsp.len=len;
	rsp.seq=a308_update_seq;
	memcpy(rsp.data,data,len);
	/*IFDPRINT(
		dprint("bt send:");
		for(int i=0;i<len;i++) dprint(" %02X",rsp.data[i]);
		dprint("\r\n");
	)*/
	result=Cmd_ms_server_send_series_status(SENSOR_ELEMENT, pNodeEventInfo->ClientAddr,pNodeEventInfo->AppkeyIndex,
			  NO_FLAGS,NODE_GET_A308_TABLE, len+6, ((PUCHAR)&rsp))->result;
	//dprint(" *** A308 SendToClient loc:%d, len:%d, result:%d\r\n",loc,len,result);
	return result;
}

typedef enum{
	A308_GET_INFO_INIT=0,
	A308_GET_INFO_CMD=1,
	A308_GET_INFO_WAIT_CMD=2,
	A308_GET_INFO_WAIT_RSP=3,
	A308_GET_INFO_FINISH=4
}A308_BTM_GET_INFO_STEP;
A308_BTM_GET_INFO_STEP client_get_info_step=A308_GET_INFO_INIT;

extern PClientInfo FindNextServerInfo(uint8 class, PClientInfo pClient);
PClientInfo pGetInfoCurrClient=0;

uint8 a308_client_get_info_retry=0;
uint8 a308_client_recieved_msg_count=0;
uint16 a308_received_id=0;
uint32 flag_rec_status=0;

void A308_GetInfo_Init_Flag(){
	uint32 f=1;
	flag_rec_status=0;
	for (uint8 i =0;i<A308_TOT_BTM_RSP;i++){
		flag_rec_status|=f;
		f=f<<1;
	}
}

void A308_GetInfo_Rst_Flag(uint8 idx){
	flag_rec_status&=~(0x00000001<<idx);
}

void A308_GetInfo_Set_Flag(uint32 status){
	flag_rec_status=status;
}


bool A308_Client_GetInfo(){
	uint16 result;
	switch(client_get_info_step){
	case A308_GET_INFO_INIT:
		pGetInfoCurrClient=FindNextServerInfo(SENSOR_A308M,0);
		if (pGetInfoCurrClient==0) return 0; /*找不到A308設備，離開*/
		client_get_info_step=A308_GET_INFO_CMD;
		a308_client_receive_span_time=0;
		a308_client_recieved_msg_count=0;
		A308_GetInfo_Init_Flag();
		return 1;
	case A308_GET_INFO_CMD:
		SetLed(LED_SERVER,ON);
		//a308_client_recieved_msg_count=0;
		a308_received_id=pGetInfoCurrClient->ServerID;
		dprint("STEP: A308_GET_INFO_CMD ID:%d, flag:0x%08X\r\n",a308_received_id,flag_rec_status);

		//result = Cmd_ms_client_get(SENSOR_ELEMENT, a308_received_id, IGNORED, 0xA5, NODE_GET_A308_TABLE)->result;
		result = Cmd_ms_client_get_column(SENSOR_ELEMENT, a308_received_id, IGNORED, 0xA5, NODE_GET_A308_TABLE,4,(uint8*)&flag_rec_status)->result;
		if(result){
			if(++a308_client_get_info_retry>=5){ /*失敗太多次，放棄*/
				client_get_info_step=A308_GET_INFO_INIT;
				return 0;
			}else{ /*重試*/
				a308_client_get_info_wait_ms=400;
				client_get_info_step=A308_GET_INFO_WAIT_CMD;
				return 1;
			}
		}else{
			a308_client_get_info_wait_ms=2000;
			client_get_info_step=A308_GET_INFO_WAIT_RSP;
			return  1;
		}
		break;
	case A308_GET_INFO_WAIT_CMD:
		if(TIMER_IS_TIMEOUT(a308_client_get_info_wait_ms)) client_get_info_step=A308_GET_INFO_CMD;
		return 1;

	case A308_GET_INFO_WAIT_RSP:

		/*此設備所有回應接收完成 || Timeout*/
		if(a308_client_recieved_msg_count>=A308_TOT_BTM_RSP || TIMER_IS_TIMEOUT(a308_client_get_info_wait_ms)){
			if(a308_client_recieved_msg_count>=A308_TOT_BTM_RSP){

			}else if(TIMER_IS_TIMEOUT(a308_client_get_info_wait_ms)){
				dprint("\r\n!!! Receive A308 Timeout\r\n");
				if(a308_client_recieved_msg_count){ /*讀取不完整，再讀一次*/
				//if(flag_rec_status){ /*讀取不完整，再讀一次*/
					dprint("!!! A308 Receive Table is missing. try again(%d/%d)\r\n",a308_client_recieved_msg_count,A308_TOT_BTM_RSP);
					//a308_client_recieved_msg_count=0;
					a308_client_get_info_retry=0;
					//client_get_info_step=A308_GET_INFO_CMD;
					a308_client_get_info_wait_ms=400;
					client_get_info_step=A308_GET_INFO_WAIT_CMD;
					return 1;
				}else{
					dprint("!!! A308 Server has no Response. ID:%d, try:%d\r\n",pGetInfoCurrClient->ServerID,a308_client_get_info_retry);
					if(++a308_client_get_info_retry<5){
						a308_client_get_info_wait_ms=400;
						client_get_info_step=A308_GET_INFO_WAIT_CMD;
						return 1;
					}
				}
			}
			a308_client_get_info_wait_ms=0;
			pGetInfoCurrClient=FindNextServerInfo(SENSOR_A308M,pGetInfoCurrClient);
			if (pGetInfoCurrClient){ /*要求讀取下一個設備*/
				A308_GetInfo_Init_Flag();
				a308_client_get_info_retry=0;
				a308_client_recieved_msg_count=0;
				client_get_info_step=A308_GET_INFO_CMD;
				return 1;
			}else{ /*所有設備都讀取完成，離開*/
				dprint("*** A308 Receive Span %d(ms)\r\n",a308_client_receive_span_time*10);
				a308_client_get_info_wait_ms=0;
				a308_client_get_info_retry=0;
				a308_client_recieved_msg_count=0;
				client_get_info_step=A308_GET_INFO_FINISH;//A308_GET_INFO_INIT;
				A308_GetInfo_Init_Flag();
				return 1;
			}
			//a308_client_get_info_wait_ms=1000;
		}else /*流程未完成，繼續等待*/
			return 1;
	case A308_GET_INFO_FINISH: /*發送結束訊息，通知Relay進入休眠*/
		if(!TIMER_IS_TIMEOUT(a308_client_get_info_wait_ms)) return 1;
		result=Cmd_ms_client_get(SENSOR_ELEMENT, 0, IGNORED, 0xA5, NODE_A308_GET_FINISHED)->result;
		dprint("*** A308 send finished notify. result:%d, retry:%d\r\n",result,a308_client_get_info_retry);
		if (result && ++a308_client_get_info_retry<10){
			a308_client_get_info_wait_ms=100;
			return 1;
		}else{
			client_get_info_step=A308_GET_INFO_INIT;
			return  0;
		}
		break;
	default:
		dprint("!!! Error A308_Client_GetInfo. unknow step:%d\r\n",client_get_info_step);
		client_get_info_step=A308_GET_INFO_INIT;
		return 0;
	}

}

void ResetGetInfoWaitTimeout(){
	if(client_get_info_step==A308_GET_INFO_WAIT_RSP) a308_client_get_info_wait_ms=3000;
}

typedef enum{
	A308_BTM_STEP_INIT=0,
	A308_BTM_STEP_RSP_TABLE_SEND=1,
	A308_BTM_STEP_RSP_TABLE_WAIT=2
}A308_BTM_STEP;
A308_BTM_STEP a308_btm_step=A308_BTM_STEP_INIT;
uint8 a308_btm_retry=0;

#define ToBtmStep(step) a308_btm_step=step

int8 GetNextIndexSendToClient(){
	uint32 f=0x00000001;
	for(int8 i=0;i<A308_TOT_BTM_RSP;i++){
		if(f&flag_rec_status) return i;
		f=f<<1;
	}
	return -1;

}

/* 發送A308內容表格
 * 回傳值 :
 * 0:	程序完成
 * 1:	程序未結束
*/
uint16 A308_SendToClient(){
	uint16 len;
	uint16 loc;
	switch(a308_btm_step){
	case A308_BTM_STEP_INIT:
		a308_btm_retry=0;
		//idxSendToClient=0;
		idxSendToClient=GetNextIndexSendToClient();
		if(idxSendToClient<0) return 0;
		/* no break */
	case A308_BTM_STEP_RSP_TABLE_SEND:

		if(idxSendToClient<0) return 0;
		loc=idxSendToClient*A308_BTM_SIZE;
		len=A308_MEM_SIZE*2-loc;

		if(!A308DataIsReceived){ //Modbus data are not filly received.

			return 0;
		}

		result=A308_SendToClientPart(loc, (len>A308_BTM_SIZE)?A308_BTM_SIZE:len, A308Table+loc);

		if(result && ++a308_btm_retry<40){
			a308_btm_wait_ms=100;
			ToBtmStep(A308_BTM_STEP_RSP_TABLE_WAIT);
			return 1;
		}else{
			dprint("*** A308 SendToClient loc:%d, len:%d, result:%d, retry:%d\r\n",loc,(len>A308_BTM_SIZE)?A308_BTM_SIZE:len,result,a308_btm_retry);

			//if(++idxSendToClient>=A308_TOT_BTM_RSP){
			A308_GetInfo_Rst_Flag(idxSendToClient);
			idxSendToClient=GetNextIndexSendToClient();
			if(idxSendToClient<0){
				ToBtmStep(A308_BTM_STEP_INIT);
				return 0;
			}else{
				a308_btm_retry=0;
				a308_btm_wait_ms=100;
				ToBtmStep(A308_BTM_STEP_RSP_TABLE_WAIT);
				return 1;
			}
		}
		break;
	case A308_BTM_STEP_RSP_TABLE_WAIT:
		if(TIMER_IS_TIMEOUT(a308_btm_wait_ms)) ToBtmStep(A308_BTM_STEP_RSP_TABLE_SEND);
		return 1;
	default:
		return 0;
	}
}


/*存到暫存表中，全表接收完成後再一次寫入目標位置*/
void A308ClientSeriesEvent(msg_ms_client_series_status_evt *pEvent){
	A308_RSP *rsp=(A308_RSP*)pEvent->sensor_data.data;
	PClientInfo pInfo=GetServerInfoPos(pEvent->server_address);
	if(!pInfo || pInfo->ServerID== 0){
		dprint("!!! A308 Series Error. ID(%d) don't existed.\r\n",pEvent->server_address);
		return;
	}else if(pInfo->ServerID!=a308_received_id){
		dprint("!!! A308 Series Error. Receive ID(%d)!= Target ID(%d)\r\n",pInfo->ServerID,a308_received_id);
		return;
	}
	int8 idx=A308_FindTableIndex(pEvent->server_address);
	if (idx==-1){
		dprint("!!! A308 SeriesError. ID(%d) there is no space to receive table.\r\n",pEvent->server_address);
		return;
	}



	//uint8 idx=(pInfo-ClientInfo)/sizeof(_ClientInfo);
	uint8 *data=A308TableBuff;//A308Table+(idx*A308_MEM_SIZE*2) ;//[A308_MEM_SIZE*2*A308_BTM_SIZE]
	//dprint("Receive A308 Table. index:%d, loc:%d, len:%d\r\n", idx, rsp->loc, rsp->len);

	if (idx>=A308_DEVICE_COUNT) {dprint("A308 receive error: device index is overrange.\r\n");return;} //exceed device number
	if ((rsp->loc+rsp->len)>(A308_MEM_SIZE*2)) {dprint("A308 receive error: target memory is overrange\r\n");return;} //exceed device table range
	if(a308_client_recieved_msg_count==0)a308_update_seq=rsp->seq;
	if(a308_update_seq!=rsp->seq){
		dprint("A308 receive error: sequence number is not matched. target:%d ,receive:%d\r\n",a308_update_seq,rsp->seq);
		a308_client_recieved_msg_count=0;
		A308_GetInfo_Init_Flag();
		return;
	}

	a308_client_recieved_msg_count++;
	A308_GetInfo_Rst_Flag(rsp->loc/A308_BTM_SIZE);
	memcpy(data+rsp->loc,rsp->data,rsp->len);

	//if(++a308_client_recieved_msg_count>=A308_TOT_BTM_RSP){
	if(flag_rec_status==0){
		memcpy(A308Table+(idx*A308_MEM_SIZE*2),data,A308_MEM_SIZE*2);
		A308IdMap[idx].id=pEvent->server_address;
	}else{

		ResetGetInfoWaitTimeout();
	}

	dprint("A308 Receive Table(%d/%d) ID:%d, Table index:%d, seq:%d, no:%d, loc:%d, len:%d, flag:0x%08X\r\n",
			a308_client_recieved_msg_count,A308_TOT_BTM_RSP,pEvent->server_address,idx,a308_update_seq,rsp->loc/A308_BTM_SIZE,rsp->loc,rsp->len,flag_rec_status);
	//dprint("Receive A308 Table. ID:%d, index:%d, loc:%d, len:%d\r\n",pEvent->server_address, idx, rsp->loc, rsp->len);
	/*IFDPRINT(
		for (int i=0;i<rsp->len;i++) dprint(" %02X", *(data+rsp->loc+i));
		dprint("\r\n");
	)*/
}

#endif

