/*
 * RS485_Transmitter.c
 *
 *  Created on: 2022年8月19日
 *      Author: user
 */
#include "global.h"


#if defined(BTM_TRANSMITTER) && TRANS_BUFF_MODE

typedef enum{
	rtRCoil=2,
	rtRWCoil=1,
	rtRRegister=4,
	rtRWRegister=3
}REG_TYPE;

typedef struct{
	uint8 reg_type;
	uint8 id;
	uint16 loc;
	uint16 value;
} RegisterBuffer;

typedef struct{
	uint8 id;
	uint8 fun;
	uint16 start;
	uint16 len;
}MBS_READ_CMD;


#define MBS_REGISTER_BUFF_COUNT 400
RegisterBuffer regBuff[MBS_REGISTER_BUFF_COUNT];
MBS_READ_CMD MbsReadCmd;
static int maxIndex=-1;
static uint8 sourceBtId[256];
static uint8 cntWaitResponse[256];

void MbsTransmitterInit(){
	memset(regBuff,0,sizeof(regBuff));
	memset(&MbsReadCmd,0,sizeof(MbsReadCmd));
	memset(sourceBtId,0,sizeof(sourceBtId));
	memset(cntWaitResponse,0,sizeof(cntWaitResponse));
}

RegisterBuffer* MbsFindEmptyBuff(){
	for(int i=0;i<MBS_REGISTER_BUFF_COUNT;i++) if(regBuff[i].id==0){
		if (i>maxIndex) maxIndex=i;
		return &regBuff[i];
	}
	return 0;
}

RegisterBuffer* MsbTransFindReg(uint8 id, uint16 loc, REG_TYPE type){
	for (int i=0;i<=maxIndex;i++){
		if(regBuff[i].id==id && regBuff[i].loc==loc && regBuff[i].reg_type==type) return &regBuff[i];
	}
	return 0;
}

void MbsTransmitterSet(uint8 id, uint16 loc, REG_TYPE type, uint16 value){
	RegisterBuffer *buff=MsbTransFindReg(id,loc,type);
	if (!buff) buff=MbsFindEmptyBuff();
	if (!buff) return;
	dprint("record mem:0x%x, id:%d, loc:%d, value:%d\r\n",buff, id,loc,value);
	buff->reg_type=type;
	buff->id=id;
	buff->loc=loc;
	buff->value=value;
}

uint16 MbsTransmitterGet(uint8 id, uint16 loc, REG_TYPE type){
	RegisterBuffer *buff=MsbTransFindReg(id,loc,type);
	if (buff) return buff->value;
	return 0;
}

void MbsTransClearBuff(uint8 id, uint16 loc, REG_TYPE type){
	for(int i=maxIndex;i>=0;i--){
		if(regBuff[i].id==id && regBuff[i].loc==loc && regBuff[i].reg_type==type){
			memset(&regBuff[i],0,sizeof(RegisterBuffer));
			if (maxIndex==i && maxIndex>=0) maxIndex--;
		}
	}
}
char* getRegTypeText(REG_TYPE typ){
	switch(typ){
	case rtRCoil: return "DI";
	case rtRWCoil: return "DO";
	case rtRRegister: return "AI";
	case rtRWRegister: return "AO";
	default: return "??";
	}
}
void MbsTransShowBuffInfo(){

	dprint(" No. | type | id | loc. | value\r\n");
	dprint("-----+------+----+------+-------\r\n");
	for(int i=0;i<=maxIndex;i++){
	if (!regBuff[i].id) continue;
		dprint(" %03d |  %s  |%03d | %04X | %d\r\n",
				i,
				getRegTypeText((REG_TYPE)regBuff[i].reg_type),
				regBuff[i].id,
				regBuff[i].loc,
				regBuff[i].value);
	}
	dprint("-----+------+----+------+-------\r\n");
}

#define ToWORD(hi,lo) ((((uint16)hi)<<8) | lo)
#define DISCONNECT_COUNT 5

void MbsTransmitterRecordCmd(PUCHAR tx, int len, bool countAlive){
	uint8 reg_len;
	uint16 reg_loc;
	if(countAlive && cntWaitResponse[tx[0]]<DISCONNECT_COUNT)cntWaitResponse[tx[0]]++; /*計算命令次數*/
	switch(tx[1]){
	case 4: case 3: case 2: case 1:
		if(len>=5){
			MbsReadCmd.id=tx[0];
			MbsReadCmd.fun=tx[1];//==4?rtRRegister:rtRWRegister;
			MbsReadCmd.start=ToWORD(tx[2],tx[3]);
			MbsReadCmd.len=ToWORD(tx[4],tx[5]);
		}
		break;
	case 6:
		MbsTransClearBuff(tx[0],ToWORD(tx[2],tx[3]),rtRWRegister);
		break;
	case 16:
		reg_loc=ToWORD(tx[2],tx[3]);
		reg_len=ToWORD(tx[4],tx[5]);
		for(int i=reg_loc+reg_len-1;i>=reg_loc;i--) MbsTransClearBuff(tx[0],i,rtRWRegister);
		break;
	case 5:
		MbsTransClearBuff(tx[0],ToWORD(tx[2],tx[3]),rtRWCoil);
		break;
	case 15:
		reg_loc=ToWORD(tx[2],tx[3]);
		reg_len=ToWORD(tx[4],tx[5]);
		for(int i=reg_loc+reg_len-1;i>=reg_loc;i--) MbsTransClearBuff(tx[0],i,rtRWCoil);
		break;
	default:
		memset(&MbsReadCmd,0,sizeof(MbsReadCmd));
	}
}

