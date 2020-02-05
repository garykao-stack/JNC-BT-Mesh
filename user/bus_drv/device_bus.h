
/*
 * device_bus.h
 *
 *  Created on: 2019/07/16
 *      Author: Richard
 */

#ifndef _DEVICE_BUS_
#define _DEVICE_BUS_

 
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_ldma.h"
#include "em_usart.h"


// Sensor
#define DEV_AD7147_0     0x00
#define DEV_AD7147_1     0x01
#define DEV_AD7147_2     0x02
#define DEV_AD7147_3     0x03
#define DEV_AD7147_4     0x04
#define DEV_AD7147_5     0x05
#define DEV_AD7147_6     0x06
#define DEV_AD7147_7     0x07

// Flash
#define DEV_MX25         0x10







#define TX_BUFFER_SIZE      12
#define RX_BUFFER_SIZE      TX_BUFFER_SIZE

#define TXRX_BUFFER_SIZE     10



#define SPI_AD7147_SEND     BIT0    //send command ok
#define SPI_AD7147_GET      BIT1    //receive data ok
#define SPI_DEVICE1_SEND    BIT2
#define SPI_DEVICE1_GET     BIT3

#define AD7147_COMPLETE (SPI_AD7147_SEND|SPI_AD7147_GET)

#define I2C_DEVICE1_SEND    BIT0
#define I2C_DEVICE1_GET     BIT1
#define I2C_DEVICE2_SEND    BIT2
#define I2C_DEVICE2_GET     BIT3

#define USART_DEVICE1_SEND  `BIT0
#define USART_DEVICE1_GET    BIT1
#define USART_DEVICE2_SEND   BIT2
#define USART_DEVICE2_GET    BIT3


extern uchar  SpiStatus,I2cStatus;


extern uchar SpiTxBuffer[TX_BUFFER_SIZE];
extern uchar SpiRxBuffer[RX_BUFFER_SIZE];

void DeviceAllInit();
void DeviceBusInit();
void DeviceInit();

PUCHAR GetTxBuff();
PUCHAR GetRxBuff();

bool DeviceOn(uchar dev_id);
bool DeviceOff(uchar dev_id);
bool DeviceSleep(uchar dev_id);
bool DeviceGetData(uchar dev_id);
bool DeviceSendCmd(uchar dev_id,PUCHAR pCmd,uchar size);
bool DeviceAccess(uchar dev_id,PUCHAR pCmd,uchar size);


#endif  //_BUS_I2C_

