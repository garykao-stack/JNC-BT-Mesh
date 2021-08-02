
/*
 * RS485.h
 *
 *  Created on: 2019/07/11
 *      Author: Richard
 */

#ifndef _BUS_RS485_
#define _BUS_RS485_
#include "bus_usart.h"

// For Rs485 REN_1 define
//#define RS485_TX_RX_PORT    gpioPortA
//#define RS485_TX_RX_PIN     (5)

#define RS485_TX_RX_PORT    gpioPortD
#define RS485_TX_RX_PIN     (13) 

#define RS485ToRx()     GPIO_PinOutClear(RS485_TX_RX_PORT,RS485_TX_RX_PIN)
#define RS485ToTx()     GPIO_PinOutSet(RS485_TX_RX_PORT,RS485_TX_RX_PIN)
#define RS485GetPin()   GPIO_PinOutGet(RS485_TX_RX_PORT,RS485_TX_RX_PIN)


#define CHECK_RS485_CMD(a,b)    {Rs485Tx(); UsartTxSendCmd(a,b); Delay_ms(15); Rs485Rx();Delay_ms(200);} //for 9600


void Rs485Init();
void Rs485Tx();
void Rs485Rx();
void Rs485StandbyMode();
uchar CheckRs485Device();
bool CheckPT485();
bool CheckAIP();
bool CheckJncSd();
bool CheckIaqs();
bool CheckCw9();
bool CheckWaterLevel();
bool CheckA308M();
bool CheckUltraSound();
bool CheckA6D6();
bool CheckCDMCo2();

uchar ScanRs485Device();
bool CheckRs485Connect();
bool ModbusCmdPT485();
bool ModbusCmdAIP();
uint16 Rs485ValuePT485();
void ModbusInitDelay(uint16 timer_ms);





#endif  //_BUS_RS485_
