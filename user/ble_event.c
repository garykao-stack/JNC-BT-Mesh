#include "global.h"
#include "leds.h"

//richard Add
/* BG stack headers */
#include "Mesh_Node.h"
#include "MeshFeatures.h"
#include "sensor_client.h"
#include "ble_event.h"

uint8 BleNodeNum = 0;              // number of active Mesh Node connections
uint8 ConnectHandle = 0xFF;         // handle of the last opened LE connection
// richard
uint8 AlterLevel;
uint8 RepeatCounter;

// Event ==> Evt, Message ==> Msg, Response ==> Resp, Attribute ==> Attri
// Characteristic ==> Charact,
EventFun BleEventFun[] =
{
    {Evt_sys_boot,                    EvtSystemBootProc},             //EVT_SYSTEM_BOOT    
    {Evt_sys_external_signal,         EvtSysExternalSignalProc},      //
    {Evt_sys_awake,                   EvtSysAwakeProc},
    {Evt_connect_opened,              EvtBleConnectionProc},          //
    {Evt_connect_parameters,          EvtBleConnectionProc},          //
    {Evt_connect_closed,              EvtBleConnectionProc},          //
    {Evt_connect_phy_status,          EvtBleConnectionProc},
    {Evt_hardware_soft_timer,         EvtSoftTimerProc},              //

    //********************** for GAP *************************************
    {Evt_gap_scan_resp,               EvtGapScanResponseProc},        //EVT_GAP_SCAN_RSP
    {Evt_gap_adv_timeout,             EvtGapAdvTimeOutProc},

    //********************** for GATT ************************************
    {Evt_gatt_server_attri_value,     EvtGattServerAttributeValueProc},
    {Evt_gatt_service,                EvtGattServiceProc},
    {Evt_gatt_charact,                EvtGattCharaProc},
    {Evt_gatt_charact_value,          EvtGattCharaValueProc},
    {Evt_gatt_mtu_exchanged,          EvtGattMtuExchangedProc},
    {Evt_gatt_procedure_completed,    EvtGattCompletedProc},
    {Evt_gatt_server_charact_status,  EvtGattServerCharactStatusProc},
    {Evt_gatt_server_user_read_req,   EvtGattServerUserReadRequest},
    {Evt_gatt_server_user_write_req,  EvtGattServerUserWriteRequestProc},
    {0, NULL},  // End
};

extern Result result;

void BleEventInit()
{
    BleNodeNum = 0;
    RepeatCounter = 0;
    ConnectHandle = 0xFF;
    //Cmd_coex_set_options(GECKO_COEX_OPTION_ENABLE, 1);    //for WiFi Coexistence Enable
}


NodeMsg BleStatus;

