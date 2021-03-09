/*
 *
 *  Created on: 2020/02/10
 *  Author: Richard
 */

#ifndef _CMD_TO_BT_MESH_
#define _CMD_TO_BT_MESH_

#define FOR_NEW_BT_CMD      1

#define TaskTiimerToCount(x) (x/TIMER_DEVICE_TASK)  // base 10ms

#define ToNextTaskStage(stage)  pDeviceTask->TaskStage = stage
#define ToDefaultWating(id)     {ToNextTaskStage(id); pDeviceTask->TaskTimeOut = pDeviceTask->TaskTimeOutValue;}
#define ToUserWaiting(id,timer) {ToNextTaskStage(id); pDeviceTask->TaskTimeOut = timer;}
#define CheckTaskTimeOut()      (pDeviceTask->TaskTimeOut)
#define CurrTaskStage()         (pDeviceTask->TaskStage)
#define TaskActive()            //(pDeviceTask->TaskTimer = DEVICE_TASK_ON)   // current task active
#define TaskAlwayOn()           // (pDeviceTask->TaskTimer = DEVICE_TASK_ALWAY_ON)   // current task active


// MM ==> MODBUS_MESH
#define MM_PENDING             1   // process pending
#define MM_USART_RX            2   // modbus protocol active
#define MM_MESH_TX             3   // send data to server
#define MM_MESH_RX_WAITING     4   // waiting server node response
#define MM_MESH_RX_OK          5   // receive data from server node
#define MM_USART_TX            6   // send data to host
#define MM_USART_TX_WAITING    7   // send data to host
#define MM_ENDING              8   // process complete
#define MM_USART_RX_WAITING    10   // receive data from server node
#define MM_WAITING_ERROR       20   // waiting server node response


//for FC4
#define MODBUS_FC4_GET_REG0     {0x01, 0x04, 0x00, 0x00, 0x00, 0x01, 0x31, 0xCA}
#define MODBUS_FC4_GET_REG1     {0x01, 0x04, 0x00, 0x01, 0x00, 0x01, 0x60, 0x0A}
#define MODBUS_FC4_GET_REG2     {0x01, 0x04, 0x00, 0x02, 0x00, 0x01, 0x90, 0x0A}
#define MODBUS_FC4_GET_REG3     {0x01, 0x04, 0x00, 0x03, 0x00, 0x01, 0xC1, 0xCA}
#define MODBUS_FC4_GET_REG4     {0x01, 0x04, 0x00, 0x04, 0x00, 0x01, 0x70, 0x0B}
#define MODBUS_FC4_GET_REG5     {0x01, 0x04, 0x00, 0x05, 0x00, 0x01, 0x21, 0xCB}
#define MODBUS_FC4_GET_REG6     {0x01, 0x04, 0x00, 0x06, 0x00, 0x01, 0xD1, 0xCB}
#define MODBUS_FC4_GET_REG7     {0x01, 0x04, 0x00, 0x07, 0x00, 0x01, 0x80, 0x0B}
#define MODBUS_FC4_GET_REG8     {0x01, 0x04, 0x00, 0x08, 0x00, 0x01, 0xB0, 0x08}
#define MODBUS_FC4_GET_REG9     {0x01, 0x04, 0x00, 0x09, 0x00, 0x01, 0xE1, 0xC8}
#define MODBUS_FC4_GET_REG60    {0x01, 0x04, 0x00, 0x60, 0x00, 0x01, 0x31, 0xD4}

#define MODBUS_FC4_GET_REG0_8   {0x01, 0x04, 0x00, 0x00, 0x00, 0x09, 0x30, 0x0C}
//for FC1
#define MODBUS_FC1_GET_REG1     {0x01, 0x01, 0x00, 0x01, 0x00, 0x01, 0xAC, 0x0A}
#define MODBUS_FC1_GET_REG7     {0x01, 0x01, 0x00, 0x07, 0x00, 0x01, 0x4C, 0x0B}

