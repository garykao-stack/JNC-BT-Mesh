#include "global.h"

//richard Add
#include "bus_usart.h"
/* BG stack headers */
#include "MeshFeatures.h"
#include "leds.h"
#include "ivi_features.h"
#include "sensor_client.h"
#include "sensor_server.h"
#include "Mesh_node.h"
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
    {Evt_ms_client_setting_status,      EvtMeshSensorClientProc},

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
// for IVI
    {Evt_mn_changed_ivupdate_state,     EvtMeshIviProc},    
    {Evt_mn_ivrecovery_needed,          EvtMeshIviProc},

    {0, NULL},  // End
};

//**********************************************************************************************
// for sensor server/client model initial by key pad
//
//**********************************************************************************************
bool MeshNodeModelInit()
{
    bool ret_code=TRUE;
   // uint8 node_model;
   if(NodeRole == NR_CLIENT)
       { SensorClientNodeInit(); 
        }
    else
       { SensorServerNodeInit();
         //SetServerAllModel();
        }
    return ret_code;
}


#define MODEL_SENSOR_SERVER                 0x1100
#define MODEL_SENSOR_SETUP_SERVER           0x1101


//
//
//
void SetServerAllModel()
{//
    msg_mt_get_local_model_pub_rsp *p_get_model_pub;

    return; // for Debug
/*    
    result = Cmd_mt_bind_local_model_app(0,0,0xFFFF,MODEL_SENSOR_SETUP_SERVER)->result;
    if(result) TraceErr1("Cmd_mt_bind_local_model_app",result); 
    else TraceOk("Cmd_mt_bind_local_model_app");

    p_get_model_pub = Cmd_mt_get_local_model_pub(0,0xFFFF,MODEL_SENSOR_SETUP_SERVER);
    if(p_get_model_pub->result == 0) 
        {
        Trace16Ptr_4(p_get_model_pub, appkey_index,pub_address, ttl, period);
        Trace16Ptr_2(p_get_model_pub, retrans,credentials);
            
        result = gecko_cmd_mesh_test_set_local_model_pub(
            0,0,0xFFFF,MODEL_SENSOR_SETUP_SERVER, //model id
            0xC100, //p_get_model_pub->pub_address,
            p_get_model_pub->ttl,
            p_get_model_pub->period, p_get_model_pub->retrans,
            p_get_model_pub->credentials)->result;
        
        if(result) TraceErr1("gecko_cmd_mesh_test_set_local_model_pub",result); 
        else TraceOk1("gecko_cmd_mesh_test_set_local_model_pub",result);
        }
    else TraceErr1("Cmd_mt_get_local_model_pub",p_get_model_pub->result);
    result = gecko_cmd_mesh_test_add_local_model_sub(0, 0xFFFF,MODEL_SENSOR_SETUP_SERVER,0xC000)->result;
    if(result) TraceErr1("gecko_cmd_mesh_test_add_local_model_sub",result); 
        else TraceOk1("gecko_cmd_mesh_test_add_local_model_sub",result);

  */
}

void GetLocalCfg()
{
    uchar state=ON;
    struct gecko_msg_mesh_test_get_local_config_rsp_t* p_local_cfg;
    p_local_cfg = Cmd_mt_get_local_config(mesh_node_dcd,pMeshNodeData->NetkeyIndex);
    if(p_local_cfg->result) {TraceErr1("Get DCD",p_local_cfg->result);}
    else PrintDataByte("Get DCD",p_local_cfg->data.data, p_local_cfg->data.len);

    struct gecko_msg_mesh_test_get_local_model_pub_rsp_t* p_get_model_pub;

    p_get_model_pub = gecko_cmd_mesh_test_get_local_model_pub(0,0xffff,MODEL_ID_CLIENT);
    if(p_get_model_pub->result) TraceErr1("MODEL_ID_CLIENT",p_get_model_pub->result);
    else{ 
        TraceOk("MODEL_ID_CLIENT");
        Trace16_1(p_get_model_pub->appkey_index);Trace16_1(p_get_model_pub->pub_address);
        Trace8_1(p_get_model_pub->ttl);Trace8_1(p_get_model_pub->period);
        Trace8_1(p_get_model_pub->retrans);Trace8_1(p_get_model_pub->credentials);
    
        }

    p_get_model_pub = gecko_cmd_mesh_test_get_local_model_pub(0,0xffff,MODEL_ID_SETUP_SERVER);
    if(p_get_model_pub->result) TraceErr1("MODEL_ID_SETUP_SERVER",p_get_model_pub->result);
    else{TraceOk("MODEL_ID_SETUP_SERVER");
        
        Trace16_1(p_get_model_pub->appkey_index);Trace16_1(p_get_model_pub->pub_address);
        Trace8_1(p_get_model_pub->ttl);Trace8_1(p_get_model_pub->period);
        Trace8_1(p_get_model_pub->retrans);Trace8_1(p_get_model_pub->credentials);
        }
}


