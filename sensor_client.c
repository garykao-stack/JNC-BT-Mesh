/***************************************************************************//**
 * @file  sensor_client.c
 * @brief Sensor client module
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
//#include "mesh_device_properties.h"
#include "global.h"
#include "MeshFeatures.h"
#include "mesh_sensor.h"
#include "sensor_client.h"
#include "cmd_to_bt_mesh.h"



/// Counter for devices registered for displaying sensor data
uint8_t registered_devices = 0;
/// Table that saves addresses of displayed sensors
uint16_t address_table[DISPLAYED_SENSORS];
/// Currently displayed property ID
uint16 PropertyIndex = 0;

/// Property IDs supported by application
const mesh_device_properties_t ServerProperties[MAX_PROPERTIES] =
{
    PRESENT_AMBIENT_TEMPERATURE, 
    PEOPLE_COUNT, 
    JNC_MODBUS_CMD,
    JNC_TEMP_RH,    //for Tempature and Relative Hunidity
    MODBUS_FC4_REG0,
    DEVICE_FIRMWARE_REVISION
};


#define PROPERITY_TEMPERATURE       BIT0
#define PROPERITY_PEOPLE            BIT1
#define PROPERITY_RS485_JNC         BIT3


#define SERVER_NODE_PROPERITY   (PROPERITY_PEOPLE|PROPERITY_TEMPATURE|PROPERITY_RS485_JNC)

MeshNode    TotalMeshNode[MAX_MESH_NODE_NUM]={
{.ServerAddr=2,.Properities=SERVER_NODE_PROPERITY},
{.ServerAddr=3,.Properities=SERVER_NODE_PROPERITY},
{.ServerAddr=4,.Properities=SERVER_NODE_PROPERITY},
{.ServerAddr=5,.Properities=SERVER_NODE_PROPERITY},
{.ServerAddr=6,.Properities=SERVER_NODE_PROPERITY},
};
uint8 CurrServerNodeNum = 0;    // number of active Mesh Node connections
uchar SysServerNodeNum;     // define all of bt mesh node nymber
uint8 ResponseNodeNum;
PMeshNode   pActiveNode;


//**********************************************************************************************
// for sensor client model initial
//
//**********************************************************************************************
void SensorClientNodeInit(void)
{TraceProc();
    SysServerNodeNum = 1;
    NodeWakeUp();
    Cmd_ms_client_init();
#ifdef FRIEND_NODE        
    FriendStatus(ON);
#endif 
}


//******************************************************************************************************
// 
//
//******************************************************************************************************
void StartScanServerNode()
{TraceProc();
    if(GetNodeStatus(STATUS_CLIENT) != TRUE) return; // server dot scan
    
    DI_Print("                     ",DI_ROW_SENSOR_DATA+0);
    DI_Print("                     ",DI_ROW_SENSOR_DATA+1);
    DI_Print("                     ",DI_ROW_SENSOR_DATA+2);
    DI_Print("                     ",DI_ROW_SENSOR_DATA+3);    
    DI_Print("                     ",DI_ROW_SENSOR_DATA+4);
    ClientSetBehavior(TIMER_EVENT_SCAN_INIT,NULL);
}



//**********************************************************************************************
// Event: gecko_evt_mesh_sensor_client_descriptor_status_id, gecko_evt_mesh_sensor_client_status_id
//        
//
// Handling of mesh sensor client events.
// It handles:
//  - sensor_client_descriptor_status
//  - sensor_client_status
//
// @param[in] pEvent  Pointer to incoming sensor server event.
//**********************************************************************************************
uint32 EvtMeshSensorClientProc(PCmdPacket pEvent)
{//TraceProc();
    uint32 ret_code = TRUE;
    msg_ms_client_descriptor_status_evt *pDescriptorStatus;
    msg_ms_client_status_evt *pClientStatus;
    pDescriptorStatus = &(pEvent->data.evt_mesh_sensor_client_descriptor_status);
    pClientStatus = &(pEvent->data.evt_mesh_sensor_client_status);
    
    uint32    event_id;
    event_id = BGLIB_MSG_ID(pEvent->header);
    
    switch(event_id)
        {
            case Evt_ms_client_descriptor_status:   Trace("Evt_ms_client_descriptor_status");// to scan all server node 
                ClientDescriptorStatus(pDescriptorStatus);
                break;
            case Evt_ms_client_status:              //Trace("Evt_ms_client_status");
                //HandleServerProperty(pClientStatus);
                
                HandleModbusBtMesh(pClientStatus); // for Modbus debug
                break;
#if MESH_COLUME_ENABLE
            
            case Evt_ms_client_series_status:       Trace("Evt_ms_client_series_status");
                HandleServerSeriesProperty(&(pEvent->data.evt_mesh_sensor_client_series_status));
                break;
            case Evt_ms_client_column_status:       Trace("Evt_ms_client_column_status");
                HandleServerColumnProperty(&(pEvent->data.evt_mesh_sensor_client_column_status));
                break;
#endif // MESH_COLUME_ENABLE
            case Evt_ms_client_setting_status:  Trace("Evt_ms_client_setting_status"); 
                HandleSettingStatus(&(pEvent->data.evt_mesh_sensor_client_setting_status));
                break;


            case Evt_ms_client_publish: // Trace("Evt_ms_client_publish: Bug"); // richard: must debug
                //HandleClientPublish(&(pEvent->data.evt_mesh_sensor_client_publish));
                break;

            
            default: TraceErr("EvtMeshSensorClientProc");
                break;
        };


    return ret_code;
}
uchar SanServerStage=0;
uchar GetPropertyStage=0;

#define TOTAL_MESH_NODE     4

uint32 TCounterScanServer;
uint32 TCounterGetProperty;


void ClientSetBehavior(uint16 event_class, uint16 param)
{
   switch(event_class) 
    {
        case TIMER_EVENT_SCAN_INIT:
            ResetServerNodeStatus();
            //SetEventTaskTimer(TD_TASK_CLIENT_SCAN_SERVER, TIMER_SCAN_SERVER_CYCLE, TIMER_EVENT_REPEAT);
            TCounterScanServer = 0;
            SanServerStage = SCAN_SERVER_START;
            break;    
        case TIMER_EVENT_SCAN_ON:
            TCounterScanServer = 0;
            SetPropertyIndex(SCAN_MESH_NODE_INDEX);      
            break;
        case TIMER_EVENT_SCAN_OFF:
            //SetEventTaskTimer(TD_TASK_CLIENT_SCAN_SERVER, TIMER_ENDING, TIMER_EVENT_ONCE);
            break;            
        case TIMER_EVENT_GET_PROPERTY_ON:
            break;
        case TIMER_EVENT_GET_PROPERTY_OFF:
            break;
            
        default: TraceErr("ClientSetBehavior");
    };
}


//******************************************************************************************************
// Detect server node number
// return current stage
//******************************************************************************************************
void ClientScanServerProc()
{TraceProc();
    TCounterScanServer++;
    switch(SanServerStage)
    {
        
        case SCAN_SERVER_PENDING:   //Trace("SCAN_SERVER_PENDING");
            //SanServerStage = SCAN_SERVER_START;
        
             break;                
        case SCAN_SERVER_START:     // Trace("SCAN_SERVER_START");                    
            ClientSetBehavior(TIMER_EVENT_SCAN_ON,NULL);
            ClientStartScanServer(); // send detect event to server node
            SanServerStage = SCAN_SERVER_WAITING;
            
            break;
        case SCAN_SERVER_WAITING:   //Trace("SCAN_SERVER_WAITING");

            if(CurrServerNodeNum >= SysServerNodeNum) SanServerStage = SCAN_SERVER_COMPLETE;
            else if(TCounterScanServer >= SCAN_SERVER_TIMEOUT_COUNTER)  SanServerStage = SCAN_SERVER_TIMEOUT; // reset and scan again
            else TraceDec1("Waiting Again....",TCounterScanServer);
            
            break;
        case SCAN_SERVER_TIMEOUT:  //Trace("SCAN_SERVER_TIMEOUT");
            SanServerStage = SCAN_SERVER_START;
            
            break;
        case SCAN_SERVER_COMPLETE:  //Trace("SCAN_SERVER_COMPLETE");
            SanServerStage = SCAN_SERVER_ENDING;
            
            break;
        case SCAN_SERVER_ENDING:    Trace("SCAN_SERVER_ENDING");
            ClientSetBehavior(TIMER_EVENT_SCAN_OFF,NULL);        
            StartGetServerProperty();   // to get property debug
            SanServerStage = SCAN_SERVER_PENDING;
            
            break;
        default: TraceDec1("XXX Scan Server Error XXX",SanServerStage);break;
    };
    
    //return SanServerStage;
}


//uint32 GetPropertyTimer;
//******************************************************************************************************
// initial get property stage 
//
//******************************************************************************************************
void StartGetServerProperty()
{
    GetPropertyStage = GET_PROPERTY_INIT;
    TCounterGetProperty = 0;
    SetPropertyIndex(START_GET_NODE_DATA);
    //SetEventTaskTimer(TD_TASK_GET_PROPERTY, 10, TIMER_EVENT_REPEAT); // start to get device data
}
//******************************************************************************************************
// Get server property data
//******************************************************************************************************
void ClientGetServerDataProc()
{//TraceProc();
    TCounterGetProperty++;
    switch(GetPropertyStage)
    {
    case GET_PROPERTY_PENDING:  //Trace("GET_PROPERTY_PENDING");
         if(GetNodeStatus(BLE_LINK_STATUS) != TRUE) GetPropertyStage = GET_PROPERTY_INIT;
        break;
    case GET_PROPERTY_INIT:     Trace("GET_PROPERTY_INIT");
        GetPropertyStage = GET_PROPERTY_START;        
        //SetEventTaskTimer(TD_TASK_GET_PROPERTY, GET_PROERTY_CYCLE, TIMER_EVENT_REPEAT); // start to get device data
        break;
    case GET_PROPERTY_START:    //Trace("GET_PROPERTY_START");
        ResponseNodeNum = 0;
        TCounterGetProperty = 0;
        SetAllNodeStatus(SERVER_STATUS_WAITING);
        GetServerProperty(PUBLISH_ADDRESS,GetCurrProperty());    // get server property, next timer triggl by Evt_ms_client_status
        GetPropertyStage = GET_PROPERTY_WAITING;
        SetEventTaskTimer(TD_TASK_GET_PROPERTY, TIMER_STAGE_GET_PROERTY_CYCLE, TIMER_EVENT_REPEAT); // start to get device data
        break;
    case GET_PROPERTY_WAITING:  //Trace("GET_PROPERTY_WAITING");
        
        if(ResponseNodeNum >= CurrServerNodeNum) GetPropertyStage = GET_PROPERTY_COMPLETE;
        else if(TCounterGetProperty >= GET_PROERTY_TIMEOUT_COUNTER)  GetPropertyStage = GET_PROPERTY_TIMEOUT; // reset and scan again
        else TraceDec1("GET_PROPERTY Waiting Again....",TCounterGetProperty);
        break;
    case GET_PROPERTY_TIMEOUT:  //TraceDec2("GET_PROPERTY_TIMEOUT",ResponseNodeNum,CurrServerNodeNum);
        // check node response number - node
        GetPropertyStage = GET_PROPERTY_INIT; 
        break;
    case GET_PROPERTY_COMPLETE:   //Trace("GET_PROPERTY_COMPLETE");
        // check node response number + node
        GetPropertyStage = GET_PROPERTY_ENDING; 
        break;
        
    case GET_PROPERTY_ENDING:   Trace("GET_PROPERTY_ENDING");
        GetPropertyStage = GET_PROPERTY_PENDING;
        break;
        
    default: TraceErr1("GetPropertyStage",GetPropertyStage); break;
    };

    
    //return GetPropertyStage;
}


uint16 GetCurrProperty()
{
    return ServerProperties[PropertyIndex];
}

bool SetPropertyIndex(uint16 index)
{
    bool ret_code=TRUE;
    PropertyIndex = index;
    return ret_code;
}


//
// to next property index
void ToNextProperty()
{
    PropertyIndex = SCAN_MESH_NODE_INDEX; //debug

   // PropertyIndex++;
   // if(PropertyIndex < MAX_PROPERTIES) PropertyIndex = 0;
}

//******************************************************************************************************
// waiting response of server node
//******************************************************************************************************
uchar ClientWaitingProperty()
{TraceProc();
    uchar ret_code;

    return ret_code;
}




//******************************************************************************************************
// clear all server node status
//******************************************************************************************************
void ResetServerNodeStatus()
{
    CurrServerNodeNum = 0;    
    memset(&TotalMeshNode,0,sizeof(TotalMeshNode)); //clear server node
    SetEventTaskTimer(TD_TASK_CLIENT_SCAN_SERVER, TIMER_ENDING, TIMER_EVENT_REPEAT);
    SetEventTaskTimer(TD_TASK_GET_PROPERTY, TIMER_ENDING, TIMER_EVENT_REPEAT);
    SetPropertyIndex(SCAN_MESH_NODE_INDEX);

}

#define LAST_NODE_ADDR      0
//******************************************************************************************************
// return mesh node pointer
//******************************************************************************************************
PMeshNode GetServerNode(word server_addr)
{
    PMeshNode p_node;
    uchar loop;
    p_node = TotalMeshNode;
    for(loop=0; loop<MAX_MESH_NODE_NUM; loop++)
    {
      if(p_node->ServerAddr == server_addr || p_node->ServerAddr == LAST_NODE_ADDR ) break;
      else p_node++;
    }

    if(loop == MAX_MESH_NODE_NUM ) {Trace("*** No Mesh Node match ****");
        p_node=NULL; } // no device

    return p_node;
}




//******************************************************************************************************
// 0: for all server mesh node
//******************************************************************************************************
void ClientGetNodeDescriptor(uint16 server_addr,uint16 property_id)
{
   Trace2("ClientGetNodeDescriptor", server_addr, property_id);
   result = Cmd_ms_client_get_descriptor(SENSOR_ELEMENT, server_addr, IGNORED, NO_FLAGS, property_id)->result;
   ShowResult("Cmd_ms_client_get_descriptor", result);

}

//******************************************************************************************************
// 0: for all server mesh node
//******************************************************************************************************
void ClientStartScanServer()
{
    //Trace1("ClientScanServerProc",property_id);
    result = Cmd_ms_client_get_descriptor(SENSOR_ELEMENT, ALL_SERVER_NODE_ADDR, IGNORED, NO_FLAGS, 
                                          SCAN_SERVER_PROPERTY)->result;
    ShowResult("ClientScanServerProc", result);
    
}


//******************************************************************************
// Handling of sensor client descriptor status event.
//
// @param[in] pStatus  Pointer to sensor client descriptor status event.
// *****************************************************************************

void ClientDescriptorStatus(msg_ms_client_descriptor_status_evt *pStatus)
{ TraceProc();
    //printf("evt:gecko_evt_mesh_sensor_client_descriptor_status_id\r\n");
    PMeshNode p_node=NULL;
    sensor_descriptor_t descriptor;
    if(pStatus->descriptors.len >= SIZE_OF_DESCRIPTOR)
    {
        mesh_lib_sensor_descriptors_from_buf(&descriptor, pStatus->descriptors.data, SIZE_OF_DESCRIPTOR);
        Trace16_3(descriptor.property_id,pStatus->server_address,pStatus->appkey_index);
        if(descriptor.property_id == GetCurrProperty() && CurrServerNodeNum < MAX_MESH_NODE_NUM)
        {
           p_node = AddServerNode(pStatus->server_address);
           //SetEventSoftTimer(SOFT_TIMER_SCAN_NODE);//, SOFT_TIMER_VALUE_SCAN_NODE); //reset detect timer
           Trace16_3(CurrServerNodeNum,pStatus->server_address,descriptor.property_id);
        }else Trace("handle_sensor_client_descriptor_status Error !!!!");
    }
}


//******************************************************************************
// Get server node data depend on property
//******************************************************************************
void GetServerProperty(uint16 server_addr,mesh_device_properties_t property)
{
     Trace16_2(server_addr,property);
     if(server_addr == PUBLISH_ADDRESS) ResponseNodeNum=0;
     
     result = Cmd_ms_client_get(SENSOR_ELEMENT, server_addr, IGNORED, NO_FLAGS, property)->result;

     //result = Cmd_ms_client_get(SENSOR_ELEMENT, 2, IGNORED, NO_FLAGS, GetCurrProperty())->result;
     
     //result = Cmd_ms_client_get(SENSOR_ELEMENT, server_addr, IGNORED, NO_FLAGS, GetCurrProperty())->result;
     //if(++server_addr > 4) server_addr = 2;
     
    ShowResult("Cmd_ms_client_get",result);

   // GetRs485Property(server_addr,property); //richard: disable
}

#define JNC_485_CMD_SIZE    16
uchar jnc_rs485_cmd[JNC_485_CMD_SIZE]=
{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
 0x09,0x10,0x12,0x13,0x14,0x15,0x16,0x17
 };

//******************************************************************************
// Get server node data depend on property for JNC Cmd test
//******************************************************************************
void GetServerProperty1(uint16 server_addr,mesh_device_properties_t property)
{
    property = JNC_MODBUS_CMD;
     Trace16_2(server_addr,property);
     if(server_addr == PUBLISH_ADDRESS) ResponseNodeNum=0;
     
     result = Cmd_ms_client_get_column(SENSOR_ELEMENT, server_addr, IGNORED, NO_FLAGS, property,JNC_485_CMD_SIZE,jnc_rs485_cmd)->result;
     
    ShowResult("Cmd_ms_client_get",result);

   // GetRs485Property(server_addr,property); //richard: disable
}



#define RS485_BUFF_SIZE_JNC             10
//******************************************************************************
// Get server node data depend on property
//******************************************************************************
void GetRs485Property(uint16 server_addr,mesh_device_properties_t property)
{
    uchar Rs485_buff[RS485_BUFF_SIZE_JNC]={0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09};
     Trace16_2(server_addr,property);
     
     if(server_addr == PUBLISH_ADDRESS) ResponseNodeNum=0;

     property = JNC_MODBUS_CMD;
     
     result = Cmd_ms_client_get_series(SENSOR_ELEMENT, server_addr, IGNORED, NO_FLAGS, property,RS485_BUFF_SIZE_JNC,Rs485_buff)->result;
     
    ShowResult("GetRs485Property",result);
}

void HandleModbusBtMesh(msg_ms_client_status_evt *pEvent)
{//TraceProc();
    uint8_t *p_sensor_data = pEvent->sensor_data.data;
    uint8_t data_len = pEvent->sensor_data.len;
    uint8_t pos = 0;
    mesh_device_properties_t property_id;
    while(pos < data_len)
        {
           if(data_len - pos > PROPERTY_ID_SIZE)
            {
               property_id = (mesh_device_properties_t)(p_sensor_data[pos] + (p_sensor_data[pos + 1] << 8));
               //Trace16_1(property_id);
               uint8_t property_len = p_sensor_data[pos + PROPERTY_ID_SIZE];
               uint8_t *property_data = NULL;
               if(property_len && (data_len - pos > PROPERTY_HEADER_SIZE))
                   {property_data = &p_sensor_data[pos + PROPERTY_HEADER_SIZE];}
               switch(property_id)
                {
                    case MODBUS_GET_REGS_VALUE: 
                        //Trace16_3(property_len, *property_data,property_id);
                        //Trace16_2(property_len, *property_data);
                        ClientUpdateModbusRegs(pEvent->server_address, property_data,property_len);
                        break;
                    default: TraceErr1("HandleModbusBtMesh",property_id);
                };
               pos += PROPERTY_HEADER_SIZE + property_len;
            }
        };
}


void HandleSettingStatus(msg_ms_client_setting_status_evt *pEvent)
{TraceProc();
 PrintDataByte("HandleSettingStatus", (PUCHAR)&pEvent->raw_value.data, (UINT)pEvent->raw_value.len);
}



//******************************************************************************
// Handling of sensor client status event.
//
// @param[in] pEvent  Pointer to sensor client status event.
//******************************************************************************
void HandleServerProperty(msg_ms_client_status_evt *pEvent)
{ TraceProc();
    //printf("evt:gecko_evt_mesh_sensor_client_status_id\r\n");
    uint8_t *p_sensor_data = pEvent->sensor_data.data;
    uint8_t data_len = pEvent->sensor_data.len;
    mesh_device_properties_t property_id;
    uint8_t pos = 0;
    temperature_8_t temperature;
    count16_t people_count;
    PMeshNode p_node;
    uint32 temp_rh;

    HandleModbusBtMesh(pEvent); return; // for Modbus Mesh Debug
    
    if((p_node = CheckServerAddr(pEvent->server_address)) == NULL ) {TraceDec1("Error: server_address",pEvent->server_address);return;}

        SetServerNodeStatus(p_node,SERVER_STATUS_COMPLETE); //update node status
        ResponseNodeNum++;
            
            while(pos < data_len)
            {
                if(data_len - pos > PROPERTY_ID_SIZE)
                {
                    property_id = (mesh_device_properties_t)(p_sensor_data[pos] + (p_sensor_data[pos + 1] << 8));
                    Trace16_1(property_id);
                    uint8_t property_len = p_sensor_data[pos + PROPERTY_ID_SIZE];
                    uint8_t *property_data = NULL;
                    if(property_len && (data_len - pos > PROPERTY_HEADER_SIZE))
                        {property_data = &p_sensor_data[pos + PROPERTY_HEADER_SIZE];}
                    
                        switch(property_id)
                        {
                            case PEOPLE_COUNT:
                                if(property_len == 2)
                                {
                                    mesh_device_property_t property = mesh_sensor_data_from_buf(PEOPLE_COUNT, property_data);
                                    people_count = property.count16;
                                    DisplayProperty(p_node,property_len,property_id,property);
                                }
                                break;

                            case PRESENT_AMBIENT_TEMPERATURE:
                                if(property_len == 1)
                                {
                                    mesh_device_property_t property = mesh_sensor_data_from_buf(PRESENT_AMBIENT_TEMPERATURE, property_data);
                                    temperature = property.temperature_8;
                                    DisplayProperty(p_node,property_len,property_id,property);
                                }
                                break;
                            case JNC_TEMP_RH:
                                if(property_len == 4)
                                {
                                    mesh_device_property_t property = mesh_sensor_data_from_buf(JNC_TEMP_RH, property_data);
                                    temp_rh = property.JncTempRh;
                                    DisplayProperty(p_node,property_len,property_id,property);
                                }
                                break;
                            case MODBUS_GET_REGS_VALUE:
                                ClientUpdateModbusRegs(pEvent->server_address,property_data,property_len);
                                break;

                            default: break;
                        }
                    pos += PROPERTY_HEADER_SIZE + property_len;
                }
                else
                {
                    pos = data_len;
                }
            } 

            return;
}

#if MESH_COLUME_ENABLE

//******************************************************************************
// Handling of sensor client Series status event.
//
// @param[in] pEvent  Pointer to sensor client Series status event.
//******************************************************************************
void HandleServerSeriesProperty(msg_ms_client_series_status_evt *pEvent)
{ TraceProc();
    
    PrintDataByte("HandleServerSeriesProperty", pEvent->sensor_data.data, pEvent->sensor_data.len);
}



//******************************************************************************
// Handling of sensor client Colume status event.
//
// @param[in] pEvent  Pointer to sensor client column status event.
//******************************************************************************
void SendServerNodeData(PUCHAR p_buff, uchar size);

void HandleServerColumnProperty(msg_ms_client_column_status_evt *pEvent)
{ TraceProc();
    //PrintDataByte("HandleServerColumnProperty", pEvent->sensor_data.data, pEvent->sensor_data.len);
    SendServerNodeData(pEvent->sensor_data.data,pEvent->sensor_data.len);
}

#endif // MESH_COLUME_ENABLE

//******************************************************************************
// Indicates that the publishing period timer elapsed and the app should/can 
// publish its state or any request
// @param[in] pEvent 
//******************************************************************************
void HandleClientPublish(msg_ms_client_publish_evt *pEvent)
{ TraceProc();
    TraceDec2("HandleClientPublish do not implement",pEvent->elem_index, pEvent->period_ms);
}





//******************************************************************************
// input: server address 
// output: server node pointer
//
//******************************************************************************
PMeshNode CheckServerAddr(uint16 server_addr)
{TraceProc();
    PMeshNode p_node=NULL;
    uchar loop;
    for(loop=0; loop<CurrServerNodeNum; loop++)
    {
        if(TotalMeshNode[loop].ServerAddr == server_addr)
            {p_node = &TotalMeshNode[loop] ;  break;}
    }
    return p_node;    
}




//******************************************************************************
// input: server address 
// output: server node pointer
//
//******************************************************************************
PMeshNode AddServerNode(word server_addr)
{TraceProc();
    PMeshNode p_node=NULL;

    p_node = GetServerNode(server_addr);
    if(p_node->ServerAddr == NULL)
        {Trace("Add New Mesh Node");
        p_node->Index = CurrServerNodeNum;
        p_node->ServerAddr = server_addr;
        p_node->RetryCount=0;
        p_node->Status = SERVER_STATUS_ACTIVE;
        CurrServerNodeNum++;
        }
    else{Trace("Mesh Node Wake-Up");
         p_node->RetryCount=0;
         p_node->Status = SERVER_STATUS_ACTIVE;   
        }
    TraceDec1("AddServerNode",CurrServerNodeNum);
    return p_node;
}

//******************************************************************************
// input: server address 
// output: server node pointer
//
//******************************************************************************
PMeshNode DeleteServerNode(word server_addr)
{TraceProc();
    PMeshNode p_node;
    uint16  loop;
    p_node = GetServerNode(server_addr);

    if(!p_node)
        {
            if(++(p_node->RetryCount) > 1)
            p_node->Status = SERVER_STATUS_DEAD;
        }

    return p_node;

}




//******************************************************************************
// input: set all server node for new  status 
//
//******************************************************************************
void SetAllNodeStatus(uchar status)
{TraceProc();
    PMeshNode p_node;
    uchar loop;
    p_node = TotalMeshNode;
    for(loop=0; loop<MAX_MESH_NODE_NUM; loop++)
        {
         p_node->Status = status; p_node->ReceiveTimer=0; 
         p_node++;
        }
}



//******************************************************************************
// input: 
// output: new status
//
//******************************************************************************
uchar SetServerNodeStatus(PMeshNode p_node,uchar status)
{TraceProc();

    //if(status == SERVER_STATUS_COMPLETE) p_node->ReceiveTimer = GetSoftTimer(SOFT_TIMER_GET_PROPERTY); // update receive timer
    p_node->Status = status;
    
    
    return p_node->Status;
}

//******************************************************************************
// input: server node pointer
// output: ccurrent status
//
//******************************************************************************
uchar GetServerNodeStatus(PMeshNode p_node)
{TraceProc();
    return p_node->Status;
}


//******************************************************************************
// input: server node pointer, property 
// output: new property
//
//******************************************************************************
uchar SetServerNodeProperty(PMeshNode p_node,uchar property)
{TraceProc();

    p_node->Properities |= property;
    
    return p_node->Properities;
}

//******************************************************************************
// input: server node pointer
// output: ccurrent property
//
//******************************************************************************
uchar GetServerNodeProperty(PMeshNode p_node)
{TraceProc();
    return p_node->Properities;
}

int16 change(int16 num)
{
	if (num < 0)
	{
		num = ~(num - 1);
	}
	else if (num >0)
	{
		num = ~num + 1;
	}
	return num;
}


void DisplayProperty(PMeshNode p_node,uint8_t property_len,mesh_device_properties_t property_id,mesh_device_property_t property)
{//TraceProc();
    char tmp[21];
    count16_t people_count;
    temperature_8_t temperature;
    uint32 temp_rh;
    float jnc_temp,jnc_temp_point,jnc_rh;
    memset(tmp,0,21);
    switch(property_id)
        {
        case PEOPLE_COUNT:
             people_count = property.count16;
            if(property_len == 2 && people_count != (count16_t)0xFFFF)
                {                
                snprintf(tmp, 21, "Adr %2x  %4d %5u", p_node->ServerAddr, p_node->RSSI, people_count); //richard modify
                }
            break;
        case PRESENT_AMBIENT_TEMPERATURE:
             temperature = property.temperature_8;
            if(property_len == 1 && temperature != (temperature_8_t)0xFF)
                {
                temperature_8_t temperature = property.temperature_8;
                //tempData = (((tempData * 2) + 499) / 1000);
                snprintf(tmp, 21, "Adr %4x Temp %3d.%1dC", p_node->ServerAddr, temperature / 2, (temperature * 5) % 10); //richard
                }
        case JNC_TEMP_RH:
             temp_rh = property.JncTempRh;
            if(property_len == 4 && temp_rh != (uint32)0xFFFFFFFF)
                {
                  jnc_temp = ((float)((int16)HIWORD(temp_rh)))/10;
                  jnc_rh = ((float)((int16)((LOWORD(temp_rh)))))/10;
                  
                  Printf("Float Temp = %2.1f, RH=%2.1f\r\n",jnc_temp,jnc_rh);
                  snprintf(tmp, 21, "Adr %2X %2.1fc %2.1f%",p_node->ServerAddr,jnc_temp,jnc_rh);
                }            
            break;
                
        };

        DI_Print(tmp, DI_ROW_SENSOR_DATA + p_node->Index);
}


/// Old


/*******************************************************************************
 * It changes currently displayed property ID.
 ******************************************************************************/
