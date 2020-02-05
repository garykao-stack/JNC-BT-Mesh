
/*
 * SPI.h
 *
 *  Created on: 2019/07/11
 *      Author: Richard
 */

#ifndef _BUS_SPI_
#define _BUS_SPI_
#include "hal-config-board.h"

#define SPI_ACTIVE_DEVICE_OFF      0xFF

#define RX_DMA_CHANNEL      0
#define TX_DMA_CHANNEL      1

#define SPI_USART           USART1
#define SPI_USART_CLOCK     cmuClock_USART1
#define SPI_LDMA_SIGNAL_TX  ldmaPeripheralSignal_USART1_TXBL     
#define SPI_LDMA_SIGNAL_RX  ldmaPeripheralSignal_USART1_RXDATAV     


#define SPI_USART_SPEED     (1000000)         // CLK freq is 1 MHz
#define SPI_WAIT_DELAY      (5)


#define SPI_MOSI_PORT       gpioPortC
#define SPI_MOSI_PIN        (6U)
#define SPI_MISO_PORT       gpioPortC
#define SPI_MISO_PIN        (7U)
#define SPI_SCLK_PORT       gpioPortC
#define SPI_SCLK_PIN        (8U)


#define SPI_CS_PORT         gpioPortC   //CS0
#define SPI_CS_PIN          (9U)

#define SPI_TEST_PORT       gpioPortA   //CS1
#define SPI_TEST_PIN        (4U)


// US1_CLK       on location 11 = PC8 per datasheet section 6.4 = EXP Header pin 8
// US1_CS        on location 11 = PC9 per datasheet section 6.4 = EXP Header pin 10
// US1_TX (MOSI) on location 11 = PC6 per datasheet section 6.4 = EXP Header pin 4
// US1_RX (MISO) on location 11 = PC7 per datasheet section 6.4 = EXP Header pin 6

#define AUTO_CS     0

#if AUTO_CS   // CS Auto control
// CS Auto control    
#define SPI_USART_LOCATION (USART_ROUTELOC0_CLKLOC_LOC11|USART_ROUTELOC0_TXLOC_LOC11|USART_ROUTELOC0_RXLOC_LOC11|USART_ROUTELOC0_CSLOC_LOC11)                              
#define SPI_USART_PIN_ENABLE (USART_ROUTEPEN_CLKPEN | USART_ROUTEPEN_TXPEN | USART_ROUTEPEN_RXPEN | USART_ROUTEPEN_CSPEN )
#else
// CS control by firmware
#define SPI_USART_LOCATION (USART_ROUTELOC0_CLKLOC_LOC11|USART_ROUTELOC0_TXLOC_LOC11|USART_ROUTELOC0_RXLOC_LOC11)
#define SPI_USART_PIN_ENABLE    (USART_ROUTEPEN_CLKPEN | USART_ROUTEPEN_TXPEN | USART_ROUTEPEN_RXPEN)
#endif

extern uchar SpiActiveDev;


void SpiInit(void);
void SpiDeInit(void);

void SpiDevOpen();
void SpiDevClose();


void SpiSetCS(uchar status);
void SpiSetAllCS(uchar status);

bool SpiWrite(PUCHAR tx_buff,uchar size);
bool SpiRead(PUCHAR cmd_buff, int cmd_size,PUCHAR rx_buff,int rx_size);



#endif  //_BUS_SPI_


