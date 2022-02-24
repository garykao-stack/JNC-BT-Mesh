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

typedef struct _G6Schedule_
{
    uchar   ScheduleID;   
    uchar   PowerPercent; // 0 ~ 4
    uchar   WeekPower;   // Sun, Mon, Tue....Sat Bit0:Sun Bit1: Mon Bit2: Tue .... Bit6: Sat Bi7:Week Status
    uint16  StartTime;   // 10h:20m ==> 10*60 + 20 = 620 minute 00 ~ 1439minute, StartTime must > EndTime
    uint16  EndTime;     //
}_G6Schedule,*PG6Schedule;

#define G6_SCHEDULE         BIT0;

#pragma pack(pop)

#define G6_ON               ON
#define G6_OFF              OFF

// G6S ==> G6 Stage
#define G6S_INFO_INIT       0x00
#define G6S_WAITING         0x10    // waiting message from host
#define G6S_CHECK_SCHE      0x11    // waiting message from host
#define G6S_NEXT_SCHE       0x12
#define G6S_SCHE_ON         0x13
#define G6S_SCHE_WAIT       0x14
#define G6S_WAIT_SCHE_OFF   0x15    //wait schedule OFF
#define G6S_SCHE_OFF        0x16
#define G6S_OTHER3          0x17
#define G6S_SCHE_OK         0x18
#define G6S_SCHE_ERROR      0x19

#define G6S_CHK_FILTER      0x20
#define G6S_FILTER_OPEN     0x21
#define G6S_FILTER_CLOSE    0x22






#define WEEK_MONDAY     BIT0
#define WEEK_TUEDAY     BIT1
#define WEEK_WEDDAY     BIT2
#define WEEK_THUDAY     BIT3
#define WEEK_FRIDAY     BIT4
#define WEEK_SATDAY     BIT5
#define WEEK_SUNDAY     BIT6
#define G6_SCHEDULE_ON  BIT7
#define WEEK_POWER_CLEAN_ALL  0

#define SetActiveSchedule(x)            pActSchedule = &(pAdjValue->G6Schedule[x])
#define SetActScheduleOn()              pActSchedule->WeekPower |= G6_SCHEDULE_ON
#define SetActScheduleOff()             pActSchedule->WeekPower &= ~G6_SCHEDULE_ON
#define SetActPowerPercent(x)           pActSchedule->PowerPercent = x;
#define SetActPowerTime(start,end)      pActSchedule->StartTime = start;pActSchedule->EndTime = end;


#define SetScheduleOn(x,status)        (pAdjValue->G6Schedule[x].WeekPower |= status)
#define SetScheduleOff(x,status)       (pAdjValue->G6Schedule[x].WeekPower &= ~status)



#define G6S_STABLE              0
#define G6S_AUTO                BIT0    // 1: schedule First
#define G6S_DOOR_OPEN           BIT1
//#define G6S_DOOR_CLOSE          BIT2
#define G6S_CLEAR_FILERT1       BIT3
#define G6S_CLEAR_ALL_FILTER    BIT4
#define G6S_WARING_FILTER1      BIT5
#define G6S_WARING_FILTER2      BIT6

#define G6S_SCHEDULE_ACTIVE     BIT14
#define G6_STATUS_CHANGE        BIT15 //(~G6S_AUTO)



#define G6S_DOOR_NOR            (G6S_DOOR_OPEN|G6S_DOOR_OPEN)




#define G6S_STATUS_INIT         0
#define G6S_CHECK_STATUS        1
#define G6S_CHK_DOOR            2
#define G6S_CHK_CLEAN           3
#define G6S_CHK_WARING          4
#define G6S_STATUS_OK           5
#define G6S_STATUS_ERROR        6
#define G6S_POWER_MENUAL        7
#define G6S_CHECK_FILTER        8


#define RELASH_TIMER            300