void sensor_client_change_property(void)
{
    ToNextProperty();
    for(uint8_t sensor = 0; sensor < DISPLAYED_SENSORS; sensor++)
    {
        DI_Print("", DI_ROW_SENSOR_DATA + sensor);
    }
    printf("New property ID is %4.4x\r\n", GetCurrProperty());
}

/*******************************************************************************
 * Publishing of sensor client get descriptor request for currently displayed
 * property id. It also resets the registered devices counter.
 ******************************************************************************/
void sensor_client_publish_get_descriptor_request(void)
{
    registered_devices = 0;
    SetPropertyIndex(SCAN_MESH_NODE_INDEX);
    for(uint8_t sensor = 0; sensor < DISPLAYED_SENSORS; sensor++)
    {// clean display
        DI_Print(" ", DI_ROW_SENSOR_DATA + sensor);
    }
    Trace16_2(PropertyIndex, GetCurrProperty());
    Cmd_ms_client_get_descriptor(SENSOR_ELEMENT, PUBLISH_ADDRESS, IGNORED, NO_FLAGS, GetCurrProperty());

}



/***************************************************************************//**
 * Handling of sensor client descriptor status event.
 *
 * @param[in] pEvent  Pointer to sensor client descriptor status event.
 ******************************************************************************/
void handle_sensor_client_descriptor_status(msg_ms_client_descriptor_status_evt *pEvent)
{ TraceProc();
    //printf("evt:gecko_evt_mesh_sensor_client_descriptor_status_id\r\n");

    sensor_descriptor_t descriptor;
    if(pEvent->descriptors.len >= SIZE_OF_DESCRIPTOR)
    {
        mesh_lib_sensor_descriptors_from_buf(&descriptor, pEvent->descriptors.data, SIZE_OF_DESCRIPTOR);
        Trace16_3(descriptor.property_id,pEvent->server_address,pEvent->appkey_index);
        if(descriptor.property_id == GetCurrProperty() && registered_devices < DISPLAYED_SENSORS)
        {
            address_table[registered_devices] = pEvent->server_address;             
            registered_devices++;
            TraceDec2("registered_devices",registered_devices,pEvent->server_address);
        }else Trace("handle_sensor_client_descriptor_status Error !!!!");
    }
}

