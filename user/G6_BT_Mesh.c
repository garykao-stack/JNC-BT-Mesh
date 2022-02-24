

#include "global.h"
#include "sensor_server.h"
#include "mesh_event.h"
#include "device_bus.h"
#include "com_port.h"
#include "bus_usart.h"
#include "sensor_server.h"
#include "Mesh_node.h"
#include "spidrv.h"
#include "G6_bt_Mesh.h"
#ifdef  BT_MESH_G6

SPIDRV_HandleData_t G6SpiMaster;
SPIDRV_Handle_t pG6SpiMaster = &G6SpiMaster;
PG6Schedule pActSchedule;
uint8   TimerFilterHR;

void G6SpiInit(void) //void initUSART1 (void)
{TraceProc();
    
    Ecode_t error_code;
    SPIDRV_Init_t SpiMasterInit = SPIDRV_MASTER_USART1;
     error_code = SPIDRV_Init( pG6SpiMaster, &SpiMasterInit );
     GPIO_PinModeSet(BSP_G6_SPEED_PORT, BSP_G6_SPEED_PIN, gpioModeInputPull, 1);     
     GPIO_PinModeSet(BSP_G6_RESET_PORT, BSP_G6_RESET_PIN, gpioModeInputPull, 1);
     
     GPIO_PinModeSet(BSP_FILTER_PORT, BSP_FILTER_PIN, gpioModeInputPull, 1); 
     GPIO_PinModeSet(BSP_USART1_CS_PORT, BSP_USART1_CS_PIN, gpioModePushPull, 1); 
     GPIO_PinModeSet(BSP_LED_UV_PORT, BSP_LED_UV_PIN, gpioModePushPull, 0);
     GPIO_PinModeSet(BSP_LED_POWER_PORT, BSP_LED_POWER_PIN, gpioModePushPull, 0); 
     
}

void G6BtMeshInit()
{TraceProc();
    uint8 loop;
    G6SpiInit();
    InitDac7760();
    InitBQ3200();
    
//    for(loop=0; loop<G6_SCHEDULE_NUM; loop++)
  //      pAdjValue->G6Schedule[loop].ScheduleID=loop;
    //pActSchedule = GetActSchedule();
    pActSchedule = NULL;//&(pAdjValue->G6Schedule[0]);
    SetEventTaskTimer(TD_TIMER_CHK_FILTER, TIMER_1MIN,TIMER_EVENT_REPEAT);

    
    if(pMeshNodeData->G6Status & G6_SET_AUTO_RUN) G6SetActStatus(G6S_AUTO,ON);
    else G6SetActStatus(G6S_AUTO,OFF);

    TimerFilterHR=pDevDate->Date.Hour;
 
    G6Test();
}


uint16   G6ActStatus,G6OutStatus;
uint16   G6Value;
uchar   TempStage;
uint16  ActMotoSpeed,DoorOpenVol;

#define SET_POWER_OFF           0
#define SET_POWER_SCHEDULE      1
#define SET_POWER_MENUAL        2

