#include "global.h"
#include "device_bus.h"
#include "bus_i2c.h"
#include "em_adc.h"
#include "bus_usart.h"

#include "eeprom.h"
#include "si7013.h"

#define TEST_BUFF_SIZE  1

#define BT_NODE_FUN_SLEEPING        BIT0    //0: Full Power 1:sleeping mode
#define BT_NODE_FUN_BT_MODBUS       BIT1    //0 disable, 1: enable
#define BT_NODE_FUN_BT_MESH         BIT2

#define EEPROM_TEST     0

#if EEPROM_TEST //for EEPROM Test

uchar test_buff11[16]={0x12,0x34,0x56,0x78,0x90,0x06,0x07,0x08,0x09,0x10,0x11,0x12,0x13,0x14,0x15,0x16};
UCHAR WriteEepromTestTbl[16]={0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
UCHAR ReadEepromTestTbl[16];

#endif

void I2CInit()
{

    BpAdcInit();
    GetBpValue();

#if (HAL_I2CSENSOR_ENABLE) //I2C0 Initialize I2C peripheral
    I2CSPM_Init_TypeDef i2cInit = I2CSPM_INIT_DEFAULT;
    I2CSPM_Init(&i2cInit);
    Delay_ms(50);

 #if EEPROM_TEST //for EEPROM Test
    PrintDataByte("EEPROM Test 1", WriteEepromTestTbl,16);
    EepromWriteBytes(0x10,16,WriteEepromTestTbl); Delay_ms(100);
    EepromReadBytes(0x10,16,ReadEepromTestTbl); Delay_ms(100);  
    PrintDataByte("EEPROM Test 2", ReadEepromTestTbl,16);
 #endif
 
#endif //HAL_I2CSENSOR_ENABLE
    GetTempature();
    GetHumidity();
#if 0
    int eeprom_num;
    GetTempHumi(); //while(1);
    while(1);
#endif    
}
 


//
//
//
void EepromInit()
{
    _PMeshNodeSysData pEeprom=0;
    uint16 DeviceInfoID;
    DeviceInfoID = EepromReadWord1((uint16)pEeprom); Trace16_1(DeviceInfoID);
    if(DeviceInfoID != DEVICE_INFO_ID ) EepromToDefault();
   
}

//
// Eeprom Write default data
//
bool EepromToDefault()
{
    bool ret_code = TRUE;
    _MeshNodeSysData mesh_sys_data;
    memset(&mesh_sys_data,0,sizeof(_MeshNodeSysData));
   
    mesh_sys_data.DeviceInfoID = DEVICE_INFO_ID;
    mesh_sys_data.SysDataInittVer = FW_VER;    // for 1.00
    mesh_sys_data.MeshNodeID = 0;
    mesh_sys_data.MeshNodeRole = MESH_NODE_ROLE_DEFAULT;
    mesh_sys_data.MeshNodeFun = MESH_NODE_FUN_DEFAULT;
    mesh_sys_data.ServerNodeNum = 1;
    mesh_sys_data.TxPower = TX_POWER_DEFAULT;
    mesh_sys_data.TxGain = COMP_TX_POWER;
    mesh_sys_data.RxGain = COMP_RX_POWER;
    mesh_sys_data.CTune = BSP_CLK_HFXO_CTUNE ;
    mesh_sys_data.SleepingTimer = 10; //10 sec
    mesh_sys_data.ModbusID = 1;
    mesh_sys_data.BaudRate = USART_BAUDRATE_DEFAULT; //115200

    EepromWriteBytes(MESH_SYS_DATA_EEPROM_ADDR,sizeof(_MeshNodeSysData),(PUCHAR)&mesh_sys_data);
    
    return ret_code;
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
    int write_bytes;
    write_bytes = EEPROM_Write(I2C_EEPROM,I2C_ADDR_EEPROM,addr,p_buff,buff_num);
    if(write_bytes < buff_num) 
        {ret_code = FALSE; TraceDec1("EepromWriteBytes Error",write_bytes);}
    else
        TraceDec1("EEPROM Write Ok ", write_bytes);
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
    int32 ret_byte=0;
    int32 tempature=0;
    ret_byte = Si7013_Measure(I2C_SI7021, I2C_ADDR_SI7021, (PUINT32)&tempature, SI7013_READ_RH);
    ret_byte = Si7013_Measure(I2C_SI7021, I2C_ADDR_SI7021, (PUINT32)&tempature, SI7013_READ_TEMP);
    if (ret_byte == 2) {
      tempature = (((17552*tempature)/65536) - 4685)/10;
    } else {
      tempature = RET_VALUE_ERROR;
    }
    
    printf("Tempature 1 ==> %3.1f -C\r\n",(float)tempature/10);
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
    if (ret_byte == 2) {/* convert to milli-percent */
      humidity = ((1250*humidity)/65536L) - 60;
    } else {
      humidity = RET_VALUE_ERROR;
    }
    printf("Humidity 2 ==> %ld%\r\n",(int16)humidity/10);
    return (int16)humidity;

}

//const uint16 AdcValue[22]={2700,};
#define ADC_VOL_MAX     2670 //2650
#define ADC_VOL_MIN     1800 //1900
#define ADC_DIFF        (ADC_VOL_MAX - ADC_VOL_MIN)
uchar BatteryPower;

#define BATTERY_POWER_DIFF      3

//
// return battery power
//
uchar GetBatteryPower()
{
    uint16 adc_value;
    uchar power_percent;
    uint16 power_diff;
    adc_value = (uint16)GetBpValue();
    if(adc_value > ADC_VOL_MAX) adc_value = ADC_VOL_MAX;
    if(adc_value <= ADC_VOL_MIN) adc_value = ADC_VOL_MIN;
    power_percent = (adc_value - ADC_VOL_MIN)*100/ADC_DIFF;

    if(power_percent >= BatteryPower)
        {//power on/ power error
         //TraceDec2("Test1",power_percent,BatteryPower);
          if((power_percent - BatteryPower) > BATTERY_POWER_DIFF)
            {//TraceDec2("Test2",power_percent,BatteryPower);
            BatteryPower = power_percent;
            }
        }
    else 
        {// normal
            //TraceDec2("Test3",power_percent,BatteryPower);
          if((BatteryPower - power_percent) < BATTERY_POWER_DIFF)
            {//TraceDec2("Test4",power_percent,BatteryPower);
             BatteryPower = power_percent;
            }
        }
    TraceDec1("Power Percent",BatteryPower);
    return BatteryPower; //75;
}


#define adcFreq             16000000
#define ADC_REF_VOLTAGE     3300

volatile uint32_t sample;
volatile uint32_t millivolts;


/**************************************************************************//**
 * @brief  Initialize ADC function for Battery PPower
 *****************************************************************************/
void BpAdcInit (void)
{

  // Enable ADC0 clock
  CMU_ClockEnable(cmuClock_ADC0, true);

  // Declare init structs
  ADC_Init_TypeDef init = ADC_INIT_DEFAULT;
  ADC_InitSingle_TypeDef initSingle = ADC_INITSINGLE_DEFAULT;

  // Modify init structs and initialize
  init.prescale = ADC_PrescaleCalc(adcFreq, 0); // Init to max ADC clock for Series 1

  initSingle.diff       = false;        // single ended
  initSingle.reference  = _ADC_SINGLECTRL_REF_VDD; //v3.3
  initSingle.resolution = adcRes12Bit;  // 12-bit resolution
  initSingle.acqTime    = adcAcqTime4;  // set acquisition time to meet minimum requirement

  // Select ADC input. See README for corresponding EXP header pin.
  initSingle.posSel = adcPosSelAPORT3YCH11; //for PA3 
  init.timebase = ADC_TimebaseCalc(0);

  ADC_Init(ADC0, &init);
  ADC_InitSingle(ADC0, &initSingle); 
  Delay_ms(50);
}


//
// Get Battery Power xx%
//
uint32 GetBpValue()
{
    uint32 adc_value;
    
    ADC_Start(ADC0, adcStartSingle); // Start ADC conversion
    while(!(ADC0->STATUS & _ADC_STATUS_SINGLEDV_MASK)); // Wait for conversion to be complete
    
    adc_value = ADC_DataSingleGet(ADC0); // Get ADC result
 //   TraceDec1("Battery Voltage 1", adc_value);
    adc_value = (adc_value * ADC_REF_VOLTAGE) / 4096; // Calculate input voltage in mV

    
  //  TraceDec1("Battery Voltage 2", adc_value);
    return adc_value;
}


////
void TempAdcInit(void) 
{

    CMU_ClockEnable(cmuClock_ADC0, true);
    /* Base the ADC configuration on the default setup. */
    ADC_Init_TypeDef       init  = ADC_INIT_DEFAULT;
    ADC_InitSingle_TypeDef init_single = ADC_INITSINGLE_DEFAULT;

    /* Initialize timebases */
    init.timebase = ADC_TimebaseCalc(0);
    init.prescale = ADC_PrescaleCalc(400000, 0);
    ADC_Init(ADC0, &init);

    /* Set input to temperature sensor. Reference must be 1.25V */
    init_single.reference   = adcRef1V25;
    init_single.acqTime     = adcAcqTime8; /* Minimum time for temperature sensor */
    init_single.posSel      = adcPosSelTEMP;
    ADC_InitSingle(ADC0, &init_single);
    Delay_ms(50);
}

uint32 GetAdcTemp(void)
{
  ADC_Start(ADC0, adcStartSingle);
  while ( ( ADC0->STATUS & ADC_STATUS_SINGLEDV ) == 0 );
  return ADC_DataSingleGet(ADC0);
}



// Convert ADC temperature sensor readings into milli-celcius
int32_t convert_to_millicelsius(int32_t adc_sample)
{
  const float TGRAD_ADCTH = 1.835; // TGRAD_ADCTH = 1.835 mV/degC (from datasheet)
  const uint32_t VFS = 1250; // VFS = 1250 mV
  const uint32_t NUM_STEPS_12BIT = 4096;
  float T_Celsius;

  /* Factory calibration temperature from device information page. */
  const uint32_t CAL_TEMP0 = ((DEVINFO->CAL & _DEVINFO_CAL_TEMP_MASK) >> _DEVINFO_CAL_TEMP_SHIFT);

  /* _DEVINFO_ADC0CAL3_TEMPREAD1V25_MASK is not correct in
     current CMSIS. This is a 12-bit value, not 16-bit. */
  const uint32_t CAL_VALUE0 = ((DEVINFO->ADC0CAL3 & _DEVINFO_ADC0CAL3_TEMPREAD1V25_MASK) >> _DEVINFO_ADC0CAL3_TEMPREAD1V25_SHIFT);

  if ((CAL_TEMP0 == 0xFF) || (CAL_VALUE0 == 0xFFF)) {
    /* The temperature sensor is not calibrated */
    return -100.0;
  }

  // e.g. EFRxG13 datasheet section 27.3.10.9 Temperature Measurement for below formula
  int32_t readout_difference = CAL_VALUE0 - adc_sample;
  T_Celsius = ((float) readout_difference * VFS);
  T_Celsius /= (NUM_STEPS_12BIT * (-1 * TGRAD_ADCTH));

  /* Calculate offset from calibration temperature */
  T_Celsius = (float) CAL_TEMP0 - T_Celsius;
  return (int32_t) (T_Celsius * 1000);
  //return (int32_t) (T_Celsius * 10);
}


int16 GetBuildInTemp()
{
    int32_t built_in_Temp;
    TempAdcInit();
    built_in_Temp = convert_to_millicelsius(GetAdcTemp());
    TraceDec1("built_in_Temp ", built_in_Temp);

    return (int16)built_in_Temp;
}



//Function for taking a single temperature measurement with EFR32 internal temperature sensor.
void measure_temperature()
{
  uint8_t htm_temperature_buffer[5]; /* Stores the temperature data in the Health Thermometer (HTM) format. */
  uint8_t flags = 0x00;   /* HTM flags set as 0 for Celsius, no time stamp and no temperature type. */
  int32_t temperature_data;     /* Stores the Temperature data read from the sensor. */
  uint32_t temperature;   /* Stores the temperature data read from the sensor in the correct format */
  uint8_t *p = htm_temperature_buffer; /* Pointer to HTM temperature buffer needed for converting values to bitstream. */

  //richard add
  TempAdcInit();


  /* Convert flags to bitstream and append them in the HTM temperature data buffer (htm_temperature_buffer) */
 // UINT8_TO_BITSTREAM(p, flags);

  temperature_data = convert_to_millicelsius(GetAdcTemp()); 

  /* Convert sensor data to correct temperature format */
  temperature = FLT_TO_UINT32(temperature_data, -3);
  TraceDec1("temperature_data ", temperature_data);
  Trace1("temperature", temperature);
  /* Convert temperature to bitstream and place it in the HTM temperature data buffer (htm_temperature_buffer) */
  //UINT32_TO_BITSTREAM(p, temperature);
  PrintDataByte("Built-in Temp",htm_temperature_buffer,5);
}



