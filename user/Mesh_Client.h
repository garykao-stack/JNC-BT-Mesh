/*
 * Global.h
 *  Created on: 2020/09/08
 *  Author: richard.huang
 */


#ifndef _MEAH_CLIENT_H_
#define _MEAH_CLIENT_H_


// NodeStage
// define server stage ==> SNS_XXXX
// define client stage ==> CNS_XXXX

//CHS ==> Client Host Stage
#define CHS_STAGE_INIT          0 
#define CHS_CHECK_RX_EVENT      1
#define CHS_CHECK_MODBUS        2
#define CHS_PREPARE_DATA        3
#define CHS_SEND_DATA           4
#define CHS_SERVER_SET          5
#define CHS_SEND_DATA_END       6
#define CHS_SEND_TO_CLIENT		7
#define CHS_MODBUS_ERROR        10
#define CHS_SEND_DATA_DELAY     20
#define CHS_WAIT_TX_FINISHED	21


#define MODBUS_TO_HOST_BUFF_NUM      30
#pragma pack(push)
#pragma pack(1)     //mapping to one byte

typedef struct
{
    uchar   ModbusID;
    uchar   FunCode;
    uchar   ByteNum;
    uint16  Data[MODBUS_TO_HOST_BUFF_NUM + 2];
} _ModbusToHost,*_PModbusToHost;

//
// for Function 4
//
typedef struct
{
    uchar   ModbusID;
    uchar   FunCode;
    uint16  StartAddr;
    uint16  TotalReg;
    uint16  ModbusCrc;
} _ModbusCmdF4,*_PModbusCmdF4;

//
// for Function 6: setup
//
typedef struct
{
    uchar   ModbusID;
    uchar   FunCode;
    uint16  StartAddr;
    uint16  Value;
    uint16  ModbusCrc;
} _ModbusCmdF6,*_PModbusCmdF6;


#pragma pack(pop)

#define MODBUS_ADDR_G6S_POWER       0x02FE
#define MODBUS_ADDR_G6S_STATUS      0x02FF
////////////////////////////////////////////////////////////////////////////
#define MODBUS_ADDR_CO2             0x0300     
#define MODBUS_ADDR_PM25            0x0301
#define MODBUS_ADDR_TEMPATURE       0x0302
#define MODBUS_ADDR_HUMIDITY        0x0303
#define MODBUS_ADDR_BATTERY_POWE    0x0308  // for new Modbus register //0x0330
#define MODBUS_ADDR_AIP_POWER       0x0004  // for AIP Power/Watt




// Client Node Stage ==> CNS_XXXX
// Server Node Stage ==> SNS_XXXX

#define NODE_STAGE_INIT         0x00

#define CNS_PRE_SEVER_INFO      0x01    // prepare to get server info
#define CNS_GET_SEVER_INFO      0x02    // 

#define CNS_WAIT_INFO           0x03    // 
#define CNS_WAIT_INFO_OK        0x04    // to buffer and host

#define CNS_INFO_TO_BUFF        0x05    // send message to client
#define CNS_INFO_TO_HOST        0x06    // 

#define CNS_GET_INFO_ERR        0x09    // 
#define CNS_GET_INFO_END        0x10    // 

#define CNS_WAIT_SET_INFO       0x11    // 
#define CNS_GET_A308_INFO		0x12



#define CNS_PRE_IVI_UPDATE      0x20    // waiting IVI update
#define CNS_IVI_UPDATE          0x21    // waiting IVI update
#define CNS_FORCE_FULL_POWER    0x22    // 



#define COUNT_NODE_DETECTED     ((6000*4)/GetInfoCycle)      // node error couter

#define CNS_SET_INFO_INIT       0x00
#define CNS_SET_WAITING         0x30    // waiting message from host
#define CNS_SET_INFO_PRE        0x31    //prepare data to btm server
#define CNS_SET_INFO_SEND       0x32
#define CNS_SET_INFO_PREEAT     0x33    //send BTM cmd again
#define CNS_SET_WAITING_INFO    0x34
#define CNS_SET_INFO_OK         0x35
#define CNS_SET_UPDATE_INFO     0x36
#define CNS_SET_INFO_ERR        0x37
#define CNS_SET_INFO_END        0x38


