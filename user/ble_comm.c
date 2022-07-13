#include "global.h" 
#include "graphics.h"
#include "sleep.h"
#include "leds.h"
#include "sensor_client.h"
#include "bus_usart.h"
#include "bus_spi.h"
#include "MeshFeatures.h" 
#include "modbus_to_mesh.h"
#include "cmd_to_bt_mesh.h"
#include "sensor_server.h"
#include "node_data.h"
#include "ivi_features.h"
#include "Mesh_node.h"
#include "G6_BT_Mesh.h"
#include "bus_rs485.h"
#include "ble_comm.h"

#ifdef BTM_A308
#include "A308_Server.h"
#endif

//PRspResult  pResult;
Result result;
_MeshNodeInfo MeshNodeInfo;
_PTimerEventTask pDeviceTask;
extern _TimerEventTask DeviceTaskTbl[];
extern void ClientTimer_10ms();
extern void Modbus_Timer();

uchar PowerKeyCount;


void BleCommInit()
{
    uchar buttton_status;
    MeshNodeStatus = 0;
    //NodeDataInit();
    buttton_status = GetButtonStatus();
    if( buttton_status == KEY_FACTORY_RESET) 
        {initiate_factory_reset();SetMeshNodeStatus(STATUS_FACTORY_RESET, ON);return;}    
    NodeRole = pMeshNodeData->MeshNodeRole; 
   if(pMeshNodeData->WorkingTimer < 5 || pMeshNodeData->WorkingTimer > (3600) ) 
        {pMeshNodeData->WorkingTimer  = TIMER_DEFAULT_WORKING;  WriteNodeData();}
   

#if BTM_FOR_DEMO_TEST
   pMeshNodeData->WorkingTimer  = TIMER_DEMO_WORKING; //richard demo only
   TraceDec1("Working Timer ==> Demo", pMeshNodeData->WorkingTimer);
#else
//  TraceDec1("Working Timer", pMeshNodeData->WorkingTimer);  
#endif    
    
     
    
    if(buttton_status == KEY_NODE_SETUP) {Trace("KEY_NODE_SETUP 1");
        NodeRole = NR_SETUP;
    } 
    else enable_button_interrupts(); // Init Key pad function
    MeshNodeInit();
    SetMeshNodeStatus(STATUS_FULL_POWER, ON);   // client node must full power
#if MESH_COLUME_ENABLE
    SetMeshNodeStatus(STATUS_MODBUS_MESH,ON);   // for modbus to bt mesh    Debug
#endif 
    //PowerKeyCount=0xFF;
    PowerKeyCount = pMeshNodeData->G6HostPPercent; TraceDec1("PowerKeyCount",PowerKeyCount);


}

//**********************************************************************************************
// return button status
// 0: server node(default), 1: client node
//**********************************************************************************************
uchar GetButtonStatus()
{
    uint button0,button1;
    uchar ret_code;
    button0 = GPIO_PinInGet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN);
    button1 = GPIO_PinInGet(BSP_BUTTON0_PORT, BSP_BUTTON1_PIN);
    
    if(button0 == LOW & button1 == HIGH)        ret_code = KEY_NODE_SETUP;
    else if(button0 == LOW & button1 == LOW)    ret_code = KEY_FACTORY_RESET;
    else ret_code = BT_NODE_ROLE_PRE_DEF;       //default server node
    return ret_code;
}
void ServerNodeTask();
    
