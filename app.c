
/***************************************************************************//**
 * @file  app.c
 * @brief Application code
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "global.h"
#include "retargetserial.h"
#include "leds.h"


//#include"node_data.h"

#include "device_bus.h"
//#include "modbus.h"
#include "sensor_dev.h"

/* Sensor client header */
#include "sensor_client.h"
#include "app.h"

/***************************************************************************//**
 * @addtogroup Application
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup app
 * @{
 ******************************************************************************/
bool BleMeshEventProc(PCmdPacket pEvent, PEventFun pEventFun);


/*******************************************************************************
 * Function prototypes.
 ******************************************************************************/
bool mesh_bgapi_listener(struct gecko_cmd_packet *evt);

/***************************************************************************//**
 * Initialise used bgapi classes.
 ******************************************************************************/
void gecko_bgapi_classes_init(void)
{
  gecko_bgapi_class_dfu_init();
  gecko_bgapi_class_system_init();
  gecko_bgapi_class_le_gap_init();
  gecko_bgapi_class_le_connection_init();
  gecko_bgapi_class_gatt_server_init();
  gecko_bgapi_class_hardware_init();
  gecko_bgapi_class_flash_init();
  gecko_bgapi_class_test_init();
  gecko_bgapi_class_mesh_node_init();
  gecko_bgapi_class_mesh_proxy_init();
  gecko_bgapi_class_mesh_proxy_server_init(); //richard add
 /* 
        gecko_bgapi_class_mesh_sensor_client_init();
        gecko_bgapi_class_mesh_sensor_server_init();
        gecko_bgapi_class_mesh_sensor_setup_server_init();
*/
  gecko_bgapi_class_mesh_test_init();
  gecko_bgapi_class_mesh_lpn_init(); //richard add 
#ifdef FRIEND_NODE
  gecko_bgapi_class_mesh_friend_init(); // for friend node
#endif  
  
}

//***************************************************************************
// Initial BLE Stack, IO port,
//
//
//****************************************************************************
void BleMeshNodeInit(gecko_configuration_t *pConfig)
{
    // Initialize stack
    
    //DeviceInit();
    
    gecko_stack_init(pConfig);
    gecko_bgapi_classes_init();
    gecko_initCoexHAL();      // Initialize coexistence interface. Parameters are taken from HAL config.
#ifndef DEBUG_PRINT    
    RETARGET_SerialInit();    // Initialize debug prints and display interface
#endif    
    DI_Init();                //Initialize Display Interface.
    //Trace("123456789");     while(1);
    /* Enable AFH */
    //gecko_init_afh(); //richard add
    
    // Initialize LEDs and buttons. Note: some radio boards share the same GPIO
    // for button & LED. Initialization is done in this order so that default
    // configuration will be "button" for those radio boards with shared pins.
    // led_init() is called later as needed to (re)initialize the LEDs
    led_init();  button_init();

    BleCommInit();  
    NodeDataInit();  
    if(MeshNodeStatus & STATUS_CLIENT)
        {Trace("Client Node Init");
         gecko_bgapi_class_mesh_sensor_client_init();
        }
        
    else
        {Trace("Server Node Init");
         gecko_bgapi_class_mesh_sensor_server_init();
         gecko_bgapi_class_mesh_sensor_setup_server_init();
        }
    
    BleEventInit();
    MeshEventInit();
    
    DeviceInit();
   // ModbusRtuInit();
    
}
/*******************************************************************************
 * Main application code.
 * @param[in] pConfig  Pointer to stack configuration.
 ******************************************************************************/
extern EventFun BleEventFun[],MeshEventFun[];
void WaterLeveMeshProc(void);
void ComPortProc(void);


void appMain(gecko_configuration_t *pConfig)
{
    
    BleMeshNodeInit(pConfig);
    PCmdPacket pEvent;
    while (1) 
    {
    // Event pointer for handling events

    // If there are no events pending then the next call to gecko_wait_event()
    // may cause device go to deep sleep.
    // Make sure that debug prints are flushed before going to sleep
    if (gecko_event_pending()) { RETARGET_SerialFlush();  }

GetMeshEvent:    
    pEvent = gecko_wait_event(); // Check for stack event
    bool pass = mesh_bgapi_listener(pEvent);
    if (pass) 
        { 
        if(BleMeshEventProc(pEvent,BleEventFun) == FALSE)
            if(BleMeshEventProc(pEvent,MeshEventFun) == FALSE)
                EventIDtoStringProc(pEvent);
        goto GetMeshEvent;
        } 


    
    if(GetNodeStatus(STATUS_PROVISIONED))
        {
          //WaterLeveMeshProc(); //debug
          ComPortProc();    
          TimerEventTaskProc();
        }
    }
}


//**********************************************************************************************
//
//
//
//
//**********************************************************************************************
bool BleMeshEventProc(PCmdPacket pEvent, PEventFun pEventFun)
{
    bool ret_code = FALSE;
    uint32 event_id;
    if(NULL == pEvent) return ret_code;
    event_id = BGLIB_MSG_ID(pEvent->header); //get event ID

    while(pEventFun->pEventProc != NULL)
    {
        if(pEventFun->EventID == event_id)   // to process the event
        {
            pEventFun->pEventProc(pEvent);
            ret_code = TRUE;
            break;
        }
        pEventFun++;    // to next ID
    };
    return ret_code;
}


