/*
 * Global.h
 *  Created on: 2019/07/31
 *  Author: richard.huang
 */


// 1.13 ==> Release for Final Version
// 1.14 ==> 1.Add (Skewness, Kurtosis), 2.Modify UltraSound for WaterLevel
//          3. Add DO485(specify version),4.Add PZem(比流器、Specify version), 5.Add A6D6, 6.Add Relay status
//          6. modify AIP status error
// 1.15
//          1. For OEM and Visual Sensor 2. Add IAQS and CW9

// 1.16     1. add Co2 sensor 2. add Gain & Offset 3. Add working Timer and Server Class
// 1.17     
/*
(1)ON/OFF維修模式 方法:
    1. Client Node:
    按下PB0 2sec以上ON.
    按下PB0 馬上放開OFF.
    2. Server Node: 可以依照上面的方法獨自 ON/OFF
    3. 維修模式ON: Blue LED 會閃爍.
    4.  維修模式ON: System Response Time < 5 Sec
    5.  維修模式啟用一次 最長維持時間 60 分鐘,會自動恢復原始設定
(2) 增加來自I6對維俢模式控制: Status: ON/OFF
    Modbus Cmd: FC6, Register=0xF000,  
    value: 0x00: OFF, 0x01:ON

(3) Win-Utility 可以修改下面參數
    1. 溫溼度 Gain and Offset
    2. Working Time
    3. 設定 BT Mesh 連接Sensor種類

(4) 增加來自I6對維俢模式控制: Status: ON/OFF
    Modbus Cmd: FC6, Register=0xF000,  
    value: 0x00: OFF, 0x01:ON    

(5) Support Android App Get BT Mesh Information  


*/

/* 1.18     1. for Android App Setup
            2. Modify Gain & Offset report error for App
            3. Add function for Utility to get information for 9 register
            4. Modify A308M Speed 小數點以下2位
            5. Add UltraSound
*/
// v1.20
//            1. Add Server + Relay Node Number to 45 ~ 50
//            2. UltraSound Add Temp & RH
//            3. Add 風速計-宇田FMT95/95
//            4. modify Temp%RH Offset Error


// v1.21
//            1. Add Server + Relay Node Number to 45
//            2. Modify Power percent display
//            3. Modify Android App Error:TempGain and TempOffset can not to be change
//            4. Modify for TempGain and TempOffset can not to be change
//            5. TempGain,TempOffset, HumGain, HumOffset can not to be reset by reset key
//            6. Under Unprovision status, then can enter to setup model

// v1.22      1. Do not delete other setup information from user
//            2. modify Battry % to down alway
//            3. Modify for Visual Sensor

// BTM G6

/*
 v1.00
            1. Add BTM G6

*/

///////////////////////////////////////////////////////////
//for G6-DAC7760
// version: 1.00


//#define BT_MESH_G6              1   //for BT Mesh Control G6
#define JNC_BT_MESH             1   //for JNC BT Mesh
//#define ULTRA_SOUND_SKYNET      1 //xxxx
//#define BTM_TRANSMITTER		1 //BTM / RS485 transmitter
#define BTM_A308				1


#ifdef BTM_A308
  #define FW_VER              100
  #define HW_VER              100
  #define DEVICE_NAME         "A308 BT Transmitter"
  #define MANUFACTORY_NAME    "JNC"
  #define NODE_DATA_ID        0xA5A5
  #define MODEL_NAME          "BTA308"

#endif

#ifdef BTM_TRANSMITTER
  #define FW_VER              100
  #define HW_VER              100
  #define DEVICE_NAME         "BT-Mesh/RS485 Transmitter"
  #define MANUFACTORY_NAME    "JNC"
  #define NODE_DATA_ID        0xA5A5
  #define MODEL_NAME          "BTM485"
#endif

#ifdef  BT_MESH_G6
  #define FW_VER              101
  #define HW_VER              100
  #define DEVICE_NAME         "G6S-BT"
  #define MANUFACTORY_NAME    "JNC"
  #define NODE_DATA_ID        0xA5A5
  #define MODEL_NAME          "G6S-BT"