/// for New Modbus Command Protocol

#define MOBUS_GET_REGS_0        {0x01, 0x04, 0x00, 0x00, 0x00, 0x0C, 0xF0, 0x0F}// 0x000 ~ 0x00C
#define MOBUS_GET_REGS_1        {0x01, 0x04, 0x02, 0xFE, 0x00, 0x0C, 0x90, 0x47}// 0x2FE ~ 0x309
#define MOBUS_GET_REGS_2        {0x01, 0x04, 0x03, 0x0A, 0x00, 0x0C, 0xD0, 0x49}// 0x30A ~ 0x315



#pragma pack(push)
#pragma pack(1)     //mapping to one byte

#define MODBUS_CMD_MAX      2 //3 //4
#define MODBUS_CMD_SIZE     8
#define REGS_ALL_NUM_MAX    12
#define FC4_REGS_START_REG  0x2FE
typedef struct  // for modbus register array
{
    //   uchar   MeshNodeAddr;
    //   uchar   ModbusID;
    uint16  RegFC4_REGS_0[REGS_ALL_NUM_MAX];   //0x00 ~ 0x0C
    uint16  RegFC4_REGS_1[REGS_ALL_NUM_MAX];   //0x2FE ~ 0x309
    uint16  RegFC4_REGS_2[REGS_ALL_NUM_MAX];   //0x30A ~ 0x315

    /*
    uint16  RegFc4_0_8[9];  // reg0 ~ reg8  for 8 kinds sensor
    uint16  RegFc4_60;
    uint16  RegFc1_1;
    uint16  RegFc1_7;
    */
} _ModbusRegs, *_PModbusRegs;


typedef struct
{
    uchar   MeshNodeAddr;
    uchar   ModbusID;
    uint16  DevModbusRegs[MODBUS_CMD_MAX][REGS_ALL_NUM_MAX];
    uint16  DevInfoCount;   // Mesh Node response counter: 200703
} _ClientModbusRegs, *_PClientModbusRegs;


#define SEND_DATA_INDEX_0           0   //0x000 ~ 0x005
#define SEND_DATA_INDEX_1           1   //0x006 ~ 0x009
#define SEND_DATA_INDEX_2           2   //0x2FE ~ 0x303
#define SEND_DATA_INDEX_3           3   //0x304 ~ 0x309
#define SEND_DATA_INDEX_4           4   //0x30A ~ 0x30F
#define SEND_DATA_INDEX_5           5   //0x310 ~ 0x315


#define SEND_DATA_INDEX_START       SEND_DATA_INDEX_2

#define SEND_DATA_INDEX_ENDING      SEND_DATA_INDEX_3 //SEND_DATA_INDEX_3 //SEND_DATA_INDEX_2

#define To_CLIENT_REGS_NUM_MAX      6
#define MODBUS_REG_CLEAN_NUM        100


#define UPDATE_REAL_TIME        0


typedef struct
{
    uchar   MeshNodeAddr;
    uchar   GetRegsStatus;
    uint16  ModbusRegs[To_CLIENT_REGS_NUM_MAX];

} _ToClientRegs, *_PToClientRegs;


typedef struct
{
    uint16 ProperityID;
    uchar  PacketSize;
    _ToClientRegs ToClientRegs;
} _SensorRegsData, *_PSensorRegsData;






typedef struct
{
    uchar   ModbusID;
    uchar   FunCode;
    uint16  Register;
    uint16  RegNum;
    uint16  ModbusCrc;
} _ModbusCmd, *_PModbusCmd;


#define TO_HOST_REGS_MAX       16

typedef struct
{
    uchar   ModbusID;
    uchar   FunCode;
    uchar   ByteNum;
    uint16  Data[TO_HOST_REGS_MAX + 2];
    uint16  ModbusCrc;
} _ModbusToHostPack;


typedef struct
{
    uchar ModbusID;
    uchar Fun;
    uchar Len;
    uint16 Regs[REGS_ALL_NUM_MAX];
} SimModbusRegs;