//*************************************************************************************
// process Device Task
//*************************************************************************************
void DeviceTaskProc()
{
    _PTimerEventTask p_task_temp;        
    CheckTaskCounter();
    pDeviceTask = pDeviceTask->pItself;

    while(pDeviceTask->pTimerTask)
        {   
            if(GetMeshNodeStatus(STATUS_IVI_UPDATE) == OFF)
                pDeviceTask->pTimerTask();  
            else{
                 if(pDeviceTask->pTimerTask == ClientIviUpdateProc || 
                    pDeviceTask->pTimerTask == ServerSetupProc )
                    pDeviceTask->pTimerTask();                         
                }
            pDeviceTask++;
        };
}

 
//
// Every 10ms active one time
//
static uint16 PrevTicks;
bool CheckDeviceTaskActive()
{
    uint16 temp_tick_num, curr_tick;
    curr_tick = Cmd_hardware_get_time()->ticks;

    if(curr_tick > PrevTicks ) temp_tick_num = curr_tick - PrevTicks;
    else temp_tick_num = (32768 - PrevTicks) + curr_tick;
    
    if(temp_tick_num < DEVICE_TASK_ACTIVE_TICKS) return FALSE;
    else {PrevTicks =curr_tick;return TRUE;}
    
}

#define DEVICE_TASK_ACTIVE_TICKS_1MS    TIMER_MS_2_TICKS(1)    //10ms

static uint16 PrevTicks1ms;
bool CheckDeviceTaskActive1ms()
{
    uint16 temp_tick_num, curr_tick;
    curr_tick = Cmd_hardware_get_time()->ticks;

    if(curr_tick > PrevTicks1ms ) temp_tick_num = curr_tick - PrevTicks1ms;
    else temp_tick_num = (32768 - PrevTicks) + curr_tick;
    
    if(temp_tick_num < DEVICE_TASK_ACTIVE_TICKS_1MS) return FALSE;
    else {PrevTicks1ms =curr_tick;return TRUE;}
    
}



void SetTimerTaskEvent(uint16 event, uchar status)
{
    if(status == ON)  TimerEvent |= event;
    else TimerEvent &= ~event;
        
}

bool GetTimerTaskEvent(uint16 event)
{
    if(TimerEvent & event) return TRUE;
    else return FALSE;
}



//
// update time-out default value
//
void SetTaskTimeOut(PTimerTask p_task)
{
    _PTimerEventTask p_devic_task;
    p_devic_task = GetTimerEventTask(p_task);
    if(p_devic_task) p_devic_task->TaskTimeOut = p_devic_task->TaskTimeOutValue;
    else TraceErr("SetDeviceTaskTimeOutValue");
}

//
// update time-out default value
//
void SetCurrTaskTimeOut()
{
    pDeviceTask->TaskTimeOut = pDeviceTask->TaskTimeOutValue;
}


//
// set task new stage
//
void SetTaskStage(PTimerTask p_task,uchar stage)
{
    _PTimerEventTask p_devic_task;
    p_devic_task = GetTimerEventTask(p_task);
    if(p_devic_task) p_devic_task->TaskStage = stage;
    else TraceErr("SetTaskStage");
}

//
// set task ON/OFF/Alway On
//
void SetTaskWork(PTimerTask p_task,uint16 status)
{
    return;
}

void CheckStageTimer()
{
    CheckNodeTimerCount();
    ClientTimer_10ms();
    Modbus_Timer();
#ifdef BTM_A308
    A308_TimeEvent();
#endif
}

extern uint16 ActMotoSpeed;
uint16  TimerEvent=0;
uchar   CurrTimerHandle;
uint16  ForcePowerCounter;
uint16  FilterCounter,TimerFilter;

#define FILTER_TIME_BASE      (6000) //(1000) //(6000)   //60*100


const uchar PowerKeyMap[7]={1,3,4,0,MENUAL_KEY_AUTO,1,2};


void BtMeshReset();

