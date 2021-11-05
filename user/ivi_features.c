
#include "global.h"

//#include "em_core.h"

//richard Add
/* BG stack headers */
#include "sensor_server.h"
#include "mesh_event.h"
#include "device_bus.h"
#include "mod_bus.h"
#include "com_port.h"
#include "bus_usart.h"
#include "sensor_client.h"
#include "sensor_server.h"
//#include "modbus_to_mesh.h"
#include "cmd_to_bt_mesh.h"
#include "Mesh_node.h"
#include "ivi_features.h"


void IviInit()
{
}

//
//
//
bool IviUpdate(uchar status)
{
   
    bool ret_code = TRUE;
    if(status == ENABLE)
    {
        Trace("***** IV index update Enable");
    }
    else
    {
        Trace("xxxxx IV index update Disable");
    }

    return ret_code;
}


void iv_config(uint8_t iv_test_mode, uint8_t iv_recovery_mode, uint8_t snb_state)
{


    Printf("%s IV test mode %s\r\n", iv_test_mode ? "Enable" : "Disable",
           Cmd_mt_set_ivupdate_test_mode(iv_test_mode)->result ? "FAILED" : "SUCCESS");
    Printf("Set IV Recovery Mode %s %s\r\n", iv_recovery_mode ? "Enable" : "Disable",
           Cmd_mn_set_ivrecovery_mode(iv_recovery_mode)->result ? "FAILED" : "SUCCESS");
    Printf("%s SNB %s\r\n", snb_state ? "Enable" : "Disable",
    Cmd_mt_set_local_config(mesh_node_beacon, 0, 1, &snb_state)->result ? "FAILED" : "SUCCESS");
}
 

//
//
//
void NodeIviUpdateProc(void)
{
        pStageInfo = GetNodeStageInfo(NODE_IVI_UPDATE_PROC);

        switch(ActiveStage())
            {
                case NODE_STAGE_INIT:   //Trace("IVI_UPDATE_INIT");
                    ToNextStage(IVI_DETECT);   //default
                    break;
                case IVI_DETECT: 
                    if(GetNodeStatus(NS_IVI_UPDATE) == ON)
                        {//Trace("To IVI_UPDATE_ACTION 1");
                         ToWaitingStage(IVI_UPDATE_ACTION,WAIT_SEC(1));
                        }
                    break;
                    
                case IVI_UPDATE_ACTION: 
                    IvIndexUpdate(ON);
                    ToWaitingStage(IVI_ACTION_WAIT,WAIT_SEC(5));
                    break;
                case IVI_ACTION_WAIT: 
                    if(CheckWaitTimeOut()) ToNextStage(IVI_UPDATE_END); 
                    break;
                case IVI_UPDATE_END:  //Trace("IVI_UPDATE_END");
                    IvIndexUpdate(OFF);
                    ToNextStage(IVI_DETECT);
                    break;

                default: TraceErr1("NodeIviUpdateProc",ActiveStage()); break;
            };
}


#define REMAIN_SEQ_NUM_MIN      (0x500) //for final SEQ to release


//
// FALSE ==> IVI Update enable
// TRUE ==> IVI Update disable
bool MeshCheckSeqNum()
{
    bool ret_code = TRUE;

#if IVI_UPDATE_ON

    msg_mn_get_seq_remaining_rsp *p_remain_seq_num = Cmd_mn_get_seq_remaining(PRIMARY_ELEM);
    if(p_remain_seq_num->result)
        TraceErr1("Cmd_mn_get_seq_remaining", p_remain_seq_num->result);
    else
    {
        // Trace1("Remain Seq Nnm",p_remain_seq_num->count);
        if(p_remain_seq_num->count < REMAIN_SEQ_NUM_MIN)
            { ret_code = TRUE;}   //modify
    }
    
    
#endif


    return ret_code;
}





uint16 IviUpdateCount;  // for debug


//
//
//
Bool IvIndexUpdate(uchar status)
{
   
    Bool ret_code = FALSE;
    if(status == ON && GetMeshNodeStatus(STATUS_IVI_UPDATE) != ON)
    {
        UsartIrq(USART_ID_RX,OFF);
        // By default, IV index update is limited in how often the update procedure can be performed.
        //  This test command can be called to set IVupdate test mode where any time limits are ignored.
        result = Cmd_mt_set_ivupdate_test_mode(ON)->result;
        result = Cmd_mt_set_iv_index(pMeshNodeData->IvIndex + IVI_INC_MIN)->result;
        pMeshNodeData->IvIndex = pMeshNodeData->IvIndex + IVI_INC_MIN;


        result = Cmd_mn_req_ivupdate()->result;
        IviUpdateStatus(ON);
        result = Cmd_mt_send_beacons()->result; //richard: to send secure network beacons
        if(result) { return ret_code;}else TraceOk(" Ok");
        ret_code = TRUE;
    }

    if(status == OFF && GetMeshNodeStatus(STATUS_IVI_UPDATE) == ON)
    {
        result = Cmd_mt_set_ivupdate_state(OFF)->result;
        if(result == bg_err_success)
        {
            IviUpdateStatus(OFF);
        result = Cmd_mt_send_beacons()->result; //richard: to send secure network beacons
         if(result){TraceErr1("xxxxx  Error", result);
         }else{ret_code = TRUE;TraceOk("----- Ok ----");}
        }
        else
        { TraceErr1("Force Ending IV Update FAILED", result); }
        
      UsartIrq(USART_ID_RX,ON);
    }
    return ret_code;
}


