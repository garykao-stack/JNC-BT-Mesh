#include "global.h"

//richard Add
/* BG stack headers */
#include "MeshFeatures.h"
#include "leds.h"
#include "sensor_client.h"
#include "sensor_server.h"
#include "mesh_event.h"



uint8_t init_done = 0;

//uint8 MeshNodeModel;
//**********************************************************************************************
//
//
//**********************************************************************************************
uint8 CheckMeshNodeModel()
{
    uint8 ret_code = MESH_SENSOR_MODEL_CLIENT;


    return ret_code;
}


//**********************************************************************************************
//
//
//**********************************************************************************************
void MeshEventInit()
{
    CurrServerNodeNum = 0;
    pActiveNode = TotalMeshNode;
    memset(TotalMeshNode,0,sizeof(TotalMeshNode));
    //MeshNodeModel = CheckMeshNodeModel();
}

EventFun    MeshEventFun[] =
{
    {Evt_mn_initialized,                EvtMeshSensorInitProc},

    ///////////////////////// for client event ///////////////////////////////////////////
    {Evt_ms_client_descriptor_status,   EvtMeshSensorClientProc},
    {Evt_ms_client_status,              EvtMeshSensorClientProc},
    {Evt_ms_client_series_status,       EvtMeshSensorClientProc},
    {Evt_ms_client_column_status,       EvtMeshSensorClientProc},
    {Evt_ms_client_publish,             EvtMeshSensorClientProc},

    ///////////////////////// for server event ///////////////////////////////////////////
    {Evt_ms_server_get_req,             EvtSensorServerEventsProc},
    {Evt_ms_server_get_column_req,      EvtSensorServerEventsProc},
    {Evt_ms_server_get_series_req,      EvtSensorServerEventsProc},
    {Evt_ms_server_publish,             EvtSensorServerEventsProc},
    {Evt_ms_setup_server_get_cadence_req,EvtSensorServerEventsProc},
    {Evt_ms_setup_server_set_cadence_req,EvtSensorServerEventsProc},
    {Evt_ms_setup_server_get_settings_req,EvtSensorServerEventsProc},
    {Evt_ms_setup_server_get_setting_req,EvtSensorServerEventsProc},
    {Evt_ms_setup_server_set_setting_req,EvtSensorServerEventsProc},

    {Evt_mg_server_client_req,          EvtMeshGenServerClientRequestProc},
    {Evt_mg_server_state_changed,       EvtMeshGenServerStateChangedProc},
    {Evt_mh_server_attention,           EvtMeshHealthServerAttentionProc},
    
    ///////////////////////// for Mesh Config event ///////////////////////////////////////
    {Evt_mn_config_set,                 EvtMeshConfigProc},
    {Evt_mn_config_get,                 EvtMeshConfigProc},
    ///////////////////////// for Proxy event /////////////////////////////////////////////
    {Evt_m_proxy_connected,             EvtMeshProxyProc},
    {Evt_m_proxy_disconnected,          EvtMeshProxyProc},
    {Evt_m_proxy_filter_status,         EvtMeshProxyProc},
    ///////////////////////// for provision event ////////////////////////////////////////
    {Evt_mn_provisioning_started,       EvtMeshNodeProvProc},
    {Evt_mn_provisioned,                EvtMeshNodeProvProc},
    {Evt_mn_provisioning_failed,        EvtMeshNodeProvProc},

    ///////////////////////// for other event ////////////////////////////////////////
    {Evt_mn_key_added,                  EvtMeshNodeKeyAddedProc},
    {Evt_mn_model_config_changed,       EvtMeshNodeModelConfigChangedProc}, // for client/server option
    {Evt_mn_reset,                      EvtMeshNodeResetProc},
        /////////////////////// For Friend Event //////////////////////////////////////////
#ifdef FRIEND_NODE
    {Evt_m_friend_friendship_established,  EvtMeshFriendProc},
    {Evt_m_friend_friendship_terminated,   EvtMeshFriendProc},
#endif    
    

    {0, NULL},  // End
};

