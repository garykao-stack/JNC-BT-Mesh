/*
 * bus_usart.h
 *
 *  Created on: 2019/11/14
 *      Author: Richard
 */

#ifndef _COM_PORT_
#define _COM_PORT_

void ComPortInit();

void ComPortProc(void);
uchar ProtocolProc();
void ComPortSendCmd(uchar size);





#endif  //_COM_PORT_

