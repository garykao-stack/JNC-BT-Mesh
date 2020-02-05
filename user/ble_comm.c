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
    //led_set_state(LED_STATE_ON);
    enable_button_interrupts();
    buttton_status = GetButtonStatus();
   // SetEventTaskTimer(TIMER_ID_DEVICE_TASK,TIMER_DEVICE_TASK,TIMER_EVENT_REPEAT); // set up task timer
   // SetEventTaskTimer(TIMER_ID_SYS_RESET,TIMER_1HR,TIMER_EVENT_REPEAT); // system reset
    //initiate_factory_reset(); return;
    
#if CLIENT_NODE // for debug
    if( buttton_status != BLE_NODE_FACTORY_RESET)
        buttton_status = MESH_SENSOR_MODEL_CLIENT; // for debug
#endif
    
    if( buttton_status == MESH_SENSOR_MODEL_CLIENT) 
        {Trace("Client Model");// client mode enable
            SetNodeStatus(STATUS_CLIENT,ON);   SetNodeStatus(STATUS_SPI_ENABLE,OFF);
            graphInit("Mesh 152 => Client\n\n");
        } 
    else if( buttton_status == MESH_SENSOR_MODEL_SERVER) 
        {Trace("Server Model");// server mode & LPN enable
            SetNodeStatus(STATUS_LPN,ON); SetNodeStatus(STATUS_SPI_ENABLE,ON);
            //graphInit("Mesh 152 => Server\n\n");
        } 
    else if( buttton_status == BLE_NODE_FACTORY_RESET) {initiate_factory_reset();}
#if MESH_MODBUS_ENABLE
    SetNodeStatus(STATUS_MODBUS_MESH,ON);   // for modbus to bt mesh    Debug
#endif    
    SetNodeStatus(STATUS_FULL_POWER, ON);   // client node must full power
    DeviceTaskInit();
    pDeviceTask = DeviceTaskTbl;

    
}


//**********************************************************************************************
// return button status
// 0: server node(default), 1: client node
//**********************************************************************************************
uchar GetButtonStatus()
{TraceProc();
    uint button0,button1;
    uchar ret_code;
    button0 = GPIO_PinInGet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN);
    button1 = GPIO_PinInGet(BSP_BUTTON0_PORT, BSP_BUTTON1_PIN);

    if(button0 == HIGH & button1 == HIGH)       ret_code = MESH_SENSOR_MODEL_SERVER;
    else if(button0 == HIGH & button1 == LOW)   ret_code = MESH_SENSOR_MODEL_CLIENT;
    else if(button0 == LOW & button1 == LOW)    ret_code = BLE_NODE_FACTORY_RESET;
    Trace2("button_port", button0,button1);

    return ret_code;
}


// BLE and Mesh common event 

//**********************************************************************************************
// Event: gecko_evt_system_external_signal_id
//
//**********************************************************************************************
uint32 EvtSysExternalSignalProc(PCmdPacket pEvent)
{//TraceProc();
    uint32 ret_code=TRUE;
    uint32 signal;
    msg_sys_external_signal_evt *pEvt_ext_signal = &pEvent->data.evt_system_external_signal;
    signal = pEvt_ext_signal->extsignals;

    if(GetNodeStatus(STATUS_CLIENT) != TRUE || GetNodeStatus(STATUS_PROVISIONED) != TRUE)
        return ret_code;

    if (signal & EXT_SIGNAL_PB1_PRESS) {Trace("PB1 pressed");
      if(GetCurrProperty() == PRESENT_AMBIENT_TEMPERATURE)
        SetPropertyIndex(PROPERTY_INDEX_PEOPLE);
      else
        SetPropertyIndex(PROPERTY_INDEX_TEMPERATURE);
    }
    if (signal & EXT_SIGNAL_PB0_PRESS) {Trace("PB0 pressed");        
        StartScanServerNode();
    }

    return ret_code;
}

extern uint8_t init_done;
extern uint8_t registered_devices;

