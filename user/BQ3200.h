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
    BYTE Day;
    BYTE Date;
    BYTE Month;
    BYTE Year;
}_RtcDate,*PRtcDate;


typedef struct _DevDate_
{
    BYTE Addr;
    _RtcDate Date;
}_DevDate,*PDevDate;

#pragma pack(pop)

extern PRtcDate pRtcDate;

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