//**********************************************************************************************
// Event: gecko_evt_hardware_soft_timer_id
//
//**********************************************************************************************
uint32 EvtSoftTimerProc(PCmdPacket pEvent)
{ //
    uint32 ret_code=TRUE;
    uchar    loop;
    CurrTimerHandle = pEvent->data.evt_hardware_soft_timer.handle;
   // TraceDec1("CurrTimerHandle", CurrTimerHandle);
   // return ret_code;
    switch (CurrTimerHandle) 
    {
        case TD_STAGE_TIMER: //Trace("TD_STAGE_TIMER");
            CheckStageTimer();
            break;
        case TD_NODE_SLEEP: //Trace("TD_NODE_SLEEP");
            SetNodeSleeping(ON);
            break;
        case TD_NODE_WAKE_UP:// Trace("TD_NODE_WAKE_UP");
            SetNodeSleeping(OFF); 
            dprint("\r\n***** Wake UP by Timer *****\r\n");
            break;
//////////////////////////////////// for Client Timer event ///////////////////////////////////////      
        case TD_TASK_CLIENT_SCAN_SERVER: //Trace("TD_TASK_CLIENT_SCAN_SERVER");
            //TimerEvent |= TIMER_EVENT_SCAN_SERVER;
            //ClientScanServerProc();
            break;
        case TD_TASK_GET_PROPERTY: //Trace("TD_TASK_GET_PROPERTY");
            //TimerEvent |= TIMER_EVENT_GET_PROPERTY;
            //ClientGetServerDataProc();
            break;
        case TD_SYS_RESET: //Trace("System Reset"); 
            Delay_ms(200);
            if(!GetMeshNodeStatus(STATUS_BLE_CONNECT)) BtMeshReset();
            break;
        case TD_GET_SENSOR_INFO: //Trace("TD_GET_SENSOR_INFO");
            SetMeshNodeStatus(STATUS_GET_SENSOR_INFO,ON);
            break;        
        case TD_GET_SENSOR_ENDING: //Trace("TD_GET_SENSOR_ENDING");
            SetMeshNodeStatus(STATUS_GET_SENSOR_ENDING,OFF);
            break;
        case TD_SYS_SETUP_RESET://Trace("TD_SETUP_RESET");
            Cmd_sys_reset(0);
            break;
        case TD_SET_MODBUS_CMD: //Trace("TD_SET_MODBUS_CMD"); // 2 sec
            SetMeshNodeStatus(STATUS_SET_MODBUS_CMD,OFF);
            break;
        case TD_CHECK_DEV_NODE: 
           // CheckNodeActionStatus();
            break;
        case TD_FORCE_POWER_STATUS: //Trace("TD_FORCE_POWER_STATUS"); // 2 sec
            if(ForcePowerCounter == OFF){
                SetForceFullPowerTime(OFF);                
                }
            else {ForcePowerCounter--; SetLedToggle(LED_BLUE);}
                
            break;
        case TD_NO_EVENT:
        case TD_UNPROVISION:
        case TD_PROVISIONING:
            TimerLedStatus();
            break;
//////////////////////////////////// for server Timer event ///////////////////////////////////////
        case TD_FACTORY_RESET:
        case TD_RESTART:
        case TD_LED_TOGGLE:
            //TimerEvent |= TIMER_EVENT_OTHER_PROC;
            TimerIdOtherProc();
            break;
#ifdef  BT_MESH_G6
        
        case TD_TIMER_FILTER_CLEAN: //TraceDec1("TD_TIMER_FILTER_CLEAN",FilterCounter);

            if(FilterCounter == 3) {
                if(pMeshNodeData->FilterAllTime1 == pMeshNodeData->FilterAllTime2){//Trace("Filter Clean 1");//clear filter 1
                     pMeshNodeData->G6Status |=G6_CLEAR_FILETER1;
                     G6SetActStatus(G6S_CLEAR_FILERT1|G6_STATUS_CHANGE,ON); CLEAR_LED_FILTER1();
                    }
                else{//Trace("Filter Clean All");//clear filter All
                     pMeshNodeData->G6Status &=~G6_CLEAR_FILETER_ALL; 
                     G6SetActStatus(G6S_CLEAR_ALL_FILTER|G6_STATUS_CHANGE,ON); CLEAR_LED_ALL_FILTER();
                    }
                
              }
            else{//Trace("Filter Error");
                    FilterCounter = 0; G6SetActStatus(G6_STATUS_CHANGE,OFF);
                }
            break;
        case TD_TIMER_DOOR_OPEN: FilterCounter = 0;
            if(!G6GetActStatus(G6S_DOOR_OPEN)){//Trace("TD_TIMER_DOOR_OPEN 1");
                G6SetActStatus(G6S_DOOR_OPEN,ON);G6SetActStatus(G6_STATUS_CHANGE,ON);
             }
            break;
        case TD_TIMER_DOOR_CLOSE: FilterCounter = 0;
                if(GetDoorStatus() == CLOSE)
                    {G6SetActStatus(G6S_DOOR_OPEN,OFF);G6SetActStatus(G6_STATUS_CHANGE,ON);}
            break;
        
      case TD_TIMER_KEY_POWER://TraceDec1("TD_TIMER_KEY_POWER 1",PowerKeyCount);
            //pMeshNodeData->G6HostPPercent = PowerKeyCount;
            pMeshNodeData->G6HostPPercent = PowerKeyMap[PowerKeyCount];
            //TraceDec1("TD_TIMER_KEY_POWER 2",pMeshNodeData->G6HostPPercent);
            //if(pMeshNodeData->G6HostPPercent == MENUAL_KEY_AUTO) {G6SetAutoRun(ON);}
            if(PowerKeyCount == MENUAL_KEY_AUTO) {G6SetAutoRun(ON);pMeshNodeData->G6HostPPercent = 0;}
            else {G6SetAutoRun(OFF);}
            G6SetActStatus(G6_STATUS_CHANGE,ON);
            WriteMeshNodeData();
            loop=0; 
            MENUAL_LED_SPEED();
            break;
      case TD_TIMER_CHK_FILTER:
            TimerFilter += ActMotoSpeed;  //TraceDec2("Check Filter",TimerFilter,ActMotoSpeed);
            if(TimerFilter > FILTER_TIME_BASE){//TraceDec1("Filter + 1HR",TimerFilter);
                TimerFilter = ActMotoSpeed;
                G6FilterTimeInc();
              }
            break;
            
#endif          
      default:   break;
    }
    return ret_code;
}




