#include "global.h"

#include "bus_usart.h"
#include "Mesh_Node.h"
#include "node_data.h"
#include "ClientModbus.h"

#if defined(BTM_TRANSMITTER) || defined(JNC_BT_MESH) || defined(BT_MESH_G6)

extern PClientInfo pClientInfo;
extern uchar GetBatteryPower();

extern uint32 ClientBroadcastCounter;
extern uint32 ServerResponseCounter[SERVER_NODE_MAX+1];
extern _ClientInfo ClientInfo[SERVER_NODE_MAX+1];
extern void ClearServerResponseCounter();

/*即時值*/
uint16 GetAiRegister(uint16 loc){
	PClientInfo pServer;
	uint8 id;
	int16 idx;
    uint32 lCount=0;
	if(loc==pMeshNodeData->MeshNodeID) return GetBatteryPower(); /*Client自身電量*/
	else if(loc<=0xff) { /*Server電量*/
		pServer=GetServerInfoPos(loc);
		if(pServer && pServer->SensorInfo.Header.SensorClass) return pServer->SensorInfo.Header.BatteryPower;
		return 0xffff;
	}else if(loc<=0x1ff){ /*計算回應百分比*/
		id=loc-0x100;
		if (ClientBroadcastCounter==0) return 0;
		pServer=GetServerInfoPos(id);
		if(pServer){
			idx=((uint8*)pServer-(uint8*)ClientInfo)/sizeof(_ClientInfo);
			return (uint16)((double)ServerResponseCounter[idx]/(double)ClientBroadcastCounter*100);
		}else return 0xffff;
	}else if(loc<=0x3ff){ /*命令/回應計數*/
		id=(loc-0x200)/2;
		if (id==0) lCount=ClientBroadcastCounter; /*命令次數*/
		else{
			pServer=GetServerInfoPos(id);
			if (pServer){ /*回應次數*/
				idx=((uint8*)pServer-(uint8*)ClientInfo)/sizeof(_ClientInfo);
				lCount=ServerResponseCounter[idx];
			}
		}
		return ((loc-0x200)%2)?(lCount>>16):(lCount&0xffff);
	}
	return 0;
}

//#define ToWORD(v) (*(uint32*)&(v))
uint32 ToWORD(float v){
	return *(uint32*)&(v);
}

#define FromWordH(dest,value) ((uint16*)&(dest))[1]=value
#define FromWordL(dest,value) ((uint16*)&(dest))[0]=value

/*
void FromWordH(float *dest,uint16 value){
	((uint16*)dest)[1]=value;
}
void FromWordL(float *dest,uint16 value){
	((uint16*)dest)[0]=value;
}*/
#define NoDataChanged 0
#define NodeDataChanged 1
#define AdjChanged 2

extern uint32 ClientResponseTestMs;
extern bool ClientResponseTestMode;

/*設定值*/
uint16 GetMemoryRegister(uint16 loc){
	switch(loc){
	case MBS_MODELNAME0: return ((uint16)MODEL_NAME[1]<<8) | MODEL_NAME[0];
	case MBS_MODELNAME1: return ((uint16)MODEL_NAME[3]<<8) | MODEL_NAME[2];
	case MBS_MODELNAME2: return ((uint16)MODEL_NAME[5]<<8) | MODEL_NAME[4];
	case MBS_VERSION: return FW_VER;
	case MBS_MESH_ID: return pMeshNodeData->MeshNodeID;
	case MBS_BAUDRATE: return pMeshNodeData->BaudRate;
	case MBS_SLEEPTIME: return pMeshNodeData->WorkingTimer;
	case MBS_REBOOT_IDEL_TIME: return pMeshNodeData->RebootForRs485IdelSecnods;
	case MBS_KEEPALIVE_BEFORE_SLEEP: return pAdjValue->RS485TransmitterData.Rs485ServerDelayBeforeSleep;
	case MBS_BUFF_TIMEOUT: return pAdjValue->RS485TransmitterData.Rs485ClientBuffTimeoutMs;
	case MBS_TEMP_GAIN_L: return ToWORD(pAdjValue->TempGain)&0xffff;
	case MBS_TEMP_GAIN_H: return ToWORD(pAdjValue->TempGain)>>16;
	case MBS_HUM_GAIN_L: return ToWORD(pAdjValue->HumGain)&0xffff;
	case MBS_HUM_GAIN_H: return ToWORD(pAdjValue->HumGain)>>16;
	case MBS_TEMP_OFFSET_L: return ToWORD(pAdjValue->TempOffset)&0xffff;
	case MBS_TIME_OFFSET_H: return ToWORD(pAdjValue->TempOffset)>>16;
	case MBS_HUM_OFFSET_L: return ToWORD(pAdjValue->HumOffset)&0xffff;
	case MBS_HUM_OFFSET_H: return ToWORD(pAdjValue->HumOffset)>>16;
	case MSS_RSP_TEST_SEC: return ClientResponseTestMs/1000;
	default: return 0;
	}
}

