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
#define CHS_MODBUS_ERROR        10
#define CHS_SEND_DATA_DELAY     20


#define MODBUS_TO_HOST_BUFF_NUM      16
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



#define CNS_PRE_IVI_UPDATE      0x20    // waiting IVI update
#define CNS_IVI_UPDATE          0x21    // waiting IVI update


#define COUNT_NODE_DETECTED     4      // node error couter

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

#define SI7021_TEMP             0x0302  //for SI7021
#define SI7021_RH               0x0303  //for SI7021

#define PT485_TEMP              SI7021_TEMP  //for AIP

#define AIP_POWER               0x0004  //for AIP
#define AIP_POWER_STATUS        0x0001  //for AIP
#define AIP_POWER_STATUS_VALUE  0x0002  //for AIP


#define A308M_XRMS              0x0004
#define A308M_XSPEED            0x0012
#define A308M_YRMS              0x0018
#define A308M_YSPEED            0x0026
#define A308M_ZRMS              0x002C
#define A308M_ZSPEED            0x003A
#define A308M_ZRMS              0x002C
#define A308M_ZSPEED            0x003A
#define A308M_TEMP              0x003C
#define A308M_XFFT_FRE          0x0100
#define A308M_XFFT_STR          0x0102
#define A308M_YFFT_FRE          0x0200
#define A308M_YFFT_STR          0x0202
#define A308M_ZFFT_FRE          0x0300
#define A308M_ZFFT_STR          0x0302


#define POSITION_WATER          0x0000    
#define POSITION_OIL            0x0001

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




void ClientNodeInit();


void ClientNodeTask();
void ClientGetNodeInfoProc();
void ClientSetNodeInfoProc();

void ClientFromHostProc();

void ClientPropertyEvent();
void PropertyLcd();
bool PrepareModbusCmd();
bool SendModbusToHost();
void ClientCheckNodeStatus();
PClientInfo GetServerInfoPos(uint16 node_addr);



bool ClientPrepareToHost();
bool ClientOtherModbusCmd();

bool ClientSi7021(PSi7021Info p_info);
bool ClientPT485(PPT485Info p_info);
bool ClientAIP(PAIPInfo p_info);
bool ClientA308m(PA308mInfo p_info);
bool ClientWaterLevel(PWaterLevelInfo p_info);
bool ClientJncSd(PSdInfo p_info);
bool ClientIAQS(PIaqsInfo p_info);
bool ClientUltraSound(PUltraSoundInfo p_info);

void ShowAllNodeInfo(void);
void ShowEventInfo(PClientInfo p_info);
uint16 GetProperityID();



#endif //_MEAH_CLIENT_H_