uint16 CountNodeEvent;
#define COUNT_NO_EVENT      (3*60)   //for 10 min

//
//
//
void TimerLedStatus()
{
    switch(CurrTimerHandle)
        {
            case TD_UNPROVISION: 
                SetLedStatus(LED_STATUS_UNPROV);
                break;
            case TD_PROVISIONING: 
                if (!init_done) SetLedStatus(LED_STATUS_PROVING);
                break;
            case TD_NO_EVENT: //TraceDec1("TD_NO_EVENT",CountNodeEvent);
                if(GetMeshNodeStatus(STATUS_IVI_UPDATE) == ON){//Trace("IVI UPDATE ON");
                     SetLedStatus(LED_STATUS_IVI_UPDATE_ON);
                    }
                if(++CountNodeEvent > COUNT_NO_EVENT){ //Trace("IVI Update reset"); 
                    Delay_ms(500);
                }
                break;
            
            default:  break;
                
        };    
    
}

void ResetEventCounter(uchar event)
{
    CountNodeEvent = 0;
}

//
//
void TimerIdOtherProc()
{
    switch(CurrTimerHandle)
        {
            case TD_PROVISIONING: //Trace("TD_PROVISIONING");
                if (!init_done) led_set_state(LED_STATE_PROV); 
                break;
            case TD_RESTART: 
                Cmd_sys_reset(0);
                break;
            case TD_FACTORY_RESET: 
                Cmd_sys_reset(0);
                break;
            default:  break;
                
        };    
}


//
//
void DeviceWakeUp()
{
    return ; // debug: wakeup disable
    
    SetTaskWork(DeviceWakeUp,DEVICE_TASK_OFF);
    //SetTaskWork(DeviceSleeping,DEVICE_TASK_ON);
    SetLedStatus(LED_STATUS_ACTIVE);
    NodeWakeUp();
}

