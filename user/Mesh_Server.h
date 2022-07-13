/*
 * Global.h
 *  Created on: 2020/09/08
 *  Author: richard.huang
 */


#ifndef _MEAH_SERVER_H_
#define _MEAH_SERVER_H_


#define TIMER_WAIT_SEND_INFO        WAIT_MS(TIMER_SERVER_SENS_INFO) 
#define TIMER_WAIT_SLEEPING         WAIT_SEC(2) //WAIT_SEC(3)     //WAIT_MS(pMeshNodeData->MeshNodeID*3)
#define TIMER_GET_SENSOR_INFO       WAIT_MS(300)



#define SENSOR_ELEMENT          0 ///< Sensor client model located in primary element
#define PUBLISH_ADDRESS         0 ///< The unused 0 address is used for publishing
#define IGNORED                 0 ///< Parameter ignored for publishing
#define NO_FLAGS                0 ///< No flags used for message

#define POWER_USB               1   //connect to USB
#define POWER_BATTERY           2   // connect battery

#define RS485_PT485             1
#define RS485_SENSOR2           2
#define RS485_SENSOR3           3
#define RS485_SENSOR4           4
#define RS485_SENSOR5           5
#define RS485_SENSOR6           6
#define RS485_SENSOR7           7
#define RS485_SENSOR8           8
#define RS485_SENSOR9           9
#define RS485_SENSOR10          10




// Server Node Stage ==> SNS_XXXX
#define SNS_SERVER_STAGE_INIT   0
#define SNS_PRE_WAITING         1
#define SNS_EVENT_WAITING       2    // waitting 500ms
#define SNS_GET_SENSOR_INFO     3    // to get temperature
#define SNS_WAIT_SEND_INFO      4    // waiting send message to client
#define SNS_SEND_INFO           5    // send message to client
#define SNS_PRE_SLEEPING        6    // Pre-sleeping
#define SNS_SLEEPING            7    // to sleeping
#define SNS_WAKE_UP             8    // wake up to power
#define SNS_PRE_SEND_INFO       9    // send message to client

#define SNS_PRE_GET_INFO        10    // send message to client
#define SNS_GET_INFO            11    // send message to client
#define SNS_WAIT_UART_RSP		12
#define SNS_SEND_UART_RSP		13
#define SNS_SEND_A308_INFO		14
#define SNS_A308_FETCH_INFO		15
#define SNS_WAIT_A308_CMD		16


#define SNS_SET_INFO_INIT       0x00
#define SNS_SET_WAITING         0x30    // waiting message from host
#define SNS_SET_INFO_PRE        0x31    //prepare data to btm server
#define SNS_SET_INFO_SEND       0x32
#define SNS_SET_WAITING_INFO    0x33
#define SNS_SET_INFO_OK         0x34
#define SNS_SET_UPDATE_INFO     0x35
#define SNS_SET_INFO_END        0x36

#define AIP_POWER_CTRL_00       0
#define AIP_POWER_CTRL_25       1
#define AIP_POWER_CTRL_50       2
#define AIP_POWER_CTRL_75       3
#define AIP_POWER_CTRL_100      4

#define AIP_POWER_CTRL_CMD(cmd)  ((PUCHAR)&AipPowerCtrlCmd[cmd][0])
#define AGB_POWER_CTRL_CMD(cmd)  ((PUCHAR)&AgbPowerCtrlCmd[cmd][0])


#define A308M_CMD_WRITE_NEW_BIAS 0
#define A308M_CMD_RESET_XBIAS   1
#define A308M_CMD_RESET_YBIAS   2
#define A308M_CMD_RESET_ZBIAS   3
#define A308M_CTRL_CMD(cmd)     ((PUCHAR)&A308MCtrlCmd[cmd][0])


#define MESH_INFO_SCALING       100

#define SetNodeInfoSize(info)   ServerInfo.NodeInfoSize = sizeof(_NodeHeader) + sizeof(info)
#define SetNodeClass(class)     pSensorHeader->SensorClass = class



#define G6_FC6_AUTO_RUN         BIT15
#define G6_FC6_CLS_FILTER1      BIT14   //set filter1 750 Warning Time
#define G6_FC6_CLS_ALL_FILTER   BIT13
#define G6_FC3_DOOR_OPEN        BIT12   


#define G6_FC3_AUTO_RUN         G6_FC6_AUTO_RUN
#define G6_FC3_WARNING_FILTER1  G6_FC6_CLS_FILTER1  
#define G6_FC3_WARNING_FILTER2  G6_FC6_CLS_ALL_FILTER

#define ALL_SETTING_ID                  0x01
#define TEMP_GAIN_SETTING_ID            0x02
#define TEMP_OFFSET_SETTING_ID          0x03
#define RH_GAIN_SETTING_ID              0x04
#define RH_OFFSET_SETTING_ID            0x05
#define WORKING_TIME_SETTING_ID         0x06
#define SENSOR_CLASS_SETTING_ID         0x07

#define SET_FULL_POWER_ON               0x10
#define SET_FULL_POWER_OFF              0x11

#define MENUAL_KEY_AUTO                 4 //5 //6 //5
#define MENUAL_POWER_OFF                5


typedef struct _BtAppData_
{
    int16   TempGain,TempOffset;    // Tempature Gain & Offset  
    int16   RhGain,RhOffset;        // RH Gain & Offset
    uint16  WorkingTimer;            // >5 and <3600 sec
    uint16  BtmClass;               //1 : for JNC Sensor(Auto Scan) 2 : PZEM 3 : Visual Sensor 4 : AGB Motor Control(? ?)
}_BtAppData,*PBtAppData;

