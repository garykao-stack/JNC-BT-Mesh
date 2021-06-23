#if DEVICE_SPI_ENABLE
#include "global.h"

//richard Add
/* BG stack headers */
#include "eeprom.h"
#include "leds.h"
#include "MeshFeatures.h"
#include "sensor_client.h"
#include "sensor_server.h"
#include "mesh_event.h"
#include "AD7147.h"
#include "bus_usart.h"
#include "water_level_mesh.h"

const float CinGainTbl[CIN_NUM_MAX]=
{
    //IC 0
    0.982098, 1.000000, 0.994308, 0.988837, 0.983488, 0.977646, 0.974781, 0.972840, 0.970064, 0.968681, 0.967812, 0.961414, 
    //IC 1
    0.992346, 1.000000, 0.994576, 0.990728, 0.986910, 0.982125, 0.979331, 0.977971, 0.974772, 0.970832, 0.968829, 0.964457,
    //IC 2
    0.982098, 1.000000, 0.994308, 0.988837, 0.983488, 0.977646, 0.974781, 0.972840, 0.970064, 0.968681, 0.967812, 0.961414, 
    //IC 3
    0.992346, 1.000000, 0.994576, 0.990728, 0.986910, 0.982125, 0.979331, 0.977971, 0.974772, 0.970832, 0.968829, 0.964457,
    //IC 4
    0.982098, 1.000000, 0.994308, 0.988837, 0.983488, 0.977646, 0.974781, 0.972840, 0.970064, 0.968681, 0.967812, 0.961414, 
    //IC 5
    0.992346, 1.000000, 0.994576, 0.990728, 0.986910, 0.982125, 0.979331, 0.977971, 0.974772, 0.970832, 0.968829, 0.964457,
    //IC 6
    0.982098, 1.000000, 0.994308, 0.988837, 0.983488, 0.977646, 0.974781, 0.972840, 0.970064, 0.968681, 0.967812, 0.961414, 
    //IC 7
    0.992346, 1.000000, 0.994576, 0.990728, 0.986910, 0.982125, 0.979331, 0.977971, 0.974772, 0.970832, 0.968829, 0.964457,
    
    
};


const uint16  BasicCdcValue[CIN_NUM_MAX]=
{
    0x7A24, 0x7A24, 0x7A25, 0x7A26, 0x7A27, 0x7A26, 0x7A26, 0x7A26, 0x7A26, 0x7A25, 0x7A26, 0x7A25, //AD7147 - 1
    0x790D, 0x790A, 0x790B, 0x7908, 0x790D, 0x7910, 0x790A, 0x790B, 0x790D, 0x790A, 0x790D, 0x790D, //AD7147 - 2
    0x7A24, 0x7A24, 0x7A25, 0x7A26, 0x7A27, 0x7A26, 0x7A26, 0x7A26, 0x7A26, 0x7A25, 0x7A26, 0x7A25, //AD7147 - 3
    0x790D, 0x790A, 0x790B, 0x7908, 0x790D, 0x7910, 0x790A, 0x790B, 0x790D, 0x790A, 0x790D, 0x790D, //AD7147 - 4
    0x7A24, 0x7A24, 0x7A25, 0x7A26, 0x7A27, 0x7A26, 0x7A26, 0x7A26, 0x7A26, 0x7A25, 0x7A26, 0x7A25, //AD7147 - 5
    0x790D, 0x790A, 0x790B, 0x7908, 0x790D, 0x7910, 0x790A, 0x790B, 0x790D, 0x790A, 0x790D, 0x790D, //AD7147 - 6
    0x7A24, 0x7A24, 0x7A25, 0x7A26, 0x7A27, 0x7A26, 0x7A26, 0x7A26, 0x7A26, 0x7A25, 0x7A26, 0x7A25, //AD7147 - 7
    0x790D, 0x790A, 0x790B, 0x7908, 0x790D, 0x7910, 0x790A, 0x790B, 0x790D, 0x790A, 0x790D, 0x790D, //AD7147 - 8
    

};


