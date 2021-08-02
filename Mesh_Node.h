/*
 * Global.h
 *  Created on: 2020/09/09
 *  Author: richard.huang
 */

//#define JNC_DO_485    1
//#define PZEM          1
//#define OEM_SENSOR    1   //visual sensor
//#define AGB_POWER     1




#ifndef _MEAH_NODE_H_
#define _MEAH_NODE_H_



#pragma pack(push)
#pragma pack(1)     //mapping to one byte


typedef struct _NodeStageInfo
{
    uchar   Stage;
    uint16  Timer;
}NodeStageInfo,*PNodeStageInfo;

typedef struct _NodeHeader_
{
    uint8   SensorClass;    // for sensor class for server node
    uint8   Status;
    uint8   BatteryPower;   // 0 ~ 100%    
}_NodeHeader,*PNodeHeader;
// Status
#define SERVER_NO_RESPONSE          BIT0    //server node no response
#define SERVER_FULL_POWER           BIT1    //server full power that can not sleeping

typedef struct _Si7021Info_
{
    int16   Tempature;      // -40°C ~ 85°C
    uint16  Humidity;       // 0% ~ 100%
}_Si7021Info,*PSi7021Info;

typedef struct _PT485Info_
{
    int16   Tempature;      // -40°C ~ 85°C
}_PT485Info,*PPT485Info;

typedef struct _AIPInfo_
{
    uint16  AipPower;       //00%, 25%, 50%, 75%, 100%, power consumption
    uint16  AipPowerStatus;
}_AIPInfo,*PAIPInfo;


typedef struct _A308mInfo_
{
    int16   Tempature;      // x0.1 -20 ~ 50°C
    
    uint16  RmsX;
    int16   SkewnessX;      // x0.1
    int16   KurtosisX;      // x0.1
    uint16  FrequencyX;
    uint16  SpeedX;
    uint16  StengthX;
    
    uint16  RmsY;
    int16   SkewnessY;
    int16   KurtosisY;
    uint16  FrequencyY;
    uint16  SpeedY;
    uint16  StengthY;

    uint16  RmsZ;
    int16   SkewnessZ;
    int16   KurtosisZ;
    uint16  FrequencyZ;
    uint16  SpeedZ;
    uint16  StengthZ;
    
}_A308mInfo,*PA308mInfo;
typedef struct _WaterLevelInfo_
{
    uint16   WaterLevel;
    uint16   OilLevel;
}_WaterLevelInfo,*PWaterLevelInfo;

typedef struct _SdInfo_
{
    int16   Tempature;      // -40°C ~ 85°C
    uint16  Humidity;       // 0% ~ 100%
    uint16  CO2;
    uint16  PM25;           //for PM2.5
    uint16  HCHO;
}_SdInfo,*PSdInfo;

typedef struct _IaqslInfo_
{
    int16   Tempature;      // -40°C ~ 85°C
    uint16  Humidity;       // 0% ~ 100%
    uint16  CO2;            //
    uint16  PM25;           //for PM2.5
    uint16  HCHO;           //
    uint16  CO;             //
    uint16  TVOC;
    uint16  O3;
    uint16  PM10;           //for PM10
  //  uint16  Reserve[7];
}_IaqsInfo,*PIaqsInfo;

typedef struct _Cw9Info_
{
    uint16  Data01;     
    uint16  Data02;      
    uint16  Data03;            
    uint16  Data04;          
    uint16  Data05;          
    uint16  Data06;          
    uint16  Data07;
    uint16  Data08;
    uint16  Data09;         
    uint16  Data10;           
    
    uint16  Data11;           
    uint16  Data12;           
    uint16  Data13;           
    uint16  Data14;              
    uint16  Data15;          
    uint16  Data16;           
    uint16  Data17;          
    
}_Cw9Info,*PCw9Info;


typedef struct _UltarSound_
{
    uint16   WaterLevel;
}_UltraSoundInfo,*PUltraSoundInfo;

typedef struct _JncDo485_
{
    uint16   DoRealValue;
    uint16   DoOffsetValue;
    uint16   TempRealValue;
    uint16   TempOffsetValue;
}_JncDo485,*PJncDo485;

typedef struct _A6D6_
{
    uint16  AiValue1,AiValue2,AiValue3,AiValue4;
    uint16  AiValue5,AiValue6,AiValue7,AiValue8;
    uchar   Di_Status;
    uchar   DO_Status; // 00 ~ 3F
}_A6D6,*PA6D6;

typedef struct _Pzem_
{
    uint16  Voltage;
    uint16  CurrentLo,CurrentHi;
    uint16  PowerLo,PowerHi;
    uint16  ElectLo,ElectHi;
    uint16  Frequecny;
    uint16  PowerFactor;
    uint16  Warning;
}_Pzem,*PPzem;

#define PZEM_ITEM_SIZE   10 