//
//
//
void IviUpdateStatus(uchar status)
{
    if(status == ON)
    {//ON
        SetMeshNodeStatus(STATUS_IVI_UPDATE, ON);
        SetNodeStatus(NS_IVI_UPDATE, ON);
        
    }
    else
    {//OFF
        SetMeshNodeStatus(STATUS_IVI_UPDATE, OFF);
        SetNodeStatus(NS_IVI_UPDATE, OFF);
    }
    ResetEventCounter(TD_NO_EVENT);
}


//
//  for Client & Server IV Index Update
//
uint32 EvtMeshIviProc(PCmdPacket pEvent)
{
   
    uint32 ret_code = TRUE;

    if(NodeRole == NR_CLIENT)
        EvtMeshIviClientProc(pEvent);
    else
        EvtMeshIviServerProc(pEvent);

    return ret_code;
}


uint32 EvtMeshIviClientProc(PCmdPacket pEvent)
{
   
    uint32 ret_code = TRUE;
    uint32    event_id;
    event_id = BGLIB_MSG_ID(pEvent->header);
    msg_mn_ivrecovery_needed_evt *p_ivrecovery;
    msg_mn_changed_ivupdate_state_evt *p_iv_update;
    p_ivrecovery = &(pEvent->data.evt_mesh_node_ivrecovery_needed);
    p_iv_update = &(pEvent->data.evt_mesh_node_changed_ivupdate_state);
    switch(event_id)
    {
        case Evt_mn_ivrecovery_needed:

            if(p_ivrecovery->network_ivindex != p_ivrecovery->node_ivindex)
            {
                Cmd_mt_set_iv_index(p_ivrecovery->network_ivindex);
            }
            result = Cmd_mn_set_ivrecovery_mode(ON)->result;

            break;

        case Evt_mn_changed_ivupdate_state: 
        
            if(p_iv_update->state == ON)
                {
                  result = Cmd_mt_send_beacons()->result; //richard: to send secure network beacons
                }
            else{Trace("IVI Update Ending");
                
                result = Cmd_mt_send_beacons()->result; //richard: to send secure network beacons
                }
                
            pMeshNodeData->IvIndex = p_iv_update->ivindex;
            break;
        default: TraceErr("EvtMeshIviProc");
            break;

    };
    return ret_code;

}


uint32 EvtMeshIviServerProc(PCmdPacket pEvent)
{
   
    uint32 ret_code = TRUE;
    uint32    event_id;
    msg_mn_ivrecovery_needed_evt *p_ivrecovery;
    msg_mn_changed_ivupdate_state_evt *p_iv_update;
    event_id = BGLIB_MSG_ID(pEvent->header);

    p_ivrecovery = &(pEvent->data.evt_mesh_node_ivrecovery_needed);
    p_iv_update = &(pEvent->data.evt_mesh_node_changed_ivupdate_state);
    switch(event_id)
    {
        case Evt_mn_changed_ivupdate_state: 
            if(p_iv_update->state == YES)
            {
                IviUpdateStatus(ON);
                SetLedStatus(LED_STATUS_OFF);
            }
            else
            {
                IviUpdateStatus(OFF);
                SetLedStatus(LED_STATUS_IVI_UPDATE_OFF);
                
            }

            break;

        case Evt_mn_ivrecovery_needed: 
            if(GetMeshNodeStatus(STATUS_PROXY_CONNECT) == TRUE)  break;

            if(p_ivrecovery->network_ivindex != p_ivrecovery->node_ivindex)
            {
                Cmd_mt_set_iv_index(p_ivrecovery->network_ivindex);
                IviUpdateStatus(ON);
            }
            pMeshNodeData->IvIndex = p_ivrecovery->node_ivindex;
            result = Cmd_mn_set_ivrecovery_mode(ON)->result;
            ShowCurrRemSeq();
            Cmd_mt_set_element_seqnum(PRIMARY_ELEM, 0); //set sequence num = 0
            ShowCurrRemSeq();
            SetMeshNodeStatus(STATUS_IVI_UPDATE, ON);
            break;
        default: TraceErr("EvtMeshIviProc");

    };
    return ret_code;

}

uint32 seqnum;

void ShowCurrRemSeq(void)
{
    msg_mt_get_element_seqnum_rsp *trsp = Cmd_mt_get_element_seqnum(PRIMARY_ELEM);
    msg_mn_get_seq_remaining_rsp *rsp = Cmd_mn_get_seq_remaining(PRIMARY_ELEM);

    seqnum = trsp->seqnum;

}




