
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

#pragma pack(push)
#pragma pack(1)     //mapping to one byte


typedef struct         //for Power On initial
{
    uint16  DeviceInfoID;       // 0xAA55 to check EEPROM is new or old
    uchar   SysDataInittVer;    // for EEPROM data structure ID
    uint16  MeshNodeID;
    uchar   MeshNodeRole;       // 0:Server 1:Client 2:Friend 3:LPN 4:BLE Master 5:BLE Slave    
    uint32  MeshNodeFun;        // BT-Modbus/BT-Mesh-Sensor/BT-Mesh
    uchar   ServerNodeNum;      // Total Server Node Num
    uchar   TxPower;            // setup Tx power
    uchar   TxGain;             //
    uchar   RxGain;             // 
    uint16  CTune;              // for BLE RF sensivity
    uint32  SleepingTimer;      // xxxx Sec
    uchar   ModbusID;           // 1 ~ 0xFE
    uchar   BaudRate;           // {2400, 4800, 9600, 19200, 38400, 57600, 115200, 128000, 256000, 460800, 921600, 1382400,1843200,2764800};
    uchar   Reserve[10];
}_MeshNodeSysData,*_PMeshNodeSysData;
#pragma pack(pop)

#define MESH_SYS_DATA_EEPROM_ADDR       0x0000

// for MeshNodeRole
#define MESH_NODE_ROLE_SERVER        0
#define MESH_NODE_ROLE_CLIENT        1
#define MESH_NODE_ROLE_FRIEND        2
#define MESH_NODE_ROLE_LPN           3
#define MESH_NODE_ROLE_MASTER        4
#define MESH_NODE_ROLE_SALVE         5
#define MESH_NODE_ROLE_DEFAULT       MESH_NODE_ROLE_SERVER

// for MeshNodeFun
#define MESH_NODE_FUN_SENSOR        0   // sensor client/server
#define MESH_NODE_FUN_MODBUS        1   // for Modbus protocol
#define MESH_NODE_FUN_BT_MESH       2   //
#define MESH_NODE_FUN_DEFAULT       MESH_NODE_FUN_SENSOR

// ServerNodeNum
#define SERVER_NODE_NUM_DEFAULT     1

#define TX_POWER_DEFAULT            TX_POWER_HI   



#define MeshSysDataDefault      /**/\
{                               /**/\
    DEVICE_INFO_ID,             /**/\
    100,                        /* ver: 1.00*/\
    MESH_NODE_ROLE_DEFAULT,     /**/\
    MESH_NODE_FUN_DEFAULT,      /**/\
    SERVER_NODE_NUM_DEFAULT,    /**/\
    TX_POWER_DEFAULT,           /**/\
    -100,                       /**/\
    -100,                       /**/\
    BSP_CLK_HFXO_CTUNE,         /**/\
    10,                         /**/\
    1,                          /**/\
    6,                          /**/\
}   




typedef struct
{
    uint16  DeviceInfoID;       //  0xAA55 to check EEPROM is new or old
    uchar   DeviceType;         //50cm/100cm
    uint16  SensorData[12*8];   // for calibration
}_WaterLevel,*_PWaterLevel;





void I2CInit();

int16 GetTempature();
int16 GetHumidity();
uint32 GetTempHumi();
uchar GetBatteryPower();



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
void BpAdcInit();
uint32 GetBpValue();

void IntTempAdcInit (void);
int32_t GetInternalTemp();
void measure_temperature();
int16 GetBuildInTemp();






#endif  //_BUS_I2C_

