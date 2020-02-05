

#include "global.h"
#include "device_bus.h"
#include "bus_usart.h"
#include "BUS_RS485.h"

void Rs485Init()
{TraceProc();
   
    GPIO_PinModeSet(RS485_TX_RX_PORT, RS485_TX_RX_PIN, gpioModePushPull, 0); //TX
    Rs485Rx();      //default
    //Rs485Tx();    //default

}

// change RS-485 to Tx status
void Rs485Tx()
{TraceProc();
    UsartSetStatus(USART_RX_WAITING,OFF);
    RS485ToTx();
}

// change RS-485 to Rx status
void Rs485Rx()
{TraceProc();
    UsartSetStatus(USART_RX_WAITING,ON);
    RS485ToRx();
}

