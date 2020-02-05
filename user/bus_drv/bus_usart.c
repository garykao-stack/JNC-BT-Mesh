#include "global.h"
#include "device_bus.h"
#include "bus_rs485.h"
#include "bus_usart.h"
const uint32 UsartBaudrate[MAX_BAUD_RATE_NUM]=
{2400, 4800, 9600, 19200, 38400, 57600, 115200, 128000, 256000, 460800, 921600, 1382400,1843200,2764800};
uchar TxBuff[USART_TX_BUFF_SIZE];
uchar RxBuff[USART_RX_BUFF_SIZE];
//volatile uchar RxBuff[USART_RX_BUFF_SIZE];
//volatile uchar TxBuff[USART_TX_BUFF_SIZE]={0x41,0x42,0x41,0x42,0x41,0x42,0x41,0x42,0x41,0x42};

PUCHAR  pTxBuff,pRxBuff;//volatile uchar *pRxBuff;
uint16  UsartStatus;
uchar   UsartIntervalTimer;         //max interval to receive next byte (XXX ms)
uchar   CounterRx,CounterTx;

void UsartInit(void)
{TraceProc();
#ifdef VCOM_USART2
    return;
#endif
    
    USART_InitAsync_TypeDef usart_init = USART_INITASYNC_DEFAULT;
    CMU_ClockEnable(USART_CLOCK, true);
    
    // set pin modes for USART TX and RX pins
    GPIO_PinModeSet(USART_PORT, USART_PIN_TX, gpioModePushPull, 1); //TX
    GPIO_PinModeSet(USART_PORT, USART_PIN_RX, gpioModeInput, 0);    //RX  
    usart_init.baudrate = UsartBaudrate[USART_BAUDRATE_DEFAULT];   //setup baudrate
    USART_InitAsync(USART, &usart_init);   // Initialize USART asynchronous mode and route pins
    
    USART->ROUTELOC0 = USART_LOCATION;
    USART->ROUTEPEN  = USART_PIN_ENABLE; //USART2 change to USART1, then debug tool no error, ?????
        
    //Enabling USART Interrupts
    NVIC_EnableIRQ(USART_RX_IRQ);
    NVIC_EnableIRQ(USART_TX_IRQ);
    UsartOpen();
    return;          
}

//
// for power saving
void UsartDeInit(void)
{
}



uchar TCounterRx,TCounterTx;   // for USART timer counter
uchar UsartCounterRx=0;

//
//
//
void UsartMonitor()
{
    // for USART RX
  if(GetTimerTaskEvent(TIMER_EVENT_USART_RX) && UsartGetStatus(USART_RX_ING))
    {
        if(UsartCounterRx != 0 && --UsartCounterRx == 0)
          { 
            SetEventTaskTimer(TIMER_ID_USART_RX, TIMER_ENDING, TIMER_EVENT_REPEAT); // timer disable
            UsartSetStage(USART_STAGE_RX_END);
            TraceDec1("Usar CounterRx ",CounterRx); PrintDataByte("Usart Rx Ending ", RxBuff,CounterRx);
          }
    }
  else // for USART TX
    if(UsartGetStatus(USART_TX_END))
      { //Trace(" Usart Tx Ending");
          UsartSetStage(USART_STAGE_TX_CLEAN);
      }

  
}



/**************************************************************************
 * @brief USART RX interrupt service routine
 *****************************************************************************/