#endif

#ifdef  JNC_BT_MESH
  #define FW_VER              124
  #define HW_VER              110
  #define DEVICE_NAME         "JNC-BT-Mesh"
  #define MANUFACTORY_NAME    "JNC"
  #define NODE_DATA_ID        0xA5A5
  #define MODEL_NAME          "BTM001"
#endif


#ifdef  ULTRA_SOUND_SKYNET
  #define FW_VER              101
  #define HW_VER              110
  #define DEVICE_NAME         "UD-BT-Mesh"
  #define MANUFACTORY_NAME    "JNC"
  #define NODE_DATA_ID        0xA5A5
  #define MODEL_NAME          "BTM-UD"
#endif


#ifndef _GLOBAL_H_
#define _GLOBAL_H_

//#define DEBUG_PRINT
#define DPRINT 1			// dprint, 0:disabled, 1:enabled
#define BTM_TEST


#ifdef DEBUG_PRINT
#define Printf printf
#else
#define Printf(fmt,...) //(0)
#endif

#if DPRINT
  #define dprint printf
  #define IFDPRINT(a) a
#else
  #define dprint(...)
  #define IFDPRINT(...)
#endif


#include "base_def.h"
// Utility
#include "infrastructure.h"

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
#include "node_data.h"
#include "ble_event.h"
#include "mesh_event.h"


#ifdef DEBUG_PRINT
    #define PrintRx(string) PrintDataByte(#string, UsartGetBuff(USART_ID_RX), MODBUS_CMD_NUM)
#else
    #define PrintRx(string)
#endif    




#define PRINT_TYPE_HEX          0
#define PRINT_TYPE_ASCII        1
#define PRINT_TYPE_HEX_ASCII    2

#define BLE_MODEL       BGM13P32F512GA
#define BLE_SDK         BLE_SDK_V212
#define BLE_MESH_SDK    MESH_SDK_V1501


#define UART_PORT       1
#define SWO_PORT        2

#define DEBUG_PORT      UART_PORT //SWO_PORT  //richard
#define VALUE_IS_NOT_KNOWN  (0xFFFF)



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
#include "debugprint.h"

extern uint32  MeshNodeStatus;
extern uint32  TraceProcCount;


void GlobalInitial();
void Delay_ms(int ms);
void EventIDtoString(uint32 header);



void PrintData(PCHAR pTitle,PUINT16 pbuff, UINT len);
void PrintDataByte(char *pTitle,BYTE* pbuff, UINT len);
void PrintDataDec(char *pTitle,WORD* pbuff, UINT len);
void PrintDataType(PCHAR pString,PUCHAR pBuff,int size,uchar type);
void PrintDataLen(PCHAR pTitle,PUINT16 pbuff,uint16 size, uchar len);
void PrintDataLenDec(PCHAR pTitle,PUINT16 pbuff,uint16 size, uchar len);
void PrintArray8(uint8array *pArrayBuff,int type);
uint16 ShowResult(char* pString, uint16 result );


#ifndef DEBUG_PRINT 

#define PrintData(a,b,c) 
#define PrintDataByte(a,b,c) 
#define PrintDataDec(a,b,c) 
#define PrintDataType(a,b,c,d) 
#define PrintDataLen(a,b,c,d) 
#define PrintDataLenDec(a,b,c,d) 
#define PrintArray8(a,b) 
#define ShowResult(a,b) 


#endif



void SetStatusOn(uint32 status);
void SetStatusOff(uint32 status);
void SetMeshNodeStatus(uint32 status,uchar on_off);
bool GetMeshNodeStatus(uint32 status);
uint16 WordSwap(uint16 value);
void DWordSwap(PUCHAR p_value);
void DWordSwapN(PUCHAR p_value, uint16 num);

void WordSwapBuff(PUINT16 pBuff,uchar size);

void JtagStatus(uchar status);
uint16 TwoValueDiff(uint16 value_a, uint16 value_b);
uchar WordToByte(word value);
word ByteToWord(uchar value);



#define USER_ID_MAX_NUM      16

#endif /* U_MEDIA_GLOBAL_H_ */
