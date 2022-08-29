/*
 * bus_usart.h
 *
 *  Created on: 2019/11/07
 *      Author: Richard
 */

#ifndef _BUS_USART_
#define _BUS_USART_

#define USART_ID_TX                 0
#define USART_ID_RX                 1
#define USART_ID_TX_RX              2


#define BAUDRATE_2400               0
#define BAUDRATE_4800               1
#define BAUDRATE_9600               2
#define BAUDRATE_19200              3   
#define BAUDRATE_38400              4
#define BAUDRATE_57600              5
#define BAUDRATE_115200             6
#define BAUDRATE_128000             7
#define BAUDRATE_256000             8
#define BAUDRATE_460800             9
#define BAUDRATE_921600             10
#define BAUDRATE_1382400            11
#define BAUDRATE_1843200            12
#define BAUDRATE_2764800            13

#define BAUDRATE_CLOSE              0xFF

//#define MAX_BAUD_RATE_NUM           14
#define USART_TX_BUFF_SIZE          (255+5+2)//50
#define USART_RX_BUFF_SIZE          (255+5+2)//50 //Bug



#define USART_BAUDRATE_DEFAULT      BAUDRATE_9600 //BAUDRATE_115200 //BAUDRATE_256000 //BAUDRATE_9600
#define DEBUG_BAUDRATE_DEFAULT      BAUDRATE_115200


#define USART_VCOM                  USART0
#define USART_CMD                   USART2

#define USART_VCOM_CLOCK            cmuClock_USART0
#define USART_CMD_CLOCK             cmuClock_USART2


#define USART_VCOM_LOCATION         (USART_ROUTELOC0_RXLOC_LOC0 | USART_ROUTELOC0_TXLOC_LOC0)                              
#define USART_VCOM_PIN_ENABLE       (USART_ROUTEPEN_TXPEN | USART_ROUTEPEN_RXPEN )
#define USART_CMD_LOCATION          (USART_ROUTELOC0_TXLOC_LOC17|USART_ROUTELOC0_RXLOC_LOC17)
#define USART_CMD_PIN_ENABLE        (USART_ROUTEPEN_TXPEN | USART_ROUTEPEN_RXPEN )

#define USART_PORT                  gpioPortF
#define USART_PIN_TX                (4) // PF4
#define USART_PIN_RX                (5) // PF5


#define USART                       USART_CMD
#define USART_CLOCK                 USART_CMD_CLOCK
#define USART_LOCATION              USART_CMD_LOCATION
#define USART_PIN_ENABLE            USART_CMD_PIN_ENABLE


#define USART_RX_IRQ                USART2_RX_IRQn
#define USART_TX_IRQ                USART2_TX_IRQn


//#define USART_COM_NUM               0
//#define DEBUG_COM_NUM               1

#define SOFT_TIMER_VALUE_USART_TX            1   // // reset TX timer 10ms
#define SOFT_TIMER_VALUE_USART_RX            1   // // reset RX timer 10ms


#define USART_INTERVAL_115200       2      //20ms
#define USART_RECEIVE_INTERVAL      3      //30ms

#define MODBUS_CMD_NUM              8       // Rs-485 command bytes
#define MODBUS_RET_NUM              USART_TX_BUFF_SIZE

#define USART_OPEN                  BIT0
#define USART_TX_ING                BIT1    // to USART
#define USART_TX_END                BIT2    // to USART
#define USART_TX_WAITING            BIT3    // to USART

#define USART_RX_WAITING            BIT4
#define USART_RX_ING                BIT5
#define USART_RX_END                BIT6
#define USART_RX_CRC_ERROR			BIT7

#define USART_FREE                  0x00




#define USART_STAGE_RX_FREE     0x01
#define USART_STAGE_RX_ING      0x02
#define USART_STAGE_RX_END      0x03
#define USART_STAGE_RX_CLEAN    0x04

#define USART_STAGE_TX_FREE     0x10
#define USART_STAGE_TX_ING      0x20
#define USART_STAGE_TX_END      0x30
#define USART_STAGE_TX_CLEAN    0x40

#define TIMER_TIMEOUT_RX        1       // 10ms
#define TIMER_TIMEOUT_TX        1       // 10ms
#define TIMER_USART_RX_CYCLE    TIMER_5MS
#define TIMER_USART_TX_CYCLE    TIMER_5MS


#define TIMER_USART_RX_ENDING   TIMER_5MS //TIMER_10MS 
#define SERVER_RX_NUM           29

#define TIME_OUT_COUNT_RX        WAIT_MS(20)    //20ms


extern uint16 volatile CounterRx,CounterTx;
extern uchar *RxBuff;//[];
extern uint16 UsartCounterTx,UsartCounterRx;

uint32 IndexToBaudrate(uint8);
void UsartInit(void);
void UsartDeInit(void);
void UsartResetRxTx(uchar tx_rx);
PUCHAR UsartGetBuff(uchar tx_rx);
uchar UsartGetRxCounter();

bool UsartOpen();
void UsartClose();


void UsartSetStage(uchar stage);

void UsartStatusReset();
void UsartSetStatus(uchar status,uchar on_off);
bool UsartGetStatus(uchar status);
uint16 UsartGetTxRxStatus(uchar status);
void UsartSetProcessTask(uchar tx_rx,uchar on_off);

bool UsartSetBaudRate(uchar baud_rate_num);
bool UsartSendCmd(uchar size);
void UsartMonitorTxProc();
void UsartMonitorRxProc();
bool UsartGetStatusRxWaiting();
bool UsartGetStatusRxEnd();
bool UsartGetStatusTxEnd();
bool UsartGetStatusTxIng();
bool UsartGetStatusRxIng();
bool UsartTxIsBusy();
bool UsartRxIsBusy();


void UsartMonitor();
void UsartMonitor1ms();

bool UsartTxSendCmd(PUCHAR pBuff,uint16 size);
void UsartShowDataRx();
void UsartIrq(uchar    dir,uchar status);
bool CheckModbusCrc(PUCHAR pbuff, uchar len);
uint16 ModbusRtu_CRC16(uchar *updata,uint16 len);
void UsartPrintBuff(uchar rx_tx);




void UsartClientProc();
bool CheckUsartRxCmd();

uint16 MbsSend(uint8 *data,uint16 len);
uint16 MbsResponseError(uint8 id, uint8 fun, uint8 err);
void MbsSetReadRegCmd(uint8 id, uint8 func, uint16 loc, uint16 len);





#endif  //_BUS_USART_