#define All_CIN_BUFF_SIZE   (CIN_NUM_MAX+1)

uint16  CdcValue[All_CIN_BUFF_SIZE];
uchar   AD7147Num=AD7147_MAX_NUM;
PUINT16 pDevStartCin;

_DeviceInfo     DeviceInfo = WaterLeveDefault;
_PDeviceInfo    pDeviceInfo=&DeviceInfo;
uchar           DevLevel;





uint16 AirLevel,WaterLevel,OilLevel,MudLevel;
uchar SensorNum;

void WaterLevelMeshInit(void)
{

    memset(&CdcValue,0,sizeof(CdcValue));
    AD7147Num = GetAD7147Num();
    SensorNum=0;
    AirLevel=WaterLevel=OilLevel=MudLevel=0;    
    AD7147Init();
    ResetGetCinInfo();
}


//
// To get all of data for sensor
//
void WaterLeveMeshProc(void)
{

    uchar scan_cin_status, scan_cin_stage;
    if(GetMeshNodeStatus(STATUS_SLEEPING) || NodeRole == NR_CLIENT || GetMeshNodeStatus(STATUS_BLE_CONNECT))
        return;

    // start to scan CINx value
    scan_cin_status = GetScanCinStatus();
    scan_cin_stage = GetScanCinStage();

    if(scan_cin_stage == GET_CIN_STAGE_PENDING)
        {
            switch(scan_cin_status)
                {
                    case SCAN_CIN_STATUS_COMPLETE:
                        
                        if(++SensorNum >= AD7147Num) {// to process CIN information
                           DeviceLevelInfo();
                           ResetGetCinInfo();
                           SensorNum = 0;
                           memset(&CdcValue,0,sizeof(CdcValue));   //clean cin buffer
                           }
                        else
                           {
                            //ResetScanCinStatus();
                            StartScanCinValue();
                           }
                        break;
                    case SCAN_CIN_STATUS_PENDING:
                        StartScanCinValue();
                        break;
                    case SCAN_CIN_STATUS_ERROR:
                        StartScanCinValue();
                        break;
                };
        }
    
    
}

//
// Transfer sensor data to water/oil/mud level
// update: AirLevel, WaterLevel, OilLevel, MudLevel
//
bool DeviceLevelInfo(void)
{
    bool ret_code=TRUE;
    //PrintDataLen("** SpiCinValue 1 **",CdcValue,DEVICE_CIN_NUM,IC_CIN_NUM);
    //PrnCinFloat();
    CinGainAdjust();
   // PrintDataLen("Cin Gain Adjust 1",CdcValue,DEVICE_CIN_NUM,IC_CIN_NUM);
    CheckCinDiff();
    //CheckAllCinStatus();
    //GetWaterPosition();

    CdcValue[DEVICE_CIN_NUM]=CIN_BUFF_END;
    pDevStartCin = CdcValue;    // start from CIN0
    switch(pDeviceInfo->DeviceKind)
        {
            case DEVICE_KIND_WATER: Trace("DEVICE_KIND_WATER");
                    WaterLevelInfo();
                break;
            case DEVICE_KIND_OIL:   Trace("DEVICE_KIND_OIL");
                    OilLevelInfo();
                break;
            case DEVICE_KIND_WATER_OIL: Trace("DEVICE_KIND_WATER_OIL");
                    WaterOilLevelInfo();
                break;
            case DEVICE_KIND_WATER_MUD:                
                break;
            default: TraceErr1("DeviceLevelInfo",pDeviceInfo->DeviceKind)                ;
                
        };
    PrintDataLenDec("** SpiCinValue 2 **",CdcValue,DEVICE_CIN_NUM,IC_CIN_NUM);
    
    return ret_code;
}


#define CIN_ADJUST_VALUE        10