//**********************************************************************************************
//
//**********************************************************************************************
bool MeshEventProc(PCmdPacket pEvent)
{
    return TRUE; //deug
    bool ret_code = FALSE;
    uint32 event_id;
    PEventFun pEventFun;
    if(NULL == pEvent) return ret_code;
    event_id = BGLIB_MSG_ID(pEvent->header); //get event ID
    pEventFun = MeshEventFun;    //initial event table

    while(pEventFun->pEventProc != NULL)
    {
        if(pEventFun->EventID == event_id)   // to process the event
        {
            ret_code = pEventFun->pEventProc(pEvent);
            break;
        }
        pEventFun++;    // to next ID
    };
    return ret_code;
}


//**********************************************************************************************
// for sensor server/client model initial by key pad
//
//**********************************************************************************************
bool MeshNodeModelInit()
{TraceProc();
    bool ret_code=TRUE;
   // uint8 node_model;
    if(MeshNodeStatus & STATUS_CLIENT) 
       { SensorClientNodeInit();}
    else
       { SensorServerNodeInit();}
    
    return ret_code;
}



//**********************************************************************************************
// Event: gecko_evt_mesh_node_initialized_id
//
//**********************************************************************************************
uint32 EvtMeshSensorInitProc(PCmdPacket pEvent)
{ TraceProc();
    uint32 ret_code = TRUE;
    msg_mn_initialized_evt *pMeshInit = &(pEvent->data.evt_mesh_node_initialized);

    if(pMeshInit->provisioned)
    {
        Printf("node is provisioned. address:%x, ivi:%ld\r\n", pMeshInit->address, pMeshInit->ivi);
        DI_Print("provisioned", DI_ROW_STATUS);
        SetNodeStatus(STATUS_PROVISIONED,ON);
        MeshNodeModelInit();
        SetTxPower(TX_POWER_HI);

        if(GetNodeStatus(STATUS_CLIENT) == TRUE) StartScanServerNode();  //to scan server node
#if 0
        gecko_cmd_mesh_node_set_adv_event_filter(0x7,0,NULL); //richard: for RSSI message
#endif  
        
        SetEventTaskTimer(TIMER_ID_DEVICE_TASK,TIMER_DEVICE_TASK,TIMER_EVENT_REPEAT); // set up task timer
        SetEventTaskTimer(TIMER_ID_SYS_RESET,TIMER_NO_SIGNAL,TIMER_EVENT_REPEAT); // system reset
    }
    else
    {
        Trace("node is unprovisioned");
        DI_Print("unprovisioned", DI_ROW_STATUS);
        //Trace("starting unprovisioned beaconing...");
        // Enable ADV and GATT provisioning bearer
        Cmd_mn_start_unprov_beaconing(PB_ADV | PB_GATT);
    }


    return ret_code;
}