int SetMemoryRegister(uint16 loc, uint16 value){
	switch(loc){
	case MBS_BAUDRATE: pMeshNodeData->BaudRate=value; return NodeDataChanged;
	case MBS_SLEEPTIME: pMeshNodeData->WorkingTimer=value; return NodeDataChanged;
	case MBS_REBOOT_IDEL_TIME: pMeshNodeData->RebootForRs485IdelSecnods=value; return NodeDataChanged;
	case MBS_KEEPALIVE_BEFORE_SLEEP: pAdjValue->RS485TransmitterData.Rs485ServerDelayBeforeSleep=value; return AdjChanged;
	case MBS_BUFF_TIMEOUT: pAdjValue->RS485TransmitterData.Rs485ClientBuffTimeoutMs=value; return AdjChanged;
	case MBS_TEMP_GAIN_L: FromWordL(pAdjValue->TempGain,value); return AdjChanged;
	case MBS_TEMP_GAIN_H: FromWordH(pAdjValue->TempGain,value); return AdjChanged;
	case MBS_HUM_GAIN_L: FromWordL(pAdjValue->HumGain,value); return AdjChanged;
	case MBS_HUM_GAIN_H: FromWordH(pAdjValue->HumGain,value); return AdjChanged;
	case MBS_TEMP_OFFSET_L: FromWordL(pAdjValue->TempOffset,value); return AdjChanged;
	case MBS_TIME_OFFSET_H: FromWordH(pAdjValue->TempOffset,value); return AdjChanged;
	case MBS_HUM_OFFSET_L: FromWordL(pAdjValue->HumOffset,value); return AdjChanged;
	case MBS_HUM_OFFSET_H: FromWordH(pAdjValue->HumOffset,value); return AdjChanged;
	case MBS_RESET_CMD_RSP_COUNT: if(value==0x3636) ClearServerResponseCounter(); return NoDataChanged;
	case MBS_REBOOT: if(value==0x3636) SetEventTaskTimer(TD_SYS_SETUP_RESET,1000,TIMER_EVENT_ONCE); // system reset; return NoDataChanged;
	case MSS_RSP_TEST_SEC: if(value)ClearServerResponseCounter(); ClientResponseTestMs=value*1000;ClientResponseTestMode=false; return NoDataChanged;
	default: return NoDataChanged;
	}
}

int SetCoilRegister(uint16 loc, bool value){
	switch(loc){
	case MSS_RSP_TEST_SEC:
		if (value){
			ClearServerResponseCounter();
			ClientResponseTestMs=3*60*1000;
		}else{
			ClientResponseTestMs=0;
		}
		ClientResponseTestMode=false;
		return NoDataChanged;
		break;
	}
	return NoDataChanged;
}

bool GetCoilRegister(uint16 loc){
	switch(loc){
	case MSS_RSP_TEST_SEC:
		return ClientResponseTestMs>0;
	break;
	}
	return false;
}

int16 ResponseModbusError(uint8 *rx, uint8 *tx){
	tx[0]=rx[0];
	tx[1]=0x80|rx[1];
	tx[2]=0;
	tx[3]=0;
	return MbsSend(tx,4);
}

//void ResponseClientInfo(){
int16 ClientModbusProc(){
	uint8 *rx=UsartGetBuff(USART_ID_RX);
	uint8 *tx=UsartGetBuff(USART_ID_TX);
	uint16 st=(((uint16)rx[2])<<8) +rx[3];
	uint16 count=(((uint16)rx[4])<<8) +rx[5];
	uint16 (*GetValue)(uint16);
	uint16 value;
	int changed=0;


	switch(rx[1]){
	case 3: case 4:

		if (count>((USART_TX_BUFF_SIZE-5)/2)) return ResponseModbusError(rx,tx); /*超出回應長度*/
		if (UsartGetRxCounter()!=8) return ResponseModbusError(rx,tx);/*命令長度不符*/
		GetValue=rx[1]==3?GetMemoryRegister:GetAiRegister;
		tx[0]=rx[0];
		tx[1]=rx[1];
		tx[2]=count*2;
		for(int i=0;i<count;i++){
			value=GetValue(st+i);
			tx[3+i*2]=value>>8;
			tx[4+i*2]=value&0xff;
		}
		return MbsSend(tx,3+count*2);
	case 6:
		if (UsartGetRxCounter()!=8) return ResponseModbusError(rx,tx);/*命令長度不符*/
		changed|=SetMemoryRegister(st,(((uint16)rx[4])<<8) +rx[5]);
		MbsSend(rx,6);
		break;
	case 16:
		if (UsartGetRxCounter()!=(9+count*2)) return ResponseModbusError(rx,tx);/*命令長度不符*/
		for(int i=0;i<count;i++) changed|=SetMemoryRegister(st+i,(((uint16)rx[i*2+7])<<8) +rx[i*2+8]);
		MbsSend(rx,6);
		break;
	case 1:
		if (UsartGetRxCounter()!=8) return ResponseModbusError(rx,tx);/*命令長度不符*/
		GetCoilRegister(st);
		memcpy(tx,rx,2);
		tx[2]=(count+7)/8;
		memset(tx+3,0,tx[2]);
		for (int i=0;i<count;i++) tx[3+(i/8)]|=GetCoilRegister(st+i)?0x01<<(i%8):0;
		MbsSend(tx,3+tx[2]);
		break;
	case 5:
		if (UsartGetRxCounter()!=8 || rx[5]!=0 || (rx[4]!=0xff && rx[4]!=0)) return ResponseModbusError(rx,tx);/*命令長度不符 or指定內容格式不符*/
		changed|=SetCoilRegister(st,rx[4]==0xff);
		MbsSend(rx,6);
		break;
	case 0xf: //func=15
		if (UsartGetRxCounter()!=(9+((count+7)/8))) return ResponseModbusError(rx,tx);/*命令長度不符*/
		for(int i=0;i<count;i++) changed|=SetCoilRegister(st+i,((rx[7+(i/8)]>>(i%8))&1)==1);
		MbsSend(rx,6);
		break;
	default:
		return ResponseModbusError(rx,tx); /*不支援的指令*/
	}


	if(changed)WriteNodeData();
	return 0;
}


#endif