//
//
void DeviceSleeping()
{
    return ; // debug: sleeping disable
    
    if(!GetMeshNodeStatus(BLE_LINK_STATUS)){ Trace("Goto Sleeping");
        SetTaskWork(DeviceSleeping,DEVICE_TASK_OFF);
        SetTaskWork(DeviceWakeUp,DEVICE_TASK_ON);
        SetLedStatus(LED_STATUS_SLEEP);
        NodeSleeping();
        }

}


//
//
void PowerTimerProc()
{
   // if(GetMeshNodeStatus(STATUS_CLIENT)) 
   if(NodeRole == NR_CLIENT)
        ClientGetPropertyProc();
    else 
        PowerModelProc();       
}

void PowerModelProc()
{

    switch(CurrTimerHandle){
        case TD_SERVER_WAKE_UP: Trace("TD_SERVER_WAKE_UP 1");
            NodeWakeUp();
            break;
        case TD_SERVER_SLEEPING: Trace("TD_SERVER_SLEEPING 1");
            if(GetMeshNodeStatus(BLE_LINK_STATUS)){// BLE connect can not slepping
              SetEventTaskTimer(TD_SERVER_SLEEPING,CYCLE_PROXY_CONNECT_WAITING,TIMER_EVENT_ONCE);    
            }
            else{      
              SetEventTaskTimer(TD_SERVER_WAKE_UP,CYCLE_SERVER_SLEEPING,TIMER_EVENT_ONCE);
              NodeSleeping();
            }
            break;
        };
}

//
// Full power can not to be limit, depend on RS485 speed
void ClientGetPropertyProc()
{
    
}


// Timer: xxx ms
Result SetEventTaskTimer(uchar event,uint32 timer,uchar single_shot)
{   
    uint32 timer_tick=0;

    if(timer >= TIMER_5MIN){
            timer_tick = (((TIMER_CLK_FREQ)*(timer/100))*10)/100; 
        }
    else {
            timer_tick = TIMER_MS_2_TICKS(timer);
        }

    //TraceDec1("timer_tick 1", timer_tick);
    
   result = Cmd_set_soft_timer(timer_tick,event,single_shot)->result;
   if(result) TraceErr1("SetEventTaskTimer",result);
   // result = Cmd_set_soft_timer(TIMER_MS_2_TICKS(timer),event,single_shot)->result;
  //  ShowResult("SetEventTaskTimer", result);
    return result;
}



//
//Get elapsed time since last reset of RTCC 
void GetTimerRtcc(PTimerRtcc p_timer)
{
    struct gecko_msg_hardware_get_time_rsp_t* p_timer_rtcc;
    p_timer_rtcc = gecko_cmd_hardware_get_time();
    p_timer->seconds = p_timer_rtcc->seconds;
    p_timer->ticks = p_timer_rtcc->ticks;
    
}

void ShowTimerRTC()
{
    TimerRtcc timer_rtc;
    GetTimerRtcc(&timer_rtc);
    TraceDec2("ShowTimerRTC",timer_rtc.seconds, timer_rtc.ticks);
    
}

/***************************************************************************//**
 * This function is called to initiate factory reset. Factory reset may be
 * initiated by keeping one of the pushbuttons pressed during reboot.
 * Factory reset is also performed if it is requested by the provisioner
 * (event gecko_evt_mesh_node_reset_id).
 ******************************************************************************/
void initiate_factory_reset(void)
{
  Printf("factory reset\r\n");

  // If connection is open then close it before rebooting
  if (ConnectHandle != 0xFF) Cmd_connect_close(ConnectHandle);

  // Perform a factory reset by erasing PS storage. This removes all the keys
  // and other settings that have been configured for this node
  Cmd_flash_ps_erase_all();
  pMeshNodeData->DataInitID = NODE_DATA_ID;   //for v1.22
  WriteNodeData();
  Cmd_set_soft_timer(TIMER_MS_2_TICKS(5000),TD_FACTORY_RESET,1);
}

