// Richard: define 2019/07/11
#ifndef _BLE_COMM_
#define _BLE_COMM_
/* BG stack headers */

#include "mesh_device_properties.h"

typedef struct _RspResult_
{
    uint16  result;
}RspResult,*PRspResult;

typedef uint16      Result;    
typedef uint16*     PResult;

typedef uint8array* PEvtRespData;

typedef uint32 (*PFunEvent)(struct gecko_cmd_packet* pEvent);
typedef bool (*PFunSensor)(void);



typedef struct _EventFun_
{
    uint32      EventID;
    PFunEvent   pEventProc;
}EventFun,*PEventFun;

#define TIMER_CLK_FREQ ((uint32_t)32768) ///< Timer Frequency used
/// Convert miliseconds to timer ticks
#define TIMER_MS_2_TICKS(ms) ((TIMER_CLK_FREQ * (uint32)ms) / 1000)
/// Time equal 0 removes the scheduled timer with the same handle

#define TIMER_EVENT(ms) ((TIMER_CLK_FREQ * (ms)) / 1000)    //richard add

#define TIMER_EVENT_ONCE        1
#define TIMER_EVENT_REPEAT      0

#define TIMER_1MS           1
#define TIMER_2MS           2
#define TIMER_3MS           3
#define TIMER_4MS           4
#define TIMER_5MS           5


#define TIMER_10MS          10
#define TIMER_20MS          (TIMER_10MS*2)
#define TIMER_30MS          (TIMER_10MS*3)
#define TIMER_40MS          (TIMER_10MS*4)
#define TIMER_50MS          (TIMER_10MS*5)



#define TIMER_100MS          100
#define TIMER_200MS         (TIMER_100MS*2)
#define TIMER_300MS         (TIMER_100MS*3)
#define TIMER_400MS         (TIMER_100MS*4)
#define TIMER_500MS         (TIMER_100MS*5)
#define TIMER_1000MS        (TIMER_100MS*10)

#define TIMER_1SEC          (TIMER_100MS*10)
#define TIMER_2SEC          (TIMER_1SEC*2)
#define TIMER_3SEC          (TIMER_1SEC*3)
#define TIMER_4SEC          (TIMER_1SEC*4)
#define TIMER_5SEC          (TIMER_1SEC*5)
#define TIMER_6SEC          (TIMER_1SEC*6)
#define TIMER_7SEC          (TIMER_1SEC*7)
#define TIMER_8SEC          (TIMER_1SEC*8)

#define TIMER_10SEC         (TIMER_1SEC*10)
#define TIMER_15SEC         (TIMER_1SEC*15)
#define TIMER_20SEC         (TIMER_1SEC*20)
#define TIMER_30SEC         (TIMER_1SEC*30)
#define TIMER_60SEC         (TIMER_1SEC*60)


#define TIMER_1MIN          (TIMER_1SEC*60)
#define TIMER_2MIN          (TIMER_1MIN*2)
#define TIMER_3MIN          (TIMER_1MIN*3)
#define TIMER_4MIN          (TIMER_1MIN*4)
#define TIMER_5MIN          (TIMER_1MIN*5)

//#define TIMER_1HR           (TIMER_1MIN*60)

#define TIMER_NO_SIGNAL  TIMER_6SEC //TIMER_1MIN //(TIMER_1MIN*1)


#define TIMER_ENDING                0

#define STAGE_TIMER_COUNT       10 //10ms every time
#define WAIT_MS(x)              ((x)/STAGE_TIMER_COUNT) // x must > 10ms
#define WAIT_SEC(x)             ((1000*x)/STAGE_TIMER_COUNT)

#define TIMER_STAGE_WAITING     (TIMER_1MS*STAGE_TIMER_COUNT)
#define TIMER_MAX_WAKE_UP       TIMER_10SEC    // max wake up time 0: do not to sleeping
#define TIMER_PRE_SLEEPING      TIMER_3SEC      //prepare to sleeping  
#define TIMER_SLEEPING          TIMER_10SEC



//#define TIMER_SCAN_SERVER_WAITING   TIMER_1SEC
#define TIMER_GET_PROPERTY      (TIMER_SLEEPING+200)
#define TIMER_WAITING_PROPERTY  TIMER_2SEC

#define TIMER_CHECK_SEQ         TIMER_5SEC //check seq num
#define TIMER_IV_INDEX_ENDING   TIMER_10SEC //

#define TIMER_SYS_SETUP         TIMER_20SEC


#define TIMER_UNPROVISION       TIMER_500MS
#define TIMER_PROVISION         TIMER_500MS     

#define TIMER_IVI_UPDATE        TIMER_1SEC //TIMER_500MS





