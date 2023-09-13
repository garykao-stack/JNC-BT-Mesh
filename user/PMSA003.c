#include "PMSA003.h"

#include <math.h>

// #define PM25_Reset  ROM_GPIOPinWrite(GPIO_PORTH_BASE, GPIO_PIN_3,0x00)
// #define PM25_None   ROM_GPIOPinWrite(GPIO_PORTH_BASE, GPIO_PIN_3,0xff)

// char *PM25_Str[]=
// {
//   "PM25_Start",
//   "PM25_Auto",
//   "PM25_Manual",
//   "PM25_ErrStauts",
//   "PM25_Shutdown",
//   "PM25_BootUP",
//   "PM25_ReadValue",
//   "PM25_FanClean"
// };
// uint8_t PMSA003AutoCMD[6]  ={0x42,0x4d,0xe1,0x00,0x01};  //開啟攀藤PM2.5自動模式       CRC = 0x0171
// uint8_t PMSA003ManualCMD[6]={0x42,0x4d,0xe1,0x00,0x00};  //關閉攀藤PM2.5自動模式       CRC = 0x0170
// uint8_t PMSA003ValueCMD[6] ={0x42,0x4d,0xe2,0x00,0x00};  //取得攀藤PM2.5數據           CRC = 0x0171
// uint8_t PMSA003Standby[6]  ={0x42,0x4d,0xe4,0x00,0x00};  //設定攀藤待機模式            CRC = 0x0173
// uint8_t PMSA003Normal[6]   ={0x42,0x4d,0xe4,0x00,0x01};  //設計攀藤正常模式            CRC = 0x0174

// PM25_Action_Status PM25_Status;
// int PM25_TimeOut = 0;
uint16_t PM25ERRORCode,PM25Ver;

float PM01Value=0,PM25Value=0,PM10Value=0,PM25rawdata,PM25CF1;
uint16_t PM03_Count = 0,PM05_Count = 0;
float PM25_TSI=0.0;

//設定滑動平均次數
// const int MovingAvgTimes[10] = {0,0,0,10,10,10,10,10,10,10};
// int MovingAvgConut[AISIZE] = {0};
// float MovingAvgSum[AISIZE] = {0};
// float MovingAvgSave[AISIZE][10] = {0};


typedef enum _eAvgSwitch{
  AS_PM25 = 0,
  AS_PM10 = 1
}eAvgSwitch;
#define AVG_SWITCH_TOTAL  2

const int MovingAvgTimes = 3;
int MovingAvgConut[AVG_SWITCH_TOTAL] = 0;
float MovingAvgSum[AVG_SWITCH_TOTAL] = 0;
float MovingAvgSave[AVG_SWITCH_TOTAL][3] = {0};
int SensorMaxValue = 1000;
//滑動平均
float MovingAvgFloatFun(float Value, eAvgSwitch avgSwitch)
{
  float Average = 0;
  int Count = MovingAvgTimes;
  MovingAvgSum[avgSwitch] = 0;
  
  MovingAvgSave[avgSwitch][MovingAvgConut[avgSwitch]] = Value;
  for(int i=0;i<MovingAvgTimes;i++)
  {
    MovingAvgSum[avgSwitch] += MovingAvgSave[avgSwitch][i];
    if(MovingAvgSave[avgSwitch][i] == 0)
      Count--;
  }
  if(Count) Average = (MovingAvgSum[avgSwitch]/Count);
  else Average = 0;
  MovingAvgConut[avgSwitch] = (MovingAvgConut[avgSwitch]+1)%MovingAvgTimes;
  
  return Average;
}
long MovingAvgLongFun(long Value, eAvgSwitch avgSwitch)
{
  long Average = 0;
  int Count = MovingAvgTimes;
  MovingAvgSum[avgSwitch] = 0;
  
  MovingAvgSave[avgSwitch][MovingAvgConut[avgSwitch]] = (float)Value;
  for(int i=0;i<MovingAvgTimes;i++)
  {
    MovingAvgSum[avgSwitch] += MovingAvgSave[avgSwitch][i];
    if(MovingAvgSave[avgSwitch][i] == 0)
      Count--;
  }
  if(Count) Average = (long)(MovingAvgSum[avgSwitch]/Count);
  else Average = 0;
  MovingAvgConut[avgSwitch] = (MovingAvgConut[avgSwitch]+1)%MovingAvgTimes;
  return Average;
}

