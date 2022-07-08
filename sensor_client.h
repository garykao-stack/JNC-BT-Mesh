/***************************************************************************//**
 * @file  sensor_client.h
 * @brief Sensor client header file
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

#ifndef SENSOR_CLIENT_H
#define SENSOR_CLIENT_H
#include "ble_msg.h"
#include "mesh_device_properties.h"

/***************************************************************************//**
 * @addtogroup SensorClient
 * @{
 ******************************************************************************/

#define SENSOR_ELEMENT        0 ///< Sensor client model located in primary element
#define SENSOR_ELEMENT_2      1
#define PUBLISH_ADDRESS       0 ///< The unused 0 address is used for publishing
#define IGNORED               0 ///< Parameter ignored for publishing
#define NO_FLAGS              0 ///< No flags used for message
#define SIZE_OF_DESCRIPTOR    8 ///< The size of descriptor is 8 bytes
#define DISPLAYED_SENSORS     5 ///< There is place for 5 sensors on the display
#define PROPERTIES_NUMBER     3 ///< Number of supported properties
#define PROPERTY_ID_SIZE      2 ///< Size of property ID in bytes
#define PROPERTY_HEADER_SIZE  3 ///< Size of property header in bytes



#define MAX_MESH_NODE_NUM       8
#define MAX_PROPERTIES          10   //for server node properity num


// server node properity define Properities
#define PROPERITY_PEOPLE        BIT0
#define PROPERITY_TEMPATURE     BIT1
#define PROPERITY_RS485_JNC     BIT2
#define PROPERITY_3             BIT3
#define PROPERITY_4             BIT4
#define PROPERITY_5             BIT5
#define PROPERITY_6             BIT6
#define PROPERITY_7             BIT7

#define PROPERTY_INDEX_TEMPERATURE          0
#define PROPERTY_INDEX_PEOPLE               1
#define PROPERTY_INDEX_JNC_MODBUS           2
#define PROPERTY_INDEX_JNC_TEMP_RH          3

#define PROPERTY_INDEX_MAX                  8


#define SCAN_MESH_NODE_INDEX    PROPERTY_INDEX_JNC_TEMP_RH //PROPERTY_INDEX_JNC_TEMP_RH //PROPERTY_INDEX_PEOPLE   // for scan server node key
#define START_GET_NODE_DATA     SCAN_MESH_NODE_INDEX //PROPERTY_INDEX_PEOPLE
#define SCAN_SERVER_PROPERTY   ServerProperties[SCAN_MESH_NODE_INDEX] //PEOPLE_COUNT

#define ALL_SERVER_NODE_ADDR    0  // can boardcasting for all mesh node


#define ERROR_SERVER_ADDR       0xFF



#define SCAN_SERVER_INIT            1
#define SCAN_SERVER_START           2
#define SCAN_SERVER_WAITING         3 //waiting response from server node
#define SCAN_SERVER_TIMEOUT         4
#define SCAN_SERVER_COMPLETE        5
#define SCAN_SERVER_ENDING          6
#define SCAN_SERVER_PENDING         7



#define GET_PROPERTY_INIT           1
#define GET_PROPERTY_START          2
#define GET_PROPERTY_WAITING        3   //waiting response from server node
#define GET_PROPERTY_TIMEOUT        4
#define GET_PROPERTY_COMPLETE       5
#define GET_PROPERTY_5              6
#define GET_PROPERTY_ENDING         7
#define GET_PROPERTY_PENDING        8



// for scan and get property
#define TIMER_EVENT_SCAN_INIT           1
#define TIMER_EVENT_SCAN_ON             2
#define TIMER_EVENT_SCAN_OFF            3

#define TIMER_EVENT_GET_PROPERTY_ON     10
#define TIMER_EVENT_GET_PROPERTY_OFF    11



// Client Timer
//#define SOFT_TIMER_SCAN_NODE        5  // =500ms time-out
#define SOFT_TIMER_VALUE_SCAN_NODE    5  // =5 sec time-out
#define SOFT_TIMER_VALUE_GET_PROPERTY     50 //50*100 ms

// Waiting Timer
#define MAX_WAITING_TIMER           (TIMER_5SEC*2)

////// for server node status
#define SERVER_STATUS_UNKNOW        0
#define SERVER_STATUS_ACTIVE        1
#define SERVER_STATUS_WAITING       2
#define SERVER_STATUS_DEAD          3
#define SERVER_STATUS_COMPLETE      4

typedef struct
{
    uchar   Index;      // 0 ~ 255
    word    ServerAddr;
    uchar   Properities;    // for device all of property
    uchar   Status;     // 0: Unknow 1: active 2: waiting server response, 3: no response(dead) 
    uchar   RetryCount;     // for Error or time-out retry
    uchar   ReceiveTimer;
    int8    RSSI;
}MeshNode,*PMeshNode;