//
// G6 Schedule
//
void G6ScheduleProc()
{
    if(GetNodeStatus(NS_G6_READY) != TRUE) return;
    if(G6GetActStatus(G6S_DOOR_OPEN)) {//Trace1("Return Door Open",G6ActStatus);
    return;}
    
    pStageInfo = GetNodeStageInfo(SENSOR_BTM_G6_PROC);    
    
    switch(ActiveStage())
        {
            case G6S_INFO_INIT: 
                    ToNextStage(G6S_WAITING);  
                break;
            case G6S_WAITING: 
                    if(!CheckWaitTimeOut()  ){break;}
                    
                    if(G6NextSchedule() == TRUE) {
                        ToNextStage(G6S_SCHE_ON);
                    }else {
                        G6SetMotoActSpeed(SET_POWER_MENUAL);
                        ToWaitingStage(G6S_WAITING,WAIT_SEC(1));
                     };
                break;
            case G6S_SCHE_ON: 
                    G6SetMotoActSpeed(SET_POWER_SCHEDULE);
                    G6SetActStatus(G6S_SCHEDULE_ACTIVE,ON);
                    ToWaitingStage(G6S_SCHE_WAIT,WAIT_SEC(0));
                break;
            case G6S_SCHE_WAIT: 
                    if(CheckWaitTimeOut()){
                        ToWaitingStage(G6S_WAIT_SCHE_OFF,WAIT_SEC(0));
                    }
                break;
            case G6S_WAIT_SCHE_OFF: 
                    if(CheckWaitTimeOut() == TRUE){
                        if(G6isOffSchedule() == TRUE) {ToNextStage(G6S_SCHE_OFF);}
                        else ToWaitingStage(G6S_WAIT_SCHE_OFF,WAIT_SEC(1));
                    }
                break;            
            case G6S_SCHE_OFF: 
                    G6SetActStatus(G6S_SCHEDULE_ACTIVE,OFF);
                    ToNextStage(G6S_SCHE_OK);
                break;
            case G6S_SCHE_OK:
                    ToNextStage(G6S_WAITING);
                break;
            
            case G6S_SCHE_ERROR: 
                    ToNextStage(G6S_SCHE_OK);
                break;                
        };                
}



//
//
//
void G6CheckStatusProc()
{
    if(GetNodeStatus(NS_G6_READY) != TRUE) return;
    pStageInfo = GetNodeStageInfo(SENSOR_G6STATUS_PROC);
    
    switch(ActiveStage())
        {
            case G6S_STATUS_INIT: 
                if(GetDoorStatus() == OPEN) G6SetActStatus(G6S_DOOR_OPEN,OPEN);
                ToWaitingStage(G6S_CHECK_STATUS,WAIT_SEC(1));
                break;
            case G6S_CHECK_STATUS: 
                if(CheckWaitTimeOut() == TRUE){
                    ToNextStage(G6S_CHECK_FILTER);
                  }
                else if(isStatusChange()) ToNextStage(G6S_CHK_DOOR);
                break;
            case G6S_CHECK_FILTER: 
                G6ChkFilter();
                ToWaitingStage(G6S_CHECK_STATUS,WAIT_SEC(1));
                break;
            
            case G6S_CHK_DOOR: 
                if(G6ChkDoor()) {ToNextStage(G6S_POWER_MENUAL);} //to next
                else ToNextStage(G6S_CHECK_STATUS);
                break;
            case G6S_POWER_MENUAL: 
                G6ChkAuto();
                ToNextStage(G6S_CHK_CLEAN);
                break;
            case G6S_CHK_CLEAN: 
                if(G6ChkClearStatus()) ToNextStage(G6S_CHK_WARING); //to next
                else ToNextStage(G6S_STATUS_ERROR);
                break;
            case G6S_CHK_WARING: 
                if(G6ChkWarningStatus()) ToNextStage(G6S_STATUS_OK); //to next
                else ToNextStage(G6S_STATUS_ERROR);
                break;
            case G6S_STATUS_OK: 
                ToNextStage(G6S_CHECK_STATUS);
                break;
            case G6S_STATUS_ERROR: 
                ToNextStage(G6S_STATUS_OK);
                break;
            
        };                
    
}


bool G6SetAutoRun(uchar status)
{
    if(status == ON){
        G6SetActStatus(G6S_AUTO,ON);pMeshNodeData->G6Status |= G6_SET_AUTO_RUN;
      }else{
        G6SetActStatus(G6S_AUTO,OFF);pMeshNodeData->G6Status &= ~G6_SET_AUTO_RUN;
      }
}