uint16 server_addr=2;

/*******************************************************************************
 * Publishing of sensor client get request for currently displayed property id.
 ******************************************************************************/
void sensor_client_publish_get_request(void)
{ 
    Trace16_1(server_addr);
    result = Cmd_ms_client_get(SENSOR_ELEMENT, PUBLISH_ADDRESS, IGNORED, NO_FLAGS, GetCurrProperty())->result;
    //result = Cmd_ms_client_get(SENSOR_ELEMENT, 2, IGNORED, NO_FLAGS,GetCurrProperty())->result;
    
    //result = Cmd_ms_client_get(SENSOR_ELEMENT, server_addr, IGNORED, NO_FLAGS, GetCurrProperty())->result;
    //if(++server_addr > 4) server_addr = 2;
    
   Trace16_1(result);
}

extern int16 BLE_RSSI[5];
/***************************************************************************//**
 * Handling of sensor client status event.
 *
 * @param[in] pEvent  Pointer to sensor client status event.
 ******************************************************************************/
void handle_sensor_client_status(msg_ms_client_status_evt *pEvent)
{ TraceProc();
    //printf("evt:gecko_evt_mesh_sensor_client_status_id\r\n");
    uint8_t *sensor_data = pEvent->sensor_data.data;
    uint8_t data_len = pEvent->sensor_data.len;
    uint8_t pos = 0;
    address_table[1]=2;address_table[2]=3;address_table[3]=4; //richard debug
    Trace16Ptr_3(pEvent,server_address ,client_address, appkey_index);
    for(uint8_t sensor = 0; sensor < DISPLAYED_SENSORS; sensor++)
    {
        if(pEvent->server_address == address_table[sensor])
        {
            while(pos < data_len)
            {
                if(data_len - pos > PROPERTY_ID_SIZE)
                {
                    mesh_device_properties_t property_id = (mesh_device_properties_t)(sensor_data[pos] + (sensor_data[pos + 1] << 8));
                    Trace16_1(property_id);
                    uint8_t property_len = sensor_data[pos + PROPERTY_ID_SIZE];
                    uint8_t *property_data = NULL;
                    if(property_len && (data_len - pos > PROPERTY_HEADER_SIZE))
                    {
                        property_data = &sensor_data[pos + PROPERTY_HEADER_SIZE];
                    }
                    if(property_id == GetCurrProperty())
                    {
                        char tmp[21];
                        switch(property_id)
                        {
                            case PEOPLE_COUNT:
                                if(property_len == 2)
                                {
                                    mesh_device_property_t property = mesh_sensor_data_from_buf(PEOPLE_COUNT, property_data);
                                    count16_t people_count = property.count16;
                                    if(people_count == (count16_t)0xFFFF)
                                    {
                                        snprintf(tmp, 21, "Adr %4x Count   N/K", address_table[sensor]);
                                    }
                                    else
                                    {
                                        //snprintf(tmp, 21, "Adr %4x Count %5u", address_table[sensor], people_count);

                                        snprintf(tmp, 21, "Adr %2x  %4d %5u", address_table[sensor], BLE_RSSI[sensor], people_count); //richard modify
                                    }
                                }
                                else
                                {
                                    snprintf(tmp, 21, "Adr %4x Count   N/A", address_table[sensor]);
                                }
                                DI_Print(tmp, DI_ROW_SENSOR_DATA + sensor);
                                break;

                            case PRESENT_AMBIENT_TEMPERATURE:
                                if(property_len == 1)
                                {
                                    mesh_device_property_t property = mesh_sensor_data_from_buf(PRESENT_AMBIENT_TEMPERATURE, property_data);
                                    temperature_8_t temperature = property.temperature_8;
                                    if(temperature == (temperature_8_t)0xFF)
                                    {
                                        snprintf(tmp, 21, "Adr %4x Temp    N/K", address_table[sensor]);
                                    }
                                    else
                                    {
                                        snprintf(tmp, 21, "Adr %4x Temp %3d.%1dC", address_table[sensor], temperature / 2, (temperature * 5) % 10);
                                    }
                                }
                                else
                                {
                                    snprintf(tmp, 21, "Adr %4x Temp    N/A", address_table[sensor]);
                                }
                                DI_Print(tmp, DI_ROW_SENSOR_DATA + sensor);
                                break;

                            default:
                                break;
                        }
                    }
                    pos += PROPERTY_HEADER_SIZE + property_len;
                }
                else
                {
                    pos = data_len;
                }
            }
        }
    }
}





