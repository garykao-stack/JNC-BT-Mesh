/*
 * mesh_event.h
 *
 *  Created on: 2019/07/04
 *      Author: Richard
 */

#ifndef _MESH_EVENT_
#define _MESH_EVENT_

#define SENSOR_SERVER_ID            0x1100
#define SENSOR_SETUP_SERVER_ID      0x1101
#define SENSOR_CLIENT_ID            0x1102



//From BLE Mesh v1.5
#define mlg_server_event_handler            mesh_lib_generic_server_event_handler
#define mlg_client_event_handler            mesh_lib_generic_client_event_handler
#define mlg_server_client_request_cb        mesh_lib_generic_server_client_request_cb
#define mgl_server_change_cb                mesh_lib_generic_server_change_cb
#define mgl_server_response                 mesh_lib_generic_server_response
#define mgl_server_update                   mesh_lib_generic_server_update
#define mgl_server_publish                  mesh_lib_generic_server_publish
#define mgl_server_register_handler         mesh_lib_generic_server_register_handler
#define mgl_client_server_response_cb       mesh_lib_generic_client_server_response_cb
#define ngl_client_get                      mesh_lib_generic_client_get
#define mgl_client_set                      mesh_lib_generic_client_set
#define mgl_client_publish                  mesh_lib_generic_client_publish
#define mgl_client_register_handler         mesh_lib_generic_client_register_handler


#define mg_state_on_off                     mesh_generic_state_on_off
#define mg_state_on_power_up                mesh_generic_state_on_power_up
#define mg_state_level                      mesh_generic_state_level
#define mg_state_power_level                mesh_generic_state_power_level
#define mg_state_power_level_last           mesh_generic_state_power_level_last
#define mg_state_power_level_default        mesh_generic_state_power_level_default
#define mg_state_power_level_range          mesh_generic_state_power_level_range
#define mg_state_transition_time            mesh_generic_state_transition_time
#define mg_state_battery                    mesh_generic_state_battery
#define mg_state_location_global            mesh_generic_state_location_global
#define mg_state_location_local             mesh_generic_state_location_local
#define mg_state_property_user              mesh_generic_state_property_user
#define mg_state_property_admin             mesh_generic_state_property_admin
#define mg_state_property_manuf             mesh_generic_state_property_manuf
#define mg_state_property_list_user         mesh_generic_state_property_list_user
#define mg_state_property_list_admin        mesh_generic_state_property_list_admin
#define mg_state_property_list_manuf        mesh_generic_state_property_list_manuf
#define mg_state_property_list_client       mesh_generic_state_property_list_client
                                          
#define ml_state_light_actual               mesh_lighting_state_lightness_actual
#define ml_state_light_linear               mesh_lighting_state_lightness_linear
#define ml_state_light_last                 mesh_lighting_state_lightness_last
#define ml_state_light_default              mesh_lighting_state_lightness_default
#define ml_state_light_range                mesh_lighting_state_lightness_range
                                          
#define ml_state_ctl                        mesh_lighting_state_ctl
#define ml_state_ctl_temp                   mesh_lighting_state_ctl_temperature
#define ml_state_ctl_default                mesh_lighting_state_ctl_default
#define ml_state_ctl_temp_range             mesh_lighting_state_ctl_temperature_range
#define ml_state_ctl_light_temp             mesh_lighting_state_ctl_lightness_temperature
                                          
                                            
#define mg_request_on_off                   mesh_generic_request_on_off
#define mg_request_on_power_up              mesh_generic_request_on_power_up
#define mg_request_level                    mesh_generic_request_level
#define mg_request_level_delta              mesh_generic_request_level_delta
#define mg_request_level_move               mesh_generic_request_level_move
#define mg_request_level_halt               mesh_generic_request_level_halt
#define mg_request_power_level              mesh_generic_request_power_level
  // last level cannot be set               // last level cannot be set
#define mg_request_power_level_default      mesh_generic_request_power_level_default
#define mg_request_power_level_range        mesh_generic_request_power_level_range
#define mg_request_transition_time          mesh_generic_request_transition_time
  // battery information cannot be set      // battery information cannot be set
#define mg_request_location_global          mesh_generic_request_location_global
#define mg_request_location_local           mesh_generic_request_location_local
#define mg_request_property_user            mesh_generic_request_property_user
#define mg_request_property_admin           mesh_generic_request_property_admin
#define mg_request_property_manuf           mesh_generic_request_property_manuf
                                          