//**********************************************************************************************
// For Normal BLE Event
//
//**********************************************************************************************
bool BleEventProc(PCmdPacket pEvent)
{
    uint32 event_id;
    PEventFun pEventFun;
    bool ret_code = FALSE;
    if(NULL == pEvent)  return ret_code;
    
    event_id = BGLIB_MSG_ID(pEvent->header); //get event ID
    pEventFun = BleEventFun;    //initial event table

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

enum {ADVERTISER0 = 0,ADVERTISER1 = 1};

//
//
//
void SetAdvertise(uchar status)
{
    if(status == ON)
      {//ON
        Trace("SetAdvertise ON");
         gecko_cmd_le_gap_set_advertise_report_scan_request(ADVERTISER0,false);
    	 gecko_cmd_le_gap_set_advertise_phy(ADVERTISER0, 1, 1);
    	 gecko_cmd_le_gap_set_advertise_configuration(ADVERTISER0,1); /* use legacy PDUs, non-anonymous advertising, resolvable address*/
    	 gecko_cmd_le_gap_set_advertise_timing(ADVERTISER0,160,160,0,0); /* 100 ms interval, continue until stopped*/
    	 gecko_cmd_le_gap_set_advertise_channel_map(ADVERTISER0,7); /*all primary advertising channels*/
    	 gecko_cmd_le_gap_start_advertising(ADVERTISER0, le_gap_general_discoverable, le_gap_connectable_scannable);

      }
    else
      {//OFF
         Trace("SetAdvertise OFF");
         //gecko_cmd_le_gap_set_advertise_timing(ADVERTISER0,5000,5000,1000,1000); /* 100 ms interval, continue until stopped*/
         gecko_cmd_le_gap_stop_advertising(ADVERTISER0);
      }
}

//**********************************************************************************************
//Event: gecko_evt_system_boot_id
//
//
//**********************************************************************************************
uint32 EvtSystemBootProc(PCmdPacket pEvent)
{
    uint32 ret_code = TRUE;
    char buf[30];
    msg_sys_boot_evt* p_sys_boot = &(pEvent->data.evt_system_boot);
    Trace16_4(p_sys_boot->major, p_sys_boot->minor, p_sys_boot->patch, p_sys_boot->build);
    Trace1("BootLoader",p_sys_boot->bootloader);
    Trace16_2(p_sys_boot->hw,p_sys_boot->hash);
    
    msg_sys_get_bt_addr_rsp *pAddr =  Cmd_sys_get_bt_addr();
    set_device_name(&pAddr->address);
    
    SetTxPower(TX_POWER_LO);
    //gecko_cmd_le_gap_set_adv_timeout(1);
    //gecko_cmd_le_gap_set_mode(le_gap_general_discoverable,le_gap_non_connectable);
   // return ret_code;
   //SetAdvertise(ON);
    
    // Initialize Mesh stack in Node operation mode, wait for initialized event
    result = Cmd_mn_init()->result;
    if(result)
    {
        snprintf(buf, 30, "init failed (0x%x)", result);
        
        //DI_Print(buf, DI_ROW_STATUS);
    }
    return ret_code;
}



//**********************************************************************************************
// Event: gecko_evt_system_awake_id
//
//**********************************************************************************************
uint32 EvtSysAwakeProc(PCmdPacket pEvent)
{
    uint32 ret_code = FAIL;

    return ret_code;
}

/// Flag for indicating DFU Reset must be performed
uint8_t boot_to_dfu = 0;

//**********************************************************************************************
// Event: gecko_evt_le_connection_opened_id, gecko_evt_le_connection_closed_id,
//        gecko_evt_le_connection_parameters_id, gecko_evt_le_connection_phy_status_id
//  Handling of mesh node provisioning events.
//  It handles:
//   - mesh_node_provisioning_started
//   - mesh_node_provisioned
//   - mesh_node_provisioning_failed
//   - PHY status
//**********************************************************************************************
uint32 EvtBleConnectionProc(PCmdPacket pEvent)
{
    uint32 ret_code = TRUE;
    msg_connect_opened_evt      *pEvt_open = &pEvent->data.evt_le_connection_opened;
    msg_connect_parameters_evt  *pEvt_params = &pEvent->data.evt_le_connection_parameters;
    msg_connect_phy_status_evt  *pEvt_phy_status = &pEvent->data.evt_le_connection_phy_status;
    msg_connect_closed_evt      *pEvt_close = &pEvent->data.evt_le_connection_closed;

    switch(BGLIB_MSG_ID(pEvent->header))
    {
        case Evt_connect_opened:   Trace("Evt_connect_opened");
            BleNodeNum++;
            ConnectHandle = pEvt_open->connection;
            SetMeshNodeStatus(STATUS_BLE_CONNECT,ON);
            SetNodeStatus(NS_LINKING,ON);
            NodeWakeUp();
           // DI_Print("connected", DI_ROW_CONNECTION);
            break;

        case Evt_connect_parameters: Trace("Evt_connect_parameters");
            Printf("Evt_connect_parameters: interval %u, latency %u, timeout %u\r\n",
                   pEvt_params->interval, pEvt_params->latency, pEvt_params->timeout);
            break;

        case Evt_connect_phy_status:  Trace1("Evt_connect_phy_status", pEvt_phy_status->phy);

            break;
        case Evt_connect_closed:  Trace("Evt_connect_closed");
            // Check if need to boot to dfu mode
            SetMeshNodeStatus(STATUS_BLE_CONNECT,OFF);
            SetNodeStatus(NS_LINKING,OFF);
            if(boot_to_dfu) { Cmd_sys_reset(2); } // Enter to DFU OTA mode
            
            Printf("evt:conn closed, reason 0x%x\r\n",  pEvt_close->reason);
            ConnectHandle = 0xFF;
            if(BleNodeNum > 0)
            {
                if(--BleNodeNum == 0)
                {
                    DI_Print("", DI_ROW_CONNECTION);
                }
            }
            break;

        default:
            break;
    }

    return ret_code;
}





//**********************************************************************************************
// Event: gecko_evt_le_connection_phy_status_id
//
//**********************************************************************************************
uint32 EvtConnectionPhyStatusProc(PCmdPacket pEvent)
{
    uint32 ret_code = FAIL;

    return ret_code;
}


int16 BLE_RSSI[5];

static void print_scan_resp(struct gecko_msg_le_gap_scan_response_evt_t *pResp)
{
	// decoding advertising packets is done here. The list of AD types can be found
	// at: https://www.bluetooth.com/specifications/assigned-numbers/Generic-Access-Profile

	// example of adv data including proxy service data:020106030328180c16281800f03f4f79774c65a2
	// (UUID 0x1828)

	// 020106-03032818-0c16281800f03f4f79774c65a2
	const uint8 proxy_UUID[2] = {0x28, 0x18};

	int i = 0;
	int ad_match_found = 0;
	int ad_len;
	int ad_type;

	while (i < (pResp->data.len - 1))
	{

		ad_len  = pResp->data.data[i];
		ad_type = pResp->data.data[i+1];

		if (ad_type == 0x03)
		{
			// type 0x03= Complete List of 16-bit Service Class UUIDs

			if(memcmp(proxy_UUID, &(pResp->data.data[i+2]),2) == 0)
			{
				ad_match_found = 1;
			}
		}

		//jump to next AD record
		i = i + ad_len + 1;
	}

	if(ad_match_found)
	{
		for(i=5;i>=0;i--)
		{
			Printf("%2.2x", pResp->address.addr[i]);    //MAC Address
		}

		Printf(", RSSI: %d\r\n", pResp->rssi);
        for( i = 0; i<pResp->data.len;i++){ printf("%02X",pResp->data.data[i]); }
         Printf("\r\n\r\n");
        if(pResp->address.addr[0] == 0x91) BLE_RSSI[0] =  pResp->rssi;
        if(pResp->address.addr[0] == 0xC5) BLE_RSSI[1] =  pResp->rssi;
        if(pResp->address.addr[0] == 0xF9) BLE_RSSI[2] =  pResp->rssi;

        //BLE_RSSI[0] =  -10;BLE_RSSI[1] =  -20;BLE_RSSI[2] =  -30;
	}
}




//**********************************************************************************************
// Event: gecko_evt_le_gap_scan_response_id
//
//**********************************************************************************************
uint32  EvtGapScanResponseProc(PCmdPacket pEvent)
{
    uint32 ret_code = TRUE;
    msg_gap_scan_resp_evt  *scan_resp;
   	scan_resp = (msg_gap_scan_resp_evt *)&(pEvent->data);
    print_scan_resp(scan_resp);

        /*
    	Printf("evt:gecko_evt_le_gap_scan_response_id\r\n");
	    Printf("RSSI %d, Type %d, Addr 0x%02X%02X%02X%02X%02X%02X, Addr Type %X, Bond %d, msg len: %x, msg: 0x", 
                scan_resp->rssi, scan_resp->packet_type, scan_resp->address.addr[0], 
                scan_resp->address.addr[1], scan_resp->address.addr[2], scan_resp->address.addr[3], 
                scan_resp->address.addr[4], scan_resp->address.addr[5], scan_resp->address_type,  
                scan_resp->bonding, scan_resp->data.len);
		for(uint8_t i = 0; i<scan_resp->data.len;i++){
			Printf("%02X",scan_resp->data.data[i]);
		}
		Printf("\r\n");    
        */
    return ret_code;
}


//**********************************************************************************************
// Event: gecko_evt_le_gap_adv_timeout_id
//
//**********************************************************************************************
uint32  EvtGapAdvTimeOutProc(PCmdPacket pEvent)
{
    uint32 ret_code = TRUE;
    msg_gap_adv_timeout_evt *p_adv_timeout;
    p_adv_timeout = (msg_gap_adv_timeout_evt *)&(pEvent->data);
    //result = gecko_cmd_le_gap_stop_advertising(p_adv_timeout->handle)->result;
    //Trace1("result", result);

    return ret_code;
}

//****************** for GATT *************************************

//*****************************************************************
// Event: gecko_evt_gatt_server_attribute_value_id
//
//*****************************************************************
uint32  EvtGattServerAttributeValueProc(PCmdPacket pEvent)
{
    uint32 ret_code = FAIL;

    return ret_code;
}


//**********************************************************************************************
// Event: gecko_evt_gatt_service_id
//
//**********************************************************************************************
uint32  EvtGattServiceProc(PCmdPacket pEvent)
{
    uint32 ret_code = FAIL;

    return ret_code;
}


//**********************************************************************************************
// Event: gecko_evt_gatt_characteristic_id
//
//**********************************************************************************************
uint32  EvtGattCharaProc(PCmdPacket pEvent)
{
    uint32 ret_code = FAIL;

    return ret_code;
}


//**********************************************************************************************
// Event: gecko_evt_gatt_characteristic_value_id
//
//**********************************************************************************************
uint32  EvtGattCharaValueProc(PCmdPacket pEvent)
{
    uint32 ret_code = FAIL;

    return ret_code;
}


//**********************************************************************************************
// Event: gecko_evt_gatt_mtu_exchanged_id
//
//**********************************************************************************************
uint32  EvtGattMtuExchangedProc(PCmdPacket pEvent)
{
    uint32 ret_code = TRUE;
    msg_gatt_mtu_exchanged_evt* pMsg = &(pEvent->data.evt_gatt_mtu_exchanged);
    Trace16Ptr_2(pMsg,connection, mtu);
    return ret_code;
}


//**********************************************************************************************
// Event: gecko_evt_system_external_signal_id
//
//**********************************************************************************************
uint32  EvtGattCompletedProc(PCmdPacket pEvent)
{
    uint32 ret_code = FAIL;

    return ret_code;
}


//**********************************************************************************************
// Event: gecko_evt_gatt_server_characteristic_status_id
//
//**********************************************************************************************
uint32  EvtGattServerCharactStatusProc(PCmdPacket pEvent)
{
    uint32 ret_code = FAIL;
    msg_gatt_server_character_status_evt* pMsg = &(pEvent->data.evt_gatt_server_characteristic_status);
    Trace16Ptr_4(pMsg,connection , characteristic, status_flags, client_config_flags);

    return ret_code;
}


//**********************************************************************************************
// Event: gecko_evt_gatt_server_user_read_request_id
//
//**********************************************************************************************
uint32  EvtGattServerUserReadRequest(PCmdPacket pEvent)
{
    uint32 ret_code = FAIL;

    return ret_code;
}


//**********************************************************************************************
// Event: gecko_evt_gatt_server_user_write_request_id
//
//**********************************************************************************************
uint32  EvtGattServerUserWriteRequestProc(PCmdPacket pEvent)
{
    uint32 ret_code = TRUE;
    msg_gatt_server_user_write_request_evt *pEvt_gatt_server = &pEvent->data.evt_gatt_server_user_write_request;
    if(gattdb_ota_control == pEvt_gatt_server->characteristic)
    {
        enter_to_dfu_ota(pEvt_gatt_server->connection);
    }
    return ret_code;
}


/***************************************************************************//**
 *  Entering to OTA DFU.
 *
 *  @param[in] connection  Connection that handle OTA DFU.
 ******************************************************************************/
void enter_to_dfu_ota(uint8_t connection)
{
  // Set flag to enter to OTA mode
  boot_to_dfu = 1;
  // Send response to Write Request
  Cmd_gatt_server_send_user_write_resp(connection,gattdb_ota_control, bg_err_success);
  // Close connection to enter to DFU OTA mode
  Cmd_connect_close(connection);
}