bool G6SetMotoActSpeed(uchar status)
{
    bool ret_code=TRUE;
    uchar moto_speed;
    if(status == SET_POWER_SCHEDULE) 
        moto_speed = G6GetSegPower(pActSchedule->PowerPercent) ;
    else if(status == SET_POWER_MENUAL) {
        if(pMeshNodeData->G6HostPPercent == MENUAL_POWER_OFF) moto_speed = 0;
        else moto_speed = G6GetSegPower(pMeshNodeData->G6HostPPercent);
    }
    else if(status == SET_POWER_OFF) moto_speed = 0;
    G6SetMotoSpeed((uint16)moto_speed);
    return ret_code;
}



bool isStatusChange()
{
    bool ret_code=FALSE;
    if(G6ActStatus & G6_STATUS_CHANGE) {
        G6SetActStatus(G6_STATUS_CHANGE,OFF);
        ret_code=TRUE;
        }
    
   return ret_code;
}

//
//
//
void G6FilterTimeInc()
{
    pMeshNodeData->FilterAllTime1++;
    pMeshNodeData->FilterAllTime2++;
    WriteMeshNodeData();
    if((pMeshNodeData->FilterAllTime1 > pMeshNodeData->FilterTime1) || 
        (pMeshNodeData->FilterAllTime2 > pMeshNodeData->FilterTime2))
        G6SetActStatus(G6_STATUS_CHANGE,ON);
    
    
}

//
// check filter1, filter2 1 minute
//
bool G6ChkFilter()
{
    bool ret_code=TRUE;
        if((pMeshNodeData->FilterTime1 > 0) && (pMeshNodeData->FilterAllTime1 > pMeshNodeData->FilterTime1) ){
            //TraceDec2("Set Filter 1 Warning",pMeshNodeData->FilterAllTime1,pMeshNodeData->FilterTime1);
             G6SetActStatus(G6S_WARING_FILTER1, ON); 
        }
        
        if((pMeshNodeData->FilterTime2 > 0) && (pMeshNodeData->FilterAllTime2 > pMeshNodeData->FilterTime2) ){
            //TraceDec2("Set Filter 2 Warning",pMeshNodeData->FilterAllTime2,pMeshNodeData->FilterTime2);
             G6SetActStatus(G6S_WARING_FILTER2, ON); 
        }
    return ret_code;
}



bool G6ChkDoor()
{
    bool ret_code=TRUE;   
    if(G6GetActStatus(G6S_DOOR_OPEN)){
        if(DoorOpenVol==0){//TraceDec2("Save Vol 1",DoorOpenVol,ActMotoSpeed);
            DoorOpenVol = ActMotoSpeed; G6SetMotoSpeed(0); 
        }
    }
    else if(!G6GetActStatus(G6S_DOOR_OPEN)){//TraceDec2("G6 Door Close",ActMotoSpeed,DoorOpenVol);
            if(DoorOpenVol){
                G6SetMotoSpeed(DoorOpenVol);DoorOpenVol=0;
         }
         else G6SetMotoSpeed(ActMotoSpeed);

         G6SetActStatus(G6S_DOOR_OPEN,OFF);
    }
    if(GetDoorStatus() == OPEN) ret_code=FALSE;
    return ret_code;
}

bool G6ChkAuto()
{TraceProc();
    bool ret_code=TRUE;

    if(G6GetActStatus(G6S_AUTO) == OFF){
        //PrintDataByte("SegPPercent", (PBYTE)&(pMeshNodeData->SegPPercent), 6);
        G6SetMotoActSpeed(SET_POWER_MENUAL);
      }
    return ret_code;

}

bool G6ChkClearStatus()
{TraceProc();
    bool ret_code=TRUE;
    if(G6GetActStatus(G6S_CLEAR_ALL_FILTER)){
        pMeshNodeData->FilterAllTime1 = 0;
        pMeshNodeData->FilterAllTime2 = 0;
        G6SetActStatus(G6S_CLEAR_ALL_FILTER|G6S_CLEAR_FILERT1,OFF);
        G6SetActStatus(G6S_WARING_FILTER1|G6S_WARING_FILTER2, OFF); 
        WriteMeshNodeData();CLEAR_LED_ALL_FILTER();
        
      }else if(G6GetActStatus(G6S_CLEAR_FILERT1)){
        pMeshNodeData->FilterAllTime1 = 0;
        G6SetActStatus(G6S_CLEAR_FILERT1,OFF); 
        G6SetActStatus(G6S_WARING_FILTER1,OFF);
        WriteMeshNodeData();CLEAR_LED_FILTER1();
        
      }
    return ret_code;
}