typedef struct _RelayNode_
{
    uint16 Status;
}_RelayNode,*PRelayNode;

typedef struct _OemSensor_
{
    uint16   Addr00;
    uint16   Addr01;
    uint16   Addr02;
    uint16   Addr03;
    uint16   Addr0A;
}_OemSensor,*POemSensor;

#define OEM_SENSOR_SIZE   4

typedef struct _AgbPower_
{
    uint16  PowerStatus;
}_AgbPower,*PAgbPower;

typedef struct _SkynetCo2_
{
    int16   Tempature;      // -40°C ~ 85°C
    uint16  Humidity;       // 0% ~ 100%
    uint16  Co2;
}_SkynetCo2,*PSkynetCo2;



typedef struct _SensorInfo_
{    
        _NodeHeader Header;
union{
        _Si7021Info Si7021Info;
        _PT485Info  PT485;
        _AIPInfo    AipInfo;
        _A308mInfo  A308mInfo;
        _WaterLevelInfo WaterLevelInfo;
        _SdInfo     SdInfo;
        _IaqsInfo   IaqsInfo;
        _Cw9Info    Cw9Info;
        _UltraSoundInfo UltraSound;
        _JncDo485   JncDo485;
        _A6D6       A6D6;
        _Pzem       Pzem;
        _RelayNode  RelayNode;
        _OemSensor  OemSensor;
        _AgbPower   AgbPower;
        _SkynetCo2  SkynetCo2;
        
     };
}_SensorInfo,*PSensorInfo;

#define SERVER_LEADING_SIZE     3
typedef struct _ServerInfo_
{
    uint16 ProperityID;
    uchar  NodeInfoSize;    
    _SensorInfo SensorInfo;
}_ServerInfo,*PServerInfo;


typedef struct _ClientInfo_
{
    uint16  ServerID;
    uint8   Count;
    uint16  Status;
    _SensorInfo SensorInfo;
}_ClientInfo,*PClientInfo;




///////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _NodeEventInfo_
{
    uint16      ElemIndex;
    uint16      ClientAddr;
    uint16      ServerAddr;
    uint16      AppkeyIndex;
    uint8       Flags;
    uint16      PropertyID;
    uint16      SettingID;
    uint8array  SensorData;
    uint16      PropertyValue;
}_NodeEventInfo, *PNodeEventInfo;


#pragma pack(pop)


// Sensor class;
#define SENSOR_DISCONNECT    0
#define SENSOR_SI7021        1     //for tempature & Humidity   
#define SENSOR_PT485         2
#define SENSOR_AIP           3
#define SENSOR_WATER_LEVEL   4
#define SENSOR_IAQS          5
#define SENSOR_JNC_SD        6

#define OTHER_MODBUS_CMD     7     //for other modbus cmd
#define SENSOR_ULTRA_SOUND   8     //
#define SENSOR_DO_485        9     // 

#define SENSOR_A308M         10      //for 
#define SENSOR_A308M_JNC     SENSOR_A308M      //for JNC

#define BTM_SENSOR           SENSOR_SI7021    //to BT Mesh Built-in sensor for temp&RH
#define SENSOR_RELAY         11     // Power Only
#define SENSOR_A6D6          12     // A6D6

#define SENSOR_PZEM          13     //
#define SENSOR_OEM           14     // visual sensor 
#define SENSOR_AGB_POWER     15     //

#define SENSOR_CW9           16     // 
#define SENSOR_SKYNET_CO2    17     // Skynet+CO2

//can not auto scan

#define SENSOR_NO_SCAN       53


// for ServerStatus
#define AIP_POWER_STATUS_MASK          (BIT1|BIT0) // BIT0,BIT1: 00    00%
#define AIP_POWER_STATUS_00             0           // BIT0,BIT1: 00    00%      
#define AIP_POWER_STATUS_25             BIT0        // BIT0,BIT1: 01    25%
#define AIP_POWER_STATUS_75             BIT1        // BIT0,BIT1: 10    75%
#define AIP_POWER_STATUS_100            AIP_POWER_STATUS_MASK// BIT1,BIT2: 11  100%
//#define SetAipPowerStatus(status)       ServerNodeInfo.ServerStatus = (ServerNodeInfo.ServerStatus & ~AIP_POWER_STATUS_MASK)|status



#define NODE_STAGE_INFO_NUM         8

//for Client and Server
#define USART_MONITOR_CLIENT_PROC   0
//////////////////////////////////////////////////////////////////////
//for Client Node
#define CLIENT_GET_NODE_INFO_PROC   1
#define CLIENT_TO_HOST_STAGE_PROC   2
#define NODE_IVI_UPDATE_PROC        3
#define CLIENT_SET_NODE_INFO_PROC   4

// for Server Node
#define SERVER_GET_INFO_PROC        1
#define SERVER_SET_NODE_PROC        2
#define SENSOR_INFO_PROC            3
#define SENSOR_RS485_INFO_PROC      4

