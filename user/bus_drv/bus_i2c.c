#include "global.h"
#include "device_bus.h"
#include "bus_i2c.h"
#include "eeprom.h"
#include "si7013.h"

#define TEST_BUFF_SIZE  1

#define BT_NODE_FUN_SLEEPING        BIT0    //0: Full Power 1:sleeping mode
#define BT_NODE_FUN_BT_MODBUS       BIT1    //0 disable, 1: enable
#define BT_NODE_FUN_BT_MESH         BIT2




uchar test_buff11[16]={0x12,0x34,0x56,0x78,0x90,0x06,0x07,0x08,0x09,0x10,0x11,0x12,0x13,0x14,0x15,0x16};

void Si7021Init()
{
    
}


void I2CInit()
{TraceProc();


#if (HAL_I2CSENSOR_ENABLE) //I2C0 Initialize I2C peripheral
    I2CSPM_Init_TypeDef i2cInit = I2CSPM_INIT_DEFAULT;
    I2CSPM_Init(&i2cInit);
    Delay_ms(50);
#endif //HAL_I2CSENSOR_ENABLE
    //GetTempature();
    //GetHumidity();
#if 0
    int eeprom_num;
    GetTempHumi(); //while(1);
    while(1);
#endif    


#if 0
    uchar   read_byte;
    uint16   read_word;
    uint32  read_dword;
    
    uint16 addr=0;
    uchar  counter=0;
    //byte test
    memset(test_buff13,0,32);
    for(counter=0; counter<32; counter++)
        {//TraceDec2("counter", counter,test_buff13[counter]);
            addr++;
        
         if(EepromWriteByte(addr,counter) == FALSE) TraceErr("EepromWriteByte");
         if(EepromReadByte(addr,&test_buff13[counter]) == FALSE) TraceErr("EepromReadByte");
         //TraceDec2("counter", counter,test_buff13[counter]);
        }
    PrintDataByte("EepromWriteByte", test_buff13,32);

    //byte test
    memset(test_buff13,0,32);
    for(counter=0; counter<16; counter++)
        {//TraceDec2("counter", counter,test_buff13[counter]);
            addr++;
        
         if(EepromWriteWord(addr,(uint16)counter+0x10) == FALSE) TraceErr("EepromWriteWord");
         if(EepromReadWord(addr,(PUINT16)&test_buff13[counter*2]) == FALSE) TraceErr("EepromReadWord");
         //TraceDec2("counter", counter,test_buff13[counter]);
        }
    PrintData("EepromWriteWord", (PUINT16)test_buff13,16);

    //byte test
    memset(test_buff13,0,32);
    for(counter=0; counter<8; counter++)
        {//TraceDec2("counter", counter,test_buff13[counter]);
            addr++;
         if(EepromWriteDword(addr,(uint32)counter+0x20) == FALSE) TraceErr("EepromWriteDword");
         if(EepromReadDword(addr,(PUINT32)&test_buff13[counter*4]) == FALSE) TraceErr("EepromReadDword");
         //TraceDec2("counter", counter,test_buff13[counter]);
        }
    PrintData("EepromWriteDWord", (PUINT16)test_buff13,16);

        
    
    if(EepromWriteBytes(addr,16,test_buff11) == FALSE) TraceErr("EepromWriteBytes");
    if(EepromReadBytes(addr,16,test_buff13) == FALSE) TraceErr("EepromReadBytes");
    PrintDataByte("EepromWriteBytes", test_buff13,16);
    
    
    while(1);
#endif

  //  EepromInit();
}



typedef struct         //for Power On initial
{
    uint16  DeviceInfoID;     //  0xAA55 to check EEPROM is new or old
    uchar   SysDataInittVer;  // for EEPROM data structure ID
    uchar   MeshNodeRole;     // 0:Server 1:Client 2:Friend 3:LPN 4:BLE Master 5:BLE Slave    
    uint32  MeshNodeFun;      // BT-Modbus/BT-Mesh-Sensor/BT-Mesh
    uchar   ServerNodeNum;  // Total Server Node Num
    uchar   TxPower;        // setup Tx power
    uchar   TxGain;         //
    uchar   RxGain;         // for RF distance, but CE failed    
    uint16  CTune;          // for BLE RF sensivity
    uint32  SleepingTimer;  // xxxx Sec
    uchar   ModbusID;       // 1 ~ 0xFE
    uchar   BaudRate;       // {2400, 4800, 9600, 19200, 38400, 57600, 115200, 128000, 256000, 460800, 921600, 1382400,1843200,2764800};
    uchar   Reserve[3];
}_MeshSysDataInit,*_PMeshSysDataInit;

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

