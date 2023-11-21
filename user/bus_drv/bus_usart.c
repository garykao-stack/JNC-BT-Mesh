#define UART_DEBUG 1
#define STATE_CHANGE_DEBUG 1

#include "global.h"

#if UART_DEBUG
#undef DEBUG_FUNCTION
#define DEBUG_FUNCTION dprint
#else
#define DEBUG_FUNCTION(...)
#endif
#include "debugprint.h"

#include "device_bus.h"

#if STATE_CHANGE_DEBUG
#define STATE_CHANGE_PRINT DEBUG_FUNCTION
#else
#define STATE_CHANGE_PRINT(...)
#endif
#include "Mesh_Node.h"
#include "bus_rs485.h"
#include "bus_usart.h"
const uint32 UsartBaudrate[]=
{2400, 4800, 9600, 19200, 38400, 57600, 115200, 128000, 256000, 460800, 921600, 1382400,1843200,2764800};
uchar OrigignalRx[USART_RX_BUFF_SIZE+1];
uchar OrigignalTx[USART_TX_BUFF_SIZE+1];
uchar *TxBuff=(uchar*)OrigignalTx+1;//[USART_TX_BUFF_SIZE];
uchar *RxBuff=(uchar*)OrigignalRx+1;//[USART_RX_BUFF_SIZE];
PUCHAR  pTxBuff,pRxBuff;
uint16  UsartStatus;
uchar   UsartIntervalTimer;         //max interval to receive next byte (XXX ms)
uint16 volatile  CounterRx,CounterTx;

#define MAX_BAUD_RATE_NUM (sizeof(UsartBaudrate)/sizeof(UsartBaudrate[0]))

bool Modbus_IsReceived();

uint32 IndexToBaudrate(uint8 idx){
	if (idx>=MAX_BAUD_RATE_NUM) return 0;
	return UsartBaudrate[idx];
}

void UsartInit(void)
{
    
    USART_InitAsync_TypeDef usart_init = USART_INITASYNC_DEFAULT;
    CMU_ClockEnable(USART_CLOCK, true);

    //TraceDec1("Baudrate", pMeshNodeData->BaudRate);
    // set pin modes for USART TX and RX pins
    GPIO_PinModeSet(USART_PORT, USART_PIN_TX, gpioModePushPull, 1); //TX
    GPIO_PinModeSet(USART_PORT, USART_PIN_RX, gpioModeInputPull, 1);    //RX

//#ifdef BTM_TRANSMITTER /*Baudrate依使用者設定來決定是否採用自訂Baudrate(不需要由定義決定)*/
    //usart_init.baudrate = UsartBaudrate[pMeshNodeData->BaudRate];
//#else

#ifdef BTM_A308
    usart_init.baudrate = UsartBaudrate[pMeshNodeData->BaudRate];   //setup baudrate
#else
    if(NodeRole == NR_CLIENT || pMeshNodeData->SensorClass==SENSOR_CUSTOM_SERIAL || pMeshNodeData->SensorClass==SENSOR_A308M )
        usart_init.baudrate = UsartBaudrate[pMeshNodeData->BaudRate];   //setup baudrate
    else
        usart_init.baudrate = UsartBaudrate[USART_BAUDRATE_DEFAULT];   //setup baudrate
#endif
//#endif
    USART_InitAsync(USART, &usart_init);   // Initialize USART asynchronous mode and route pins
    
    USART->ROUTELOC0 = USART_LOCATION;
    USART->ROUTEPEN  = USART_PIN_ENABLE; //USART2 change to USART1, then debug tool no error, ?????
        
    //Enabling USART Interrupts
    UsartIrq(USART_ID_RX,ON);
    UsartIrq(USART_ID_TX,ON);
    UsartOpen();
    dprint("RS485 Initialize. baudrate:%d\r\n",usart_init.baudrate);
    return;          
}

