
// Richard: define 2019/08/19
#ifndef _NODE_DATA_
#define _NODE_DATA_
#include "em_msc.h"
#include "G6_BT_Mesh.h"

//#define NODE_USER_PAGE       USERDATA_BASE //((uint32_t *)0x0FE00000) /**< Address of the user page */
#pragma pack(push) 
#pragma pack(1)     //mapping to one byte

//Max 56 bytes
typedef struct 
{
  uint16    DataInitID;         // 0xA5A5
  uint16    StructVer;          // 1.00
  uchar     MeshNodeRole;       // Server=1 /Setup Server=2 /Client=3 /Friend=4 /LPN=5
  uint16    MeshNodeID;         // 1 ~ 254
  uchar     TotalNodes;         // for client node
  uint16    SleepingTimer;      // 0: sleeping diable  xx sec
  uchar     BaudRate;           // default:6: 0:2400,1:4800,2:9600,3:19200,4:38400,5:57600,6:115200  
  uchar     SensorClass;        // PT485,AIP,A308M,SD,AIQS,WaterLevel...
  uint16    ElemIndex;
  uint16    ElementAddr;
  uint16    IvIndex,AppkeyIndex,NetkeyIndex;
  uint16    NetKey,AppKey,DeviceKey;
  uchar     TxPower;            // for BLE power
  uchar		RelayEnabled;//uchar     Reserver1;              // for BLE RF sensivity
  uint16    WorkingTimer;
  uchar     G6HostPPercent;      // from host setup power percent 0,1,2,3,4
  ////// to add other Items
  uint16    Status;
  uint16    FilterTime1,FilterAllTime1;
  uint16    FilterTime2,FilterAllTime2;  // Hour
  uchar     G6Status,G6ActPercent;
  uchar     SegPPercent[6]; // for segment power percent
} _Mesh_Node_Data,*_PMesh_Node_Data;

// Mesh Node Status
#define NODE_TEMP_HUM               BIT0    //0: from other device 1: from BT itself
#define NODE_SERVER_TO_RS485        BIT1    //0: from other device 1: from BT itself

#define G6_SCHEDULE_NUM             5



typedef struct
{
    float   TempGain,TempOffset,HumGain,HumOffset;
  _G6Schedule G6Schedule[G6_SCHEDULE_NUM];
}_AdjustValue,*_PAdjustValue;

#define G6_AUTO_RUN             BIT0
#define G6_SET_AUTO_RUN         BIT1    //by modbus
#define G6_CLEAR_FILETER1       BIT2
#define G6_CLEAR_FILETER_ALL    BIT3



#pragma pack(pop) 

#define NODE_DATA_SIZE      sizeof(_Mesh_Node_Data)
#define ADJUST_VALUE_SIZE   sizeof(_AdjustValue)



typedef struct
{
    word    SensorData[96];
    word    SensorDataBackup[96];
    word    SensorID1,SensorID2;    //0xAA55
}SensorData,*PSensorData;

#define PS_KEY_MESH_NODE_DATA   0x4000
#define PS_KEY_ADJUST_VALUE     0x4001


#define PS_KEY_WL_CAL           0x4003  // for 96x2 bytes
#define PS_KEY_WL_CAL_1         0x4003  // for 50   bytes
#define PS_KEY_WL_CAL_2         0x4004
#define PS_KEY_WL_CAL_3         0x4005
#define PS_KEY_WL_CAL_4         0x4006



#define CAL_DATA_ADDR               (USERDATA_BASE)             // to save calibration data
#define CTUNE_UD_ADDR               (USERDATA_BASE+0x100)
#define CAL_DATA_BACKUP_ADDR        (CAL_DATA_ADDR+0x100+16)    // to backup calibration data


#define ONE_LEVEL_BUFF_SIZE         (12*2)
#define ERROR_PS_KEY_TOO_LEN        0x018A      // PS data too length


void UD_Flash_Error(msc_Return_TypeDef err);



extern _PMesh_Node_Data pMeshNodeData;
extern _PAdjustValue    pAdjValue;

#define ACCESS_MESH_DATA        1
#define ACCESS_ADJUST_DATA      2

//extern Result result;


void NodeDataInit();
void MeshNodeDataReset();
void BtmG6Reset();


Result WriteNodeData();
Result ReadNodeData();
bool EraseCalibrationData();
MSC_Status_TypeDef WriteCalibrationData(uchar index,void* pBuff);
int ReadCalibrationData(uchar index,void* pBuff);
uchar GetMeshNodeData(uint16 key, PUCHAR pnode_data);
Result SetMeshNodeData(uint16 key, PUCHAR pnode_data, uchar size);
void MeshNodeSetupReset();
Result WriteMeshNodeData();
Result WriteAdjValue();


#endif //_NODE_DATA_

