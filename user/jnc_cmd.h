/*
 * jnc_cmd.h
 *
 *  Created on: 2019/10/31
 *  Author: Richard
 *
 */

#ifndef _JNC_CMD_
#define _JNC_CMD_

#define JNC_CMD_SIZE        16

#define JNC_CMD_OK              0
#define JNC_CMD_ERR_CHECK       1


void JncCmdInit(void);
uchar JncCmdProc(void);




#endif  //_JNC_CMD_

