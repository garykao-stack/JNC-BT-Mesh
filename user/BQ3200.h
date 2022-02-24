/*
 * BQ3200.h
 *  Created on: 2021/11/17   Author: Richard
 */

#ifndef _BQ3200_ 
#define _BQ3200_
#pragma pack(push)
#pragma pack(1)     //mapping to one byte

typedef struct _RtcDate_
{
    BYTE Sec;
    BYTE Min;
    BYTE Hour;
    BYTE Week;
    BYTE Date;
    BYTE Month;
    BYTE Year;
}_RtcDate,*PRtcDate;


typedef struct _DevDate_
{
    BYTE StartAddr;  // Start address to get info
    _RtcDate Date;
}_DevDate,*PDevDate;

#pragma pack(pop)

//#define ShowRtc(x) Printf("Year=%d Month=%d Date=%d Week=%d \r\nHour=%d Min=%d Sec=%d \r\n", \
//                   x->Date.Year,x->Date.Month,x->Date.Date,x->Date.Week, x->Date.Hour,x->Date.Min,x->Date.Sec)

#define ShowRtc(x) Printf("Date=%d Week=%d Hour=%d Min=%d Sec=%d \r\n", \
                   x->Date.Date,x->Date.Week, x->Date.Hour,x->Date.Min,x->Date.Sec)

extern PDevDate    pDevDate;

#define BQ_ADDR     0xD0    //I2C address
#define BQ_I2C      I2C0

void InitBQ3200();
bool SetRtcDate(PDevDate p_dev_date,uint16 data_len);
bool GetRtcDate(PDevDate p_dev_date,uint16 data_len);

byte bcdToDec(byte val);
uint8 BcdToDec(uint8 bcd);
uint8 DecToBcd(uint8 dec);

BYTE bcd_decimal_code( BYTE bcd);
BYTE decimal_bcd_code(BYTE decimal);
bool GetSysDate();
bool SetSysDate(PDevDate p_date);
uint16 CalculateWeek( uint16 year , uint16 month, uint16 day );


Bool CheckDateTimeOut(PRtcDate p_date);




#endif  