void UsartBaudrateChange(){
/*
	USART_InitAsync_TypeDef usart_init = USART_INITASYNC_DEFAULT;
#ifdef BTM_TRANSMITTER //當Baudrate變更時立即採用(目前沒有作用)
    usart_init.baudrate = UsartBaudrate[pMeshNodeData->BaudRate];
#else
    if(NodeRole == NR_CLIENT)
        usart_init.baudrate = UsartBaudrate[pMeshNodeData->BaudRate];   //setup baudrate
    else
        usart_init.baudrate = UsartBaudrate[USART_BAUDRATE_DEFAULT];   //setup baudrate
#endif
    USART_InitAsync(USART, &usart_init);
    UsartResetRxTx(USART_ID_TX_RX);
    UsartSetStatus(USART_OPEN,ON);
    //Initializae USART Interrupts
    Rs485Rx();*/
}

//
// for power saving
void UsartDeInit(void)
{
    UsartIrq(USART_ID_RX,OFF);
    UsartIrq(USART_ID_TX,OFF);

}


//
// status: 0: OFF,  1:ON
//
//
void UsartIrq(uchar dir ,uchar status)
{
    if(USART_ID_TX == dir)
        {// TX
            if(status == ON)  {UsartResetRxTx(USART_ID_TX);NVIC_EnableIRQ(USART_TX_IRQ); }
            else NVIC_DisableIRQ(USART_TX_IRQ);
        }
    else
        {// RX
            if(status == ON)  {UsartResetRxTx(USART_ID_RX);NVIC_EnableIRQ(USART_RX_IRQ); }
            else NVIC_DisableIRQ(USART_RX_IRQ);
        }

    
}


void UsartOnOff(uchar status)
{
    if(status == ON)
        {
         NVIC_EnableIRQ(USART_RX_IRQ);
         NVIC_EnableIRQ(USART_TX_IRQ);
        }
    else
        {
        NVIC_DisableIRQ(USART_RX_IRQ);
        NVIC_DisableIRQ(USART_TX_IRQ);
        }
}



uint16 TCounterRx,TCounterTx;   // for USART timer counter
uint16 UsartCounterRx=0;
uint16 UsartCounterTx=0;
//UES ==> USART EVENT Stage
#define UES_STAGE_INIT          0 
#define UES_STANDBY_MODE        1
#define UES_CHECK_RX_END        2
#define UES_CHECH_CMD           3
#define UES_CHECH_CMD_OK        4
#define UES_CHECH_CMD_END       5
#define UES_CHECK_TX_END        6
#define UES_WAIT_RX_EVENT_END	7


