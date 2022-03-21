/***************************************************************************//**
 * @file  sensor.c
 * @brief Sensor module
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
#include "MeshFeatures.h"

#include "leds.h"
#include "mesh_sensor.h"
#include "sensor_server.h"
/* Sensor headers */
#include "people_count_sensor.h"
#include "temperature_sensor.h"
#include "bus_usart.h"
#include "modbus_to_mesh.h"
#include "cmd_to_bt_mesh.h"
#include "Mesh_node.h"

/***************************************************************************//**
 * @addtogroup Sensor
 * @{
 ******************************************************************************/

#define SENSOR_ELEMENT     0 ///< Sensor model located in primary element
#define NUMBER_OF_SENSORS  5 ///< Number of supported Property IDs
#define PUBLISH_ADDRESS    0 ///< The unused 0 address is used for publishing
#define IGNORED            0 ///< Parameter ignored for publishing
#define NO_FLAGS           0 ///< No flags used for message
/// Descriptors of supported sensors
/* The following properties are defined
 * 1. People count property(property ID: 0x004C)
 * NOTE: the properties must be ordered in ascending order by property ID
 */
static const sensor_descriptor_t descriptors[NUMBER_OF_SENSORS] = // define property of server node
{
    {
        .property_id = NODE_GET_TEMP,
        .positive_tolerance = TOLERANCE_UNSPECIFIED,
        .negative_tolerance = TOLERANCE_UNSPECIFIED,
        .sampling_function = SAMPLING_UNSPECIFIED,
        .measurement_period = MEASUREMENT_PERIOD_UNDEFINED,
        .update_interval = UPDATE_INTERVAL_UNDEFINED
    },
    {
        .property_id = NODE_GET_RH,
        .positive_tolerance = TOLERANCE_UNSPECIFIED,
        .negative_tolerance = TOLERANCE_UNSPECIFIED,
        .sampling_function = SAMPLING_UNSPECIFIED,
        .measurement_period = MEASUREMENT_PERIOD_UNDEFINED,
        .update_interval = UPDATE_INTERVAL_UNDEFINED
    },
    {
        .property_id = NODE_GET_BATTERY_POWER,
        .positive_tolerance = TOLERANCE_UNSPECIFIED,
        .negative_tolerance = TOLERANCE_UNSPECIFIED,
        .sampling_function = SAMPLING_UNSPECIFIED,
        .measurement_period = MEASUREMENT_PERIOD_UNDEFINED,
        .update_interval = UPDATE_INTERVAL_UNDEFINED
    },
    
    {
        .property_id = MODBUS_GET_REGS_VALUE,
        .positive_tolerance = TOLERANCE_UNSPECIFIED,
        .negative_tolerance = TOLERANCE_UNSPECIFIED,
        .sampling_function = SAMPLING_UNSPECIFIED,
        .measurement_period = MEASUREMENT_PERIOD_UNDEFINED,
        .update_interval = UPDATE_INTERVAL_UNDEFINED
    },
    
    {
        .property_id = PEOPLE_COUNT,    // for simulation
        .positive_tolerance = TOLERANCE_UNSPECIFIED,
        .negative_tolerance = TOLERANCE_UNSPECIFIED,
        .sampling_function = SAMPLING_UNSPECIFIED,
        .measurement_period = MEASUREMENT_PERIOD_UNDEFINED,
        .update_interval = UPDATE_INTERVAL_UNDEFINED
    }

};



void handle_server_get_series(PCmdPacket pCmdEvent)
{//TraceProc();
    uchar test[5]={0x12,0x39,0x24,0x19,0x09};
    msg_ms_server_get_series_request_evt *p_data; 
    p_data = (msg_ms_server_get_series_request_evt * )&(pCmdEvent->data.evt_mesh_sensor_server_get_series_request);
    
   // Trace16Ptr_3(p_data, client_address, appkey_index, property_id);
   // PrintDataByte("handle_server_get_series", p_data->column_ids.data, p_data->column_ids.len);
   PrintDataByte("handle_server_get_series", test, 3);
    
}


void handle_server_get_column(PCmdPacket pCmdEvent)
{//TraceProc();
    uchar test[5]={0x23,0x19,0x23,0x01,0x29};
    msg_ms_server_get_column_request_evt *p_data;
    p_data = (msg_ms_server_get_column_request_evt * )&(pCmdEvent->data.evt_mesh_sensor_server_get_column_request);
    
    
   // Trace16Ptr_3(p_data, client_address, appkey_index, property_id);
    PrintDataByte("handle_server_get_column", test, 5);
}


/*******************************************************************************
 *  Handling of mesh sensor server events.
 *  It handles:
 *   - sensor_server_get_request
 *   - sensor_server_get_column_request
 *   - sensor_server_get_series_request
 *   - sensor_setup_server_get_cadence_request
 *   - sensor_setup_server_set_cadence_request
 *   - sensor_setup_server_get_settings_request
 *   - sensor_setup_server_get_setting_request
 *   - sensor_setup_server_set_setting_request
 *
 *  @param[in] pEvent  Pointer to incoming sensor server event.
 ******************************************************************************/
 