// check the difference value
//
void CheckCinDiff()
{
    PUINT16 pCdcValue,pBaseCdcValue;
    uint16  diff_value;
    uchar ic_num,cin_num;
    
    pCdcValue = CdcValue;
    pBaseCdcValue = (PUINT16)BasicCdcValue;
    for(ic_num=0; ic_num<AD7147Num; ic_num++)
        { 
        for(cin_num=0; cin_num<IC_CIN_NUM; cin_num++)
            {
             diff_value = 0 ; *pCdcValue -= CIN_ADJUST_VALUE;
             if(*pCdcValue > *pBaseCdcValue)    diff_value = *pCdcValue - *pBaseCdcValue;
             *pCdcValue = diff_value;    // save different value to buff
             pCdcValue++;pBaseCdcValue++;    // next CIN
            }
        }
  
}


//
// adjust Cin value
void CinGainAdjust()
{
    uchar loop;
    PUINT16 pcin_buff;
    pcin_buff = CdcValue;
    
    for(loop=0; loop< DEVICE_CIN_NUM; loop++)
        {
         *pcin_buff = (uint16)((float)*pcin_buff*CinGainTbl[loop]);
          pcin_buff++;
        }
}

void PrnCinFloat()
{
    uchar loop;
    float value1,value2;
    for(loop=0; loop< IC_CIN_NUM; loop++)
        {
           value1 = (float)CdcValue[1]/(float)CdcValue[loop] ;
           Printf("Gain Value1 %f \r\n",value1);
        }
    for(loop=0; loop< IC_CIN_NUM; loop++)
        {
           value1 = (float)CdcValue[1+IC_CIN_NUM]/(float)CdcValue[loop+IC_CIN_NUM] ;
           Printf("Gain Value2 %f \r\n",value1);
        }
        
}


//
//
uchar GetWaterPosition()
{
    uchar   loop,water_count;
    PUINT16 pCdcValue;
    pCdcValue = pDevStartCin;

    water_count=LEVEL_NO_VALUE;
    loop = 0;
    while(*pCdcValue != CIN_BUFF_END)
        {
            if(*pCdcValue >= WATER_VALUE_MIN) 
                {
                  water_count = loop;
                  pDevStartCin = pCdcValue;
                }
            pCdcValue++; loop++;  
        };
    TraceDec1("**** Water Counter *****", water_count);
    return water_count;
}


uchar RippleCount;
#define LEVEL_DOWN_COUNT    2
#define VALUE_DIFF_MAX      40 //6
#define RIPPLE_COUNTER      1 //5
//
//
void WaterLevelInfo()
{
    uchar water_level;
    uchar value_diff;
    water_level = GetWaterPosition();
    water_level = CinToCm(water_level);

    if(water_level > WaterLevel) value_diff = water_level - WaterLevel;
    else value_diff = WaterLevel - water_level;
    
    TraceDec3("value_diff", water_level,WaterLevel,value_diff);
    
    if(value_diff <= VALUE_DIFF_MAX || RippleCount >=RIPPLE_COUNTER)
       {WaterLevel = water_level; RippleCount=0;}
    else{// status error
         RippleCount++;
         if(water_level > WaterLevel && WaterLevel <= (DevLevel-LEVEL_DOWN_COUNT)) 
            WaterLevel += LEVEL_DOWN_COUNT;
         else if(WaterLevel > LEVEL_DOWN_COUNT) 
            WaterLevel -= LEVEL_DOWN_COUNT;
    }

    AirLevel  = DevLevel - WaterLevel;
    OilLevel = 0;
    TraceDec3("Debug-1:",AirLevel,DevLevel,WaterLevel);
    Printf("*          Air = %02d cm,   Water = %02d cm          *\r\n",AirLevel,WaterLevel);
    
}