/// Flag for indicating that initialization was performed
//**********************************************************************************************
// Event: gecko_evt_mesh_node_provisioning_started_id, gecko_evt_mesh_node_provisioned_id
//        gecko_evt_mesh_node_provisioning_failed_id
//
//**********************************************************************************************
uint32 EvtMeshNodeProvProc(PCmdPacket pEvent)
{TraceProc();
    uint32 ret_code = TRUE;
    uint32    event_id;
    event_id = BGLIB_MSG_ID(pEvent->header);
    msg_mn_proving_started_evt  *pProv_started = &pEvent->data.evt_mesh_node_provisioning_started;
    msg_mn_provisioned_evt      *pProvisioned = &pEvent->data.evt_mesh_node_provisioned;
    msg_mn_proving_failed_evt   *pProv_failed = &pEvent->data.evt_mesh_node_provisioning_failed;

    switch(event_id)
    {
        case Evt_mn_provisioning_started:
            TraceDec1("Evt_mn_provisioning_started", pProv_started->result);
            DI_Print("provisioning...", DI_ROW_STATUS);
#ifdef FEATURE_LED_BUTTON_ON_SAME_PIN
            led_init(); /* shared GPIO pins used as LED output */
#endif
            // start timer for blinking LEDs to indicate which node is being provisioned
            SetEventTaskTimer(TIMER_ID_PROVISIONING,250, TIMER_EVENT_REPEAT);
            SetNodeStatus(STATUS_PROVISIONING,ON);
            init_done = 0;
            break;

        case Evt_mn_provisioned:
            Trace("Evt_mn_provisioned");

            if(MeshNodeStatus & STATUS_CLIENT) Cmd_ms_client_init();
            else SensorServerNodeInit();

            SetNodeStatus(STATUS_PROVISIONED,ON);
        
            Printf("node provisioned, got address=0x%2.0X, ivi:%ld\r\n", pProvisioned->address, pProvisioned->iv_index);
            // stop LED blinking when provisioning complete
            SetEventTaskTimer(TIMER_ID_PROVISIONING,TIMER_REMOVE, TIMER_EVENT_REPEAT);
            led_set_state(LED_STATE_OFF);
            DI_Print("provisioned", DI_ROW_STATUS);
#ifdef FEATURE_LED_BUTTON_ON_SAME_PIN
            button_init(); /* shared GPIO pins used as button input */
#endif
            enable_button_interrupts();
            //if(MeshNodeStatus & STATUS_CLIENT) 
            //    SetEventTaskTimer(TIMER_ID_SENSOR_DESCRIPTOR,TIMER_SENSOR_DESCRIPTOR, TIMER_EVENT_ONCE);
            init_done = 1;
            SetNodeStatus(STATUS_PROVISIONING,OFF);
            break;

        case Evt_mn_provisioning_failed:
            Trace("Evt_mn_provisioning_failed");
            Printf("provisioning failed, code %x\r\n", pProv_failed->result);
            DI_Print("prov failed", DI_ROW_STATUS);
            // start a one-shot timer that will trigger soft reset after small delay
            SetEventTaskTimer(TIMER_ID_RESTART,2000, TIMER_EVENT_ONCE);
            SetNodeStatus(STATUS_PROVISIONING,OFF);
            break;

        default: TraceErr("EvtMeshNodeProvProc"); break;
    }
    return ret_code;
}

//**********************************************************************************************
// Event: gecko_evt_mesh_node_config_get_id, gecko_evt_mesh_node_config_set_id
//
//**********************************************************************************************
uint32 EvtMeshConfigProc(PCmdPacket pEvent)
{TraceProc();
    uint32 ret_code = TRUE;
    uint32 event_id;
    event_id = BGLIB_MSG_ID(pEvent->header);
    msg_mn_config_set_evt *p_config_set = &(pEvent->data.evt_mesh_node_config_set);
    msg_mn_config_get_evt *p_config_get = &(pEvent->data.evt_mesh_node_config_get);

    switch(event_id)
    {
        case Evt_mn_config_set: Trace("Evt_mn_config_set");
                PrintDataByte("EvtMeshConfigSetProc",p_config_set->value.data,p_config_set->value.len);
                Trace16_2(p_config_set->id, p_config_set->netkey_index);
            break;
        case Evt_mn_config_get: Trace("Evt_mn_config_get");
                Trace16_2(p_config_get->id, p_config_get->netkey_index);
            break;
       default: TraceErr("EvtMeshConfigProc"); break;     
    }
    return ret_code;
}


//**********************************************************************************************
// Event:   gecko_evt_mesh_proxy_connected_id, gecko_evt_mesh_proxy_disconnected_id, 
//          gecko_evt_mesh_proxy_filter_status_id
//
//**********************************************************************************************
uint32 EvtMeshProxyProc(PCmdPacket pEvent)
{TraceProc();
    uint32 ret_code = TRUE;
    uint32 event_id;
    event_id = BGLIB_MSG_ID(pEvent->header);
    msg_m_proxy_connected_evt *p_connected = &(pEvent->data.evt_mesh_proxy_connected);
    msg_m_proxy_disconnected_evt *p_disconnected = &(pEvent->data.evt_mesh_proxy_disconnected);
    msg_m_proxy_filter_status_evt *p_filter = &(pEvent->data.evt_mesh_proxy_filter_status);

    switch(event_id)
    {
        case Evt_m_proxy_connected: Trace1("Evt_m_proxy_connected",p_connected->handle);
            SetNodeStatus(STATUS_PROXY_CONNECT,ON);
            break;
        case Evt_m_proxy_disconnected: Trace2("Evt_m_proxy_disconnected",p_disconnected->handle,p_disconnected->reason);
            SetNodeStatus(STATUS_PROXY_CONNECT,OFF);
            //if(!(MeshNodeStatus & STATUS_PROVISIONING)) StartGetServerProperty(); // start to get property
            break;
        case Evt_m_proxy_filter_status: Trace3("Evt_m_proxy_filter_status",p_filter->handle, p_filter->type, p_filter->count);
            break;
       default: TraceErr("EvtMeshConfigProc"); break;     
    }
    return ret_code;
}