//
//
//
#define UART_NEW_PROC 1
#if UART_NEW_PROC
void UsartClientProc()
{
	if(!GetNodeStatus(NS_USART_RX_EVENT)){
		if(Modbus_IsReceived()){
			/*dprint("Uart Rec:");
			for(int i=0;i<CounterRx;i++) dprint(" %02X",RxBuff[i]);
			dprint("\r\n");*/
			if(CheckUsartRxCmd()){
				UsartSetStage(USART_STAGE_RX_END);
				SetNodeStatus(NS_USART_RX_EVENT,ON);
			}else{
				//printf("RX Check Error\r\n");// cmd error
				//UsartSetStatus(USART_RX_CRC_ERROR,ON);
				UsartSetStage(USART_STAGE_RX_END);
				UsartSetStage(USART_STAGE_RX_CLEAN);
				UsartResetRxTx(USART_ID_RX);
			}

		}
	}

	/*if(UsartGetStatus(USART_TX_ING|USART_TX_END)==(USART_TX_ING|USART_TX_END)){

	}*/

}
#else
void UsartClientProc()
{
    pStageInfo = GetNodeStageInfo(USART_MONITOR_CLIENT_PROC);
    
    switch(ActiveStage())
        {
        case NODE_STAGE_INIT: Trace("UES_STAGE_INIT");
            ToNextStage(UES_STANDBY_MODE);   //default
            break;
        case UES_STANDBY_MODE: //UartTrace("UES_STANDBY_MODE");
            if(UsartGetStatusRxIng() == TRUE)
                {
                 UsartSetStage(USART_STAGE_RX_ING);
                 ToWaitingStage(UES_CHECK_RX_END,WAIT_MS(30)); 
                }
            else if(UsartGetStatusTxIng()) 
                {//UartTrace("---- To Check Tx Ending ------");// Tx ending
                 ToWaitingStage(UES_CHECK_TX_END,WAIT_MS(60));            
                }
            break;
        case UES_CHECK_TX_END: //Trace("UES_CHECK_TX_END");
           //if(UsartGetStatusTxEnd() && (CheckWaitTimeOut() == TRUE) )
           if(UsartGetStatusTxEnd())
            {//Trace1("Tx End 1",UsartStatus);
            UsartSetStage(USART_STAGE_TX_END);ToNextStage(UES_STANDBY_MODE);  
            }
           else if(CheckWaitTimeOut() == TRUE)
            {//Trace1("Tx End 2",UsartStatus);
             if(CounterTx == 0) 
              {Delay_ms(5);
              UsartSetStage(USART_STAGE_TX_END);ToNextStage(UES_STANDBY_MODE);}
             else {ToNextStage(UES_STANDBY_MODE); }
            }
            break;
        case UES_CHECK_RX_END: //TraceDec1("UES_CHECK_RX_END 1",CounterRx);
            /*if(CheckWaitTimeOut() == TRUE)
        		ToNextStage(UES_CHECH_CMD);
			else if((NodeRole == NR_CLIENT) && (CounterRx >= MODBUS_CMD_NUM))
				ToNextStage(UES_CHECH_CMD);*/
        	if(Modbus_IsReceived()){
        		dprint("Uart Rec:");
        		for(int i=0;i<CounterRx;i++) dprint(" %02X",RxBuff[i]);
        		dprint("\r\n");
        		if(CheckUsartRxCmd()){
        			UsartSetStage(USART_STAGE_RX_END);
        		}else{
        			Trace("RX Check Error");// cmd error
        			UsartSetStage(USART_STAGE_RX_CLEAN);
        		}
        		SetNodeStatus(NS_USART_RX_EVENT,ON);
        		ToNextStage(UES_WAIT_RX_EVENT_END);
        	}
        	break;
        case UES_WAIT_RX_EVENT_END:
        	if(!GetNodeStatus(NS_USART_RX_EVENT))
        		ToNextStage(UES_STANDBY_MODE);
        	break;
        /* 簡化到步驟UES_CHECK_RX_END中
        case UES_CHECH_CMD: //Trace("UES_CHECH_CMD");
            //SetLed(LED_GREEN,OFF);
            if(CheckUsartRxCmd()){//Trace("RX Check Ok"); PrintDataByte("RX Data 1",RxBuff , 8);
                ToNextStage(UES_CHECH_CMD_OK); 
            }else{
            	Trace("RX Check Error");// cmd error
                UsartSetStage(USART_STAGE_RX_CLEAN);                   
                ToNextStage(UES_CHECH_CMD_END);
            }
            break;
        case UES_CHECH_CMD_OK: //TraceDec1("UES_CHECH_CMD_OK",CounterRx);
            UsartSetStage(USART_STAGE_RX_END);            
            ToNextStage(UES_CHECH_CMD_END);   
            break;
        case UES_CHECH_CMD_END: //Trace("UES_CHECH_CMD_END");
            SetNodeStatus(NS_USART_RX_EVENT,ON); 
            ToNextStage(UES_STANDBY_MODE);   
            break;*/
            
        default: TraceErr1("UsartClientProc",ActiveStage()); break;
        };
}
#endif






