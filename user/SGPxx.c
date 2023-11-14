#include "SGPxx.h"

#include "global.h"
// #include "device_bus.h"
// #include <stddef.h>
// #include "SHT3x.h"
#include "i2cspm.h"
// #include "stddef.h"

#define SGPxx_I2C I2C0

SGPxx_STATE SGPxx_MeasureMode = SGPxx_IDLE;
//typedef enum {Measuring = 0,Measured = 1}SGPxx_Working;
//static SGPxx_Working SGPxx_Status = Measured;
uint8_t SGPxxSensor_TimeOut = 0;
bool SGPxx_NoGetACK = false;

uint8_t SGPxx_VOCData[2],SGPxx_CO2Data[2];
uint8_t SGPxx_VOCCRC,SGPxx_CO2CRC;
uint16_t SGPxx_VOC_Value;

uint16_t max_value = 0;

bool bValue = false;

// void SGPxxSensor_Start(void)
// {
//   TRHSensor_Start();
// }
// void SGPxxSensor_Stop(void)
// {
//   TRHSensor_Stop();
// }
// uint8_t SGPxxSensor_GetAck()
// {
//   uint8_t ack;
//   ack = TRHSensor_GetAck();
  
//   return ack ;
// }
// void SGPxxSensor_Ack()
// {
//   TRHSensor_Ack();
// }
// void SGPxxSensor_NAck()
// {
//   TRHSensor_NAck();
// }
// void SGPxxSensor_Send(uint8_t v)
// {
//   TRHSensor_Send(v);
// }
// uint8_t SGPxxSensor_Get(void)
// {
//   uint8_t v=0;
//   v = TRHSensor_Get();    
//   return v;
// }

// CRC原始算法
unsigned char crc_high_first(unsigned char *ptr, unsigned char len)
{
  unsigned char i; 
  unsigned char crc=0xff; /* 計算的初始crc值 */

  while(len--)
  {
    crc ^= *ptr++;  /* 每次先與需要計算的數據異或,計算完指向下一數據 */
    for (i=8; i>0; --i)   /* 下面這段計算過程與計算一個字節crc一樣 */
    { 
      if (crc & 0x80)
          crc = (crc << 1) ^ 0x31;
      else
          crc = (crc << 1);
    }
  }
  return (crc); 
}

bool SGPxx_I2C_write(I2C_TypeDef *i2c, uint16_t addr, uint8_t *write_data, uint16_t write_len) {
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;

  seq.addr = (addr << 1);
  seq.flags = I2C_FLAG_WRITE;

  seq.buf[0].data = write_data;
  seq.buf[0].len = write_len;

  ret = I2CSPM_Transfer(i2c, &seq);

  if (ret != i2cTransferDone) // && ret != i2cTransferNack)
    return false;

  return true;
}
bool SGPxx_I2C_read(I2C_TypeDef *i2c, uint16_t addr, uint8_t *read_data, uint16_t read_len) {
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;

  seq.addr = (addr << 1);
  seq.flags = I2C_FLAG_READ;

  seq.buf[0].data = read_data;
  seq.buf[0].len = read_len;
  
  seq.buf[1].data = read_data;
  seq.buf[1].len  = read_len;

  ret = I2CSPM_Transfer(i2c, &seq);

  if (ret != i2cTransferDone)
    return false;

  return true;
}
bool SGPxx_I2C_write_read(I2C_TypeDef *i2c, uint16_t addr, uint8_t *write_data, uint16_t write_len, uint8_t *read_data, uint16_t read_len) {
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;

  seq.addr = (addr << 1);
  seq.flags = I2C_FLAG_WRITE_READ;

  seq.buf[0].data = write_data;
  seq.buf[0].len = write_len;

  seq.buf[1].data = read_data;
  seq.buf[1].len = read_len;

  ret = I2CSPM_Transfer(i2c, &seq);

  if (ret != i2cTransferDone)
    return false;

  return true;
}

bool SGPxxSoftReset(void) {
  uint8_t write_data[2] = {SGPxx_soft_reset[0], SGPxx_soft_reset[1]};
  bool ret = SGPxx_I2C_write(SGPxx_I2C, SGPxx_address, write_data, 2);

  if (ret) {
    SGPxx_NoGetACK = true;
  } else {
    SGPxxSensor_TimeOut = 0;
    SGPxx_MeasureMode = SGPxx__INIT;
  }
  return ret;
}
bool SGPxx_Init(void) {
  uint8_t write_data[2] = {SGPxx_Iaq_Init_msb, SGPxx_Iaq_Init_lsb};
  bool ret = SGPxx_I2C_write(SGPxx_I2C, SGPxx_address, write_data, 2);

  if (ret)
    SGPxx_MeasureMode = SGPxx_Measure;
  else
    SGPxx_NoGetACK = true;
  return ret;
}