uint32 EvtSensorServerEventsProc(PCmdPacket pCmdEvent)
{
    uint32 ret_code = TRUE;
    uint32 event_id;
    event_id = BGLIB_MSG_ID(pCmdEvent->header);
    switch(event_id)
    {
        case Evt_ms_server_get_req: 
            EvtGetRequestProc(pCmdEvent);
            break;
        case Evt_ms_setup_server_get_setting_req: Trace("Evt_ms_setup_server_get_setting_req");
            EvtSetGettingRequestProc(pCmdEvent);
            break;
        case Evt_ms_setup_server_set_setting_req: Trace("Evt_ms_setup_server_set_setting_req");
            EvtSetSettingRequestProc(pCmdEvent);
            break;
/*        
        case Evt_ms_server_get_series_req: //TraceErr("Evt_ms_server_get_series_req");
            handle_server_get_series(pCmdEvent);
            break;
 
        case Evt_ms_server_get_column_req: //TraceErr("Evt_ms_server_get_column_req");
            handle_server_get_column(pCmdEvent);
            break;
*/                
        default:  //TraceErr1("EvtSensorServerEventsProc",event_id);
        break;
    }
    
    return ret_code;
}



/*******************************************************************************
 * Sensor initialization.
 * This is called at each boot if provisioning is already done.
 * Otherwise this function is called after provisioning is completed.
 ******************************************************************************/
void SensorServerNodeInit(void)
{
    NodeWakeUp();
    uint16_t result = mesh_lib_sensor_server_init(SENSOR_ELEMENT, NUMBER_OF_SENSORS, descriptors);
    Printf("sensor init result %02x\r\n", result);
    // Initialize the People Count Sensor
    set_people_count(0);
    // Initialize the Temperature Sensor
    init_temperature_sensor();
   // NodeSleeping(ON); NodeProxy(OFF);// while(1);
}

uint16 ModbusReg0=0x1001,ModbusReg1=0x2002,ModbusReg2=0x3003,ModbusReg3=0x4004;

//
// Get Modbus information
//
uint16 GetDevModbusInfo(uint16 reg)
{//TraceProc() ;
    uint16 ret_code=0;
   if(reg == 0) ret_code = ModbusReg0;
   else if(reg == 1) ret_code = ModbusReg1;
   else if(reg == 2) ret_code = ModbusReg2;
   else if(reg == 3) ret_code = ModbusReg3;
   else TraceErr("Modbus Reg");
}



/***************************************************************************//**
 * Handling of sensor server get request event.
 * It sending sensor status message with data for all of supported Properties ID,
 * if there is no Property ID field in request. If request contains Property ID
 * that is supported, functions reply with the sensor status message with data
 * for this Property ID, in other case the message contains no data.
 *
 * @param[in] pEvent  Pointer to sensor server get request event.
 * gecko_evt_mesh_sensor_server_get_request_id
 ******************************************************************************/
void EvtServerGetRequestProc(   PCmdPacket pCmdEvent)
{//TraceProc() ;
    //Trace("gecko_evt_mesh_sensor_server_get_request_id");
    msg_ms_server_get_request_evt *pEvent = &(pCmdEvent->data.evt_mesh_sensor_server_get_request);
    uint8_t sensor_data[10];
    //uint8_t sensor_data[30];
    uint8_t len = 0;
    Trace16_1(pEvent->property_id);
    if((pEvent->property_id == PEOPLE_COUNT) || (pEvent->property_id == 0))
    {
        count16_t people_count = get_people_count();
        Printf("people_count: %u\r\n", people_count);
        len += mesh_sensor_data_to_buf(PEOPLE_COUNT, &sensor_data[len], (uint8_t *)&people_count);
    }
    if((pEvent->property_id == PRESENT_AMBIENT_TEMPERATURE) || (pEvent->property_id == 0))
    {
        temperature_8_t temperature = get_temperature();
        Printf("temperature 3: %d\r\n", temperature);
        len += mesh_sensor_data_to_buf(PRESENT_AMBIENT_TEMPERATURE, &sensor_data[len], (uint8_t *)&temperature);
    }
   if((pEvent->property_id == JNC_TEMP_RH) || (pEvent->property_id == 0))
    {
        uint32 temp_rh = GetTempAndRh();
        Trace1("Temp and Humidity", temp_rh);
        len += mesh_sensor_data_to_buf(JNC_TEMP_RH, &sensor_data[len], (uint8_t *)&temp_rh);
    }
/*   
    if((pEvent->property_id == MODBUS_FC4_REG0) || (pEvent->property_id == 0))
     {
         uint16 modbus_reg = GetDevModbusInfo(0);
         Trace1("modbus_reg 0", modbus_reg);
         len += mesh_sensor_data_to_buf(MODBUS_FC4_REG0, &sensor_data[len], (uint8_t *)&modbus_reg);
     }
*/    
    if((pEvent->property_id == MODBUS_GET_REGS_VALUE) || (pEvent->property_id == 0))
     {
        ServerGetReguset(pCmdEvent);
       // ServerSendDataToClient(pCmdEvent,1); 
        return;
     }

    if(len > 0)
    {
        Cmd_ms_server_send_status(SENSOR_ELEMENT, pEvent->client_address, pEvent->appkey_index, NO_FLAGS, len, sensor_data);
    }
    else
    {//richard: unsupported sensor 
        sensor_data[0] = pEvent->property_id & 0xFF;
        sensor_data[1] = ((pEvent->property_id) >> 8) & 0xFF;
        sensor_data[3] = 0; // Length is 0 for unsupported property_id
        Cmd_ms_server_send_status(SENSOR_ELEMENT, pEvent->client_address, pEvent->appkey_index, NO_FLAGS, 3, sensor_data);
    }
}