//
// Check Modbus command
//
 bool CheckUsartRxCmd()
{
    bool ret_code=TRUE;
    //uint16 crc_value;
    PUCHAR p_rx_buff;

    p_rx_buff = UsartGetBuff(USART_ID_RX);
    /*if(CounterRx < UsartRxCount) {
    	PrintDataByte("RX: Host Len Error 1",UsartGetBuff(USART_ID_RX),CounterRx);TraceErr1("RX: Host CMD",CounterRx);
        ret_code = FALSE;
    }else{ */
    	//PrintDataByte("Host Cmd oK",UsartGetBuff(USART_ID_RX),CounterRx);
        if(CheckModbusCrc(p_rx_buff,CounterRx) == FALSE){
        	PrintDataByte("xxx Host CRC Error 2 xxx",UsartGetBuff(USART_ID_RX),CounterRx);
            ret_code = FALSE;
        }
    //}
    return ret_code;
}

int MbsRxTimeout=0;
int MbsTxTimeout=0;
#define MBS_REC_TIME_OUT_MS 40
#define MBS_TIMER_MS 10
#define MBS_REC_TIMEOUT_COUNT (MBS_REC_TIME_OUT_MS/MBS_TIMER_MS)


void Modbus_Timer()
{
	if (CounterRx && MbsRxTimeout<MBS_REC_TIMEOUT_COUNT) MbsRxTimeout++;
	if (UsartTxIsBusy() && MbsTxTimeout){
		MbsTxTimeout--;
		if(!MbsTxTimeout){
			CounterTx=0;
			UsartSetStatus(USART_TX_END,ON);
			RS485ToRx();
		}
	}
}
bool Modbus_IsReceived()
{
	return CounterRx && (MbsRxTimeout>=MBS_REC_TIMEOUT_COUNT);
}


/**************************************************************************
 * @brief USART RX interrupt service routine
 *****************************************************************************/
void USART2_RX_IRQHandler(void)
{
  uint32_t flags;
  flags = USART_IntGet(USART);
  USART_IntClear(USART, flags);
  if(Modbus_IsReceived())UsartResetRxTx(USART_ID_RX);
   if((CounterRx < USART_RX_BUFF_SIZE)){
	   MbsRxTimeout=0;
        *pRxBuff++ = USART_RxDataGet(USART);//USART_Rx(USART);
        CounterRx++; UsartSetStatus(USART_RX_ING,ON);
        SetLedStatus(LED_UART_RX_ON);
    } else {
    	USART_Rx(USART);   // clean RX buffer Rx
    	UsartResetRxTx(USART_ID_RX);
    }
}

//*************************************************************************
//* @brief USART TX interrupt service routine
//*************************************************************************
void USART2_TX_IRQHandler(void)
{
  uint32_t flags;
  flags = USART_IntGet(USART);
  USART_IntClear(USART, flags);
  if((flags & USART_IF_TXC) && (CounterTx > 0))
  //if((flags & USART_IF_TXC) && (CounterTx > 0) && (CounterTx <USART_TX_BUFF_SIZE))
    {
     USART_Tx(USART, *pTxBuff++); // Transmit byte
     CounterTx--;
    }
  else
    {// Tx ending
     UsartSetStatus(USART_TX_END,ON); 
     RS485ToRx();
    }
}

