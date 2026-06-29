/***************************************************************************//**
 * @file  sensor.h
 * @brief Sensor header file
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

#ifndef SENSOR_H
#define SENSOR_H
extern uchar DeviceModBusID;
/***************************************************************************//**
 * @defgroup Sensor Sensor Module
 * @brief Sensor Module Implementation
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup Sensor
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * Sensor initialization.
 * This is called at each boot if provisioning is already done.
 * Otherwise this function is called after provisioning is completed.
 ******************************************************************************/
void SensorServerNodeInit(void);

/* v1.45: 給中繼端(Client)用的精簡 sensor server model 啟用 — 只 init model 讓它能收
 * 序號 column-get(0x8072/0x8073),不做感測器(people/temp)初始化、不發佈感測資料。 */
void SensorServerModelInitForClient(void);

/***************************************************************************//**
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
 *  @param[in] pEvt  Pointer to incoming sensor server event.
 ******************************************************************************/
//void HandleSensorServerEventsProc(struct gecko_cmd_packet *pEvent);
uint32 EvtSensorServerEventsProc(PCmdPacket pCmdEvent);

void EvtServerGetRequestProc(   PCmdPacket pCmdEvent);
void EvtServerGetColumeRequest(PCmdPacket pCmdEvent);
void EvtServerGetSeriesReqest(PCmdPacket pCmdEvent);
void EvtServerPubEvent(PCmdPacket pCmdEvent);
void EvtSetupServerGetCadenceRequst(PCmdPacket pCmdEvent);
void EvtSetupServerSetCadenceRequst(PCmdPacket pCmdEvent);
void EvtSetupServerGetSettingsRequest(PCmdPacket pCmdEvent);
void EvtSetupServerGetSettingRequest(PCmdPacket pCmdEvent);
void EvtSetupServerSetSettingRequest(PCmdPacket pCmdEvent);

void ServerGetSensorData();
void SensorDataToClient();




/** @} (end addtogroup Sensor) */

#endif /* SENSOR_H */