void PM25Sensor_Calculate(void)
{
  PM25Value = MovingAvgFloatFun(PM25Value, AS_PM25);
  
  if(PM25Value != 0)
    PM25_TSI = (1/(0.62/(PM25Value>=913?913:PM25Value)-0.0003))*0.38;
  else
    PM25_TSI = 0;
  if(PM25_TSI < 10)
    PM25Value = pow(PM25_TSI,0.87)*2-PM25_TSI*0.47;
  else
    PM25Value = PM25_TSI;

  PM25Value = PM25Value>SensorMaxValue?SensorMaxValue:PM25Value;


  // calculate PM10
  float PM25_TSI_Value = PM25Value;
  
  if(PM25_TSI_Value<=15)
    PM10Value = PM25_TSI_Value * 2;
  else if(PM25_TSI_Value <= 30)
    PM10Value = PM25_TSI_Value * 1.5 + 10;
  else if(PM25_TSI_Value <= 45)
    PM10Value = PM25_TSI_Value * 1.2 + 25;
  else
    PM10Value = PM25_TSI_Value * 0.9682 + 38.26374;
  PM10Value = MovingAvgFloatFun(PM10Value, AS_PM10);
}

void CalculatePMSA003Value(uint8_t* pbuff, uint16_t len) {
    // if (len >= 32) {
        PM01Value = (((uint16_t)pbuff[10]) << 8) | pbuff[11];
        //台大陳志傑(CQ)實驗室測試結果表示，PM2.5數值要使用PM10的讀值比較接近真值
        PM25Value = (((uint16_t)pbuff[12]) << 8) | pbuff[13];
        // PM25Value = (((uint16_t)pbuff[14]) << 8) | pbuff[15];
        PM25CF1 = (((uint16_t)pbuff[6]) << 8) | pbuff[7];
        PM10Value = (((uint16_t)pbuff[14]) << 8) | pbuff[15];
        PM03_Count = (((uint16_t)pbuff[16]) << 8) | pbuff[17];
        PM05_Count = (((uint16_t)pbuff[18]) << 8) | pbuff[19];

        if (PM25CF1 < 10) {
          PM25CF1 = PM25CF1 + (float)PM03_Count / 1000;
        }

        if(PM25Value < 10)
        {
          //於PM2.5數值上增加PM0.3的顆粒Count的補償，避免出現零的數值
          PM25Value = PM25Value + (float)PM03_Count/1000;
          //於PM10數值上增加PM0.3及PM0.5的顆粒Count的補償
          PM10Value = PM10Value + (float)PM03_Count/1000 + (float)PM05_Count/100;
        }

        PM25Ver = pbuff[28];
        PM25ERRORCode = pbuff[29];

        PM25Sensor_Calculate();

        // return PM25Value;  // ?g/m3
    // }
    // return -1;
}

float GetPMSA003Value_PM25() {
    return PM25Value;  // ?g/m3
}

float GetPMSA003Value_PM10() {
    return PM10Value;  // ?g/m3
}

uint16_t GetPMSA003CRC(uint8_t* pbuff, uint16_t len) {
    uint16_t crc = 0x0000;
    for (int i = 0; i < len; i++)
        crc += pbuff[i];
    return crc;
}

bool CheckPM25Valid(uint8_t* pbuff, uint16_t len) {
    if (pbuff[0] != 0x42 || pbuff[1] != 0x4D)
        return false;
    uint16_t modbus_crc1, modbus_crc2;
    if (len < 32) return false;
    modbus_crc1 = (pbuff[30] << 8) | pbuff[31];
    modbus_crc2 = GetPMSA003CRC(pbuff, len - 2);

    return (modbus_crc1 == modbus_crc2);
}

// void PM25Sensor_Action(void)
// {
//   // dprint("\r\n PM2.5 Action = %s\t PM25_TimeOut = %d",PM25_Str[PM25_Status],PM25_TimeOut); // test print
//   switch(PM25_Status)
//   {
//   case PM25_Start:
//     // PM25_Status = PM25_Auto;
//     PM25_Status = PM25_Manual;
//   break;
//   case PM25_Auto:
//     PM25CMDSend(PMSA003AutoCMD,5);
//     break;
//   case PM25_Manual:
//     PM25CMDSend(PMSA003ManualCMD,5);
//     break;
//   case PM25_ErrStauts:
//     PM25_Status = PM25_Shutdown;
//     break;
//   case PM25_Shutdown:
//     PM25CMDSend(PMSA003Standby,5);
//     break;
//   case PM25_BootUP:
//     PM25CMDSend(PMSA003Normal,5);
//     break;
//   case PM25_ReadValue:
//     PM25CMDSend(PMSA003ValueCMD,5);
//     break;
//   case PM25_FanClean:
//     PM25_Status = PM25_Manual;
//     break;
//   }
//   PM25Sensor_Calculate();
// }
