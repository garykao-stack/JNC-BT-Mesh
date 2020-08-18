
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



void Rs485Init();
void Rs485Tx();
void Rs485Rx();


#endif  //_BUS_RS485_