//
//
//
void UsartSetStage(uchar stage)
{

    switch(stage)
        {
            case USART_STAGE_RX_ING: //Trace("USART_STAGE_RX_ING");
            	MbsRxTimeout=0;
                UsartSetStatus(USART_RX_ING,ON);
                UsartSetStatus(USART_RX_END,OFF);
                UsartSetStatus(USART_RX_WAITING,ON);
                Rs485Rx();
                break;
            case USART_STAGE_RX_END: //Trace("USART_STAGE_RX_END");
                UsartSetStatus(USART_RX_END,ON);
                UsartSetStatus(USART_RX_ING,OFF);
                UsartSetStatus(USART_TX_ING,OFF);
                UsartSetStatus(USART_RX_WAITING,OFF);
                USART_IntClear(USART, USART_IFS_RXUF);  
                break;
            case USART_STAGE_RX_CLEAN:
                UsartResetRxTx(USART_ID_RX);
                break;
            case USART_STAGE_TX_ING: //Trace("USART_STAGE_TX_ING");                            
                pTxBuff = TxBuff;   ///initial Tx buffer;                
                UsartSetStatus(USART_TX_ING,ON);
                UsartSetStatus(USART_STAGE_TX_END,OFF);
                USART_IntSet(USART, USART_IFS_TXC); // start USART to transfer data: Tx active
                Rs485Tx();  // switch RS485 to Transfer status
                break;
            case USART_STAGE_TX_END: //Trace("USART_STAGE_TX_END");
                TCounterTx = TIMER_ENDING;
                CounterTx=0;
                pTxBuff = TxBuff;   ///initial Tx buffer;  for BUg                
                Rs485Rx(); // switch RS485 to receive status
                USART_IntClear(USART, USART_IFS_RXUF);
                UsartSetStatus(USART_TX_ING,OFF);
                UsartSetStatus(USART_TX_END,OFF);
                break;
            case USART_STAGE_TX_CLEAN: //Trace("USART_STAGE_TX_CLEAN");
                 UsartResetRxTx(USART_ID_TX);
                break;
            
            default: TraceErr1("UsartSetStage FAIL",stage);              
            
        };
    
}

//
// Initial Rx/Tx information
//
void UsartResetRxTx(uchar tx_rx)
{
   // TraceDec1("tx_rx", tx_rx);
    if(tx_rx == USART_ID_TX)
        {
            pTxBuff = TxBuff; CounterTx = 0; TCounterTx = 0;
            UsartSetStatus(USART_TX_ING|USART_TX_END,OFF);     
            memset(pTxBuff,0,USART_TX_BUFF_SIZE);   //reset buffer
        }
    else if(tx_rx == USART_ID_RX)
        {
            pRxBuff = RxBuff; CounterRx = 0; TCounterRx = 0;MbsRxTimeout=0;
            UsartSetStatus(USART_RX_ING|USART_RX_END|USART_RX_CRC_ERROR,OFF);
            UsartSetStatus(USART_RX_WAITING,ON);
            memset(pRxBuff,0,USART_RX_BUFF_SIZE);
            SetNodeStatus(NS_USART_RX_EVENT,OFF);
            SetLedStatus(LED_UART_RX_OFF);

        }
    else if(tx_rx == USART_ID_TX_RX)
        {
            pTxBuff = TxBuff; CounterTx = 0; pRxBuff = RxBuff; CounterRx = 0;MbsRxTimeout=0;
            TCounterTx=TCounterRx=0;
            UsartStatus = USART_FREE;
            SetNodeStatus(NS_USART_RX_EVENT,OFF);
            UsartSetStatus(USART_RX_WAITING,ON);
            UsartSetStatus(USART_RX_CRC_ERROR,OFF);
            memset(pTxBuff,0,USART_TX_BUFF_SIZE);   //reset buffer
            memset(pRxBuff,0,USART_RX_BUFF_SIZE);
            SetLedStatus(LED_UART_RX_OFF);
        }
    else TraceErr1("UsartResetRxTx",tx_rx);
}

//
//
void UsartSetProcessTask(uchar tx_rx,uchar on_off)
{
    return;
    
    if(on_off == ON)
        {
          if(tx_rx == USART_ID_TX) 
            {// USART Task active
             TCounterTx = 1; SetEventTaskTimer(TD_TASK_USART_TX, TIMER_USART_TX_CYCLE, TIMER_EVENT_ONCE);
            }
            else 
            {
             TCounterRx = 1; SetEventTaskTimer(TD_TASK_USART_RX, TIMER_USART_RX_CYCLE, TIMER_EVENT_REPEAT);
            }
        }
    else
        {// Task OFF
          if(tx_rx == USART_ID_TX) 
            {// USART Task active
             TCounterTx = 0; SetEventTaskTimer(TD_TASK_USART_TX, TIMER_ENDING, TIMER_EVENT_ONCE);
            }
            else 
            {
             TCounterRx = 0; SetEventTaskTimer(TD_TASK_USART_RX, TIMER_ENDING, TIMER_EVENT_REPEAT);
            }
        }
}