#define GRP_SVR_PUB_ADDRESS         0xC100
#define GRP_SVR_PUB_TTL             3
#define GRP_SVR_PUB_PERIOD          0
#define GRP_SVR_PUB_RETRANS         0
#define GRP_SVR_PUB_CREDENTIALS     0
#define GRP_SVR_SUB_ADDRESSES       { 0xC000 }

#define GRP_CLI_PUB_ADDRESS         0xC000
#define GRP_CLI_PUB_TTL             3
#define GRP_CLI_PUB_PERIOD          0
#define GRP_CLI_PUB_RETRANS         0
#define GRP_CLI_PUB_CREDENTIALS     0
#define GRP_CLI_SUB_ADDRESSES       { 0xC100 }

#define MAX_PUB_SUB_MODELS          20


//uint16 MeshNodeID;
//uint32 MeshNodeIVI;

//**********************************************************************************************
// Event: gecko_evt_mesh_node_initialized_id
//
//**********************************************************************************************
uint32 EvtMeshSensorInitProc(PCmdPacket pEvent)
{TraceProc();
    uint32 ret_code = TRUE;
    msg_mn_initialized_evt *pMeshInit = &(pEvent->data.evt_mesh_node_initialized);
    if(NodeRole == NR_CLIENT)
        iv_config(IV_TEST_MODE, IV_RECOVERY_MODE, SNB_STATE);
    else
        iv_config(IV_TEST_MODE, 0, SNB_STATE);


    if(pMeshInit->provisioned)
    { //GetLocalCfg();  //debug
        SetMeshNodeStatus(STATUS_PROVISIONED,ON);  
        pMeshNodeData->MeshNodeID = pMeshInit->address;
        pMeshNodeData->IvIndex = pMeshInit->ivi;
        Printf("node1 is provisioned. Mesh Node ID:0x%x, %d, IVI:%ld\r\n", 
                pMeshNodeData->MeshNodeID,pMeshNodeData->MeshNodeID, pMeshNodeData->IvIndex);
        MeshNodeModelInit();
        SetTxPower(TX_POWER_LO);
        SetEventTaskTimer(TD_GET_SENSOR_INFO,200,TIMER_EVENT_ONCE);        
#if BT_RSSI
        Cmd_mn_set_adv_event_filter(0xF,0,NULL); //for RSSI event
#endif  
        DI_Print("provisioned", DI_ROW_STATUS);
        SetNodePublish(OFF); SetNodeSleeping(OFF);
        
    if(NodeRole == NR_SETUP) {Trace("BT Mesh Setup ON 1");
        SetEventTaskTimer(TD_PROVISIONING,TIMER_UNPROVISION,TIMER_EVENT_REPEAT);
        SetLed(LED_RED,OFF);SetLed(LED_BLUE,ON);
    }
    
    }
    else
    {
        //Trace("starting unprovisioned beaconing...");// Enable ADV and GATT provisioning bearer
        Cmd_mn_start_unprov_beaconing(PB_ADV | PB_GATT); 
        Trace("node is unprovisioned: starting unprovisioned beaconing..."); //DI_Print("unprovisioned", DI_ROW_STATUS);
        SetLedStatus(LED_STATUS_OFF);
        if(NodeRole != NR_SETUP)
            {
             SetLedStatus(LED_STATUS_OFF);
             SetEventTaskTimer(TD_UNPROVISION,TIMER_UNPROVISION,TIMER_EVENT_REPEAT); // system reset
            }
        else{Trace("BT Mesh Setup ON 2");
            SetLed(LED_RED,OFF);SetLed(LED_BLUE,ON);
            SetEventTaskTimer(TD_PROVISIONING,TIMER_UNPROVISION,TIMER_EVENT_REPEAT);
            }
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
{
    uint32 ret_code = TRUE;
    uint32    event_id;
    event_id = BGLIB_MSG_ID(pEvent->header);
    msg_mn_proving_started_evt  *pProv_started = &pEvent->data.evt_mesh_node_provisioning_started;
    msg_mn_provisioned_evt      *pProvisioned = &pEvent->data.evt_mesh_node_provisioned;
    msg_mn_proving_failed_evt   *pProv_failed = &pEvent->data.evt_mesh_node_provisioning_failed;

    switch(event_id)
    {
        case Evt_mn_provisioning_started: TraceDec1("Evt_mn_provisioning_started", pProv_started->result);
            DI_Print("provisioning...", DI_ROW_STATUS);
            // start timer for blinking LEDs to indicate which node is being provisioned
            SetEventTaskTimer(TD_PROVISIONING,TIMER_PROVISION, TIMER_EVENT_REPEAT);
            SetEventTaskTimer(TD_UNPROVISION,TIMER_ENDING, TIMER_EVENT_REPEAT);
            SetLedStatus(LED_STATUS_START_PROV);
            init_done = 0;
            break;

        case Evt_mn_provisioned: Trace("Evt_mn_provisioned");
            
            if(NodeRole == NR_CLIENT)
                Cmd_ms_client_init();
            else 
                SensorServerNodeInit();
            
            Printf("node provisioned, got address=0x%2.0X, ivi:%ld\r\n", pProvisioned->address, pProvisioned->iv_index);
            DI_Print("provisioned", DI_ROW_STATUS);
            init_done = 1;
           SetEventTaskTimer(TD_PROVISIONING,TIMER_ENDING, TIMER_EVENT_REPEAT);            
            break;

        case Evt_mn_provisioning_failed:  Trace("Evt_mn_provisioning_failed");
            Printf("provisioning failed, code %x\r\n", pProv_failed->result);
            DI_Print("prov failed", DI_ROW_STATUS);
            // start a one-shot timer that will trigger soft reset after small delay
            SetEventTaskTimer(TD_RESTART,2000, TIMER_EVENT_ONCE);
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
{
    uint32 ret_code = TRUE;
    uint32 event_id;
    uint16 netkey_index;
    event_id = BGLIB_MSG_ID(pEvent->header);
    msg_mn_config_set_evt *p_config_set = &(pEvent->data.evt_mesh_node_config_set);
    msg_mn_config_get_evt *p_config_get = &(pEvent->data.evt_mesh_node_config_get);
    //pMeshNodeData->NetkeyIndex = p_config_set->netkey_index; TraceDec1("netkey index 1",pMeshNodeData->NetkeyIndex);
    //WriteNodeData();

    switch(event_id)
    {
        case Evt_mn_config_set: //Trace("Evt_mn_config_set");
                //PrintDataByte("EvtMeshConfigSetProc",p_config_set->value.data,p_config_set->value.len);
                SetEventTaskTimer(TD_SYS_SETUP_RESET,TIMER_SYS_SETUP,TIMER_EVENT_ONCE); // system reset
                //Trace16_2(p_config_set->id, p_config_set->netkey_index);
            break;
        case Evt_mn_config_get: //Trace("Evt_mn_config_get");
                //Trace16_2(p_config_get->id, p_config_get->netkey_index);
               
            break;
       default: TraceErr("EvtMeshConfigProc"); break;     
    }
    SetLedToggle(LED_SERVER);
    return ret_code;
}


//**********************************************************************************************
// Event:   gecko_evt_mesh_proxy_connected_id, gecko_evt_mesh_proxy_disconnected_id, 
//          gecko_evt_mesh_proxy_filter_status_id
//
//**********************************************************************************************
uint32 EvtMeshProxyProc(PCmdPacket pEvent)
{
    uint32 ret_code = TRUE;
    uint32 event_id;
    event_id = BGLIB_MSG_ID(pEvent->header);
    msg_m_proxy_connected_evt *p_connected = &(pEvent->data.evt_mesh_proxy_connected);
    msg_m_proxy_disconnected_evt *p_disconnected = &(pEvent->data.evt_mesh_proxy_disconnected);
    msg_m_proxy_filter_status_evt *p_filter = &(pEvent->data.evt_mesh_proxy_filter_status);

    switch(event_id)
    {
        case Evt_m_proxy_connected: Trace1("Evt_m_proxy_connected",p_connected->handle);
            SetEventTaskTimer(TD_SYS_SETUP_RESET,TIMER_ENDING,TIMER_EVENT_ONCE); // for Reset
            SetMeshNodeStatus(STATUS_PROXY_CONNECT,ON);
            break;
        case Evt_m_proxy_disconnected: Trace2("Evt_m_proxy_disconnected",p_disconnected->handle,p_disconnected->reason);
            SetMeshNodeStatus(STATUS_PROXY_CONNECT,OFF);
            //SetEventTaskTimer(TD_SYS_SETUP_RESET,TIMER_SYS_SETUP,TIMER_EVENT_ONCE);
            SetEventTaskTimer(TD_SYS_SETUP_RESET,TIMER_5SEC,TIMER_EVENT_ONCE);
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
{ 
    uint32 ret_code = TRUE; 
    msg_mn_key_added_evt* p_key_added = &(pCmdEvent->data.evt_mesh_node_key_added);
   // Printf("got new %s key with index %x netkey_index %x\r\n", p_key_added->type == 0 ? "network" : "application",
    //       p_key_added->index,p_key_added->netkey_index);

    if(p_key_added->type == 0)
        {//TraceDec2("Add netkey index",p_key_added->index,p_key_added->netkey_index);
        pMeshNodeData->NetkeyIndex = p_key_added->index;    // netkey_index
        }
    else
        {//TraceDec2("Add appkey index",p_key_added->index,p_key_added->netkey_index);
        pMeshNodeData->AppkeyIndex = p_key_added->index;    // appkey_index
        }
    //Trace1("Add key Netkey Index",p_key_added->netkey_index);
    WriteNodeData();
    return ret_code;
}

//**********************************************************************************************
// Event: gecko_evt_mesh_node_model_config_changed_id
//
//**********************************************************************************************
uint32 EvtMeshNodeModelConfigChangedProc(PCmdPacket pCmdEvent)
{
    uint32 ret_code = TRUE;
    msg_mn_model_config_changed_evt *pEvent = &(pCmdEvent->data.evt_mesh_node_model_config_changed);
   
    if(pEvent->model_id == MODEL_ID_SERVER)
        pMeshNodeData->MeshNodeRole = NR_SERVER;
    else if(pEvent->model_id == MODEL_ID_SETUP_SERVER)
        pMeshNodeData->MeshNodeRole = NR_SETUP_SERVER;    
    else if(pEvent->model_id == MODEL_ID_CLIENT)
        pMeshNodeData->MeshNodeRole = NR_CLIENT;
    SetLedToggle(LED_SERVER);
    WriteNodeData();
    Trace16Ptr_4(pEvent, mesh_node_config_state, element_address, vendor_id, model_id);
    SetEventTaskTimer(TD_SYS_SETUP_RESET,TIMER_SYS_SETUP,TIMER_EVENT_ONCE);

    return ret_code;
}



//**********************************************************************************************
// Event: gecko_evt_mesh_node_reset_id
//
//**********************************************************************************************
uint32 EvtMeshNodeResetProc(PCmdPacket pCmdEvent)
{ 
    uint32 ret_code = TRUE;
    printf("evt: gecko_evt_mesh_node_reset_id\r\n");
    initiate_factory_reset();
   // Cmd_sys_reset(0);

    return ret_code;
}


//**********************************************************************************************
// Event: gecko_evt_mesh_generic_server_client_request_id
//
//**********************************************************************************************
uint32 EvtMeshGenServerClientRequestProc(PCmdPacket pEvent)
{ 
    uint32 ret_code = TRUE;


    return ret_code;
}


//**********************************************************************************************
// Event: gecko_evt_mesh_generic_server_state_changed_id
//
//**********************************************************************************************
uint32 EvtMeshGenServerStateChangedProc(PCmdPacket pEvent)
{ 
    uint32 ret_code = TRUE;


    return ret_code;
}


//**********************************************************************************************
// Event: gecko_evt_mesh_health_server_attention_id
//
//**********************************************************************************************
uint32 EvtMeshHealthServerAttentionProc(PCmdPacket pEvent)
{ 
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



