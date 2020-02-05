/*
 * Global.h
 *  Created on: 2019/07/31
 *  Author: richard.huang
 */

#define FW_VER              90 // ==> 0.90
#define HW_VER              90
#define DEVICE_NAME         "JNC-BT-Mesh"
#define MANUFACTORY_NAME    "JNC"


#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#define DEBUG_PRINT

#ifdef DEBUG_PRINT
#define Printf printf
#else
#define Printf(fmt,...) //(0)
#endif




#include "base_def.h"
/* standard library headers */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
/* BG stack headers */
#include "bg_types.h"
#include "native_gecko.h"
/* Libraries containing default Gecko configuration values */
#include "em_emu.h"
#include "em_cmu.h"
#include "em_rtcc.h"
/* Device initialization header */
#include "hal-config.h"

#if defined(HAL_CONFIG)
    #include "bsphalconfig.h"
#else
    #include "bspconfig.h"
#endif
#include "leds.h"

/* Display Interface header */
#include "display_interface.h"
#include "udelay.h"
#include "sleep.h"

#include "gatt_db.h"
#include "ble_msg.h"
#include "cmd_rsp_evt.h"
#include "ble_comm.h"

#include "ble_event.h"
#include "mesh_event.h"




#define PRINT_TYPE_HEX          0
#define PRINT_TYPE_ASCII        1
#define PRINT_TYPE_HEX_ASCII    2

#define BLE_MODEL       BGM13P32F512GA
#define BLE_SDK         BLE_SDK_V212
#define BLE_MESH_SDK    MESH_SDK_V1501


#define UART_PORT      1
#define SWO_PORT        2

#define DEBUG_PORT      UART_PORT //SWO_PORT  //richard



typedef bd_addr MacAddress;


/* 如果想要一次印出多個變數，可以這樣做
#define print_var(var)     do { Printf("%s: %s\n", #var, var);     } while (0)
#define print_three_var(var) do{ print_var(var);  print_var(var##2);  print_var(var##3); } while (0)
int s = 10; int s2 = 20; int s3 = 30;
print_three_var(s);
//印出 s: 10 
//     s2: 20
//     s3 :30

*/
typedef struct _EventIDToString_
{
    uint32      event_id;
    char       *pString;
}EventIDToString,*PEventIDToString;

// Debug Message
#define TraceNum(str,num) Printf("%s => %d\r\n",str,num)

