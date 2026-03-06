
#include "global.h"
#include "init_board.h"
//richard Add
/* BG stack headers */
#include "sensor_server.h"
#include "mesh_event.h"
#include "bus_usart.h"
#include "bus_rs485.h"
#include "bus_I2C.h"
#include "jnc_cmd.h"
#include "com_port.h"
#include "ivi_features.h"
#include "MeshFeatures.h"
#include "cmd_to_bt_mesh.h"
#include <stdbool.h>
#include "ble_comm.h"

#include "people_count_sensor.h"
#include "mesh_sensor.h"
#include "Mesh_Node.h"

#ifdef BTM_A308
#include "A308_Server.h"
#endif


uchar   CountErr;
uint16  NodeStage;
uint32  NodeStatus=0;
uchar   NodeRole;       // 0: server, 1: client, 2:Friend, 3: LPN
_NodeEventInfo  NodeEventInfo;
PNodeEventInfo  pNodeEventInfo;
//uint16  GetInfoCycle=WAIT_SEC(TIMER_GET_INFO_SLEEPING);
uint32  GetInfoCycle;
uchar   UsartRxCount;   // Receive data from Rx bytes
PFunSensor pFunSensor=0;
uint32 keepAliveBeforeSleepMs=0;


NodeStageInfo NodeStageInfoTbl[NODE_STAGE_INFO_NUM]=
{
    {NODE_STAGE_INIT,0}, {NODE_STAGE_INIT,0}, {NODE_STAGE_INIT,0}, {NODE_STAGE_INIT,0},
    {NODE_STAGE_INIT,0}, {NODE_STAGE_INIT,0}, {NODE_STAGE_INIT,0}, {NODE_STAGE_INIT,0}
};

//
//
//
void CheckNodeTimerCount()
{
    uchar loop;
    PNodeStageInfo p_node = NodeStageInfoTbl;

    for(loop=0; loop< NODE_STAGE_INFO_NUM; loop++)
    {
        if(p_node->Timer) p_node->Timer--;
        p_node++;
    }
}




uint16  NodeWaitingTimer,GetSensorTimer;
PNodeStageInfo pStageInfo;


void MeshNodeInit()
{
    SetWaitTimer(0);
    NodeStatus = 0;
    pNodeEventInfo = &NodeEventInfo;
    SetEventTaskTimer(TD_STAGE_TIMER,  TIMER_STAGE_WAITING,TIMER_EVENT_REPEAT);
#ifdef BTM_A308
    A308_Initialize();
#endif

    if(NodeRole == NR_SETUP)BtMeshSetupInit();
    else if(NodeRole == NR_CLIENT) {printf("Node Role: Client\r\n");ClientNodeInit();}
    else if(NodeRole == NR_SERVER) {printf("Node Role: Server\r\n");ServerNodeInit();}
    else if(NodeRole == NR_SETUP_SERVER) {printf("Node Role: Setup Server\r\n");ServerSetupNodeInit();}
    else {ServerNodeInit();} //default server node
}




PNodeStageInfo GetNodeStageInfo(uchar value)
{
    return &NodeStageInfoTbl[value];

}

void DebugClinetNodeStage(uint32 v)
{
    dprint("Ct: %d =>%4x,%4x,%4x \r\n",v,NodeStageInfoTbl[1].Stage,NodeStageInfoTbl[2].Stage,NodeStageInfoTbl[4].Stage);
}
void DebugServerNodeStage(uint32 v)
{
    dprint("St: %d =>%4x,%4x,%4x \r\n",v,NodeStageInfoTbl[1].Stage,NodeStageInfoTbl[2].Stage,NodeStageInfoTbl[3].Stage);
}

void SetMeshNodeStage(uint16 stage)
{
   pStageInfo->Timer = stage; 
}

//
// set up waiting timer
//
void SetWaitTimer(uint16 timer)
{
    pStageInfo->Timer = timer;
//    NodeWaitingTimer = timer;
}


//
// TRUE ==> time out
//
bool CheckWaitTimeOut()
{
    bool ret_code = FALSE;
    if(!pStageInfo->Timer) ret_code = TRUE;
    return ret_code;    
}

/**
 * @brief 設置或檢查預讀計時器 (pre read timer)。
 * 預先讀取時間內，每秒讀取一筆 sensor 資料，增加讀值穩定。
 * @param bReset 重置 pre read timer
 * @param time bReset true 時，設置 pre read 總時間 (N*10 ms)
 * @param communicate_time bReset true 時，設置通訊耗時 (N*10 ms)
 * @return 是否該執行 pre read
 */
bool CheckPreReadTimer(bool bReset, uint16 time, uint16 communicate_time)
{
    static uint16 t = 0;
    static uint16 communicate_t = 0;
    // 初始化 pre read timer
    if (bReset) {
        t = time;
        communicate_t = communicate_time;
        return true;
    }

    // 剩不到 1.5 秒就不執行 pre read
    if (ActiveWaiting() <= 150)
        return false;

    bool ret_code = ActiveWaiting() < (t + 100 - communicate_t);
    if (ret_code) {
    if (t > 100) // 100: 1000ms
        t -= (100 - communicate_t);
    else
        t = 0;
    }
    return ret_code;
}