#define GetProperty(x)      ServerProperties[x]


#define SCAN_SERVER_TIMEOUT_CYCLE   TIMER_5SEC  // one time for scan server node
#define TIMER_SCAN_SERVER_CYCLE     TIMER_500MS
#define SCAN_SERVER_TIMEOUT_COUNTER (SCAN_SERVER_TIMEOUT_CYCLE/TIMER_SCAN_SERVER_CYCLE)

#define GET_PROERTY_TIMEOUT_CYCLE   TIMER_5SEC  // one time for scan server node
#define GET_PROERTY_CYCLE           CYCLE_CLIENT_GET_PROPERTY //TIMER_500MS
#define TIMER_STAGE_GET_PROERTY_CYCLE 100
#define GET_PROERTY_TIMEOUT_COUNTER (GET_PROERTY_TIMEOUT_CYCLE/GET_PROERTY_CYCLE)



extern MeshNode     TotalMeshNode[MAX_MESH_NODE_NUM];
extern uint8        CurrServerNodeNum;   // number of active Mesh Node connections
extern PMeshNode    pActiveNode;
extern uchar        GetPropertyStage,SanServerStage;




/***************************************************************************//**
 * @defgroup SensorClient Sensor Client Module
 * @brief Sensor Clinet Module Implementation
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup SensorClient
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * It changes currently displayed property ID.
 ******************************************************************************/
void sensor_client_change_property(void);

/***************************************************************************//**
 * Publishing of sensor client get descriptor request for currently displayed
 * property id. It also resets the registered devices counter.
 ******************************************************************************/
void sensor_client_publish_get_descriptor_request(void);

/***************************************************************************//**
 * Publishing of sensor client get request for currently displayed property id.
 ******************************************************************************/
void sensor_client_publish_get_request(void);

/***************************************************************************//**
 * Handling of mesh sensor server events.
 * It handles:
 *  - sensor_client_descriptor_status
 *  - sensor_client_status
 *
 * @param[in] pEvt  Pointer to incoming sensor server event.
 ******************************************************************************/
void handle_sensor_client_events(struct gecko_cmd_packet *pEvt);
uint32 HandleSensorClientEventsProc(PCmdPacket pEvent);


/** @} (end addtogroup Sensor) */

void handle_sensor_client_descriptor_status(  msg_ms_client_descriptor_status_evt *pEvent);
void handle_sensor_client_status(  msg_ms_client_status_evt *pEvent);


void SensorClientNodeInit(void);

void StartScanServerNode();
void StartGetServerProperty(void);


void ResetServerNodeStatus(void);
void ClientScanServerProc(void);
void ClientGetNodeDescriptor(uint16 server_addr,uint16 property_id);
void ClientStartScanServer();

void ClientDescriptorStatus(msg_ms_client_descriptor_status_evt *pStatus);
PMeshNode GetServerNode(word server_addr);
uint32 EvtMeshSensorClientProc(PCmdPacket pEvent);
void GetServerProperty(uint16 server_addr,mesh_device_properties_t property);
void GetRs485Property(uint16 server_addr,mesh_device_properties_t property);

void HandleServerProperty(msg_ms_client_status_evt *pEvent);
void HandleServerSeriesProperty(msg_ms_client_series_status_evt *pEvent);
void HandleServerColumnProperty(msg_ms_client_column_status_evt *pEvent);

PMeshNode CheckServerAddr(uint16 server_addr);

PMeshNode AddServerNode(word server_addr);
PMeshNode DeleteServerNode(word server_addr);
void SetAllNodeStatus(uchar status);
uchar SetServerNodeStatus(PMeshNode p_node,uchar status);
uchar GetServerNodeStatus(PMeshNode p_node);
uchar SetServerNodeProperty(PMeshNode p_node,uchar property);
uchar GetServerNodeProperty(PMeshNode p_node);
void HandleClientPublish(msg_ms_client_publish_evt *pEvent);
uchar ClientWaitingProperty();
void ClientGetServerDataProc();
uchar ServerResponseLose();
void ToNextProperty();
uint16 GetCurrProperty();
bool SetPropertyIndex(uint16 index);
void ClientSetBehavior(uint16 event_class, uint16 param);

void HandleModbusBtMesh(msg_ms_client_status_evt *pEvent);
void HandleSettingStatus(msg_ms_client_setting_status_evt *pEvent);






void DisplayProperty(PMeshNode p_node,uint8_t property_len,mesh_device_properties_t property_id,mesh_device_property_t property);
#endif /* SENSOR_CLIENT_H */