/***************************************************************************
 * Set device name in the GATT database. A unique name is generated using
 * the two last bytes from the Bluetooth address of this device. Name is also
 * displayed on the LCD.
 *
 * @param[in] pAddr  Pointer to Bluetooth address.
 ******************************************************************************/
void set_device_name(bd_addr *pAddr)
{
  char name[25];
  uint16_t result;

  // Create unique device name using the last two bytes of the Bluetooth address
  //if(MeshNodeStatus & STATUS_CLIENT)
  if(NodeRole == NR_CLIENT)
    snprintf(name, 25, "%s-C %02x:%02x",DEVICE_NAME, pAddr->addr[1], pAddr->addr[0]);
  else
    snprintf(name, 25, "%s-S %02x:%02x",DEVICE_NAME, pAddr->addr[1], pAddr->addr[0]);

  Printf("Device name 1 : '%s'\r\n", name);

  result = Cmd_gatt_server_write_attri_value(gattdb_device_name,0,strlen(name),(uint8_t *)name)->result;
  if (result) {
    Printf("gecko_cmd_gatt_server_write_attribute_value() failed, code %x\r\n", result);
  }

  // Show device name on the LCD
  DI_Print(name, DI_ROW_NAME);
}


//
// Set up BT mesh Tx power
//
void SetTxPower(int16 power)
{
    gecko_cmd_system_halt(ON);
    result = Cmd_sys_set_tx_power(TX_POWER_HI)->set_power;  
    gecko_cmd_system_halt(OFF);
}
//
// sleep: OFF
// wake up: ON
void PowerMode(uchar status)
{
    if(status == POWER_MODE_WAKEUP){ //Trace("Power Mode Wake up");
         SetLedStatus(LED_STATUS_ACTIVE); // the can increment power consumption
        }
    else{// Trace("Power Mode Sleeping"); // sleep mode
         SetLedStatus(LED_STATUS_OFF); // the can increment power consumption
        
        }
}


void SaveMeshNodeInfo(uchar save_id, void* p_event)
{
    switch(save_id)
        {
            case SAVE_INFO_GET_COLUME_REGUEST:
                msg_ms_server_get_column_request_evt *pEvent = (msg_ms_server_get_column_request_evt *)p_event;                
                MeshNodeInfo.ElemIndex = SENSOR_ELEMENT;
                MeshNodeInfo.ClientAddr = pEvent->client_address;
                MeshNodeInfo.AppKey =   pEvent->appkey_index;
                MeshNodeInfo.PropertyID = pEvent->property_id; 
                MeshNodeInfo.pBuff =    pEvent->column_ids.data;
                MeshNodeInfo.BuffSize = pEvent->column_ids.len;
                break;
        };
}

//
// 60 min
//
void SetForceFullPowerTime(uchar status)
{
    if(status == ON)
        {//ON
         
         if(NodeRole == NR_CLIENT)  ClientGetInfoActionNow();
         else {
        	 ServerGetInfoActionNow(); SetNodeSleeping(OFF);
        	 dprint("\r\n***** Wake Up by SetForceFullPowerTime\r\n");
         }

         SetNodeStatus(NS_FORCE_FULL_POWER,ON);
         ForcePowerCounter = 60*60; //60 Min to close
         SetEventTaskTimer(TD_FORCE_POWER_STATUS, TIMER_1SEC, TIMER_EVENT_REPEAT);
         SetNodeStatus(NS_FULL_POWER,ON);   // force to full power
         
        }
    else
        {//OFF
         if(NodeRole == NR_CLIENT)  SetLed(LED_BLUE,ON);
         else {ServerSetPowerStatus(); SetLed(LED_BLUE,OFF);}
         
         SetNodeStatus(NS_FORCE_FULL_POWER,OFF);
         ForcePowerCounter = OFF;
         SetEventTaskTimer(TD_FORCE_POWER_STATUS, 0, TIMER_EVENT_ONCE);
         
        }
}