#define TASK_TIME_OUT           (TIMER_1SEC)
#define TASK_TIME_OUT_1         (TASK_TIME_OUT/TIMER_DEVICE_TASK)
#define TASK_TIME_OUT_2         (TIMER_2SEC/TIMER_DEVICE_TASK)
#define TASK_TIME_OUT_3         (TIMER_3SEC/TIMER_DEVICE_TASK)
#define TASK_TIME_OUT_4         (TIMER_4SEC/TIMER_DEVICE_TASK)

#define TASK_TIMER              (TIMER_100MS)
#define TASK_TIMER_1            (TASK_TIMER/TIMER_DEVICE_TASK)
#define TASK_TIMER_2            (TASK_TIMER/TIMER_DEVICE_TASK)
#define TASK_TIMER_3            (TASK_TIMER/TIMER_DEVICE_TASK)
#define TASK_TIMER_4            (TASK_TIMER/TIMER_DEVICE_TASK)

#define TASK_TIMER_WAKEUP       (TIMER_5SEC/TIMER_DEVICE_TASK)
#define TASK_TIMER_SLEEPING     (TIMER_1SEC/TIMER_DEVICE_TASK)


//
// initial task active
//
void DeviceTaskInit()
{
     if(GetNodeStatus(STATUS_CLIENT))
        {
            if(GetNodeStatus(STATUS_MODBUS_MESH))
                DeviceTaskTbl[0].TaskTimer = DEVICE_TASK_ON;
            else{
                DeviceTaskTbl[1].TaskTimer = DEVICE_TASK_ON;
                DeviceTaskTbl[2].TaskTimer = DEVICE_TASK_ON;
                }
        }
     else
        {
            SetTaskWork(ModbusToMeshServerProc,DEVICE_TASK_ON);
            //SetTaskWork(DeviceWakeUp,DEVICE_TASK_ON);
            DeviceWakeUp();
        }
}


_TimerEventTask DeviceTaskTbl[]=
{
    // for client node
    {TIMER_EVENT_FREE,ModbusToMeshClientProc,   MM_PENDING,         DEVICE_TASK_OFF,TASK_TIMER_1,0,TASK_TIME_OUT_1},
    {TIMER_EVENT_FREE,ClientScanServerProc,     SCAN_SERVER_PENDING,DEVICE_TASK_OFF,TASK_TIMER_1,0,TASK_TIME_OUT_1},
    {TIMER_EVENT_FREE,ClientGetServerDataProc,  GET_PROPERTY_PENDING,DEVICE_TASK_OFF,TASK_TIMER_1,0,TASK_TIME_OUT_1},

    
    // for server node
    {TIMER_EVENT_FREE,ModbusToMeshServerProc,   MM_PENDING,         DEVICE_TASK_OFF,TIMER_TASK_DEVICE,0,TASK_TIME_OUT_1},    
    {TIMER_EVENT_WAKE_UP,DeviceWakeUp,          NULL,               DEVICE_TASK_OFF,TASK_TIMER_WAKEUP,0,TASK_TIME_OUT_1},    
    {TIMER_EVENT_FREE,DeviceSleeping,           NULL,               DEVICE_TASK_OFF,TASK_TIMER_SLEEPING,0,TASK_TIME_OUT_1},

    {0,0,0,0,NULL,0}
};
    

//
// process timer event
//
void TimerEventTaskProc()
{//TraceProc();

   //Trace16Ptr_2(pDeviceTask,Timer,pTimerTask);
    if(GetTimerTaskEvent(TIMER_EVENT_DEVICE_TASK))
        {// process timer event counter
         SetTimerTaskEvent(TIMER_EVENT_DEVICE_TASK,OFF);
         TimerEventCountProc();
        }

    pDeviceTask = DeviceTaskTbl;

    while(pDeviceTask->pTimerTask){
       
    if(pDeviceTask->TaskTimer == DEVICE_TASK_ON || pDeviceTask->TaskTimer == DEVICE_TASK_ALWAY_ON)
        {// execute task
         pDeviceTask->TaskTimer = pDeviceTask->TaskTimerValue;  //reset counter 
         pDeviceTask->pTimerTask();
        }
    pDeviceTask++;  // to next task  
        };
        
    if(pDeviceTask->pTimerTask == NULL) pDeviceTask = DeviceTaskTbl;
}