//
// set up node status
//
void SetNodeStatus(uint32 status, uchar on_off) 
{ 
    if(on_off == ON) NodeStatus |= status;
    else NodeStatus &= ~status;
}

//
// return ON/OFF
//
bool GetNodeStatus(uint32 status)
{
    bool ret_code = OFF;
    if(NodeStatus & status) ret_code = ON;
    return ret_code;
}


#define NODE_TIMER_SLEEPING             5000 //10000   //xxx-ms
#define NODE_TIMER_WAKE_UP              10000 //5000 //10000   //xxx-ms




//
//
//
void SetSleepingTimer(uchar status, int32 sleep_ms)
{
    //int32 sleeping_timer;
    
    if(status == ON)
        {// sleeping on
        /*sleeping_timer = (TIMER_SERVER_SLEEPING*1000-keepAliveBeforeSleepMs - TIMER_SERVER_SENS_INFO)-((uint32)GetDeviceInfoDelay+PreReadDelay+syncTime)*10;
        if (syncTime > 0)
            dprint("sync time %d(ms)\r\n",syncTime * 10);
        syncTime = 0;*/
        if (sleep_ms<0)sleep_ms=1000; /*當休眠時間<0,至少休眠一秒，如GetDeviceInfoDelay未變更，應該不會發生此狀況*/
        dprint("sleep for sleeping_timer %d(ms)\r\n",sleep_ms);
        SetEventTaskTimer(TD_NODE_WAKE_UP,      sleep_ms, TIMER_EVENT_ONCE);
        SetEventTaskTimer(TD_NO_EVENT,          TIMER_ENDING, TIMER_EVENT_ONCE); 
        SetEventTaskTimer(TD_GET_SENSOR_INFO,   TIMER_ENDING, TIMER_EVENT_ONCE);
        SetEventTaskTimer(TD_STAGE_TIMER,       TIMER_ENDING, TIMER_EVENT_ONCE);
        }
    else
        {// wake up
        SetEventTaskTimer(TD_STAGE_TIMER,       TIMER_STAGE_WAITING,TIMER_EVENT_REPEAT);
        if(NodeRole == NR_CLIENT)
            SetEventTaskTimer(TD_GET_SENSOR_INFO, TIMER_CLIENT_GET_SENSOR_INFO, TIMER_EVENT_ONCE);
        else
            SetEventTaskTimer(TD_GET_SENSOR_INFO, TIMER_SERVER_GET_SENSOR_INFO, TIMER_EVENT_ONCE);
        }
}


//
// set up node sleeping on/off
//
bool SetNodeSleeping(uchar status, int32 sleep_ms)
{
    bool ret_code=TRUE;

    if(status == ON){//Trace("Set Node Sleeping ON"); //SetNodeStatus(NS_SLEEPING,ON);
         SetSleeping(ON, sleep_ms);
        }
    else{//Trace("Set Node Sleeping OFF"); //SetNodeStatus(NS_SLEEPING,OFF);
         SetSleeping(OFF,sleep_ms);
        }
    
    return ret_code;
}



extern uint32 BootingSeconds;
//
// ON: system into power saving
//      1. All timer action  2. All device power on
// OFF: system into action(Full power)
//      
//
void SetSleeping(uchar status, int32 sleep_ms)
{
    //if(NodeRole == NR_CLIENT) return;
    SetSleepingTimer(status,sleep_ms);
    if(status == ON)
      { //sleeping
      //Trace("SetSleeping ON");
    	dprint("************ Enter Sleep Mode ************\r\n");
        SetMeshNodeStatus(STATUS_SLEEPING,ON);
        SetNodeStatus(NS_SLEEPING,ON);
        SystemPower(OFF); 
        NodeLpn(ON); NodeProxy(OFF);  NodeBeacon(OFF);
        SetLedStatus(LED_STATUS_SLEEP);
      }
    else
      { //wake up
      //Trace("SetSleeping OFF");
    	BootingSeconds=0;
        NodeLpn(OFF); NodeProxy(ON); NodeBeacon(ON);
        SetMeshNodeStatus(STATUS_SLEEPING,OFF);
        SetNodeStatus(NS_SLEEPING,OFF);
        SystemPower(ON);
        SetLedStatus(LED_STATUS_ACTIVE);
        if(GetNodeStatus(NS_SERVER_RS485_ENABLE) == ON)  SetLedStatus(LED_STATUS_SERVER_TO_RS485);
        UsartResetRxTx(USART_ID_TX_RX);
      }
    
}

#define BTM_MODE_ID             0x1100
#define BTM_PBULIC_TIME_ON      200     //200ms
#define BTM_PBULIC_TIME_OFF     0     //200ms

//
//
//
void SetNodePublish(uchar status)
{
    struct gecko_msg_mesh_test_get_local_model_pub_rsp_t* pEvent;
    uint8 public_timer=0;    
    pEvent = Cmd_mt_get_local_model_pub(0,0xFFFF,BTM_MODE_ID);             
    if(status == ON) {public_timer = BTM_PBULIC_TIME_ON;}
    Cmd_mt_set_local_model_pub(0,pEvent->appkey_index,0xFFFF,BTM_MODE_ID,pEvent->pub_address,pEvent->ttl,public_timer,    // 200ms
                               pEvent->retrans,pEvent->credentials);
}