//
//
//
void EepromInit()
{TraceProc();
    _PMeshSysDataInit pEeprom=0;
    uint16 DeviceInfoID;
    DeviceInfoID = EepromReadWord1((uint16)pEeprom); Trace16_1(DeviceInfoID);
    if(DeviceInfoID != DEVICE_INFO_ID ) EepromToDefault();
   
}

//
// Eeprom Write default data
//
bool EepromToDefault()
{TraceProc();
    bool ret_code = TRUE;
    _MeshSysDataInit mesh_sys_data;
    memset(&mesh_sys_data,0,sizeof(_MeshSysDataInit));
   
    mesh_sys_data.DeviceInfoID = DEVICE_INFO_ID;
    mesh_sys_data.SysDataInittVer = 100;    // for 1.00
    mesh_sys_data.MeshNodeRole = MESH_NODE_ROLE_DEFAULT;
    mesh_sys_data.MeshNodeFun = MESH_NODE_FUN_DEFAULT;
    mesh_sys_data.ServerNodeNum = 1;
    mesh_sys_data.TxPower = TX_POWER_DEFAULT;
    mesh_sys_data.TxGain = -100;
    mesh_sys_data.RxGain = -100;
    mesh_sys_data.CTune = BSP_CLK_HFXO_CTUNE ;
    mesh_sys_data.SleepingTimer = 10; //10 sec
    mesh_sys_data.ModbusID = 1;
    mesh_sys_data.BaudRate = 6; //115200

    EepromWriteBytes(MESH_SYS_DATA_EEPROM_ADDR,sizeof(_MeshSysDataInit),(PUCHAR)&mesh_sys_data);
    
    return ret_code;
}

uchar EepromReadByte1(uint16 addr)
{
    uchar ret_value = 0xFF;
    if(EEPROM_Read(I2C_EEPROM,I2C_ADDR_EEPROM,addr,(PUCHAR)&ret_value,1) < 0)  
       TraceErr("EepromReadByte");
    return ret_value;
}

uint16 EepromReadWord1(uint16 addr)
{
    uint16 ret_value = 0xFFFF;
    if(EEPROM_Read(I2C_EEPROM,I2C_ADDR_EEPROM,addr,(PUCHAR)&ret_value,2) < 0)  
       TraceErr("EepromReadWord");
    return ret_value;
}



bool EepromReadByte(uint16 addr,PUCHAR p_value)
{
    bool ret_code = TRUE;
    if(EEPROM_Read(I2C_EEPROM,I2C_ADDR_EEPROM,addr,(PUCHAR)p_value,1) < 0)  
        {ret_code = FALSE; TraceErr("EepromReadByte");}
    return ret_code;
}

bool EepromReadWord(uint16 addr,PUINT16 p_value)
{
    bool ret_code = TRUE;
    if(EEPROM_Read(I2C_EEPROM,I2C_ADDR_EEPROM,addr,(PUCHAR)p_value,2) < 0)  
        {ret_code = FALSE; TraceErr("EepromReadWord");}
    return ret_code;
}

bool EepromReadDword(uint16 addr,PUINT32 p_value)
{
    bool ret_code = TRUE;
    if(EEPROM_Read(I2C_EEPROM,I2C_ADDR_EEPROM,addr,(PUCHAR)p_value,4) < 0)  
        {ret_code = FALSE; TraceErr("EepromReadDword");}
    return ret_code;

}

bool EepromReadBytes(uint16 addr,uint16 buff_num,PUCHAR p_buff)
{
    
    bool ret_code = TRUE;
    EEPROM_Read(I2C_EEPROM,I2C_ADDR_EEPROM,addr,p_buff,buff_num);
    return ret_code;
}

bool EepromWriteByte(uint16 addr,uchar value)
{
    bool ret_code = TRUE;
    if(EEPROM_Write(I2C_EEPROM,I2C_ADDR_EEPROM,addr,&value,1) < 0)  
        {ret_code = FALSE; TraceErr("EepromWriteByte");}
    return ret_code;
}

bool EepromWriteWord(uint16 addr,word value)
{
    bool ret_code = TRUE;
    if(EEPROM_Write(I2C_EEPROM,I2C_ADDR_EEPROM,addr,(PUCHAR)&value,2) < 0)  
        {ret_code = FALSE; TraceErr("EepromWriteWord");}
    return ret_code;
}