//
//
bool UsartOpen()
{
    bool ret_code=FALSE;
    
    UsartResetRxTx(USART_ID_TX_RX);
    UsartSetStatus(USART_OPEN,ON);
    //Initializae USART Interrupts
    Rs485Rx();
    USART_IntEnable(USART, USART_IEN_RXDATAV);
    USART_IntEnable(USART, USART_IEN_TXC);
    return ret_code;
}

//
//
void UsartClose()
{
    USART_IntDisable(USART, USART_IEN_RXDATAV);
    USART_IntDisable(USART, USART_IEN_TXC);
}


bool UsartSendCmd(uchar size)
{
    CounterTx = size;    
    //UsartSetStage(USART_STAGE_TX_END);
    UsartResetRxTx(USART_ID_RX);
    UsartSetStage(USART_STAGE_TX_ING);
    MbsTxTimeout=size;
    return TRUE;
}

bool UsartTxSendCmd(PUCHAR pBuff,uint16 size)
{
    bool ret_code=FALSE;
    PUCHAR p_tx_buff = UsartGetBuff(USART_ID_TX);
    if (size>USART_TX_BUFF_SIZE) return FALSE; //check array length

    if(p_tx_buff!=pBuff)memcpy(p_tx_buff,pBuff,size);
   
   //if(UsartGetStatusTxIng() == OFF || CounterTx == 0)
    //ret_code = UsartSendCmd(size);


    if(UsartTxIsBusy()||CounterTx){
    	dprint("USART Tx Error 1 CounterTx:%d\r\n",CounterTx);
    	UsartSetStage(USART_STAGE_TX_END);
    }

    ret_code = UsartSendCmd(size);

    /*if(!UsartTxIsBusy() || CounterTx == 0){
        ret_code = UsartSendCmd(size);
    }else {
	   dprint("USART Tx Error 1 CounterTx:%d\r\n",CounterTx);
       UsartSetStage(USART_STAGE_TX_END);
    }*/
    
    return ret_code;
}

void UsartShowDataRx()
{
    if(CounterRx > 0){
    	TraceDec1("UsartShowDataRx CounterRx", CounterRx);
    	PrintDataByte("Usart Rx Data", RxBuff, (uint)CounterRx);
	}
}


//
// setup COM port
bool UsartSetBaudRate(uchar baud_rate_num)
{
    bool ret_code=FALSE;
    //USART_BaudrateAsyncSet()
    return ret_code;
}
// return buffer pointer
PUCHAR UsartGetBuff(uchar tx_rx)
{
    if(tx_rx == USART_ID_TX) return TxBuff;
    else return RxBuff;
}


// return Rx buffer counter
uchar UsartGetRxCounter()
{
    return CounterRx;
}

void UsartStatusReset()
{
    UsartStatus = 0;
}

//
// set up Tx, Rx status
void UsartSetStatus(uchar status,uchar on_off)
{
    if(on_off == ON) UsartStatus |= status; //status ON
    else {UsartStatus &= ~status;}  //status OFF
}

//
// return current status TRUE/FALSE
bool UsartGetStatus(uchar status)
{
    if(UsartStatus & status) return TRUE;
    else return FALSE;
}
//
// return Rx/Tx status
uint16 UsartGetTxRxStatus(uchar status)
{
    return UsartStatus;
}

//
// return current status TRUE/FALSE
bool UsartGetStatusRxWaiting()
{
    if(UsartStatus & USART_RX_WAITING) return TRUE;
    else return FALSE;
}