/*******************************************************************************
 * Timer handles defines.
 ******************************************************************************/

#define TD_PROCESS_OFF        0x00

#define TD_TASK_CLIENT_SCAN_SERVER   1
#define TD_TASK_GET_PROPERTY       2
#define TD_SERVER_WAKE_UP     3
#define TD_SERVER_SLEEPING    4

#define TD_HANDLE_WAITING     6

#define TD_TASK_USART_TX      7  //for USART RX Timeout
#define TD_TASK_USART_RX      8  //for USART RX Timeout
#define TD_TASK_DEVICE_TIMER  9  //for Server Power Model
#define TD_TASK_GET_CIN_VALUE 10 // to get one sensor CIN value

#define TD_RESTART            11
#define TD_FACTORY_RESET      12
#define TD_PROVISIONING       13

//#define TD_TASK_MM_CLIENT     14
//#define TD_TASK_MM_SERVER     15


//#define TD_PROXY_CONNECT      5

//#define TD_PROCESS_TASK       30

//#define TD_DEVICE_TASK        31  // for timer counter

#define TD_CHECK_DEV_NODE   14  //to check all of th node status for 10sec

#define TD_SYS_RESET          60  // for 10 ms

#define TD_GET_SENSOR_INFO    70  // for 1 sec once
#define TD_GET_SENSOR_ENDING  71

#define TD_FORCE_POWER_STATUS  72


#define TD_LED_TOGGLE         75  // for 1 sec once

#define TD_EXIT_IVUPDATE_STATE 77

#define TD_SYS_SETUP_RESET     78

#define TD_SET_MODBUS_CMD     79      // limit modbus cmd timing
// LED Status
#define TD_UNPROVISION        80
#define TD_NO_EVENT           81  // check BT event or to reset


//sleep
#define TD_NODE_SLEEP       90
#define TD_NODE_WAKE_UP     91

//Timer
#define TD_STAGE_TIMER       100


#define TIMER_PROCESS_TASK          TIMER_30MS  // 10ms
#define TIMER_DEVICE_TASK           TIMER_10MS  //TIMER_50MS  // 10ms
#define TIMER_SYS_RESET             TIMER_50MS  // 10ms

#define BT_RSSI                     0
#define COMP_TX_POWER               -50
#define COMP_RX_POWER               -50
#define TX_POWER_HI                 100 //for +10db
#define TX_POWER_MID                50  //for +50db
#define TX_POWER_LO                 0   //for +0db



// NS ==> NODE_STATUS
/// Richard: for Mesh node status ////////////////////
#define STATUS_CLIENT               BIT0  // 0: Server 1: Client
#define STATUS_SLEEPING             BIT1  // for 0:Wake up 1: Sleeping
#define STATUS_FULL_POWER           BIT2  // device can not sleeping
#define STATUS_FRIEND               BIT3
#define STATUS_LPN                  BIT4
#define STATUS_PROXY_ON             BIT5  // BLE connect from Host(App/PC)
#define STATUS_PROXY_CONNECT        BIT6  // BLE connect from Host(App/PC)
#define STATUS_BLE_CONNECT          BIT7
#define STATUS_PROVISIONED          BIT8  // 
// for Client Node
#define STATUS_SCAN_SERVER_NODE     BIT9  // 0: Scan Server Node 1: Get server property
//#define STATUS_GET_PROPERTY         BIT8  // 
#define STATUS_SPI_ENABLE           BIT10  //
#define STATUS_GET_SENSOR_ENDING    BIT11  // 
#define STATUS_SET_MODBUS_CMD       BIT12  // 



// for Server Node
#define STATUS_GET_SENSOR_INFO      BIT13  // to get all sensor data
#define STATUS_SERVER_MODBUS_TIME_OUT  BIT14  // to get all sensor data
#define STATUS_SERVER_SET_DEVICE    BIT15  //client want to set/write device.




// for ModBus & RS485
#define STATUS_USART_RX_OK          BIT16   // Receive data from RX
#define STATUS_USART_TX_OK          BIT17   // Tx Ok
#define STATUS_MOD_BUS              BIT18  // 1: ModBus cmd
#define STATUS_JNC_CMD              BIT19  // 1: JNC cmd
//#define STATUS_PROVISIONING         BIT20  // BLE connect from Host(App/PC)

#if MESH_COLUME_ENABLE

#define STATUS_MODBUS_MESH_PENDING  BIT21  // 1: RS485 to BT Mesh
#define STATUS_MODBUS_MESH          BIT22  // 1: Modbus to BT Mesh

#endif