#if 0
/*******************************************************************************
 * Handling of mesh sensor client events.
 * It handles:
 *  - sensor_client_descriptor_status
 *  - sensor_client_status
 *
 * @param[in] pEvent  Pointer to incoming sensor server event.
 ******************************************************************************/
//**********************************************************************************************
// Event: gecko_evt_mesh_sensor_client_descriptor_status_id,gecko_evt_mesh_sensor_client_status_id
//
//
//**********************************************************************************************
uint32 HandleSensorClientEventsProc(PCmdPacket pEvent)
{ //TraceProc();
    uint32 ret_code = TRUE;
    uint32 event_id;
    event_id = BGLIB_MSG_ID(pEvent->header);
    switch(event_id)
        {
            case Evt_ms_client_descriptor_status: Trace("Evt_ms_client_descriptor_status");
                handle_sensor_client_descriptor_status(&(pEvent->data.evt_mesh_sensor_client_descriptor_status));
                
                break;
            case Evt_ms_client_status: Trace("Evt_ms_client_status");
                //richard: debug
                address_table[1]=2;address_table[2]=3;address_table[3]=4;
                
                handle_sensor_client_status(&(pEvent->data.evt_mesh_sensor_client_status));
                break;
            default: 
                break;
        };

    
    return ret_code;
}
#endif

/** @} (end addtogroup Sensor) */