bool G6ChkWarningStatus()
{TraceProc();
    bool ret_code=TRUE;
    /*
    if(G6ActStatus & G6S_WARING_FILTER1){Trace("G6S_WARING_FILTER1 1");
        G6SetActStatus(G6S_WARING_FILTER1,OFF);
      }
    else if(G6ActStatus & G6S_WARING_FILTER2){Trace("G6S_WARING_FILTER2 2");
         G6SetActStatus(G6S_WARING_FILTER2,OFF);
        }
    */        
    return ret_code;
}

void G6SetActStatus(uint16 g6_status, uint8 status)
{
    if(status == ON) G6ActStatus |= g6_status;
    else G6ActStatus &= ~g6_status;
}


bool G6GetActStatus(uint16 status)
{
    bool ret_code=FALSE;
    if(G6ActStatus & status) 
        ret_code=TRUE;
    return ret_code;
}


bool GetDoorStatus()
{
    bool ret_code=CLOSE;
    if(GPIO_PinInGet(BSP_FILTER_PORT,BSP_FILTER_PIN) == HIGH) 
        ret_code = OPEN;
    return ret_code;      
}

//
// setup current week power status
//
bool SetActPowerStatus(uchar week_power,uchar status)
{
    bool ret_code=TRUE;

    if(week_power == WEEK_POWER_CLEAN_ALL) {
        pActSchedule->WeekPower=0;
        pActSchedule->StartTime=0;
        pActSchedule->EndTime = 0;
        }
    else{
           if(status == ON) {
            pActSchedule->WeekPower |= week_power;
            //pActSchedule->WeekPower |= G6_SCHEDULE_ON;
            }
           else {
            pActSchedule->WeekPower &= ~week_power;
            //pActSchedule->WeekPower &= ~G6_SCHEDULE_ON;
            }
        }

    return ret_code;
}

//
// setup week power status
//
bool SetPowerStatus(uchar schedule,uchar week_power,uchar status)
{
    bool ret_code=TRUE;
    PG6Schedule p_schedule = &(pAdjValue->G6Schedule[schedule]);

    if(week_power == WEEK_POWER_CLEAN_ALL) {
        p_schedule->WeekPower=0;
        p_schedule->StartTime=0;
        p_schedule->EndTime = 0;
        
        }
    else{
           if(status == ON) 
            p_schedule->WeekPower |= status;
           else
            p_schedule->WeekPower &= ~status;
        }

    return ret_code;
}


//
//
//
void CleanAllPowerStatus()
{
    uchar loop;
    PG6Schedule p_schedule = &(pAdjValue->G6Schedule[0]);
    for(loop=0; loop<G6_SCHEDULE_NUM; loop++)
        {
         p_schedule->WeekPower = 0;
         p_schedule->PowerPercent = 0;
         p_schedule->StartTime = 0;
         p_schedule->EndTime = 0;
         p_schedule++;
        }
}

//
//
//
PG6Schedule GetActSchedule()
{
    PG6Schedule ret_code=NULL;
    PG6Schedule p_schedule = &(pAdjValue->G6Schedule[0]);
    uchar loop=0;

    for(loop=0; loop<G6_SCHEDULE_NUM; loop++)
       {
         if(p_schedule->WeekPower & G6_SCHEDULE_ON){
            ret_code = p_schedule; //active schedule
            break;
            }else Trace16_1(p_schedule->WeekPower);
        p_schedule++;                
       }
    return ret_code;
}