void OilLevelInfo()
{
    uchar   loop,Oil_count;
    PUINT16 pCdcValue;
    pCdcValue = pDevStartCin;

    Oil_count = LEVEL_NO_VALUE;
    loop = 0;
    while(*pCdcValue != CIN_BUFF_END)
        {
            if(*pCdcValue >= AIR_VALUE_MAX) 
                {
                  Oil_count = loop;  pDevStartCin = pCdcValue;
                }
            pCdcValue++; loop++;  
        };
    if(WaterLevel == 0) OilLevel = CinToCm(Oil_count);
    else if(Oil_count == LEVEL_NO_VALUE) OilLevel = 0;
    else  OilLevel = Oil_count+1;
    
    AirLevel  = DevLevel - WaterLevel - OilLevel;
    TraceDec3("Debug-2:",AirLevel,DevLevel,WaterLevel);
    Printf("*          Air = %02d cm,   Oil = %02d cm          *\r\n",AirLevel,OilLevel);
                
    
}

void WaterOilLevelInfo()
{
    WaterLevelInfo(); 
    if(*pDevStartCin > WATER_VALUE_MIN) pDevStartCin++;
    OilLevelInfo();
    TraceDec3("Debug-3:",AirLevel,OilLevel,WaterLevel);
    Printf("*****************************************************************\r\n");
    Printf("*          Air = %02d cm,   Oil = %02d cm,   Water = %02d cm          *\r\n",AirLevel,OilLevel,WaterLevel); 
    Printf("*****************************************************************\r\n");
    
}







const float   WaterOilFilter[WATER_LEVL_SIGNAL_SIZE+5]=
{
 0.35, 0.40, 0.41, 0.41, 0.41, 0.41, 0.41, 0.41, 0.41, 0.41, 0.41, 0.41, 
 0.41, 0.41, 0.41, 0.41, 0.41, 0.41, 0.41, 0.41, 0.41, 0.41, 0.41, 0.41, 
 0.48, 0.48, 0.48, 0.48
};


const float   WaterFilter[WATER_LEVL_SIGNAL_SIZE+5]=
{
 0.70, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 
 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75,
 0.75,0.75,0.75,0.75,0.75
};

const float   OilFilter[WATER_LEVL_SIGNAL_SIZE+5]=
{
 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 
 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75,
 0.75,0.75,0.75,0.75,0.75
};
 

#define WATER_FILTER_VALUE      0.7
#define OIL_FILTER_VALUE        0.6

//*******************************************************************************************
//Water Level Noise Filter
//
//
//*******************************************************************************************
void WaterOilNoiseFilter()
{
    uint16    *pCdcValue;
    uchar    water_position,oil_pos,loop;
    float   diff1;
    uchar    media_type;
    water_position = oil_pos =0;
    media_type = DEVICE_KIND_WATER_OIL;
    pCdcValue = (uint16*)&CdcValue;

    water_position = GetWaterPosition();                // get water position
    TraceDec1("water_position 1", water_position);
    if(water_position != LEVEL_NO_VALUE)
        {TraceDec1("water_position 2", water_position);
        water_position++; // point to next media(Oil or air)
        pCdcValue = pCdcValue + (DEVICE_CIN_NUM -1 - water_position);
        media_type = pDeviceInfo->DeviceKind;
        diff1 = (float)(water_position%12)*0.03; 
        Printf("diff123 %d  %d\r\n",(uchar)(diff1*1000),water_position%12);
        
        }
    else 
        {// for Oil / Air
         water_position = 0;
         pCdcValue = pCdcValue + (DEVICE_CIN_NUM -1 - water_position);
         media_type = DEVICE_KIND_OIL;
         
        }

    //oil_pos = DEVICE_TOTAL_CIN_NUM - water_position;
    oil_pos = water_position;

    //if(oil_pos > 12) oil_pos = oil_pos%12;
    if(oil_pos%12 == 0) oil_pos=0; else oil_pos = 12 - oil_pos%12;
    
    TraceDec3("*pCdcValue", *pCdcValue,oil_pos,water_position);
    TraceDec1("media_type",media_type);
    
    for(loop=0; loop < oil_pos; loop++)
        {
         //TraceDec2("pCdcValue value", *pCdcValue,loop);
         //Printf("(FilterRate[loop]-diff1)= %f ",(WaterOilFilter[loop]-diff1)*100);
         if(media_type == DEVICE_KIND_WATER_OIL )
            {*pCdcValue = (float)(*pCdcValue)*(WaterOilFilter[loop]-diff1+0.1);}
         else if(media_type == DEVICE_KIND_WATER )
            //{*pCdcValue = (float)(*pCdcValue)*((WaterFilter[loop])); }
            {*pCdcValue = (float)(*pCdcValue)*0.5; }
         else if(media_type == DEVICE_KIND_OIL )
            {*pCdcValue = (float)(*pCdcValue)*0.8;} //((OilFilter[loop])); }
         
         //else {*pCdcValue = (float)(*pCdcValue)*(WATER_FILTER_VALUE); }
         pCdcValue--;
        }

/*
    for(loop=0; loop<10; loop++)
        {
         *pCdcValue *=0.85;
         pCdcValue--;
        }
*/    
    TraceDec1("***** Soft Filter *****",pDeviceInfo->DeviceKind)  ;
    //ShowCinDiff();
    //TraceDec1("WATER_LEVEL_SIZE 6", WATER_LEVEL_SIZE)  ;
}