//
// return current status TRUE/FALSE
bool UsartGetStatusRxEnd()
{
    if(UsartStatus & USART_RX_END) return TRUE;
    else return FALSE;
}

//
// return current status TRUE/FALSE
bool UsartGetStatusRxIng()
{
    if(UsartStatus & USART_RX_ING) return TRUE;
    else return FALSE;
}


//
// return current status TRUE/FALSE
bool UsartGetStatusTxEnd()
{
    if(UsartStatus & USART_TX_END) return TRUE;
    else return FALSE;
}


//
// return current status TRUE/FALSE
bool UsartGetStatusTxIng()
{
    if(UsartStatus & USART_TX_ING) return TRUE;
    else return FALSE;
}

bool UsartTxIsBusy()
{
	return (UsartStatus&USART_TX_ING) && ((~UsartStatus)&USART_TX_END);
}

bool UsartRxIsBusy(){
	return (UsartStatus & USART_RX_ING) && CounterRx && !GetNodeStatus(NS_USART_RX_EVENT);
}

void UsartPrintBuff(uchar rx_tx)
{
    if(rx_tx == USART_ID_RX )
        PrintDataByte("RX Buffer", RxBuff, CounterRx);
    else if(rx_tx == USART_ID_TX )
        PrintDataByte("TX Buffer", RxBuff, CounterTx);
}

//
//
//
bool CheckModbusCrc(PUCHAR pbuff, uchar len)
{
    bool ret_code = TRUE;
    uint16 modbus_crc1,modbus_crc2;
    if(len <=2) return FALSE;
    modbus_crc1 = *((PUINT16)&pbuff[len-2]);
    modbus_crc2 = ModbusRtu_CRC16(pbuff, len-2);
    //Trace16_2(modbus_crc1, modbus_crc2);

    if(modbus_crc1 != modbus_crc2) 
      {PrintDataByte("xxx Modbus CRC Error xxx", pbuff, len);
       
        ret_code = FALSE;
      }
    return ret_code;
}


const uchar ModbusCrcHi[]=
{
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40
};

const uchar ModbusCrcLo[] =
{
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
0x40
};

uint16   ModbusRtu_CRC16(uchar *updata,uint16 len)
{
  uchar uchCRCHi=0xff;
  uchar uchCRCLo=0xff;
  uint16  uindex;
  while(len--)
  {
  uindex=uchCRCHi^*updata++;
  uchCRCHi=uchCRCLo^ModbusCrcHi[uindex];
  uchCRCLo=ModbusCrcLo[uindex];
  }
  //return (uchCRCHi<<8|uchCRCLo);  //MSB
  return (uchCRCLo<<8|uchCRCHi);    //LSB
}



uint16 MbsSend(uint8 *data,uint16 len){
	uint16 crc;
	PUCHAR tx = UsartGetBuff(USART_ID_TX);
	if (tx!=data) memcpy(tx,data,len);
	crc=ModbusRtu_CRC16(data, len);
	tx[len]=crc&0xff;
	tx[len+1]=crc>>8;
	return UsartTxSendCmd(tx,len+2);
}

uint16 MbsResponseError(uint8 id, uint8 fun, uint8 err){
	PUCHAR tx = UsartGetBuff(USART_ID_TX);
	tx[0]=id;
	tx[1]=0x80|fun;
	tx[2]=err; /* function error */
	return MbsSend(tx,3);
}

void MbsSetReadRegCmd(uint8 id, uint8 func, uint16 loc, uint16 len){
	uint16 crc;
	PUCHAR data = UsartGetBuff(USART_ID_TX);
	data[0]=id;
	data[1]=func;
	data[2]=loc>>8;
	data[3]=loc&0xff;
	data[4]=len>>8;
	data[5]=len&0xff;
	crc=ModbusRtu_CRC16(data, 6);
	data[6]=crc&0xff;
	data[7]=crc>>8;
	UsartTxSendCmd(data,8);
}

