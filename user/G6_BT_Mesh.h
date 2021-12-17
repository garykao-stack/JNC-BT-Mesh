/*
 *
 *  Created on: 2021/11/11
 *  Author: Richard
 */

#ifndef _G6_BT_MESH_
#define _G6_BT_MESH_
#include "DAC7760.h"
#include "BQ3200.h"


#pragma pack(push)
#pragma pack(1)     //mapping to one byte

typedef struct _G6Status_
{
    BYTE Status;    //ON or OFF
    _RtcDate Date;
}_G6Status,*PG6Status;

typedef struct _G6OnDate_
{
    uchar Status;   //ON/OFF
    uchar PowerOnDay;  // Sun, Mon, Tue....Sat
    uchar PowerOnH,PowerM;      //00:00 : ON Hour: Minute
    uchar PowerOffH,PowerOffM;  // 11:10 : OFF
    uchar PowerPercent;
}_G6OnDate,*PG6OnDate;



#pragma pack(pop)

#define G6_ON       ON
#define G6_OFF      OFF


void G6BtMeshInit();
void G6ControlProc();
uint8 CheckG6Status();
uint8 CheckG6PowerOn(PDevDate p_date);


Bool G6SetVol(uint16 value);
uint16 G6GetVol(uint16 value);
#endif