//**********************************************************************************************
// Event: gecko_evt_mesh_node_key_added_id
//
//**********************************************************************************************
uint32 EvtMeshNodeKeyAddedProc(PCmdPacket pCmdEvent)
{TraceProc();
    uint32 ret_code = TRUE;


    return ret_code;
}

//**********************************************************************************************
// Event: gecko_evt_mesh_node_model_config_changed_id
//
//**********************************************************************************************
uint32 EvtMeshNodeModelConfigChangedProc(PCmdPacket pCmdEvent)
{TraceProc();
    uint32 ret_code = TRUE;
    msg_mn_model_config_changed_evt *pEvent = &(pCmdEvent->data.evt_mesh_node_model_config_changed);
    Trace16Ptr_4(pEvent, mesh_node_config_state, element_address, vendor_id, model_id);

    return ret_code;
}



//**********************************************************************************************
// Event: gecko_evt_mesh_node_reset_id
//
//**********************************************************************************************
uint32 EvtMeshNodeResetProc(PCmdPacket pCmdEvent)
{ TraceProc();
    uint32 ret_code = TRUE;
    printf("evt: gecko_evt_mesh_node_reset_id\r\n");
    initiate_factory_reset();


    return ret_code;
}


//**********************************************************************************************
// Event: gecko_evt_mesh_generic_server_client_request_id
//
//**********************************************************************************************
uint32 EvtMeshGenServerClientRequestProc(PCmdPacket pEvent)
{ TraceProc();
    uint32 ret_code = TRUE;


    return ret_code;
}


//**********************************************************************************************
// Event: gecko_evt_mesh_generic_server_state_changed_id
//
//**********************************************************************************************
uint32 EvtMeshGenServerStateChangedProc(PCmdPacket pEvent)
{ TraceProc();
    uint32 ret_code = TRUE;


    return ret_code;
}


//**********************************************************************************************
// Event: gecko_evt_mesh_health_server_attention_id
//
//**********************************************************************************************
uint32 EvtMeshHealthServerAttentionProc(PCmdPacket pEvent)
{ TraceProc();
    uint32 ret_code = TRUE;
    struct gecko_msg_mesh_health_server_attention_evt_t *pData = (void *) & (pEvent->data);
   Trace16_2(pData->timer, pData->elem_index);

    return ret_code;
}


#ifdef FRIEND_NODE
//**********************************************************************************************
// Event: gecko_evt_mesh_lpn_friendship_established_id, gecko_evt_mesh_lpn_friendship_failed_id
//          gecko_evt_mesh_lpn_friendship_terminated_id
//
//**********************************************************************************************
uint32 EvtMeshLpnFriendProc(PCmdPacket pEvent)
{
    TraceProc();
    uint32 ret_code = TRUE;
    uint32    event_id;
    uint16  lpn_address;
    event_id = BGLIB_MSG_ID(pEvent->header);

    switch(event_id)
    {
        case Evt_m_friend_friendship_established:
            lpn_address = pEvent->data.evt_mesh_friend_friendship_established.lpn_address;
            Trace1("Evt_m_friend_friendship_established lpn_address",lpn_address);
            break;
        case Evt_m_friend_friendship_terminated: 
            result = pEvent->data.evt_mesh_friend_friendship_terminated.reason;
            Trace1("Evt_m_friend_friendship_terminated",result);
            break;
    };            

    return ret_code;
}

#endif



