#include "global.h" 
#include "graphics.h"

//richard Add
 /* BG stack headers */

#include "sleep.h"
#include "leds.h"
#include "sensor_client.h"
#include "bus_usart.h"
#include "AD7147.h"
#include "bus_spi.h"
#include "MeshFeatures.h" 
#include "modbus_to_mesh.h"
#include "cmd_to_bt_mesh.h"

#include "sensor_server.h"
#include "node_data.h"
#include "ivi_features.h"
#include "Mesh_node.h"
#include "ble_comm.h"

//PRspResult  pResult;
Result result;
_MeshNodeInfo MeshNodeInfo;
_PTimerEventTask pDeviceTask;
extern _TimerEventTask DeviceTaskTbl[];


void BleCommInit()
{TraceProc();
    uchar buttton_status;
    MeshNodeStatus = 0;
   // UDELAY_Calibrate();
    NodeDataInit();
    NodeRole = pMeshNodeData->MeshNodeRole; TraceDec1("Node Role 1", NodeRole);
    MeshNodeInit();
    
    buttton_status = GetButtonStatus();

     if( buttton_status == KEY_FACTORY_RESET) 
        {initiate_factory_reset();return;}

    SetMeshNodeStatus(STATUS_FULL_POWER, ON);   // client node must full power
#if MESH_COLUME_ENABLE
    SetMeshNodeStatus(STATUS_MODBUS_MESH,ON);   // for modbus to bt mesh    Debug
#endif    
    

    
}

//**********************************************************************************************
// return button status
// 0: server node(default), 1: client node
//**********************************************************************************************
uchar GetButtonStatus()
{//TraceProc();
    uint button0,button1;
    uchar ret_code;
    button0 = GPIO_PinInGet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN);
    button1 = GPIO_PinInGet(BSP_BUTTON0_PORT, BSP_BUTTON1_PIN);
    
    if(button0 == LOW & button1 == HIGH)        ret_code = KEY_SERVER_TO_RS485;
    else if(button0 == LOW & button1 == LOW)    ret_code = KEY_FACTORY_RESET;
    else ret_code = BT_NODE_ROLE_PRE_DEF;       //default server node
    Trace3("button_port", button0,button1,ret_code);
    return ret_code;
}

/*
_TimerEventTask ClientTaskTbl[]=
{
    // for client node
    {ClientIviUpdateProc,       IVI_SEQ_WAITING,TIMER_IVI_DETECT,TIMER_IVI_DETECT,  ClientTaskTbl},    
    {ClientGetSensorDataProc,   MM_PENDING,     TASK_TIME_OUT_1,TASK_TIME_OUT_4,    ClientTaskTbl},
    {ClientSendDataToHostProc,  MM_PENDING,     TASK_TIME_OUT_1,TASK_TIME_OUT_1,    ClientTaskTbl},    
    {NULL,0,0,0,ClientTaskTbl}
};

_TimerEventTask ServerNodeTaskTbl[]=
{
    // for server node
    {ServerSetupProc,           SERVER_SETUP_PENDING,TASK_TIME_OUT_1,TASK_TIME_OUT_1,  ServerNodeTaskTbl},    
    {ServerGetSensorDataProc,   MM_PENDING, TASK_TIME_OUT_1,TASK_TIME_OUT_1,        ServerNodeTaskTbl},    
    {ServerToClientProc,        MM_PENDING, 20,20, ServerNodeTaskTbl},    
    {NULL,0,0,0,ServerNodeTaskTbl}
};
*/
#include "bus_rs485.h"
void ServerNodeTask();
    
