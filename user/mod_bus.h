/*
 * modbus.h
 *
 *  Created on: 2019/10/31
 *  Author: Richard
 *
 */

#ifndef _MOD_BUS_
#define _MOD_BUS_

#define COM_PORT_BUFF_SIZE      32
#define MOD_BUS_CMD_SIZE        16


extern uchar  ComPortBuff[COM_PORT_BUFF_SIZE];


#define MOD_BUS_OK              0
#define MOD_BUS_ERR_CHECK       1


void ModBusInit(void);
uchar ModBusProc(void);



#endif  //_MOD_BUS_