void USART2_RX_IRQHandler(void)
{
  uint32_t flags;
  flags = USART_IntGet(USART);
  USART_IntClear(USART, flags);
  if(CounterRx < USART_RX_BUFF_SIZE && UsartGetStatus(USART_RX_WAITING))
    {
       // if(CounterRx == 8) Trace16_1(CounterRx);
        
        if(CounterRx == 0) SetEventTaskTimer(TIMER_ID_USART_RX, TIMER_10MS, TIMER_EVENT_REPEAT); // timer enable
        CounterRx++; *pRxBuff++ = USART_Rx(USART);        
        UsartSetStatus(USART_RX_ING,ON);
        UsartCounterRx = 2; // for 20ms timeout
    } else 
    {USART_Rx(USART);   // clean RX buffer Rx
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

  if(CounterTx)
    {
     USART_Tx(USART, *pTxBuff++); // Transmit byte
     CounterTx--; 
    }
  else
    {// Tx ending
     UsartSetStage(USART_STAGE_TX_END); 
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
                UsartSetStatus(USART_RX_ING,ON);
                UsartSetStatus(USART_RX_END,OFF);
                UsartSetStatus(USART_RX_WAITING,ON);
                Rs485Rx();
               // UsartSetProcessTask(USART_ID_RX,ON);
                break;
            case USART_STAGE_RX_END: Trace("USART_STAGE_RX_END");
                UsartSetStatus(USART_RX_END,ON);
                UsartSetStatus(USART_RX_ING,OFF);
                UsartSetStatus(USART_RX_WAITING,OFF);
                //UsartSetProcessTask(USART_ID_RX,OFF);
                break;
            case USART_STAGE_RX_CLEAN: 
                UsartResetRxTx(USART_ID_RX);
                break;
            case USART_STAGE_TX_ING: Trace("USART_STAGE_TX_ING");                            
                pTxBuff = TxBuff;   ///initial Tx buffer;                
                TCounterTx = TIMER_TIMEOUT_TX;                                             
                Rs485Tx();  // switch RS485 to Transfer status
                UsartSetStage(USART_STAGE_RX_CLEAN);
                UsartSetStatus(USART_TX_ING,ON);
                UsartSetStatus(USART_STAGE_TX_END,OFF);
                USART_IntSet(USART, USART_IFS_TXC); // start USART to transfer data: Tx active
                //UsartSetProcessTask(USART_ID_TX,ON);
                break;
            case USART_STAGE_TX_END: Trace("USART_STAGE_TX_END");
                TCounterTx = TIMER_ENDING;
                Rs485Rx(); // switch RS485 to receive status
                UsartSetStatus(USART_TX_ING,OFF);
                UsartSetStatus(USART_TX_END,ON);
                //UsartSetProcessTask(USART_ID_TX,ON);
                break;
            case USART_STAGE_TX_CLEAN: Trace("USART_STAGE_TX_CLEAN");
                 Rs485Rx(); 
                 UsartResetRxTx(USART_ID_TX);
                 //UsartSetProcessTask(USART_ID_TX,OFF);
                break;
            
            default: TraceErr("UsartSetStage FAIL");              
            
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
            pRxBuff = RxBuff; CounterRx = 0; TCounterRx = 0;
            UsartSetStatus(USART_RX_ING|USART_RX_END,OFF);
            UsartSetStatus(USART_RX_WAITING,ON);
            memset(pRxBuff,0,USART_RX_BUFF_SIZE);
        }
    else if(tx_rx == USART_ID_TX_RX)
        {
            pTxBuff = TxBuff; CounterTx = 0; pRxBuff = RxBuff; CounterRx = 0;
            TCounterTx=TCounterRx=0;
            UsartStatus = USART_FREE;
            UsartSetStatus(USART_RX_WAITING,ON);
            memset(pTxBuff,0,USART_TX_BUFF_SIZE);   //reset buffer
            memset(pRxBuff,0,USART_RX_BUFF_SIZE);
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
             TCounterTx = 1; SetEventTaskTimer(TIMER_ID_TASK_USART_TX, TIMER_USART_TX_CYCLE, TIMER_EVENT_ONCE);
            }
          else 
            {
             TCounterRx = 1; SetEventTaskTimer(TIMER_ID_TASK_USART_RX, TIMER_USART_RX_CYCLE, TIMER_EVENT_REPEAT);
            }
        }
    else
        {// Task OFF
          if(tx_rx == USART_ID_TX) 
            {// USART Task active
             TCounterTx = 0; SetEventTaskTimer(TIMER_ID_TASK_USART_TX, TIMER_ENDING, TIMER_EVENT_ONCE);
            }
        else 
            {
             TCounterRx = 0; SetEventTaskTimer(TIMER_ID_TASK_USART_RX, TIMER_ENDING, TIMER_EVENT_REPEAT);
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
    USART_IntEnable(USART, USART_IEN_RXDATAV);
    USART_IntEnable(USART, USART_IEN_TXC);
    //Rs485Rx();
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
    UsartResetRxTx(USART_ID_RX);
    UsartSetStage(USART_STAGE_TX_ING);
    return TRUE;
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


//
// set up Tx, Rx status
void UsartSetStatus(uchar status,uchar on_off)
{
    if(on_off == ON) UsartStatus |= status; //status ON
    else {UsartStatus &= ~status;}  //status OFF
}

//
// return Rx/Tx status
uint16 UsartGetTxRxStatus(uchar status)
{
    return UsartStatus;
}

//
// return current status TRUE/FALSE
bool UsartGetStatus(uchar status)
{
    if(UsartStatus & status) return TRUE;
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



