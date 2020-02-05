
/*
 * I2C.h
 *
 *  Created on: 2019/12/10
 *      Author: Richard
 */

#ifndef _MODBUS_TO_MESH_
#define _MODBUS_TO_MESH_

// MM ==> MODBUS_MESH
#define MM_PENDING             1   // process pending
#define MM_USART_RX            2   // modbus protocol active
#define MM_MESH_TX             3   // send data to server
#define MM_MESH_RX_WAITING     4   // waiting server node response
#define MM_MESH_RX_OK          5   // receive data from server node
#define MM_USART_TX            6   // send data to host
#define MM_USART_TX_WAITING    7   // send data to host
#define MM_ENDING              8   // process complete
#define MM_USART_RX_WAITING    10   // receive data from server node
#define MM_WAITING_ERROR       20   // waiting server node response


void ModbusToMeshInit();
void ModbusToMeshClientProc();
void ModbusToMeshServerProc();
void ModBusCmdToDevice();
Result MeshServerSendModbusCmd();



#endif  //_RS483_TO_MESH_