//
// process timer counter
// 10~50ms once
void TimerEventCountProc()
{
   uchar loop;
   _PTimerEventTask p_allDevTask;
   p_allDevTask = DeviceTaskTbl;
   while(p_allDevTask->pTimerTask)
    {
       if(p_allDevTask->TaskTimer != DEVICE_TASK_OFF && p_allDevTask->TaskTimer != DEVICE_TASK_ON) p_allDevTask->TaskTimer--;
       if(p_allDevTask->TaskTimeOut) p_allDevTask->TaskTimeOut--;
       p_allDevTask++; // to next task
    };
   
}


//
// return task pointer
// 
_PTimerEventTask GetTimerEventTask(PTimerTask p_task)
{
    _PTimerEventTask p_device_task=DeviceTaskTbl;

    while(p_device_task->pTimerTask!=NULL && p_device_task->pTimerTask != p_task)
            p_device_task++;
    if(!p_device_task->pTimerTask) {TraceErr("GetTimerEventTask");
        p_device_task = NULL;
        }

  return p_device_task;
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
{TraceProc();
    _PTimerEventTask p_devic_task;
    p_devic_task = GetTimerEventTask(p_task);
    if(p_devic_task)
        {        
         p_devic_task->TaskTimer = status;
         if(status == DEVICE_TASK_ON) 
            p_devic_task->TaskTimer = p_devic_task->TaskTimerValue; // update counter
        }
    else TraceErr("SetTaskWork");
}



uint16  TimerEvent=0;
uchar CurrTimerHandle;

//**********************************************************************************************
// Event: gecko_evt_hardware_soft_timer_id
//
//**********************************************************************************************
uint32 EvtSoftTimerProc(PCmdPacket pEvent)
{ //TraceProc();
    uint32 ret_code=TRUE;
    uchar    stage;
    CurrTimerHandle = pEvent->data.evt_hardware_soft_timer.handle;
   // return ret_code;
    switch (CurrTimerHandle) 
    {
        case TIMER_ID_DEVICE_TASK: //Trace("TIMER_ID_DEVICE_TASK"); 50ms repeat
             SetTimerTaskEvent(TIMER_EVENT_DEVICE_TASK,ON);
            break;
        case TIMER_ID_USART_RX: //Trace("TIMER_ID_USART_RX");
             SetTimerTaskEvent(TIMER_EVENT_USART_RX,ON);
            break;
        
        case TIMER_ID_TASK_GET_CIN_VALUE: 
            SetTimerTaskEvent(TIMER_EVENT_GET_CIN_VALUE,ON);
            //GetAD7147CinxProc();
            break;        
        case TIMER_ID_SERVER_WAKE_UP:
            SetTimerTaskEvent(TIMER_EVENT_WAKE_UP,ON);
           // PowerTimerProc();
            break;
        case TIMER_ID_SERVER_SLEEPING:
            SetTimerTaskEvent(TIMER_EVENT_WAKE_UP,OFF);
           // PowerTimerProc();
            break;
//////////////////////////////////// for Client Timer event ///////////////////////////////////////      
        case TIMER_ID_TASK_CLIENT_SCAN_SERVER: //Trace("TIMER_ID_TASK_CLIENT_SCAN_SERVER");
            //TimerEvent |= TIMER_EVENT_SCAN_SERVER;
            //ClientScanServerProc();
            break;
        case TIMER_ID_TASK_GET_PROPERTY: //Trace("TIMER_ID_TASK_GET_PROPERTY");
            //TimerEvent |= TIMER_EVENT_GET_PROPERTY;
            //ClientGetServerDataProc();
            break;
        case TIMER_ID_SYS_RESET: Trace("System Reset"); Delay_ms(500); //while(1);
            Cmd_sys_reset(0);
            break;
//////////////////////////////////// for server Timer event ///////////////////////////////////////
        case TIMER_ID_PROVISIONING:
        case TIMER_ID_FACTORY_RESET:
        case TIMER_ID_RESTART:
            //TimerEvent |= TIMER_EVENT_OTHER_PROC;
            TimerIdOtherProc();
        break;
      default: TraceDec1("Timer Message Error",CurrTimerHandle);  break;
    }
    return ret_code;
}


//
//
void TimerIdOtherProc()
{TraceProc();
    switch(CurrTimerHandle)
        {
            case TIMER_ID_PROVISIONING: Trace("TIMER_ID_PROVISIONING");
                if (!init_done) led_set_state(LED_STATE_PROV); 
                break;
            case TIMER_ID_RESTART: Trace("TIMER_ID_RESTART");
                Cmd_sys_reset(0);
                break;
            case TIMER_ID_FACTORY_RESET:  Trace("TIMER_ID_FACTORY_RESET");
                Cmd_sys_reset(0);
                break;
            default: TraceErr("TimerIdOtherProc");
                
        };    
}


//
//
void DeviceWakeUp()
{TraceProc();

    SetTaskWork(DeviceWakeUp,DEVICE_TASK_OFF);
    //SetTaskWork(DeviceSleeping,DEVICE_TASK_ON);
    SetLedStatus(LED_STATUS_ACTIVE);
    NodeWakeUp();
}

//
//
void DeviceSleeping()
{TraceProc();
    
    if(!GetNodeStatus(BLE_LINK_STATUS))
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
{TraceProc();
    if(GetNodeStatus(STATUS_CLIENT)) ClientGetPropertyProc();
    else PowerModelProc();       
}

void PowerModelProc()
{TraceProc();

    switch(CurrTimerHandle)
        {
        case TIMER_ID_SERVER_WAKE_UP: Trace("TIMER_ID_SERVER_WAKE_UP 1");
            NodeWakeUp();
            break;
        case TIMER_ID_SERVER_SLEEPING: Trace("TIMER_ID_SERVER_SLEEPING 1");
            if(GetNodeStatus(BLE_LINK_STATUS))
            {// BLE connect can not slepping
              SetEventTaskTimer(TIMER_ID_SERVER_SLEEPING,CYCLE_PROXY_CONNECT_WAITING,TIMER_EVENT_ONCE);    
            }
            else
            {      
              SetEventTaskTimer(TIMER_ID_SERVER_WAKE_UP,CYCLE_SERVER_SLEEPING,TIMER_EVENT_ONCE);
              NodeSleeping();
            }
            break;
        };
}

//
// Full power can not to be limit, depend on RS485 speed
void ClientGetPropertyProc()
{TraceProc();
    
}



// Timer: xxx ms
Result SetEventTaskTimer1(uchar event,uint32 timer,uchar single_shot)
{   
    //result = Cmd_set_soft_timer(timer_tick,event,single_shot)->result;
   result = Cmd_set_soft_timer(TIMER_MS_2_TICKS(timer),event,single_shot)->result;
    ShowResult("SetEventTaskTimer", result);
    return result;
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
  Cmd_set_soft_timer(TIMER_MS_2_TICKS(2000),TIMER_ID_FACTORY_RESET,1);
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
  if(MeshNodeStatus & STATUS_CLIENT)
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
{TraceProc();
    //gecko_init_afh();
    gecko_cmd_system_halt(1);
    result = Cmd_sys_set_tx_power(TX_POWER_HI)->set_power;  // Bug ==> current MAX tx power 14dB can be setted.. but it must 19dB for BGM13P
    gecko_cmd_system_halt(0);
    TraceDec1("Mesh Init Set Tx Power", result);
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
                msg_ms_server_get_column_request_evt *pEvent = p_event;                
                MeshNodeInfo.ElemIndex = SENSOR_ELEMENT;
                MeshNodeInfo.ClientAddr = pEvent->client_address;
                MeshNodeInfo.AppKey =   pEvent->appkey_index;
                MeshNodeInfo.PropertyID = pEvent->property_id; 
                MeshNodeInfo.pBuff =    pEvent->column_ids.data;
                MeshNodeInfo.BuffSize = pEvent->column_ids.len;
                break;
        };
}