#define CIN_CHECK_VALUE     1800 //WATER_VALUE_MIN // default water value
uint16 CinStatus[AD7147_MAX_NUM];

//
// update CIN status
//
bool CheckAllCinStatus()
{
    uint16 *pCinValue,cin_mask,cin_status;
    bool ret_code;
    uchar loop,ic_num;
    ret_code = TRUE;
    for(ic_num=0; ic_num<AD7147_MAX_NUM; ic_num++)
        {
        cin_status = 0; cin_mask = 0x0800;
        pCinValue = (uint16*)&CdcValue+(ic_num*IC_CIN_NUM);
        for(loop=0; loop<IC_CIN_NUM; loop++)
            { //TraceDec1("CinStatus 1 ",*pCinValue);
            if(*pCinValue >= CIN_CHECK_VALUE) cin_status |=cin_mask;
            pCinValue++;  cin_mask >>=1;
            }
        CinStatus[ic_num] = cin_status;
        }
    return ret_code;
}

#define CIN_DIFF_NUM        (WATER_LEVL_SIGNAL_SIZE*2+1)

//
// CINx convert t CM
// Input=CIN num(00 ~ 95), Output: 1 ~ 99 cm
//
uchar CinToCm(uchar cin_level)
{
    if(cin_level == LEVEL_NO_VALUE) return 0;
    
    if(cin_level > CIN_DIFF_NUM) cin_level++;

    cin_level += BOTTOM_THICKNESS+1;
    return cin_level;
}



// return total number of AD7147
uchar GetAD7147Num()
{
    uchar ic_num = 2;
    //return AD7147Num;
    //return AD7147_MAX_NUM;
    if(ic_num == 2) DevLevel = DEVICE_LEN_1;
    else if(ic_num == 4) DevLevel = DEVICE_LEN_2;
    else if(ic_num == 6) DevLevel = DEVICE_LEN_3;
    else if(ic_num == 8) DevLevel = DEVICE_LEN_4;

    
    return ic_num;
}



uint16 GetDevLevelInfo(uchar kind)
{
    uint16 ret_code;
    if(kind == LEVEL_INFO_WATER) ret_code = WaterLevel;
    else if(kind == LEVEL_INFO_OIL) ret_code = OilLevel;
    else if(kind == LEVEL_INFO_MUD) ret_code = MudLevel;
    else ret_code = 0;

    Trace1("GetDevLevelInfo = ",ret_code);
    return ret_code;
}

#endif