#define CLASS_TO_UTILITY    0
#define CLASS_TO_BTM        1

#define G6_ADDR_BEHAVIOR    0x03

#define RTC_ADDR_YEAR       0x38
#define RTC_ADDR_MONTH      0x39
#define RTC_ADDR_DATE       0x3A
#define RTC_ADDR_HOUR       0x3B
#define RTC_ADDR_MIN        0x3C
#define RTC_ADDR_SEC        0x3D
#define RTC_ADDR_WEEK       0x3E

#define SEG1_ON_OFF         0xB8
#define SEG2_ON_OFF         0xB9
#define SEG3_ON_OFF         0xBA
#define SEG4_ON_OFF         0xBB
#define SEG5_ON_OFF         0xBC

#define SEG1_POWER          0xC0
#define SEG2_POWER          0xC1
#define SEG3_POWER          0xC2
#define SEG4_POWER          0xC3
#define SEG5_POWER          0xC4

#define SEG1_WEEK           0xC8
#define SEG2_WEEK           0xC9
#define SEG3_WEEK           0xCA
#define SEG4_WEEK           0xCB
#define SEG5_WEEK           0xCC

#define SEG1_START_TIME     0xD0
#define SEG2_START_TIME     0xD1
#define SEG3_START_TIME     0xD2
#define SEG4_START_TIME     0xD3
#define SEG5_START_TIME     0xD4

#define SEG1_END_TIME       0xD8
#define SEG2_END_TIME       0xD9
#define SEG3_END_TIME       0xDA
#define SEG4_END_TIME       0xDB
#define SEG5_END_TIME       0xDC

#define POWER_PERCENT_SEG0  0xF0
#define POWER_PERCENT_SEG1  0xF1
#define POWER_PERCENT_SEG2  0xF2
#define POWER_PERCENT_SEG3  0xF3
#define POWER_PERCENT_SEG4  0xF4


#define RUNING_TIME_FILTER1 0xFA    //setup warning for filter1
#define RUNING_TIME_FILTER2 0xFB    //setup warning for filter2



extern const uchar AipPowerCtrlCmd[5][8];
extern const uchar A308MCtrlCmd[4][8];
extern uint16  GetDeviceInfoDelay;     //Nx10ms


void ServerNodeInit();
void ServerSetupNodeInit();

void ServerNodeTask();
void ServerGetInfoProc();
void ServerSetNodeProc();

bool SendInfoToClient();
bool SendRxToClient(PUCHAR data,uint8_t loc,uint8_t count);
uchar PreAllSensorInfo();
uchar CheckPowerStatus();
void GetRs485InfoProc();
bool SendCmdToRs485();
bool UpdateSensorInfo();
void EvtGetRequestProc(PCmdPacket pCmdEvent);
void EvtSetSettingRequestProc(PCmdPacket pCmdEvent);
void EvtSetGettingRequestProc(PCmdPacket pCmdEvent);

bool ServerSendModbusCmd(PUCHAR modbus_cmd,uchar len);

bool GetSensorInfo();
bool GetSkynetInfo();
bool GetPT485Info();
bool GetAipInfo();
bool GetA308mInfo();
bool GetWaterLevelInfo();
bool GetJncSdInfo();
bool GetUltraSoundInfo();
bool GetDo485();
bool GetA6D6Info();
bool GetPzem();
bool GetRelay();
bool GetOemSensor();
bool GetAgbPower();
bool GetIaqsInfo();
bool GetCw9Info();
bool GetIaqsCw9();
bool GetSkynetCo2Info();
bool GetBtmMeshInfo();
bool GetBtmG6Info();
bool GetVelocityInfo();
bool G6SetupInfo();
bool G6GetInfo();
bool ModbusSetupFC6();


bool G6SetSegPowerPercent(uint16 addr,uint16 value);
bool G6SetBehavior(uint16 addr,uint16 value);
bool G6SetInitRtc(uint16 addr,uint16 value);
bool G6SetSegOnOff(uint16 addr,uint16 value);
bool G6SetSegPower(uint16 addr,uint16 value);
bool G6SetSegWeek(uint16 addr,uint16 value);
bool G6SetSegStartTime(uint16 addr,uint16 value);
bool G6SetSegEndTime(uint16 addr,uint16 value);
bool G6SetTimeFilter(uint16 addr,uint16 value);




temperature_8_t get_temperature(void);
uint16 GetTempRH(uchar select);
uint16 GetTempAndRH(int16 *Temp, uint16 *humidity);
uint16 GetAverageTempAndRH(int16 *Temp, uint16 *humidity);



void BtMeshSetupInit();
void BtMeshSetupTask();
void MeshNodeSetupProc();
bool MeshNodeSetInfoProc();
bool MeshNodeGetInfoProc();
void BtmRfsense(Bool status);
void ServerGetInfoActionNow();
void ServerSetPowerStatus();
Bool BtmG6SetCtrl(uint16 property_id);
uint16  SensorClassChange(uint16 class,uint8 status);
Bool GetG6InfoFC3(uint16 start_addr,uint16 total_reg);
Bool G6FC3GetInitRtc(uint16 start_addr);

Bool G6FC3GetSegOnOff(uint16 start_addr);
Bool G6FC3GetSegPower(uint16 start_addr);
Bool G6FC3GetSegWeek(uint16 start_addr);
Bool G6FC3GetSegStartTime(uint16 start_addr);
Bool G6FC3GetSegEndTime(uint16 start_addr);
Bool G6FC3GetSegPowerPercent(uint16 start_addr);
Bool G6FC3GetTimeFilter(uint16 start_addr);


#endif //_MEAH_SERVER_H_