#define GetMacroValue(var)  \
        Printf("#define %s_Hex 0x%08lX\r\n",#var,(uint32)var)
#define TraceProc() \
        Printf("\r\n //***   %s %ld   ***//\r\n",__func__,TraceProcCount++)
#define TraceStep(str) \
        Printf("-- %s\r\n",str)

#define Trace(str) \
        Printf("%s\r\n",str)
#define TraceStr(str1,str2) \
        Printf("%s = %s\r\n",str1,str2)
#define TraceOk(str) \
                Printf("\r\nOK! ** %s **\r\n",str)

#define TraceErr(str) \
        Printf("\r\n\r\nError! ==**** %s ****==\r\n\r\n",str)
#define TraceErr1(str,v1) \
        Printf("\r\n\r\nError! ==**** %s Code= %Xh ****==\r\n\r\n",str,v1)
#define TraceErr2(str,v1,v2) \
        Printf("\r\n\r\nError! ==**** %s Code1=%Xh Code2=%Xh ****==\r\n\r\n",str,v1,v2)

#define Trace1(str,v1) \
        Printf("%s = 0x%08lX\r\n",str,(uint32)v1)
#define Trace2(str,v1,v2) \
        Printf("%s = 0x%08lX %s=0x%08lX\r\n",str,(uint32)v1,#v2,(uint32)v2)
#define Trace3(str,v1,v2,v3) \
        Printf("%s: %s = 0x%08lX %s=0x%08lX %s=0x%08lX\r\n",str,#v1,(uint32)v1,#v2,(uint32)v2,#v3,(uint32)v3)

#define TraceDec1(str,v1) \
        Printf("%s = %ld\r\n",str,(uint32)v1)
#define TraceDec2(str,v1,v2) \
        Printf("%s = %ld %s=%ld\r\n",str,(uint32)v1,#v2,(uint32)v2)
#define TraceDec3(str,v1,v2,v3) \
        Printf("%s: %s = %ld %s=%ld %s=%ld\r\n",str,#v1,(uint32)v1,#v2,(uint32)v2,#v3,(uint32)v3)
#define TraceDec4(str,v1,v2,v3,v4) \
        Printf("%s: %s = %ld %s=%ld %s=%ld %s=%ld\r\n",str,#v1,(uint32)v1,#v2,(uint32)v2,#v3,(uint32)v3,uint32)v4)


#define Trace32_1(v1) \
        Printf("=> %s = 0x%08lX \r\n",#v1,(uint32)v1);
#define Trace32_2(v1,v2) \
        Printf("=> %s = 0x%08lX %s=0x%08lX\r\n",#v1,(uint32)v1,#v2,(uint32)v2)
#define Trace32_3(v1,v2,v3) \
        Printf("=> %s = 0x%08lX %s=0x%08lX %s=0x%08lX\r\n",#v1,(uint32)v1,#v2,(uint32)v2,#v3,(uint32)v3)
#define Trace32_4(v1,v2,v3,v4) \
        Printf("=> %s = 0x%04X %s=0x%04X \r\n%s=0x%04X %s=0x%04X\r\n",#v1,(uint16)v1,#v2,(uint16)v2,#v3,(uint16)v3,#v4,(uint16)v4)


#define Trace16_1(v1) \
        Printf("=> %s = 0x%04X \r\n",#v1,(uint16)v1);
#define Trace16_2(v1,v2) \
        Printf("=> %s = 0x%04X %s=0x%04X\r\n",#v1,(uint16)v1,#v2,(uint16)v2)
#define Trace16_3(v1,v2,v3) \
        Printf("=> %s = 0x%04X %s=0x%04X %s=0x%04X\r\n",#v1,(uint16)v1,#v2,(uint16)v2,#v3,(uint16)v3)
#define Trace16_4(v1,v2,v3,v4) \
        Printf("=> %s = 0x%04X %s=0x%04X \r\n %s=0x%04X %s=0x%04X\r\n",#v1,(uint16)v1,#v2,(uint16)v2,#v3,(uint16)v3,#v4,(uint16)v4)


 
#define Trace8_1(v1) \
        Printf("=> %s = 0x%02X \r\n",#v1,(uint8)v1);
#define Trace8_2(v1,v2) \
        Printf("=> %s = 0x%02X %s=0x%02X\r\n",#v1,(uint8)v1,#v2,(uint8)v2)
#define Trace8_3(v1,v2,v3) \
        Printf("=> %s = 0x%02X %s=0x%02X %s=0x%02X\r\n",#v1,(uint8)v1,#v2,(uint8)v2,#v3,(uint8)v3)
#define Trace8_4(v1,v2,v3,v4) \
        Printf("=> %s = 0x%02X %s=0x%02X \r\n%s=0x%02X %s=0x%02X\r\n",#v1,(uint8)v1,#v2,(uint8)v2,#v3,(uint8)v3,#v4,(uint8)v4)

#define Trace16Ptr_1(ptr,v1) \
        Printf("%s=%04Xh \r\n",#v1,ptr->v1)
#define Trace16Ptr_2(ptr,v1,v2) \
        Printf("%s=%04Xh %s=%04Xh \r\n",#v1,ptr->v1,#v2,ptr->v2)
#define Trace16Ptr_3(ptr,v1,v2,v3) \
        Printf("%s=%04Xh %s=%04Xh %s=%04Xh \r\n",#v1,ptr->v1,#v2,ptr->v2,#v3,ptr->v3)
#define Trace16Ptr_4(ptr,v1,v2,v3,v4) \
                Printf("%s=%04Xh %s=%04Xh \r\n%s=%04Xh %s=%04Xh \r\n",#v1,ptr->v1,#v2,ptr->v2,#v3,ptr->v3,#v4,ptr->v4)

#define Trace32Ptr_1(ptr,v1) \
        Printf("%s=%08lXh \r\n",#v1,(uint32)ptr->v1)
#define Trace32Ptr_2(ptr,v1,v2) \
        Printf("%s=%08lXh %s=%08lXh \r\n",#v1,(uint32)ptr->v1,#v2,(uint32)ptr->v2)
#define Trace32Ptr_3(ptr,v1,v2,v3) \
        Printf("%s=%08lXh %s=%08lXh %s=%08lXh \r\n",#v1,(uint32)ptr->v1,#v2,(uint32)ptr->v2,#v3,(uint32)ptr->v3)
#define Trace32Ptr_4(ptr,v1,v2,v3,v4) \
        Printf("%s=%08lXh %s=%08lXh \r\n %s=%08lXh %s=%08lXh \r\n",#v1,(uint32)ptr->v1,#v2,(uint32)ptr->v2,#v3,(uint32)ptr->v3,#v4,(uint32)ptr->v4)


extern uint32  MeshNodeStatus;
extern uint32  TraceProcCount;


void GlobalInitial();
void Delay_ms(int ms);
void EventIDtoString(uint32 header);



void PrintData(PCHAR pTitle,PUINT16 pbuff, UINT len);
void PrintDataLen(PCHAR pTitle,PUINT16 pbuff,uint16 size, uchar len);
void PrintDataLenDec(PCHAR pTitle,PUINT16 pbuff,uint16 size, uchar len);


void PrintDataByte(char *pTitle,BYTE* pbuff, UINT len);
void PrintDataDec(char *pTitle,WORD* pbuff, UINT len);

void PrintDataType(PCHAR pString,PUCHAR pBuff,int size,uchar type);
void PrintData1(PUCHAR pCmdID, uchar size);
uint16 ShowResult(char* pString, uint16 result );
void SetStatusOn(uint32 status);
void SetStatusOff(uint32 status);
void SetNodeStatus(uint32 status,uchar on_off);
bool GetNodeStatus(uint32 status);
uint16 WordSwap(uint16 value);
void WordSwapBuff(PUINT16 pBuff,uchar size);

void JtagStatus(uchar status);
uint ModbusRtu_CRC16(uchar *updata,uint len);


#define USER_ID_MAX_NUM      16

#endif /* U_MEDIA_GLOBAL_H_ */