#define BATTERY_POWER           0x0308  //for 

#define SENSOR_CO2              0x0300  //for Co2
#define SI7021_TEMP             0x0302  //for SI7021
#define SI7021_RH               0x0303  //for SI7021


#define PT485_TEMP              SI7021_TEMP  //for AIP

#define AIP_POWER               0x0004  //for AIP
#define AIP_POWER_STATUS        0x0001  //for AIP
#define AIP_POWER_STATUS_VALUE  0x0002  //for AIP


#define RMS_X                   0x0004
#define SKEWNESS_X              0x0008
#define KURTOSIS_X              0x000A
#define SPEED_X                 0x0012

#define RMS_Y                   0x0018
#define SKEWNESS_Y              0x001C
#define KURTOSIS_Y              0x001E
#define SPEED_Y                 0x0026

#define RMS_Z                   0x002C
#define SKEWNESS_Z              0x0030
#define KURTOSIS_Z              0x0032
#define SPEED_Z                 0x003A

#define A308M_TEMP              0x003C
#define FREQUENCY_X             0x0100
#define STRENGTH_X              0x0102
#define FREQUENCY_Y             0x0200
#define STRENGTH_Y              0x0202
#define FREQUENCY_Z             0x0300
#define STRENGTH_Z              0x0302


#define BT_RMS_X                0x0602
#define BT_SKEWNESS_X           0x0604
#define BT_KURTOSIS_X           0x0606
#define BT_SPEED_X              0x060A

#define BT_RMS_Y                0x060E
#define BT_SKEWNESS_Y           0x0610
#define BT_KURTOSIS_Y           0x0612
#define BT_SPEED_Y              0x0616

#define BT_RMS_Z                0x061A
#define BT_SKEWNESS_Z           0x061C
#define BT_KURTOSIS_Z           0x061E
#define BT_SPEED_Z              0x0622

#define BT_A308M_TEMP           0x0624
#define BT_FREQUENCY_X          0x0626
#define BT_STRENGTH_X           0x0628
#define BT_FREQUENCY_Y          0x0632
#define BT_STRENGTH_Y           0x0634
#define BT_FREQUENCY_Z          0x063E
#define BT_STRENGTH_Z           0x0640



//for Water Level
#define POSITION_WATER          0x0000    
#define POSITION_OIL            0x0001

//for UltraSound
#define UD_BATTERY_VOL          0x001D
#define UD_INPUT_VOL            0x001E
#define UD_OUTPUT_VOL           0x001F
#define UD_CHARGE_CURR          0x0020
#define UD_INPUT_CURR           0x0021
#define UD_TEMP                 0x0022
#define UD_RH                   0x0023

//for Velocity
#define FTM_RAW_VEL_1           0x09
#define FTM_RAW_VEL_2           0x0A
#define FTM_VEL_1               0x0401
#define FTM_VEL_2               0x0402
#define FTM_VEL_TEMP_1          0x0415
#define FTM_VEL_TEMP_2          0x0416


#define JNC_SD_CO2              0x300
#define JNC_SD_PM25             0x301
#define JNC_SD_TEMP             0x302
#define JNC_SD_RH               0x303

// for FC4 JNC IAQS Modbus Reg
#define FC04_REG_TEMP           0x0000
#define FC04_REG_RH             0x0001
#define FC04_REG_CO2            0x0002
#define FC04_REG_PM25           0x0003
#define FC04_REG_HCHO           0x0004
#define FC04_REG_CO             0x0005
#define FC04_REG_TVOC           0x0006
#define FC04_REG_O3             0x0007
#define FC04_REG_PM10           0x0008


