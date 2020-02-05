
/*
 * water_level_mesh.h
 *
 *  Created on: 2019/10/31
 *  Author: Richard
 *
 */

#ifndef _WATER_LEVEL_MESH_
#define _WATER_LEVEL_MESH_
// device level kind
#define DEVICE_KIND_WATER           0
#define DEVICE_KIND_OIL             1
#define DEVICE_KIND_WATER_OIL       2
#define DEVICE_KIND_WATER_MUD       3

#define CIN_BUFF_END                0xFFFF
#define LEVEL_NO_VALUE              0xFF
#define DEVICE_CIN_NUM              (AD7147Num*IC_CIN_NUM)//96  // DeviceIcNum*IC_CIN_NUM
#define WATER_LEVL_SIGNAL_SIZE      24  //24cm



#define DEVICE_ID_DEFAULT           1

////////////////////////////////////////////////////////////////////////////////////        
#define AIR_VALUE_MIN               90 //900
#define AIR_VALUE_MIN_TEST          110 //80 //120 //100     // debug for 3M
#define AIR_VALUE_MAX               100 //800//900

#define OIL_VALUE_MIN               130 //145 //120
#define OIL_VALUE_MAX               800 //1100

#define WATER_VALUE_MIN             1800 //2400 //1800 //1300 //1400 //1800 //1200 //2100 //2800 //OIL_VALUE_MAX //Debug for 3M
#define WATER_VALUE_MAX             3000 //4000

// device level kind
#define DEVICE_KIND_WATER           0
#define DEVICE_KIND_OIL             1
#define DEVICE_KIND_WATER_OIL       2
#define DEVICE_KIND_WATER_MUD       3

#define  DEVICE_KIND_DEFAULT        DEVICE_KIND_WATER//DEVICE_KIND_WATER_OIL //DEVICE_KIND_OIL //DEVICE_KIND_WATER
// CIN media status
#define CIN_MEDIA_AIR               0x00
#define CIN_MEDIA_WATER             0x01
#define CIN_MEDIA_OIL               0x02
#define CIN_MEDIA_MUD               0x03
#define CIN_MEDIA_EXIT              0x10

#define CIN_MEDIA_ERROR             0xFF

#define BOTTOM_THICKNESS            2

// device length
#define DEVICE_LEN_ERR              00  // error
#define DEVICE_LEN_1                (BOTTOM_THICKNESS+24)   //26 cm
#define DEVICE_LEN_2                (DEVICE_LEN_1+24)       //50 cm
#define DEVICE_LEN_3                (DEVICE_LEN_2+24+1)     //75 cm
#define DEVICE_LEN_4                (DEVICE_LEN_3+24)     //98 cm


extern uint16 AirLevel,WaterLevel,OilLevel,MudLevel;
extern uchar  WaterLevelSize;


typedef struct _DeviceInfo_
{
    // start save to EEPROM   
    uint16  DeviceInfoID;           //  0xAA55 to check EEPROM is new or old
    uint16  DeviceID;           //  for water level ID
///////////////////////////////////////////    
    uchar   DeviceInfoVer;
    uchar   BaudRate;           //  default:0(2400)    
    uchar   DeviceKind;         // 0:water 1:....
    uchar   DeviceLength;       // for device length: 24, 49, 74, and 99
///////////////////////////////////////////        
    uchar   Sleeping;           // 0: sleeping diable 1: sleeping mode
    uchar   ReserveChar[3];
///////////////////////////////////////////        
    uint32  ReserveUint32[5];
    PUINT16 pBasicCdcValue;     // to save CDC value for air
}_DeviceInfo,*_PDeviceInfo;

#define WaterLeveDefault        /**/\
{                               /**/\
    DEVICE_INFO_ID,             /**/\
    DEVICE_ID_DEFAULT,          /**/\
    100,                        /**/\
    USART_BAUDRATE_DEFAULT,     /**/\
    DEVICE_KIND_DEFAULT,        /**/\
    OFF,                        /**/\
}   

#define LEVEL_INFO_WATER        0
#define LEVEL_INFO_OIL          1
#define LEVEL_INFO_MUD          2



void WaterLevelMeshInit(void);
uchar GetAD7147Num(void);

bool DeviceLevelInfo(void);
void CheckCinDiff();
bool CheckAllCinStatus();
uchar GetWaterPosition();
uchar CinToCm(uchar Cinx);
void CinGainAdjust();
void OilLevelInfo();
void WaterLevelInfo();
void WaterOilLevelInfo();
uint16 GetDevLevelInfo(uchar kind);







void PrnCinFloat();









#endif  //_WATER_LEVEL_MESH_