bool SGPxx_Measure_iaq(void) {
  uint8_t write_data[2] = {SGPxx_measure_iaq_msb, SGPxx_measure_iaq_lsb};
  bool ret = SGPxx_I2C_write(SGPxx_I2C, SGPxx_address, write_data, 2);

  if (ret)
    SGPxx_MeasureMode = SGPxx_Value;
  else
    SGPxx_NoGetACK = true;
  return ret;
}
bool SGPxx_GetValue(void) {
  uint8_t read_data[6] = {0};
  bool ret = SGPxx_I2C_read(SGPxx_I2C, SGPxx_address, read_data, 6);

  if (!ret)
    return false;

  SGPxx_CO2Data[0] = read_data[0];
  SGPxx_CO2Data[1] = read_data[1];
  SGPxx_CO2CRC = read_data[2];
  SGPxx_VOCData[0] = read_data[3];
  SGPxx_VOCData[1] = read_data[4];
  SGPxx_VOCCRC = read_data[5];

  return true;
}

void SGPxx_Load(void)
{
  char GetCRC;
  //int CO2;
  if(SGPxxSensor_TimeOut >= 5)
  {
    // if(I2C_Sensor_Quantity & 0x03) GetNext_I2C_Sensor();
    SGPxx_MeasureMode = SGPxx_softreset;
    SGPxxSensor_TimeOut = 0;
  }
  else
  {
    SGPxxSensor_TimeOut++;
    switch(SGPxx_MeasureMode)
    {
      case SGPxx_IDLE:
        SGPxx_MeasureMode = SGPxx__INIT;
        SGPxx_NoGetACK = false;
      break;
      case SGPxx_softreset:
        SGPxxSoftReset();
        if(!SGPxx_NoGetACK)SGPxxSensor_TimeOut = 0;
      break;
      case SGPxx__INIT:
        SGPxx_Init();
        if(!SGPxx_NoGetACK)SGPxxSensor_TimeOut = 0;
      break;
      case SGPxx_Measure:
        SGPxx_Measure_iaq();
        if(!SGPxx_NoGetACK)SGPxxSensor_TimeOut = 0;
      break;
      case SGPxx_Value:
        SGPxx_GetValue();
        if(!SGPxx_NoGetACK)SGPxxSensor_TimeOut = 0;
        
        GetCRC = crc_high_first(SGPxx_VOCData,2);
        if(SGPxx_VOCCRC == GetCRC)
        {
          SGPxx_VOC_Value = (SGPxx_VOCData[0] << 8) | SGPxx_VOCData[1];
          bValue = true;
          SGPxx_MeasureMode = SGPxx_Measure;
          SGPxxSensor_TimeOut = 0;
          // if(I2C_Sensor_Quantity & 0x03) GetNext_I2C_Sensor();
        }

      break;
    }
  }
}

bool SGPxx_IsReady(bool bReset) {
  if(bReset)
    SGPxx_MeasureMode = SGPxx_IDLE;

  if (SGPxx_MeasureMode == SGPxx_Measure)
    return true;

  for (int i = 0; i < 10; i++) {
    SGPxx_Load();
    if (SGPxx_MeasureMode == SGPxx_Measure)
      return true;

    Delay_ms(10);
  }

  return SGPxx_MeasureMode == SGPxx_Measure;
}

bool SGPxx_GetTvoc(uint16_t *value) {
  bValue = false;

  for (int i = 0; i < 10; i++) {
    SGPxx_Load();
    if (bValue)
      break;
    Delay_ms(10);
  }

  if (bValue) {
    *value = SGPxx_VOC_Value;
    max_value = MAX(max_value, SGPxx_VOC_Value);
    // dprint("====> TVOC: %d\r\n", SGPxx_VOC_Value);
  }

  return bValue;
}

void SGPxx_ResetTvocMax() {
  max_value = 0;
}

bool SGPxx_GetTvocMax(uint16_t *value) {
  if (SGPxx_GetTvoc(value)) {
    *value = max_value;
    return true;
  }
  return false;
}
