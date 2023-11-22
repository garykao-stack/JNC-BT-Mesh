
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

#include "ble_comm.h"
#include"Mesh_node.h"

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
  gecko_bgapi_class_mesh_test_init();
  gecko_bgapi_class_mesh_lpn_init(); //richard add 
#ifdef FRIEND_NODE
  gecko_bgapi_class_mesh_friend_init(); // for friend node
#endif  
  
}

const char* SensorClassStr[]=
{
    "NO Sensor","Auto Scan","PZEM","Visual Sensor","DC600","FTM94","BTM-G6","BTM485"
};
extern void DebugShowSetting();
extern uint32 IndexToBaudrate(uint8);
//***************************************************************************
// Initial BLE Stack, IO port,
//
//
//****************************************************************************
void BleMeshNodeInit(gecko_configuration_t *pConfig)
{
    uint8 sensor_index;
    gecko_stack_init(pConfig);
    gecko_bgapi_classes_init();
   // gecko_initCoexHAL();      // Initialize coexistence interface. Parameters are taken from HAL config.
//#ifndef DEBUG_PRINT    
    RETARGET_SerialInit();    // Initialize debug prints and display interface
//#endif    
    DI_Init();                //Initialize Display Interface.
    //Trace("123456789");     while(1);
    //gecko_init_afh(); //richard add
    
    // Initialize LEDs and buttons. Note: some radio boards share the same GPIO
    // for button & LED. Initialization is done in this order so that default
    // configuration will be "button" for those radio boards with shared pins.
    // led_init() is called later as needed to (re)initialize the LEDs
    UDELAY_Calibrate();
    led_init();  
    button_init();

    // 以下兩個函式都有溫濕度讀取
    DeviceInit();
    BleCommInit();
    
#if defined(JNC_DO_485)
        printf("%s: Firmware Version for DO-485 Only ==> v%1.2f %02d Sec \r\n\r\n",MODEL_NAME, 1.00, TIMER_GET_INFO_SLEEPING);
#elif defined(PZEM)
        printf("%s: Firmware Version for PZEM Only ==> v%1.2f %02d Sec \r\n\r\n",MODEL_NAME, 1.00, TIMER_GET_INFO_SLEEPING);
#elif defined(OEM_SENSOR)
        printf("%s: Firmware Version for OEM_SENSOR Only ==> v%1.2f %02d Sec \r\n\r\n",MODEL_NAME, 1.00, TIMER_GET_INFO_SLEEPING);
#else 
        printf("%s: Firmware Version ==> v%1.2f \r\n", MODEL_NAME, FW_VER/100.0);
#endif
    sensor_index = SensorClassChange(pMeshNodeData->SensorClass,CLASS_TO_UTILITY);
    //msg_mn_get_element_addr_rsp* r_addr=Cmd_mn_get_element_addr(0);

    printf("\r\nMAC:%02X:%02X\r\n"
    		"ID:%d\r\n"
    		"baudrate:%d\r\n"
    		"Sensor Class = %s\r\n"
    		"Working Timer = %d sec\r\n"
    		"RebootForRs485IdelSecnods= %d sec\r\n"
    		"RebootMinutes=%d min\r\n"
    		"Temp-Gain = %0.2f, Temp-Offset = %0.2f\r\n"
    		"RH-Gain   = %0.2f, RH-Offset   = %0.2f\r\n"
#if defined(BTM_TRANSMITTER) || defined(JNC_BT_MESH)
    		"RS485 Client Buff Trigger Timeout = %d ms\r\n"
    		"RS485 Server Sleep After Response = %d sec\r\n"
#endif
    		"\r\n",
    		pMeshNodeData->ElementAddr>>8,
			pMeshNodeData->ElementAddr&0xff,
			pMeshNodeData->MeshNodeID,
			IndexToBaudrate(pMeshNodeData->BaudRate),
			SensorClassStr[sensor_index],
			TIMER_GET_INFO_SLEEPING,
			pMeshNodeData->RebootForRs485IdelSecnods,
			pMeshNodeData->RebootMinutes,
			pAdjValue->TempGain,
			pAdjValue->TempOffset,
			pAdjValue->HumGain,
			pAdjValue->HumOffset
#if defined(BTM_TRANSMITTER) || defined(JNC_BT_MESH)
			,pAdjValue->RS485TransmitterData.Rs485ClientBuffTimeoutMs,
			pAdjValue->RS485TransmitterData.Rs485ServerDelayBeforeSleep
#endif
			);




    if(NodeRole == NR_CLIENT){
         gecko_bgapi_class_mesh_sensor_client_init();

    }else{
         gecko_bgapi_class_mesh_sensor_server_init();
         gecko_bgapi_class_mesh_sensor_setup_server_init();

    }
    BleEventInit();
    MeshEventInit();
    DebugShowSetting();

    /*msg_coex_get_counters_rsp *rsp_cnt;
    rsp_cnt=gecko_cmd_coex_get_counters(0);
    dprint("get_counters result:%d\r\n",rsp_cnt->result);
    for(int i=0;i<rsp_cnt->counters.len;i++){
    	dprint("%d: %d\r\n",i,rsp_cnt->counters.data[i]);
    }*/

    /*msg_mn_get_seq_remaining_rsp* seq_remain;
    seq_remain=gecko_cmd_mesh_node_get_seq_remaining(0);
    dprint("seq_remain, result:0x%x, count:%d\r\n",seq_remain->result,seq_remain->count);*/
}
/*******************************************************************************
 * Main application code.
 * @param[in] pConfig  Pointer to stack configuration.
 ******************************************************************************/
extern EventFun BleEventFun[],MeshEventFun[];
//void WaterLeveMeshProc(void);
//void ComPortProc(void);

uint32  debug_count=0;
void ServerNodeTask();
void ClientNodeTask();
void BtMeshSetupTask();

extern void UsartClientProc();
//int cntMainLoop=0;

void appMain(gecko_configuration_t *pConfig)
{
    
    BleMeshNodeInit(pConfig);
    PCmdPacket pEvent;
    dprint("Sensor Info Size:%d\r\n",sizeof(_SensorInfo));

    while(1)
    {
    	//cntMainLoop++;
		// Event pointer for handling events

		// If there are no events pending then the next call to gecko_wait_event()
		// may cause device go to deep sleep.
		// Make sure that debug prints are flushed before going to sleep
	//GetMeshEvent:
		if (gecko_event_pending()) { RETARGET_SerialFlush();  }
		pEvent = gecko_wait_event(); // Check for stack event
		bool pass = mesh_bgapi_listener(pEvent);
		if (pass) {
			if(BleMeshEventProc(pEvent,BleEventFun) == FALSE)
				if(BleMeshEventProc(pEvent,MeshEventFun) == FALSE)
					EventIDtoStringProc(pEvent);
		}
		if(!CheckRunDevTask()) continue;

		//dprint("===\r\nMax Sensor Info Size:%d\r\n===\r\n",sizeof(_SensorInfo));


		//dprint("Node Role:%d\r\n",NodeRole);
		UsartClientProc();
		if(NodeRole == NR_CLIENT)
			ClientNodeTask();
		else if(NodeRole == NR_SERVER || NodeRole == NR_SETUP_SERVER)
			ServerNodeTask();
		else BtMeshSetupTask(); //window Utility


    } 
}

bool CheckRunDevTask()
{
    bool ret_code=TRUE;
    if(!GetMeshNodeStatus(STATUS_PROVISIONED) && (NodeRole != NR_SETUP)) 
        ret_code = FALSE;

    return ret_code;
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