#define ml_request_light_actual             mesh_lighting_request_lightness_actual
#define ml_request_light_linear             mesh_lighting_request_lightness_linear
#define ml_request_light_default            mesh_lighting_request_lightness_default
#define ml_request_light_range              mesh_lighting_request_lightness_range
                                          
#define ml_request_ctl                      mesh_lighting_request_ctl
#define ml_request_ctl_temp                 mesh_lighting_request_ctl_temperature
#define ml_request_ctl_default              mesh_lighting_request_ctl_default
#define ml_request_ctl_temp_range           mesh_lighting_request_ctl_temperature_range


//structure

typedef struct mesh_generic_on_off_state                       mg_on_off_state;
typedef struct mesh_generic_on_power_up_state                  mg_on_power_up_state;
typedef struct mesh_generic_level_state                        mg_level_state;
typedef struct mesh_generic_power_level_state                  mg_power_level_state;
typedef struct mesh_generic_power_level_last_state             mg_power_level_last_state;
typedef struct mesh_generic_power_level_default_state          mg_power_level_default_state;
typedef struct mesh_generic_power_level_range_state            mg_power_level_range_state;
typedef struct mesh_generic_transition_time_state              mg_transition_time_state;
typedef struct mesh_generic_battery_state                      mg_battery_state;
typedef struct mesh_generic_location_global_state              mg_location_global_state;
typedef struct mesh_generic_location_local_state               mg_location_local_state;
typedef struct mesh_generic_property_type                      mg_property_type;
typedef struct mesh_generic_property_list_state                mg_property_list_state;
typedef struct mesh_generic_property_state                     mg_property_state;
typedef struct mesh_lighting_lightness_state                   ml_light_state;
typedef struct mesh_lighting_lightness_range_state             ml_light_range_state;
typedef struct mesh_lighting_ctl_state                         ml_ctl_state;
typedef struct mesh_lighting_ctl_temperature_state             ml_ctl_temp_state;
typedef struct mesh_lighting_ctl_lightness_temperature_state   ml_ctl_light_temp_state;
typedef struct mesh_lighting_ctl_temperature_range_state       ml_ctl_temp_range_state;

typedef struct mesh_generic_request     mg_request;
typedef struct mesh_generic_state       mg_state;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Provisioning bearers defines.
 ******************************************************************************/
#define PB_ADV   0x1 ///< Advertising Provisioning Bearer
#define PB_GATT  0x2 ///< GATT Provisioning Bearer

#define TIMER_SENSOR_DESCRIPTOR     2000


/// Flag for indicating that initialization was performed
extern uint8_t init_done;
extern uint8 MeshNodeModel;
extern uint16 MeshNodeID;
extern uint32 MeshNodeIVI;


//function
void MeshEventInit();

bool MeshEventProc(PCmdPacket pEvent);
uint32 EvtMeshConfigProc(PCmdPacket pEvent);
uint32 EvtMeshProxyProc(PCmdPacket pEvent);

// for Mesh Event procedure
uint32 MeshSystemBootProc(PCmdPacket pEvent);

uint32 EvtMeshSystemBootProc(PCmdPacket pEvent);
uint32 EvtMeshSoftTimerProc(PCmdPacket pEvent);

uint32 EvtMeshSensorInitProc(PCmdPacket pEvent);
uint32 EvtMeshNodeProvProc(PCmdPacket pEvent);

uint32 EvtMeshNodeKeyAddedProc(PCmdPacket pEvent);
uint32 EvtMeshNodeModelConfigChangedProc(PCmdPacket pEvent);
uint32 EvtMeshNodeResetProc(PCmdPacket pEvent);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32 EvtMeshSensorClientDescriptorStatusProc(PCmdPacket pEvent);
uint32 EvtMeshSensorClientStatusProc(PCmdPacket pEvent);




///////////////////////////////////////////////////////////////////////////////////////////////////////////////

//gecko_evt_mesh_health_xxxx event
uint32 EvtMeshHealthServerAttentionProc(PCmdPacket pEvent);
uint32 EvtMeshGenServerClientRequestProc(PCmdPacket pEvent);
uint32 EvtMeshGenServerStateChangedProc(PCmdPacket pEvent);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void sensor_client_change_property(void);
void sensor_client_publish_get_descriptor_request(void);

////// for Friend Node //////////////////////////////////////////////////////////////////////////////////////////
void FriendNodeInit();
void FriendNodeDeInit();
uint32 EvtMeshLpnFriendProc(PCmdPacket pEvent);
void GetLocalCfg();




#endif

