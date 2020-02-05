
/*
 * I2C.h
 *
 *  Created on: 2019/07/11
 *      Author: Richard
 */

#ifndef _BUS_I2C_
#define _BUS_I2C_
#include "i2cspm.h"


#define DEV_I2C_TCA9535         0
#define DEV_I2C_24FC512         1
#define DEV_I2C_Si7060          2

#define I2C_ADDR_TCA9535        0x40
#define I2C_ADDR_EEPROM         0xA0        // EEPROM 24FC512
#define I2C_ADDR_SI7021         0x80        // Tempature & Humidity sernsor

#define I2C_SI7021              I2C0
#define I2C_EEPROM              I2C0





void I2CInit();

int16 GetTempature();
int16 GetHumidity();
uint32 GetTempHumi();


#define EEPROM_DEFAULT_VALUE        (-1)

bool EepromReadByte(uint16 addr,PUCHAR p_value);
bool EepromReadWord(uint16 addr,PUINT16 p_value);
bool EepromReadDword(uint16 addr,PUINT32 p_value);
bool EepromReadBytes(uint16 addr,uint16 buff_num,PUCHAR p_buff);
bool EepromWriteByte(uint16 addr,uchar value);
bool EepromWriteWord(uint16 addr,word value);
bool EepromWriteDword(uint16 addr,uint32 value);
bool EepromWriteBytes(uint16 addr,uint16 buff_num,PUCHAR p_buff);

bool EepromToDefault();
uchar EepromReadByte1(uint16 addr);
uint16 EepromReadWord1(uint16 addr);
void EepromInit();




#endif  //_BUS_I2C_