#define STATUS_SERVER_TO_CLIENT     BIT23  // Send data to client node
#define STATUS_IVI_UPDATE           BIT24  // Enter IV index stage
#define STATUS_TEMP_HUM             BIT25  // Temp&Hum from SD(external)=0 or BT itself =1
#define STATUS_FACTORY_RESET        BIT26  // 



#define BLE_LINK_STATUS     (STATUS_PROXY_CONNECT | STATUS_BLE_CONNECT)


#define CYCLE_BT_MESH_BASE          TIMER_1SEC

#define CYCLE_SERVER_SLEEPING       CYCLE_BT_MESH_BASE
#define CYCLE_SERVER_WORKING        CYCLE_BT_MESH_BASE
#define CYCLE_CLIENT_GET_PROPERTY   (CYCLE_BT_MESH_BASE+0)  //

#define CYCLE_PROXY_CONNECT_WAITING     500 //500ms
#define CYCLE_GOTO_SLEEPING             200


#define MESH_SENSOR_MODEL_SERVER    0
#define MESH_SENSOR_MODEL_CLIENT    1
#define MESH_NODE_FRIEND            2
#define MESH_NODE_LPN               3
#define MESH_NODE                   4


#define BT_NODE_ROLE_PRE_DEF        0   // by predefine
#define BT_NODE_ROLE_SERVER         1   // change to server node
#define BT_NODE_ROLE_SETUP_SERVER   2   // 
#define BT_NODE_ROLE_CLIENT         3   // 
#define BT_NODE_ROLE_FRIEND         4   // 
#define BT_NODE_ROLE_LPN            5   // 

#define BT_NODE_BT_TEMP_HUM         6   //

///// New Firmware

#define KEY_SERVER_TO_RS485         7   //





#define KEY_FACTORY_RESET           10   // reset bt node to unprovision
#define KEY_NODE_SETUP              15   // BT Mesh enter setup model

#define BT_NODE_ROLE_OTHER          20



typedef struct
{
    uint32              seconds;
    uint16              ticks;    
}TimerRtcc,*PTimerRtcc;

#define POWER_MODE_SLEEP        0
#define POWER_MODE_WAKEUP       1


#define SAVE_INFO_GET_COLUME_REGUEST    1


typedef struct
{
    uint16  ElemIndex;
    uint16  ClientAddr;
    uint16  ServerAddr;
    uint16  AppKey;
    uint16  NetKey;
    uint16  DeviceKey;
    uint16  PropertyID;
    PUCHAR  pBuff;
    uchar   BuffSize;    
}_MeshNodeInfo,*_PMeshNodeInfo;


typedef struct _TimerEventTask1_
{   
 //   uint16  Event;    
    PTimerTask  pTimerTask; // process task pointer
    uchar   TaskStage;
    uint16  TaskTimer;        // 0: task enable, 0xFE: alway exec, 0xFF: disable, other: timer--
    uint16  TaskTimerValue;     // default value
    uint16  TaskTimeOut;
    uint16  TaskTimeOutValue;
    struct _TimerEventTask_ *pItself;
}_TimerEventTask1,*_PTimerEventTask1;


typedef struct _TimerEventTask_
{   
    PTimerTask  pTimerTask; // process task pointer
    uchar   TaskStage;
//    uint16  TaskTimer;        // 0: task enable, 0xFE: alway exec, 0xFF: disable, other: timer--
//    uint16  TaskTimerValue;     // default value
    uint16  TaskTimeOut;
    uint16  TaskTimeOutValue;
    struct _TimerEventTask_ *pItself;
}_TimerEventTask,*_PTimerEventTask;




#define DEVICE_TASK_ON               0x0000
#define DEVICE_TASK_ALWAY_ON         0xFFFE
#define DEVICE_TASK_OFF              0xFFFF

#define TIMER_TASK_DEVICE           2   //20ms


#define TIMER_EVENT_FREE            0x0000
 
#define TIMER_EVENT_DEVICE_TASK         BIT0    // First
#define TIMER_EVENT_GET_CIN_VALUE       BIT1
#define TIMER_EVENT_WAKE_UP             BIT2    //1: wakeup 0:Sleeping
//#define TIMER_EVENT_SERVER_SLEEPING     BIT3
#define TIMER_EVENT_USART_TX            BIT4
#define TIMER_EVENT_USART_RX            BIT5
#define TIMER_EVENT_SCAN_SERVER         BIT6
#define TIMER_EVENT_GET_PROPERTY        BIT7
#define TIMER_EVENT_8                   BIT8
#define TIMER_EVENT_9                   BIT9
#define TIMER_EVENT_10                  BIT10
#define TIMER_EVENT_11                  BIT11
#define TIMER_EVENT_12                  BIT12
#define TIMER_EVENT_13                  BIT13
#define TIMER_EVENT_14                  BIT14
#define TIMER_EVENT_OTHER_PROC          BIT15