bool EepromWriteDword(uint16 addr,uint32 value)
{
    bool ret_code = TRUE;
    if(EEPROM_Write(I2C_EEPROM,I2C_ADDR_EEPROM,addr,(PUCHAR)&value,4) < 0)  
        {ret_code = FALSE; TraceErr("EepromWriteDword");}
    return ret_code;

}

bool EepromWriteBytes(uint16 addr,uint16 buff_num,PUCHAR p_buff)
{
    
    bool ret_code = TRUE;
    if(EEPROM_Write(I2C_EEPROM,I2C_ADDR_EEPROM,addr,p_buff,buff_num) < 0) 
        {ret_code = FALSE; TraceErr("EepromWriteBytes");}
    return ret_code;
}



#define RET_VALUE_ERROR         (-1)



//
//
//
uint32 GetTempHumi()
{
    int16 temp_data,humi_data;
    uint32 temp_humi;
    PUINT16 p_temp_humi;
    Trace("******************\r\n");

    temp_data = humi_data = 0;
    humi_data = GetHumidity();
    temp_data = GetTempature();
    temp_humi = MAKEDWORD(temp_data,humi_data);
    //Trace1("Temp Humi",temp_humi);
    TraceDec2("Temp & Humi",temp_data,humi_data); 

    return temp_humi;

}



//
//
//
int16 GetTempature()
{
    int32 ret_byte;
    int32 tempature;    
    ret_byte = Si7013_Measure(I2C_SI7021, I2C_ADDR_SI7021, (PUINT32)&tempature, SI7013_READ_TEMP);

    Trace1("GetTempature 1",tempature);
    
    if (ret_byte == 2) {
      tempature = (((17552*tempature)/65536) - 4685)/10;
    } else {
      tempature = RET_VALUE_ERROR;
    }
    
    TraceDec1("GetTempature 2-1",(int16)tempature);
    //if((int16)tempature < -100) 
      //  {TraceDec1("GetTempature 2-2",(int16)tempature);tempature = -100;}
    
    return (int16)tempature;
    //return -100;
}

//
//
//
int16 GetHumidity()
{
    int32 ret_byte;
    uint32 humidity;
   
    ret_byte = Si7013_Measure(I2C_SI7021, I2C_ADDR_SI7021, &humidity, SI7013_READ_RH);
   // Trace1("humidity 1",humidity);
    if (ret_byte == 2) {/* convert to milli-percent */
      humidity = ((1250*humidity)/65536L) - 60;
    } else {
      humidity = RET_VALUE_ERROR;
    }
    TraceDec1("GetHumidity",(int16)humidity);
    return (int16)humidity;

}



//
//
//
int16 GetTempature1()
{
    int32 ret_byte;
    uint32 tempature;
    
    ret_byte = Si7013_Measure(I2C_SI7021, I2C_ADDR_SI7021, &tempature, SI7013_READ_TEMP);

    Trace1("GetTempature 1",tempature);
    
    if (ret_byte == 2) {
      tempature = (((tempature) * 21965L) >> 13) - 46850; // convert to milli-degC  ((tempature*175.72)/65536) -46.85
    } else {
      tempature = RET_VALUE_ERROR;
    }
    TraceDec1("GetTempature 2",tempature);
    return (int16)tempature;
}

//
//
//
int16 GetHumidity1()
{
    int32 ret_byte;
    uint32 humidity;
    
    ret_byte = Si7013_Measure(I2C_SI7021, I2C_ADDR_SI7021, &humidity, SI7013_READ_RH);
    Trace1("humidity 1",humidity);
    if (ret_byte == 2) {/* convert to milli-percent */
      humidity = (((humidity) * 15625L) >> 13) - 6000;
    } else {
      humidity = RET_VALUE_ERROR;
    }
    TraceDec1("GetHumidity 2",humidity);
    return (int16)humidity;

}



int32_t Si7013_MeasureRHAndTemp1(I2C_TypeDef *i2c, uint8_t addr, uint32_t *rhData, int32_t *tData)
{
  int ret = Si7013_Measure(i2c, addr, rhData, SI7013_READ_RH);
 // TraceDec1("ret 1 RH",*rhData);
  
  if (ret == 2) {
    /* convert to milli-percent */
    *rhData = (((*rhData) * 15625L) >> 13) - 6000;
  } else {
    return -1;
  }

  ret = Si7013_Measure(i2c, addr, (uint32_t *) tData, SI7013_READ_TEMP);
 // TraceDec1("ret 2 Temp",*tData);

  if (ret == 2) {
    *tData = (((*tData) * 21965L) >> 13) - 46850; /* convert to milli-degC */
  } else {
    return -1;
  }

  return 0;
}

   