int MbsTransmitterPrepareBuff(PUCHAR buff, int max_count){
	RegisterBuffer *reg;
	if ((3+MbsReadCmd.len*2)>max_count) return 0;
	if(cntWaitResponse[buff[0]]>=DISCONNECT_COUNT){
		dprint("id:%d is disconnected\r\n", buff[0]);
		return 0; /*已判斷為斷線，不取出暫存值*/
	}
	switch(MbsReadCmd.fun){
	case 4: case 3:
		for(int i=0;i<MbsReadCmd.len;i++){
			reg=MsbTransFindReg(MbsReadCmd.id,MbsReadCmd.start+i,(REG_TYPE)MbsReadCmd.fun);
			if (!reg) return 0;
			buff[3+i*2]=reg->value>>8;
			buff[4+i*2]=reg->value&0xff;
		}
		buff[0]=MbsReadCmd.id;
		buff[1]=MbsReadCmd.fun;
		buff[2]=MbsReadCmd.len*2;
		return 3+MbsReadCmd.len*2;
	case 2: case 1:
		buff[0]=MbsReadCmd.id;
		buff[1]=MbsReadCmd.fun;
		buff[2]=(MbsReadCmd.len+7)/8;
		for(int i=0;i<buff[2];i++) buff[3+i]=0;
		for(int i=0;i<MbsReadCmd.len;i++){
			reg=MsbTransFindReg(MbsReadCmd.id,MbsReadCmd.start+i,(REG_TYPE)MbsReadCmd.fun);
			dprint("Find Buff(0x%X) id:%d, fun:%d, loc:%d", reg,reg->id,reg->reg_type,reg->loc);
			if (!reg){
				dprint("\r\n");
				return 0;
			}else{
				dprint(", value:%d\r\n", reg->value);
			}
			buff[3+(i/8)]|=reg->value?0x1<<(i%8):0;
		}

		return 3+buff[2];
	default:
		return  0;
	}
}

void MbsTransmitterRecordRes(PUCHAR rx, int len, uint16 reg_loc, uint16 reg_len){
	//uint8 reg_len;
	//dprint("Record Response:%d\r\n",len);
	/*if (MbsReadCmd.id!=rx[0] || MbsReadCmd.fun!=rx[1]){
		dprint("Mismatch id %d>%d, fun:%d>%d\r\n",MbsReadCmd.id,rx[0],MbsReadCmd.fun,rx[1]);
		return;
	}else*/
	cntWaitResponse[rx[0]]=0; /*已收到回應，重置命令計數*/
	if(len<4){
		dprint("Mbs response Error: len is short:%d\r\n", len);
		return;
	}else if(rx[1]&0x80){
		dprint("Mbs Response Error code:%d\r\n", (((uint16)rx[2])<<8)|rx[3]);
		return;
	}

	switch(rx[1]){
	case 4: case 3:
		//reg_len=rx[2]/2;
		for(int i=0;i<reg_len/*MbsReadCmd.len*/;i++)
			MbsTransmitterSet(
					rx[0],
					reg_loc+i,//MbsReadCmd.start+i,
					rx[1]==4?rtRRegister:rtRWRegister,
					ToWORD(rx[3+i*2],rx[4+i*2]));
		break;
	case 1:case 2:
		for(int i=0;i<reg_len;i++)
			MbsTransmitterSet(
					rx[0],
					reg_loc+i,
					rx[1]==2?rtRCoil:rtRWCoil,
					(rx[3+(i/8)]&(1<<i))?1:0);
		break;
	}
}

void SetSourceBtId(uint8 mbsId, uint8 btId){
	sourceBtId[mbsId]=btId;
}

uint8 GetSourceBtId(uint8 mbsId){
	return sourceBtId[mbsId];
}
#else
void MbsTransmitterInit(){}
void MbsTransmitterRecordCmd(PUCHAR tx, int len, bool countAlive){}
void MbsTransmitterRecordRes(PUCHAR rx, int len, uint16 reg_loc, uint16 reg_len){}
int MbsTransmitterPrepareBuff(PUCHAR buff, int max_count){return 0;}
void SetSourceBtId(uint8 mbsId, uint8 btId){}
uint8 GetSourceBtId(uint8 mbsId){return 0;}
void MbsTransShowBuffInfo(){}
#endif