#define TIMER_EVENT_NUM                 (sizeof_array(TimerEventTask)-1) //16

#define DEVICE_TASK_ACTIVE              1
#define DEVICE_TASK_TIMER_DEF           2
#define DEVICE_TASK_TIMEOUT_DEF         3
#define DEVICE_TASK_STAGE               4



extern uint8_t init_done;
extern uint8_t registered_devices;
extern uint16 CountNodeEvent;

#define TASK_TIME_OUT           (TIMER_1SEC)
#define TASK_TIME_OUT_1         (TIMER_1SEC/TIMER_DEVICE_TASK)
#define TASK_TIME_OUT_2         (TIMER_2SEC/TIMER_DEVICE_TASK)
#define TASK_TIME_OUT_3         (TIMER_3SEC/TIMER_DEVICE_TASK)
#define TASK_TIME_OUT_4         (TIMER_4SEC/TIMER_DEVICE_TASK)
#define TASK_TIME_OUT_8         (TIMER_8SEC/TIMER_DEVICE_TASK)


#define TASK_TIMER              (TIMER_100MS)
#define TASK_TIMER_1            (TASK_TIMER/TIMER_DEVICE_TASK)
#define TASK_TIMER_2            (TASK_TIMER/TIMER_DEVICE_TASK)
#define TASK_TIMER_3            (TASK_TIMER/TIMER_DEVICE_TASK)
#define TASK_TIMER_4            (TASK_TIMER/TIMER_DEVICE_TASK)

#define TASK_TIMER_WAKEUP       (TIMER_5SEC/TIMER_DEVICE_TASK)
#define TASK_TIMER_SLEEPING     (TIMER_1SEC/TIMER_DEVICE_TASK)


#define DEVICE_TASK_ACTIVE_MS       2//10    //10ms
#define DEVICE_TASK_ACTIVE_TICKS    TIMER_MS_2_TICKS(DEVICE_TASK_ACTIVE_MS)    //10ms



extern uint16  TimerEvent;
extern _PTimerEventTask pDeviceTask;
extern Result  result;
extern PRspResult pResult;
extern _MeshNodeInfo MeshNodeInfo;
extern uint16  GetPropertyID;

void BleCommInit();

uint32 EvtSysExternalSignalProc(PCmdPacket pEvent);
uint32 EvtSoftTimerProc(PCmdPacket pEvent);
void   EvtSoftTimerMsSecProc(uchar handle);
void   EvtSoftTimerClientProc(uchar handle);
void   EvtSoftTimerServerProc(uchar handle);

void initiate_factory_reset(void);
void set_device_name(bd_addr *pAddr);
uchar GetButtonStatus();
void SetTxPower(int16 power);
void PowerMode(uchar status);

Result SetEventTaskTimer(uchar event,uint32 timer,uchar single_shot);
bool SetDevicePowerModel(uchar power);

void PowerTimerProc(void);
void PowerModelProc(void);
void ClientGetPropertyProc(void);
void TimerIdOtherProc(void);
void SaveMeshNodeInfo(uchar save_id,void* p_event);


void  SetTimerTaskStatus(uchar task_id,uchar status);
uchar GetTimerTaskIndexStatus(uchar task_id);
void  SetTimerTaskCounter(uchar task_id, uint16 counter);
uint16 GetTimerTaskIndexCounter(uchar task_id);

void DeviceTaskProc();
void CheckTaskCounter();
_PTimerEventTask GetTimerEventTask(PTimerTask p_timer_task);
void SetDeviceTask(PTimerTask p_timer_task, uint16 set_item,uint16 value);
void SetTimerTaskEvent(uint16 event, uchar status);
bool GetTimerTaskEvent(uint16 event);
void SetTaskTimeOut(PTimerTask p_task);
void SetCurrTaskTimeOut();

void SetTaskStage(PTimerTask p_task,uchar stage);
void SetTaskWork(PTimerTask p_task,uint16 status);
void DeviceWakeUp();
void DeviceSleeping();
void DeviceTaskInit();
void GetTimerRtcc(PTimerRtcc p_timer);
void ShowTimerRTC();
bool CheckDeviceTaskActive();
bool CheckDeviceTaskActive1ms();

_PTimerEventTask GetDeviceTaskTbl();
void TimerLedStatus();
void ResetEventCounter(uchar event);
void SetForceFullPowerTime(uchar status);












#endif //_BLE_COMM_