// for Node Setup
#define MESH_NODE_SETUP_PROC        1



#define TIMER_NODE_SLEEPING         1000    //xx sec
#define TIMER_DEFAULT_WORKING       60  // xx Sec define

#define SERVER_NODE_MAX             50

#define TIMER_GET_INFO_FULL_POWER   1 //2 //5 // xxx Sec
#define TIMER_GET_INFO_SLEEPING     (pMeshNodeData->WorkingTimer)//18 //62 //18 // xxx sec
#define TIMER_SERVER_SLEEPING       (TIMER_GET_INFO_SLEEPING - 2)    
#define TIMER_CLI_WAIT_INFO         WAIT_SEC(2)//WAIT_SEC(4) //WAIT_SEC(3)
#define TIMER_SERVER_DELAY          40  //ms
#define TIMER_SERVER_SENS_INFO      (pMeshNodeData->MeshNodeID*TIMER_SERVER_DELAY)



#define MODEL_ID_SERVER             0x1100
#define MODEL_ID_SETUP_SERVER       0x1101
#define MODEL_ID_CLIENT             0x1102


//NodeRole: define node role
//Node Role ==> NR_
#define NR_SERVER               0
#define NR_SETUP_SERVER         1
#define NR_CLIENT               2
#define NR_FRIEND               3
#define NR_LPN                  4
#define NR_MASTER               5
#define NR_SLAVE                6
#define NR_SETUP                7
#define NR_DEFAULT              NR_SERVER


// NodeStatus
// define node status: ==> NS_XXXXX
#define NS_PROVISIONING         BIT0    // node is provisioning by provisioner
#define NS_SYS_URGENT           BIT1    // system urgent: do not sleeping
#define NS_SLEEPING             BIT2    //
#define NS_PROXY_ON             BIT3    //
#define NS_LINKING              BIT4    // BLE/Proxy link
#define NS_IVI_UPDATE           BIT5    // waiting IV index update
#define NS_TASK_ACTION          BIT6    // 10ms action
#define NS_GET_INFO_ACT         BIT7    // BT Mesh Get info Event action
#define NS_FULL_POWER           BIT8    // do not sleeping(connect USB power)
#define NS_SET_NODE_ACT         BIT9    // BT Mesh Set info Event action


///Client node
#define NS_USART_RX_EVENT       BIT10    //
#define NS_CLIENT_GET_INFO      BIT11    //
#define NS_CLIENT_SET_INFO      BIT12    //


//Server node
#define NS_GET_SENSOR_INFO      BIT16    // star to get sensor information
#define NS_GET_SENSOR_ERR       BIT17    // 
#define NS_GET_SENSOR_OK        BIT18    // get sensor info ending
#define NS_SERVER_RS485_ENABLE  BIT19    // RS-485 Enable: Get info from RS-485 sensor
//#define NS_SERVER_READY         BIT20    // sensor ready to get information


// Device
#define NS_USART_RX_ACTION      BIT24    // 
#define NS_USART_TX_ENDING      BIT25    //

#define NS_RELAY_ONLY           BIT30   // only for relay role 
//#define NS_SIMLUATION           BIT31    // node simulation active


#define NS_DO_NOT_SLEEPING      (NS_SYS_URGENT|NS_LINKING|NS_IVI_UPDATE)    // node do not sleeping





#define ActiveStage()               pStageInfo->Stage
#define ToNextStage(stage)          pStageInfo->Stage = stage 
#define ToWaitingStage(stage,timer) pStageInfo->Stage = stage; pStageInfo->Timer = timer

//#define ToNextSensorStage(stage) SensorStage = stage

// ServerStatus
#define SS_RS485                BIT0        // 0: disable, 1: enable RS-485 enable
#define SS_DEVICE_POWER         BIT1        // 0: ON, 1: OFF device power ON/OFF



void MeshNodeInit();


void SetMeshNodeStage(uint16 stage);
void SetWaitTimer(uint16 timer);
bool CheckWaitTimeOut();
void SetNodeStatus(uint32 status, uchar on_off);
bool GetNodeStatus(uint32 status);
PNodeStageInfo GetNodeStageInfo(uchar value);
bool SetNodeSleeping(uchar status);
void SetSleeping(uint8 status);
void SetSleepingTimer(uchar status);
void CheckNodeTimerCount();
void SetNodePublish(uchar status);




extern PFunSensor pFunSensor;
extern uint32   NodeStatus;
extern uint16   NodeStage;
extern uint16   WaitingTimer;
extern uchar    NodeRole;
extern PNodeStageInfo pStageInfo;
extern PNodeEventInfo  pNodeEventInfo;
extern uchar CountErr;
extern uint16 GetInfoCycle;
extern uchar   UsartRxCount;   // Receive data from Rx bytes
#include "Mesh_Client.h"
#include "Mesh_Server.h"

#endif //_MEAH_NODE_H_


