#if MESH_COLUME_ENABLE
/*
 *
 *  Created on: 2019/12/10
 *      Author: Richard
 */

#ifndef _MODBUS_TO_MESH_
#define _MODBUS_TO_MESH_




void ModbusToMeshInit();
void ModbusToMeshClientProc();
void ModbusToMeshServerProc();
void ModBusCmdToDevice();
Result MeshServerSendModbusCmd();

#endif  //_RS483_TO_MESH_

#endif //MESH_COLUME_ENABLE

