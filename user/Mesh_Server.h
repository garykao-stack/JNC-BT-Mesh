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


extern const uchar AipPowerCtrlCmd[5][8];
extern const uchar A308MCtrlCmd[4][8];
extern uint16  GetDeviceInfoDelay;     //Nx10ms


void ServerNodeInit();
void ServerSetupNodeInit();

void ServerNodeTask();
void ServerGetInfoProc();
void ServerSetNodeProc();

bool SendInfoToClient();
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
bool GetSi7021Info();
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




#endif //_MEAH_SERVER_H_