//*************************************************************************************
// process Device Task
//*************************************************************************************
void DeviceTaskProc()
{//TraceProc();
    _PTimerEventTask p_task_temp;
    
    //if(CheckDeviceTaskActive1ms() == TRUE) UsartMonitor1ms();
    //if(CheckDeviceTaskActive1ms() == FALSE) {SetLedToggle(LED_RED); return;}
    //if(CheckDeviceTaskActive() == FALSE) {return;}

    //ServerNodeTask();
    
 //   SetLedToggle(LED_RED);
 //   SetLedToggle(LED_BLUE);   
 //   return;
    
    //UsartMonitor1ms();
    UsartMonitor();
    CheckTaskCounter();
    pDeviceTask = pDeviceTask->pItself;

    while(pDeviceTask->pTimerTask)
        {   
            if(GetMeshNodeStatus(STATUS_IVI_UPDATE) == OFF)
                pDeviceTask->pTimerTask();  
            else
                {
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


/*
//
// return task array table
//
_PTimerEventTask GetDeviceTaskTbl()
{
     if(GetMeshNodeStatus(STATUS_CLIENT)) 
        return ClientTaskTbl;
     else 
        return ServerNodeTaskTbl;
}

//
// process timer counter
// 10~50ms once
void CheckTaskCounter()
{
  // uchar loop;
   _PTimerEventTask p_allDevTask;
   p_allDevTask = GetDeviceTaskTbl();
   while(p_allDevTask->pTimerTask)
    {
       if(p_allDevTask->TaskTimeOut) p_allDevTask->TaskTimeOut--;
       p_allDevTask++; // to next task
    };
   
}

//
// return task pointer
// 
_PTimerEventTask GetTimerEventTask(PTimerTask p_task)
{
    _PTimerEventTask p_device_task=GetDeviceTaskTbl();

    while(p_device_task->pTimerTask!=NULL && p_device_task->pTimerTask != p_task)
            p_device_task++;
    if(!p_device_task->pTimerTask) {TraceErr("GetTimerEventTask");
        p_device_task = NULL;
        }

  return p_device_task;
}

*/
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
{//TraceProc();
    return;

/*
    _PTimerEventTask p_devic_task;
    p_devic_task = GetTimerEventTask(p_task);
    if(p_devic_task)
        {        
         p_devic_task->TaskTimer = status;
         if(status == DEVICE_TASK_ON) 
            p_devic_task->TaskTimer = p_devic_task->TaskTimerValue; // update counter
        }
    else TraceErr("SetTaskWork");
*/    
}

void CheckStageTimer()
{//TraceProc();
    CheckNodeTimerCount();
}

uint16  TimerEvent=0;
uchar CurrTimerHandle;


void BtMeshReset();

//**********************************************************************************************
// Event: gecko_evt_hardware_soft_timer_id
//
//**********************************************************************************************
uint32 EvtSoftTimerProc(PCmdPacket pEvent)
{ //TraceProc();
    uint32 ret_code=TRUE;
    uchar    stage;
    CurrTimerHandle = pEvent->data.evt_hardware_soft_timer.handle;
   // TraceDec1("CurrTimerHandle", CurrTimerHandle);
   // return ret_code;
    switch (CurrTimerHandle) 
    {
        //case TD_DEVICE_TASK: //Trace("TD_DEVICE_TASK"); 50ms repeat
          //   SetTimerTaskEvent(TIMER_EVENT_DEVICE_TASK,ON);
            //break;
        case TD_STAGE_TIMER: //Trace("TD_STAGE_TIMER");
            CheckStageTimer();
            //SetLedToggle(LED_BLUE);
            break;
            
        case TD_NODE_SLEEP: Trace("TD_NODE_SLEEP");
            SetNodeSleeping(ON);
            break;
        case TD_NODE_WAKE_UP: 
            SetNodeSleeping(OFF);   
            Trace("TD_NODE_WAKE_UP");
            //NodeLpn(ON);
            break;
       // case TD_USART_RX: //Trace("TD_USART_RX Ending");
             //SetTimerTaskEvent(TIMER_EVENT_USART_RX,ON);
            // UsartSetStage(USART_STAGE_RX_END); // for server node 23 bytes
             //UsartSetStage(USART_STAGE_TX_CLEAN);
             //if(UsartGetRxCounter() == 8) UsartSetStage(USART_STAGE_RX_END);
             //else {TraceErr1("TD_USART_RX: Usart Rx Ending",UsartGetRxCounter());UsartSetStage(USART_STAGE_RX_CLEAN);}
             
        //    break;
        /*
        case TD_SERVER_WAKE_UP:
            SetTimerTaskEvent(TIMER_EVENT_WAKE_UP,ON);
           // PowerTimerProc();
            break;
        case TD_SERVER_SLEEPING:
            SetTimerTaskEvent(TIMER_EVENT_WAKE_UP,OFF);
           // PowerTimerProc();
            break;
        */
//////////////////////////////////// for Client Timer event ///////////////////////////////////////      
        case TD_TASK_CLIENT_SCAN_SERVER: //Trace("TD_TASK_CLIENT_SCAN_SERVER");
            //TimerEvent |= TIMER_EVENT_SCAN_SERVER;
            //ClientScanServerProc();
            break;
        case TD_TASK_GET_PROPERTY: //Trace("TD_TASK_GET_PROPERTY");
            //TimerEvent |= TIMER_EVENT_GET_PROPERTY;
            //ClientGetServerDataProc();
            break;
        case TD_SYS_RESET: Trace("System Reset"); Delay_ms(200); //while(1);
            if(!GetMeshNodeStatus(STATUS_BLE_CONNECT)) BtMeshReset();
            break;
        case TD_GET_SENSOR_INFO: //Trace("TD_GET_SENSOR_INFO");
            SetMeshNodeStatus(STATUS_GET_SENSOR_INFO,ON);
            break;        
        case TD_GET_SENSOR_ENDING: //Trace("TD_GET_SENSOR_ENDING");
            SetMeshNodeStatus(STATUS_GET_SENSOR_ENDING,OFF);
            break;
        case TD_SYS_SETUP_RESET: Trace("TD_SETUP_RESET");
            Cmd_sys_reset(0);
            break;
        case TD_SET_MODBUS_CMD: Trace("TD_SET_MODBUS_CMD"); // 2 sec
            SetMeshNodeStatus(STATUS_SET_MODBUS_CMD,OFF);
            break;
        case TD_CHECK_DEV_NODE: TraceErr("TD_CHECK_DEV_NODE");
           // CheckNodeActionStatus();
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
      default: TraceDec1("Timer Message Error",CurrTimerHandle);  break;
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
            case TD_UNPROVISION: Trace("TD_UNPROVISION");
                SetLedStatus(LED_STATUS_UNPROV);
                break;
            case TD_PROVISIONING: //Trace("TD_PROVISIONING");
                if (!init_done) SetLedStatus(LED_STATUS_PROVING);
                break;
            case TD_NO_EVENT: //TraceDec1("TD_NO_EVENT",CountNodeEvent);
                if(GetMeshNodeStatus(STATUS_IVI_UPDATE) == ON)  
                    {Trace("IVI UPDATE ON");
                     SetLedStatus(LED_STATUS_IVI_UPDATE_ON);
                    }
                if(++CountNodeEvent > COUNT_NO_EVENT) 
                { Trace("IVI Update reset"); Delay_ms(500);
                    //debug to disable
                    //Cmd_sys_reset(0);
                }
                break;
            
            default: TraceErr1("SysLedStatus",CurrTimerHandle); break;
                
        };    
    
}

void ResetEventCounter(uchar event)
{
    CountNodeEvent = 0;
}

//
//
void TimerIdOtherProc()
{//TraceProc();
    switch(CurrTimerHandle)
        {
            case TD_PROVISIONING: //Trace("TD_PROVISIONING");
                if (!init_done) led_set_state(LED_STATE_PROV); 
                break;
            case TD_RESTART: Trace("TD_RESTART");
                Cmd_sys_reset(0);
                break;
            case TD_FACTORY_RESET:  Trace("TD_FACTORY_RESET");
                Cmd_sys_reset(0);
                break;
           // case TD_LED_TOGGLE:  //Trace("TD_LED_TOGGLE");
           //     SetLedToggle(LED0);
           //     break;
            
            default: TraceErr1("TimerIdOtherProc",CurrTimerHandle); break;
                
        };    
}


//
//
void DeviceWakeUp()
{//TraceProc();
    return ; // debug: wakeup disable
    
    SetTaskWork(DeviceWakeUp,DEVICE_TASK_OFF);
    //SetTaskWork(DeviceSleeping,DEVICE_TASK_ON);
    SetLedStatus(LED_STATUS_ACTIVE);
    NodeWakeUp();
}

//
//
void DeviceSleeping()
{//TraceProc();
    return ; // debug: sleeping disable
    
    if(!GetMeshNodeStatus(BLE_LINK_STATUS))
        { Trace("Goto Sleeping");
        SetTaskWork(DeviceSleeping,DEVICE_TASK_OFF);
        SetTaskWork(DeviceWakeUp,DEVICE_TASK_ON);
        SetLedStatus(LED_STATUS_SLEEP);
        NodeSleeping();
        }

}


//
//
void PowerTimerProc()
{//TraceProc();
   // if(GetMeshNodeStatus(STATUS_CLIENT)) 
   if(NodeRole == NR_CLIENT)
        ClientGetPropertyProc();
    else 
        PowerModelProc();       
}

void PowerModelProc()
{//TraceProc();

    switch(CurrTimerHandle)
        {
        case TD_SERVER_WAKE_UP: Trace("TD_SERVER_WAKE_UP 1");
            NodeWakeUp();
            break;
        case TD_SERVER_SLEEPING: Trace("TD_SERVER_SLEEPING 1");
            if(GetMeshNodeStatus(BLE_LINK_STATUS))
            {// BLE connect can not slepping
              SetEventTaskTimer(TD_SERVER_SLEEPING,CYCLE_PROXY_CONNECT_WAITING,TIMER_EVENT_ONCE);    
            }
            else
            {      
              SetEventTaskTimer(TD_SERVER_WAKE_UP,CYCLE_SERVER_SLEEPING,TIMER_EVENT_ONCE);
              NodeSleeping();
            }
            break;
        };
}

//
// Full power can not to be limit, depend on RS485 speed
void ClientGetPropertyProc()
{//TraceProc();
    
}


// Timer: xxx ms
Result SetEventTaskTimer(uchar event,uint32 timer,uchar single_shot)
{   
    uint32 timer_tick=0;

    if(timer >= TIMER_5MIN)
        {
            timer_tick = (((TIMER_CLK_FREQ)*(timer/100))*10)/100; 
        }
    else
        {
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
  DI_Print("\n***\nFACTORY RESET\n***", DI_ROW_STATUS);

  // If connection is open then close it before rebooting
  if (ConnectHandle != 0xFF) Cmd_connect_close(ConnectHandle);

  // Perform a factory reset by erasing PS storage. This removes all the keys
  // and other settings that have been configured for this node
  Cmd_flash_ps_erase_all();
  // Reboot after a small delay
  Cmd_set_soft_timer(TIMER_MS_2_TICKS(2000),TD_FACTORY_RESET,1);
}

/***************************************************************************//**
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
{//TraceProc();
    //gecko_init_afh();
    gecko_cmd_system_halt(ON);
    result = Cmd_sys_set_tx_power(TX_POWER_HI)->set_power;  
    gecko_cmd_system_halt(OFF);
    //TraceDec1("Mesh Init Set Tx Power", result); //richard: Check Power
}
//
// sleep: OFF
// wake up: ON
void PowerMode(uchar status)
{
    if(status == POWER_MODE_WAKEUP)
        { Trace("Power Mode Wake up");
         SetLedStatus(LED_STATUS_ACTIVE); // the can increment power consumption
        }
    else
        { Trace("Power Mode Sleeping"); // sleep mode
         SetLedStatus(LED_STATUS_OFF); // the can increment power consumption
        
        }
}


void SaveMeshNodeInfo(uchar save_id,void* p_event)
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


// BLE and Mesh common event 

//**********************************************************************************************
// Event: gecko_evt_system_external_signal_id
//
//**********************************************************************************************
uint32 EvtSysExternalSignalProc(PCmdPacket pEvent)
{TraceProc();
    uint32 ret_code=TRUE;
    /*
    uint32 signal;
    uint16              result;
    uint32  setting_data=0x12345678;
    msg_sys_external_signal_evt *pEvt_ext_signal = &pEvent->data.evt_system_external_signal;
    signal = pEvt_ext_signal->extsignals;

  result = Cmd_ms_client_set_setting(SENSOR_ELEMENT, PUBLISH_ADDRESS, IGNORED, 0x02,MODBUS_GET_REGS_VALUE,
                                    8,4,(uint8*)&setting_data)->result;
  if(result) TraceErr("Cmd_ms_client_set_setting");
  else TraceOk("Cmd_ms_client_set_setting");
*/        
    // debug
    return ret_code;
/*    
    if(GetMeshNodeStatus(STATUS_CLIENT) != TRUE || GetMeshNodeStatus(STATUS_PROVISIONED) != TRUE)
        return ret_code;

    if (signal & EXT_SIGNAL_PB1_PRESS) {Trace("PB1 pressed");
      if(GetCurrProperty() == PRESENT_AMBIENT_TEMPERATURE)
        SetPropertyIndex(PROPERTY_INDEX_PEOPLE);
      else
        SetPropertyIndex(PROPERTY_INDEX_TEMPERATURE);
    }
    
    if (signal & EXT_SIGNAL_PB0_PRESS) {
        Trace("PB0 pressed");
        StartScanServerNode(); // debug
    }

    return ret_code;
*/    
}




