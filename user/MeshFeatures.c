#include "global.h" 
#include "init_board.h"
#include "sensor_client.h"
#include "cmd_to_bt_mesh.h"
#include "MeshFeatures.h"


//**********************************************************************************************
// Event: gecko_evt_mesh_lpn_friendship_established_id, gecko_evt_mesh_lpn_friendship_failed_id
//          gecko_evt_mesh_lpn_friendship_terminated_id
//
//**********************************************************************************************
uint32 EvtMeshFriendProc(PCmdPacket pEvent)
{
    
    uint32 ret_code = TRUE;
    uint32    event_id;
    uint16  lpn_address;
    event_id = BGLIB_MSG_ID(pEvent->header);

    switch(event_id)
    {
        case Evt_m_friend_friendship_established:
            lpn_address = pEvent->data.evt_mesh_friend_friendship_established.lpn_address;
            //Trace1("Evt_m_friend_friendship_established lpn_address",lpn_address);
            break;
        case Evt_m_friend_friendship_terminated: 
            result = pEvent->data.evt_mesh_friend_friendship_terminated.reason;
            //Trace1("Evt_m_friend_friendship_terminated",result);
            break;
    };            

    return ret_code;
}


//
//
Result NodeSleeping()
{
    if(GetMeshNodeStatus(STATUS_FULL_POWER)) {PowerMode(POWER_MODE_WAKEUP);return RESULT_OK;}

    result = NodeLpn(ON);
    result += NodeProxy(OFF);  // App proxy link bug error
    SetMeshNodeStatus(STATUS_SLEEPING,ON);
    PowerMode(POWER_MODE_SLEEP);
    if(result) TraceErr1("DeviceSleeping",result);
    return result;    
}

//
//
Result NodeWakeUp()
{
    PowerMode(POWER_MODE_WAKEUP);
    result = NodeLpn(OFF);
    result += NodeProxy(ON);
    SetMeshNodeStatus(STATUS_SLEEPING,OFF);
    if(result) TraceErr1("DeviceWakeUp",result);
    return result;    
}




//
// set LPN ON/OFF
//
Result NodeLpn(uint8 status)
{

    
    if(status == ON)
        {
        result = Cmd_m_lpn_init()->result;    ShowResult("gecko_cmd_mesh_lpn_init", result);
        if (result) return result;
        
        result = Cmd_m_lpn_config(mesh_lpn_queue_length,2)->result; ShowResult("mesh_lpn_queue_length", result);
        if (result)  return result;
          
        result = Cmd_m_lpn_config(mesh_lpn_poll_timeout,LPN_POLL_TIMEOUT)->result; ShowResult("mesh_lpn_poll_timeout", result);
         if (result)  return result;
        
         /*
        result = Cmd_m_lpn_config(mesh_lpn_receive_delay,10)->result; ShowResult("mesh_lpn_receive_delay", result);
        if (result)  return result;
        
        result = Cmd_m_lpn_config(mesh_lpn_request_retries,0)->result; ShowResult("mesh_lpn_request_retries", result);
        if (result)  return result;
        
        result = Cmd_m_lpn_config(mesh_lpn_retry_interval,0)->result; ShowResult("mesh_lpn_retry_interval", result);
        if (result)  return result;
         */              
        }
    else
        {// LPN OFF
        result = Cmd_m_lpn_deinit()->result;  ShowResult("LPN deinit failed", result);
         return result;
        }
  //  return result;    
}




//*******************************************************************************************
// Mesh Proxy ON/OFF
//*******************************************************************************************
Result NodeProxy(uint8 status)
{
    //if(status == ON) Trace("Proxy ON"); else Trace("Proxy OFF");    
    result = gecko_cmd_mesh_test_set_local_config(mesh_node_gatt_proxy, 0, 1, &status)->result;
    if(result) Trace1("NodeProxy Error",result);

    return result;
}

//*******************************************************************************************
// Mesh Beacon ON/OFF
//*******************************************************************************************
Result NodeBeacon(uint8 status)
{

   // if(status == ON) Trace("Beacon ON"); else Trace("Beacon OFF");    
    result = gecko_cmd_mesh_test_set_local_config(mesh_node_beacon, 0, 1, &status)->result;
    if(result) Trace1("Beacon Error",result);

    return result;
}


//*******************************************************************************************
// Mesh Relay ON/OFF
//*******************************************************************************************
Result NodeRelay(uint8 status)
{
    
    //if(status == ON) Trace("Relay ON"); else Trace("Relay OFF");    
    result = gecko_cmd_mesh_test_set_local_config(mesh_node_relay, 0, 1, &status)->result;
    if(result) Trace1("Relay Error",result);

    return result;
}


//*******************************************************************************************
// Friend Status ON/OFF
//*******************************************************************************************
Result NodeFriend(uint8 status)
{
#ifdef FRIEND_NODE        
    if(status == ON)
        {
            result = Cmd_m_friend_init()->result; ShowResult("Friend ON",result);
        }
    else{
            result = Cmd_m_friend_deinit()->result; ShowResult("Friend OFF",result);
        }
    return result;
#else 
    return FALSE;
#endif 
}




/////////////// for Proxy Function ///////////////////////////////////





