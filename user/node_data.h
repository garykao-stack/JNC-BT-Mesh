
// Richard: define 2019/08/19
#ifndef _NODE_DATA_
#define _NODE_DATA_

//#define NODE_USER_PAGE       USERDATA_BASE //((uint32_t *)0x0FE00000) /**< Address of the user page */

typedef struct 
{
  word      EepromID;           // 0xAA55
  word      StructVer;          // for this structure version: 100 ==>  1.00

  word      MeshNodeBehavior;    // Client/Server/Friend/LPN
  word      Reserve01;
  
  byte      MeshNodeID;          // 0 ~ 255
  byte      NodeBehavior;       // 0.Mesh-Client, 1.Mesh-Server, 2: BLE-Master, 3.BLE-Slave, 4. BLE-Master&Slave
  byte      BaudRate;           //  default:6: 0:2400,1:4800,2:9600,3:19200,4:38400,5:57600,6:115200  
  byte      Sleeping;           // 0: sleeping diable  >1: over 1 that sleeping mode active for timer
  
  byte      DeviceKind;         //
  byte      DeviceLength;       // for device length: 24, 49, 74, and 99
  byte      TxPower;            // for BLE power
  byte      TxGain,RxGain;      // for RF distance, but CE failed
  
  byte      CTune;              // for BLE RF sensivity
  byte      TotalNodes;         // for client node
  byte      Reservel2,Reservel3;
  //byte      Reservel1,Reservel2,Reservel3,Reservel4;
  dword     Reserve[3];
} Node_Data,*PNode_Data;

#define NODE_DATA_SIZE     sizeof(Node_Data)


typedef struct
{
    word    SensorData[96];
    word    SensorDataBackup[96];
    word    SensorID1,SensorID2;    //0xAA55
}SensorData,*PSensorData;

#define PS_KEY_MESH_NODE_DATA       0x4000
#define CAL_DATA_ADDR               (USERDATA_BASE)             // to save calibration data
#define CTUNE_UD_ADDR               (USERDATA_BASE+0x100)
#define CAL_DATA_BACKUP_ADDR        (CAL_DATA_ADDR+0x100+16)    // to backup calibration data


#define ONE_LEVEL_BUFF_SIZE     (12*2)
#define ERROR_PS_KEY_TOO_LEN        0x018A      // PS data too length


static void UD_Flash_Error(msc_Return_TypeDef err);


void NodeDataInit();

int WriteNodeData(PNode_Data pNode_data);
int ReadNodeData(PNode_Data pNode_data);
bool EraseCalibrationData();
MSC_Status_TypeDef WriteCalibrationData(byte index,void* pBuff);
int ReadCalibrationData(byte index,void* pBuff);


#endif //_NODE_DATA_