#pragma pack(pop)



// SGS: Sever Get sensor
#define SGS_TASK_PENDING            1   // Time-out to get all registers
#define SGS_SEND_CMD                2   // send modbus command  (Tx)
#define SGS_TX_WAITING              3   // waiting Tx  (Tx)
#define SGS_RX_WAITING              4   // waiting Rx  (Tx)
#define SGS_UPDTAE_REGS_VALUE       5   // update register value
#define SGS_TO_NEXT_CMD             6   // send next cmd to other registers
#define SGS_ENDING                  7   // ending for next time-out
#define SGS_ERR_TX                  8   // Tx error
#define SGS_ERR_RX                  9   // Rx error
#define SGS_WAITING_ENDING          10
#define SGS_RX_WAITING_TIME_OUT     11
#define SGS_CLEAN_MODBUS_REGS       12
#define SGS_TX_RANDOM_WAITING       13  // for random waiting
#define SGS_TX_SEND_WAITING         14  // for random waiting
#define SGS_TO_NEXT_CMD_DELAY       15   // send next cmd to other registers







#define CLIENT_HOST_PENDING         1
#define CLIENT_HOST_PREPARE         2   //
#define CLIENT_HOST_SEND_DATA       3
#define CLIENT_HOST_ENDING          4


#define SERVER_SETUP_PENDING        1
#define SERVER_SETUP_TO_CLIENT      2
#define SERVER_SETUP_ENDING         3




#define SGS_TIMER_PENDING           10
#define DEV_INFO_COUNT              15


#define NODE_TBL_POS_ERR            0xFF
#define TIMER_CLIENT_GET_SENSOR_INFO    TIMER_5SEC //TIMER_10SEC //TIMER_20SEC   //TIMER_5SEC(Test-crash)

//TIMER_5SEC //TIMER_30SEC //TIMER_3SEC //TIMER_10SEC //TIMER_1MIN
#define TIMER_SERVER_GET_SENSOR_INFO    TIMER_2SEC //(TIMER_CLIENT_GET_SENSOR_INFO-TIMER_2SEC) //TIMER_2SEC

#define TIMER_SERVER_RESET          0  //
#define TIMER_CKECK_DEV_INFO        (TIMER_CLIENT_GET_SENSOR_INFO+TIMER_2SEC) //TIMER_10SEC

extern uchar AllNodeEventNum;

void CmdToBtMeshInit();
void MeshNodeToReset(uint32 timer);

void ClientGetSensorDataProc();
void ClientSendDataToHostProc();
void ClientUpdateModbusRegs(uint16 server_adrr, PUCHAR pbuff, uchar len);
bool ClientHostRequest(void);
bool ClientHostSendData();
bool ClientHostSendData(void);
bool ClientHostPrepare();
void ClientModbusCmdInit();
Result ClientGetServerReg();
void ClientShowAllNodes();
_PClientModbusRegs ClientGetNodePos(uchar node_id);





void ServerGetSensorDataProc();
Bool ServerSendDataToClient(PCmdPacket pCmdEvent, uint status);
bool ServerPubModbusRegs(void);
void ServerModbusCmdInit();
void ServerToClientProc();
void ServerGetReguset(PCmdPacket pCmdEvent);
Bool ServerModbusRegsToClient(uchar status);
void GetWaitingTickValue(void);
bool GetModbusValue(uchar modbus_id, uint16 modbus_reg, uchar reg_num, PUINT16 pbuff);
bool ClientSendDataToServer(_PModbusCmd pCmd);
void ServerGetRegusetToClient(PCmdPacket pCmdEvent);
void ServerSetDevice(msg_ms_setup_server_set_setting_request_evt *pEvent);
void DevG6sStatusToClient(void);

void ServerSetupProc();
uchar CheckNodeActionStatus();

#endif  //_CMD_TO_BT_MESH_