bool isScheduleStatus()
{
    bool ret_code=OFF;
    if(pActSchedule->WeekPower & G6_SCHEDULE_ON){
        ret_code=ON;
        }
    return ret_code;
}

void SetWeekStatus(uchar schedule,uchar week,uchar status)
{
    uchar week_status;    
    PG6Schedule p_schedule;
    p_schedule = &(pAdjValue->G6Schedule[schedule]);
    week_status = 0x01<<(week-1);
    if(status == ON)
        p_schedule->WeekPower |= week_status;
    else 
        p_schedule->WeekPower &= ~week_status; 
}
//
// set to active schedule
//
bool G6NextSchedule()
{
    Bool ret_code=FALSE;
    uchar loop;
        
        if(!G6GetActStatus(G6S_AUTO)) return ret_code;
        
        if(pActSchedule == NULL | (pActSchedule == &(pAdjValue->G6Schedule[4])))
            pActSchedule = &(pAdjValue->G6Schedule[0]);
        else 
            pActSchedule++; 
        
   
    if(G6isOnSchedule() == TRUE){
          ret_code = TRUE; 
      }
    return ret_code;
}




//
// check schedule on/off: must to check RTC
//
bool G6isOnSchedule()
{
    bool ret_code=FALSE;
    BYTE week_bit;
    uint16 time_curr;
    if(isScheduleStatus() == OFF || !G6UpdateRtcDate()) {return ret_code;}
    
    // check schedule
    week_bit = (0x01<<(pDevDate->Date.Week-1));
    if(!(pActSchedule->WeekPower & week_bit)) {return ret_code;}
    time_curr = pDevDate->Date.Hour*60+pDevDate->Date.Min;
    if(time_curr >= pActSchedule->StartTime && time_curr < pActSchedule->EndTime){
            ret_code=TRUE;
        }
    return ret_code;
}

//
//
//
bool G6isOffSchedule()
{
    bool ret_code=FALSE;
    uint16 time_curr;
    if(!G6GetActStatus(G6S_AUTO)) return TRUE; //Schedule must to OFF
    G6UpdateRtcDate();
    time_curr = pDevDate->Date.Hour*60+pDevDate->Date.Min;
    if(time_curr >= pActSchedule->EndTime)
        {
         ret_code=TRUE;
        }
    return ret_code;
}


//
// Get RTC value
//
bool G6UpdateRtcDate()
{
    bool ret_code=GetSysDate();
    return ret_code;
}

uchar  G6GetSegPower(uchar seg_power)
{
    uchar moto_speed;
    
    moto_speed = pMeshNodeData->SegPPercent[seg_power];
        
    return moto_speed;
}

#define SPEED_UINT_MAX          20
#define SPEED_TO_VOL_UINT       5    
// uint %
const uint16 SpeedToVol[SPEED_UINT_MAX+1]=
{
   00, 10,30, 40, 45, 50, 55, 60, 65,70, 
   75, 80, 82, 85, 87, 90, 92, 95, 97,100,
   100
};

//
//unit: 
//
void G6SetMotoSpeedVol(uint16 speed)
{
    uint16 percent;
    if(speed > 100) speed = 100;    
    percent = SpeedToVol[speed/SPEED_TO_VOL_UINT];
    DacSetVol(percent);
}


//
// Input: 0% ~ 100% for Moto Speed
//
Bool G6SetMotoSpeed(uint16 moto_speed)
{
    Bool ret_code=TRUE;
    if(moto_speed > VOL_PERCENT_MAX)  moto_speed = VOL_PERCENT_MAX;
    if(moto_speed != ActMotoSpeed){
        if(moto_speed == 0){G6PowerLed(OFF); }
        else {G6PowerLed(ON);}
    
        ActMotoSpeed = moto_speed; 
        pMeshNodeData->G6ActPercent = moto_speed;
        DacSetVol(moto_speed);
      }
    return ret_code; 
}