// for FC4 JNC Ultra Sound Modbus Reg
#define FC04_DISTANCE           0x0000
#define FC04_SET_DISTANCE       0x0001
#define FC04_DISTANCE100        0x0002
#define FC04_DISTANCE200        0x0003
#define FC04_DISTANCE300        0x0004
#define FC04_DISTANCE400        0x0005
#define FC04_DISTANCE500        0x0006
#define FC04_DISTANCE600        0x0007
#define FC04_DISTANCE700        0x0008
#define FC04_DISTANCE800        0x0009

// for Do485
#define DO485_REAL_VALUE        0x0000
#define DO485_REAL_OFFSET       0x0002
#define DO485_TEMP_VALUE        0x000A
#define DO485_TEMP_OFFSET       0x000B

#define MODBUS_AIP_POWER_00     0x00
#define MODBUS_AIP_POWER_25     0x19
#define MODBUS_AIP_POWER_50     0x32
#define MODBUS_AIP_POWER_75     0x4B
#define MODBUS_AIP_POWER_100    0x64

#define MODBUS_308M_NEW_BIAS    0x00
#define MODBUS_RESET_XBIAS      0x00
#define MODBUS_RESET_YBIAS      0x01
#define MODBUS_RESET_ZBIAS      0x02

#define MODBUS_AGB_POWER_00     0x00
#define MODBUS_AGB_POWER_25     0x01
#define MODBUS_AGB_POWER_50     0x02
#define MODBUS_AGB_POWER_75     0x03
#define MODBUS_AGB_POWER_100    0x04


// for OEM Sensor
#define OEM_ADDR_00             0x0000
#define OEM_ADDR_01             0x0001
#define OEM_ADDR_02             0x0002
#define OEM_ADDR_03             0x0003
#define OEM_ADDR_0A             0x000A

// for BTM-G6 

#define BTM_G6_STATUS           0x0003
#define BTM_G6_TIME_FILTER1     0x0004
#define BTM_G6_TIME_FILTER2     0x0005


// Client Node Status




void ClientNodeInit();


void ClientNodeTask();
void ClientGetNodeInfoProc();
void ClientSetNodeInfoProc();

void ClientFromHostProc();

void ClientSeriesEvent(msg_ms_client_series_status_evt *pEvent);
void ClientColumnEvent(msg_ms_client_column_status_evt *pEvent);
void ClientPropertyEvent(msg_ms_client_status_evt *pEvent);
void PropertyLcd();
bool PrepareModbusCmd();
bool SendModbusToHost();
void ClientCheckNodeStatus();
int ClientSensorClassCount();
PClientInfo GetServerInfoPos(uint16 node_addr);



bool ClientPrepareToHost();
bool ClientOtherModbusCmd();

bool ClientSkynet(PSi7021Info p_info);
bool ClientPT485(PPT485Info p_info);
bool ClientAIP(PAIPInfo p_info);
bool ClientA308m(PA308mInfo p_info);
bool ClientWaterLevel(PWaterLevelInfo p_info);
bool ClientJncSd(PSdInfo p_info);
bool ClientIAQS(PIaqsInfo p_info);
bool ClientUltraSound(PUltraSoundInfo p_info);
bool ClientJncDo485(PJncDo485 p_info);
bool ClientA6D6(PA6D6 p_info);
bool ClientPzem(PPzem p_info);
bool ClientRelay(PRelayNode p_info);
bool ClientOemSensor(POemSensor p_info);
bool ClientIAQS(PIaqsInfo p_info);
bool ClientCW9(PCw9Info p_info);
bool ClientSkynetCo2(PSkynetCo2 p_info);
bool ClientBtmG6(PBtmG6 p_info);
bool ClientVelocity(PVelocity p_info);
bool ClientJYGD15(_JYGD15Info *info);

void ShowAllNodeInfo(void);
void ShowEventInfo(PClientInfo p_info);
uint16 GetProperityID();
void ModbusAddCrc();
void ClientGetInfoActionNow();



#endif //_MEAH_CLIENT_H_
