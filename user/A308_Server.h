/*
 * A308_Server.h
 *
 *  Created on: 2022年7月5日
 *      Author: user
 */

#ifndef USER_A308_SERVER_H_
#define USER_A308_SERVER_H_


void A308_ModbusAction();
void A308_TimeEvent();
void A308ClientSeriesEvent(msg_ms_client_series_status_evt *pEvent);
void A308_Initialize();
uint16 A308_SendToClient();
bool A308_Client_Modbus_Response();
bool A308_Connected();
bool A308_Client_GetInfo();
void A308_ResetModbusCmd();
uint16 A308_Fetch_Timeout_Ms();
void A308_StopModbusAction();
void A308_GetInfo_Set_Flag(uint32 status);

#endif /* USER_A308_SERVER_H_ */