#define CLEAR_LED_FILTER1() \
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); Delay_ms(RELASH_TIMER);\
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); Delay_ms(RELASH_TIMER);\
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); Delay_ms(RELASH_TIMER);\
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); 
        
#define CLEAR_LED_ALL_FILTER() \
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); Delay_ms(RELASH_TIMER);\
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); Delay_ms(RELASH_TIMER);\
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); Delay_ms(RELASH_TIMER);\
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); Delay_ms(RELASH_TIMER);\
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); Delay_ms(RELASH_TIMER);\
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); Delay_ms(RELASH_TIMER);\
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); Delay_ms(RELASH_TIMER);\
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); 

#define CLEAR_LED_FILTER_RESET() \
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); GPIO_PinOutToggle(BSP_LED_UV_PORT,BSP_LED_UV_PIN); Delay_ms(RELASH_TIMER);\
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); GPIO_PinOutToggle(BSP_LED_UV_PORT,BSP_LED_UV_PIN); Delay_ms(RELASH_TIMER);\
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); GPIO_PinOutToggle(BSP_LED_UV_PORT,BSP_LED_UV_PIN); Delay_ms(RELASH_TIMER);\
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); GPIO_PinOutToggle(BSP_LED_UV_PORT,BSP_LED_UV_PIN); Delay_ms(RELASH_TIMER);\
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); GPIO_PinOutToggle(BSP_LED_UV_PORT,BSP_LED_UV_PIN); Delay_ms(RELASH_TIMER);\
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); GPIO_PinOutToggle(BSP_LED_UV_PORT,BSP_LED_UV_PIN);

#define CLEAR_LED_G6_RESET() \
        GPIO_PinOutToggle(BSP_LED_UV_PORT,BSP_LED_UV_PIN); Delay_ms(RELASH_TIMER);\
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); Delay_ms(RELASH_TIMER);\
        GPIO_PinOutToggle(BSP_LED_UV_PORT,BSP_LED_UV_PIN); Delay_ms(RELASH_TIMER);\
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); Delay_ms(RELASH_TIMER);\
        GPIO_PinOutToggle(BSP_LED_UV_PORT,BSP_LED_UV_PIN); Delay_ms(RELASH_TIMER);\
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); Delay_ms(RELASH_TIMER);\
        GPIO_PinOutToggle(BSP_LED_UV_PORT,BSP_LED_UV_PIN); Delay_ms(RELASH_TIMER);\
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); Delay_ms(RELASH_TIMER);
        
#define MENUAL_LED_SPEED() \
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); Delay_ms(RELASH_TIMER);\
        GPIO_PinOutToggle(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN); Delay_ms(RELASH_TIMER);


void G6BtMeshInit();
void G6ScheduleProc();
void G6CheckStatusProc();
bool G6NextSchedule();
bool G6isOnSchedule();
bool G6isOffSchedule();
bool G6UpdateRtcDate();
bool isScheduleStatus();
void SetWeekStatus(uchar schedule,uchar week,uchar status);

bool SetActPowerStatus(uchar week_power,uchar status);
bool SetPowerStatus(uchar schedule,uchar week_power,uchar status);
PG6Schedule GetActSchedule();
void CleanAllPowerStatus();
void G6PowerLed(uchar status);
bool GetDoorStatus();

void G6SetActStatus(uint16 g6_status, uint8 status);
bool G6GetActStatus(uint16 status);
bool isStatusChange();
bool G6ChkDoor();
bool G6ChkAuto();
bool G6ChkClearStatus();
bool G6ChkWarningStatus();
bool G6ChkFilter();

bool G6SetMotoActSpeed(uchar status);
bool G6SetAutoRun(uchar status);
uchar G6GetSegPower(uchar seg_power);
void G6FilterTimeInc();



void G6Test();





Bool G6SetMotoSpeed(uint16 moto_speed);
uint16 G6GetVol(uint16 value);
void G6PowerDecInc(uint16 status);

#endif