int16 PowerTest;
extern uint16 ActMotoSpeed;

// BLE and Mesh common event 
//**********************************************************************************************
// Event: gecko_evt_system_external_signal_id
//
//**********************************************************************************************
uint32 EvtSysExternalSignalProc(PCmdPacket pEvent)
{
    uint32 ret_code=TRUE;
    uint32 ext_signal;
    ext_signal = pEvent->data.evt_system_external_signal.extsignals;
#ifndef  BT_MESH_G6

    if(ext_signal == PB_SPEED_NORMAL){//Trace("PB_SPEED_NORMAL");
        GetPropertyID = NODE_GET_INFO_FULL_POWER_OFF;
        SetForceFullPowerTime(OFF);
        //SetVolDecInc(1);
        } 
    else if(ext_signal == PB_SPEED_5SEC){//Trace("PB_SPEED_5SEC");
        GetPropertyID = NODE_GET_INFO_FULL_POWER_ON;  
        SetForceFullPowerTime(ON);
        }
    
    else if(ext_signal == PB1_PRESS_ON){//Trace("PB1_PRESS_ON");
        }
#else 

    if(ext_signal == PB_SPEED_NORMAL){
        } 
    else if(ext_signal == PB1_PRESS_ON){
        }
   
#endif

#ifdef  BT_MESH_G6
    else if(ext_signal == PB_SPEED_KEY){
         SetEventTaskTimer(TD_TIMER_KEY_POWER, TIMER_1SEC,TIMER_EVENT_ONCE);//clear open
         //if(PowerKeyCount == 0xFF) PowerKeyCount = pMeshNodeData->G6HostPPercent;        
    
         if(++PowerKeyCount>MENUAL_KEY_AUTO) PowerKeyCount=0;

         if(G6GetActStatus(G6S_AUTO) ==TRUE) PowerKeyCount = 0; 
         
         TraceDec2("PB_SPEED_KEY",PowerKeyCount,pMeshNodeData->G6HostPPercent);
        }
    else if(ext_signal == PB_CLEAN_FILTER1){//Trace("PB_CLEAN_FILTER1 1");
        }
    else if(ext_signal == PB_CLEAN_FILTER2){//Trace("PB_CLEAN_FILTER2 2");
        }
     
   else if(ext_signal == PB_DOOR_CLOSE){//Trace("PB_DOOR_CLOSE 3");
        SetEventTaskTimer(TD_TIMER_DOOR_OPEN, 0,TIMER_EVENT_ONCE);
        FilterCounter++;
        if(FilterCounter >= 3)
            SetEventTaskTimer(TD_TIMER_DOOR_CLOSE, 0,TIMER_EVENT_ONCE);//clear open
        else  
            SetEventTaskTimer(TD_TIMER_DOOR_CLOSE, TIMER_2SEC,TIMER_EVENT_ONCE);//clear open
         
         SetEventTaskTimer(TD_TIMER_FILTER_CLEAN, TIMER_1SEC,TIMER_EVENT_ONCE);         
            
       }
   else if(ext_signal == PB_DOOR_OPEN){//Trace("PB_DOOR_OPEN 4");
         SetEventTaskTimer(TD_TIMER_DOOR_CLOSE, 0,TIMER_EVENT_ONCE);//clear open 
         SetEventTaskTimer(TD_TIMER_DOOR_OPEN, TIMER_1SEC,TIMER_EVENT_ONCE);
       }
  else if(ext_signal == PB_G6_RESET){//Trace("PB_G6_RESET");
        BtmG6Reset();
        CLEAR_LED_G6_RESET();
      }
#endif    
    return ret_code;
}