#if MESH_COLUME_ENABLE


/***************************************************************************
 * Handling of sensor server get column request event.
 * Used Property IDs does not have sensor series column state,
 * so reply has the same data as request according to specification.
 *
 * @param[in] pEvent  Pointer to sensor server get column request event.
 * gecko_evt_mesh_sensor_server_get_column_request_id
****************************************************************************/
void ModBusCmdToDevice(void);


uchar DeviceModBusID=1;//1;//2; //1;

void EvtServerGetColumeRequest(PCmdPacket pCmdEvent)
{// TraceProc() ;
    msg_ms_server_get_column_request_evt *pEvent = &(pCmdEvent->data.evt_mesh_sensor_server_get_column_request);
    //TraceDec1("DeviceModBusID",DeviceModBusID);    
    Trace16_1(pEvent->elem_index);
    if(pEvent->property_id != JNC_MODBUS_CMD) return;
    Trace1("Column property ID", pEvent->property_id);
    PrintDataByte("ColumeRequest",  pEvent->column_ids.data, pEvent->column_ids.len);

    if(pEvent->column_ids.data[0] == DeviceModBusID){ TraceDec1("Modbud ID Ok", DeviceModBusID);
        //Cmd_ms_server_send_column_status(SENSOR_ELEMENT, pEvent->client_address, pEvent->appkey_index,
          //                              NO_FLAGS, pEvent->property_id, pEvent->column_ids.len, pEvent->column_ids.data);        
        SaveMeshNodeInfo(SAVE_INFO_GET_COLUME_REGUEST,pEvent);
        ModBusCmdToDevice(); // send modbus cmd to modbus device
        }
    else TraceErr1("Device ID Fail",DeviceModBusID);
}

/***************************************************************************
 * Handling of sensor server get series request event.
 * Used Property IDs does not have sensor series column state,
 * so reply has only Property ID according to specification.
 *
 * @param[in] pEvent  Pointer to sensor server get series request event.
 * gecko_evt_mesh_sensor_server_get_series_request_id
 ******************************************************************************/
 #define    TO_CLIENT_BUFF_SIZE     9
void EvtServerGetSeriesReqest(PCmdPacket pCmdEvent)
{//TraceProc() ;
    msg_ms_server_get_series_request_evt *pEvent = &(pCmdEvent->data.evt_mesh_sensor_server_get_series_request);
    uchar ToClientBuff[TO_CLIENT_BUFF_SIZE]={0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49};

    //PrintDataByte("EvtServerGetSeriesReqest", pEvent->column_ids.data ,pEvent->column_ids.len);

    Cmd_ms_server_send_series_status(SENSOR_ELEMENT, pEvent->client_address, pEvent->appkey_index, NO_FLAGS,
                                    pEvent->property_id, TO_CLIENT_BUFF_SIZE, ToClientBuff);
    
  //Cmd_ms_server_send_series_status(SENSOR_ELEMENT, pEvent->client_address, pEvent->appkey_index, NO_FLAGS,
  //                                pEvent->property_id, 0, NULL);
}

#endif // MESH_COLUME_ENABLE


/***************************************************************************
 * Handling of sensor server publish event.
 * It is used to signal the elapse of the publish period, when the server app
 * shall publish the sensor states
 *
 * gecko_evt_mesh_sensor_server_publish_id
 * @param[in] pEvent  Pointer to sensor server publish request event
 ******************************************************************************/
void EvtServerPubEvent(PCmdPacket pCmdEvent)
{//TraceProc() ;
    msg_ms_server_publish_evt *pEvent = &(pCmdEvent->data.evt_mesh_sensor_server_publish);
  //  uint8_t sensor_data[32];
  //  uint8_t len = 0;
  //  TraceDec2("EvtServerPubEvent",pEvent->elem_index,pEvent->period_ms);
  //  ServerPubModbusRegs();
    return;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////




