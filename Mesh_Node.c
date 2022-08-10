
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
uint16  GetInfoCycle;
uchar   UsartRxCount;   // Receive data from Rx bytes
PFunSensor pFunSensor=0;



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
void SetSleepingTimer(uchar status)
{
    uint32 sleeping_timer;
    
    if(status == ON)
        {// sleeping on
        sleeping_timer = (TIMER_SERVER_SLEEPING*1000 - TIMER_SERVER_SENS_INFO)-(uint32)GetDeviceInfoDelay;;
        dprint("sleep for sleeping_timer %d(ms)\r\n",sleeping_timer);
        SetEventTaskTimer(TD_NODE_WAKE_UP,      sleeping_timer, TIMER_EVENT_ONCE);
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
bool SetNodeSleeping(uchar status)
{
    bool ret_code=TRUE;

    if(status == ON){//Trace("Set Node Sleeping ON"); //SetNodeStatus(NS_SLEEPING,ON);
         SetSleeping(ON);
        }
    else{//Trace("Set Node Sleeping OFF"); //SetNodeStatus(NS_SLEEPING,OFF);
         SetSleeping(OFF);
        }
    
    return ret_code;
}




//
// ON: system into power saving
//      1. All timer action  2. All device power on
// OFF: system into action(Full power)
//      
//
void SetSleeping(uchar status)
{
    if(NodeRole == NR_CLIENT) return;
    SetSleepingTimer(status);
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