//
// 0% ~ 100%
//
uint16 G6GetVol(uint16 value)
{TraceProc();
    Bool ret_code=TRUE;

    return ret_code; 
}
//
// Dec:0 Inc:1
//
void G6PowerDecInc(uint16 status)
{
    uint16 g6_power = pMeshNodeData->G6ActPercent;
    if(status == INC)   
        {if(g6_power <100) g6_power++;}
    else 
        { if(g6_power >0) g6_power--;}
  
    G6SetMotoSpeed(g6_power); 
    pMeshNodeData->G6ActPercent = g6_power;WriteAdjValue();
}

#define POWER_LED_ON        GPIO_PinOutSet
#define POWER_LED_OFF       GPIO_PinOutClear
#define POWER_LED_Triggle   GPIO_PinOutToggle

void G6PowerLed(uchar status)
{

    if(status == ON){
          POWER_LED_ON(BSP_LED_UV_PORT,BSP_LED_UV_PIN);
          POWER_LED_ON(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN);
        }
    else{
         POWER_LED_OFF(BSP_LED_UV_PORT,BSP_LED_UV_PIN);
         POWER_LED_OFF(BSP_LED_POWER_PORT,BSP_LED_POWER_PIN);
        }
}


#define TEST_HR         14
#define TEST_MIN        55
#define TEST_WEEK       WEEK_TUEDAY
void G6Test()
{TraceProc();
    uint16 start_time,end_time;
   _DevDate    DevDate={0x00,0,TEST_MIN,TEST_HR,2,1,2,22}; // 2022/02/01 week 2 14:55 0sec
   SetSysDate(&DevDate); //ok
   
   CleanAllPowerStatus();
   SetActiveSchedule(0);
   SetActPowerStatus(TEST_WEEK,ON); SetActScheduleOn();
   SetActPowerPercent(0);   //pMeshNodeData->SegPPercent[0] = 17; //for 17%
   start_time=TEST_HR*60+TEST_MIN; //13:13
   end_time=start_time+1;
   SetActPowerTime(start_time,end_time);

   SetActiveSchedule(1);
   SetActPowerStatus(TEST_WEEK,ON); SetActScheduleOn();
   SetActPowerPercent(1); //pMeshNodeData->SegPPercent[1] = 27; //for 17%
   start_time++;   end_time++;
   SetActPowerTime(start_time,end_time);

   
   SetActiveSchedule(2);
   SetActPowerStatus(TEST_WEEK,ON); SetActScheduleOn();
   SetActPowerPercent(2); //pMeshNodeData->SegPPercent[2] = 37; //for 17%
   start_time++;   end_time++;
   SetActPowerTime(start_time,end_time);

   SetActiveSchedule(3);
   SetActPowerStatus(TEST_WEEK,ON); SetActScheduleOn();
   SetActPowerPercent(3); //pMeshNodeData->SegPPercent[3] = 47; //for 17%
   start_time++;   end_time++;
   SetActPowerTime(start_time,end_time);

   SetActiveSchedule(4);
   SetActPowerStatus(TEST_WEEK,ON); SetActScheduleOn();
   SetActPowerPercent(4); //pMeshNodeData->SegPPercent[4] = 100; //for 100%
   start_time++;   end_time++;
   SetActPowerTime(start_time,end_time);

   SetActiveSchedule(0);
  // pMeshNodeData->G6HostPPercent = 3;
   //PrintDataByte("G6Test 3", (PUCHAR)&(pAdjValue->G6Schedule), sizeof(pAdjValue->G6Schedule));
   pMeshNodeData->G6Status &= G6_SET_AUTO_RUN;
   G6SetActStatus(G6S_AUTO,ON);
   WriteAdjValue();
   
   pActSchedule = NULL;
}

#else
bool G6SetAutoRun(uchar status)
{
    return FALSE;
}

void G6SetActStatus(uint16 g6_status, uint8 status)
{
    
}

#endif



