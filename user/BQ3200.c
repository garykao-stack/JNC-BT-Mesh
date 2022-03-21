#include "global.h"
#include "sensor_server.h"
#include "mesh_event.h"
#include "device_bus.h"
#include "com_port.h"
#include "bus_usart.h"
#include "sensor_client.h"
#include "sensor_server.h"
#include "Mesh_node.h"

#include "i2cspm.h"
#include "BQ3200.h"

_DevDate    DevDateIInit={0x00,0,0,24,5,31,12,21};
_DevDate    DevDate;
PDevDate    pDevDate=&DevDate;

//
// I2C Address 0x68
void InitBQ3200()
{
    pDevDate->StartAddr = 0;
    GetSysDate();
    if(pDevDate->Date.Year  < 21){SetSysDate(&DevDateIInit);}

}



//
//
//
bool GetSysDate()
{

    bool ret_code=TRUE;
    pDevDate->StartAddr=0;
    ret_code = GetRtcDate(pDevDate,sizeof(_DevDate));
    if(ret_code != TRUE) {return ret_code;}
    pDevDate->Date.Sec  = BcdToDec(pDevDate->Date.Sec  &=0x7F);  //get value
    pDevDate->Date.Min  = BcdToDec(pDevDate->Date.Min  &=0x7F);
    pDevDate->Date.Hour = BcdToDec(pDevDate->Date.Hour &=0x3F);
    pDevDate->Date.Week = BcdToDec(pDevDate->Date.Week &=0x0F);
    pDevDate->Date.Date = BcdToDec(pDevDate->Date.Date &=0x3F);
    pDevDate->Date.Month= BcdToDec(pDevDate->Date.Month&=0x1F);
    pDevDate->Date.Year = BcdToDec(pDevDate->Date.Year);
    if(pDevDate->Date.Week == 0) pDevDate->Date.Week = 7;
    return ret_code;
    
}

//
//
//
bool SetSysDate(PDevDate p_date)
{
    bool ret_code=FALSE;
    pDevDate->StartAddr=0;
    pDevDate->Date.Year =DecToBcd(p_date->Date.Year);
    pDevDate->Date.Month=DecToBcd(p_date->Date.Month);
    pDevDate->Date.Date =DecToBcd(p_date->Date.Date);
    pDevDate->Date.Week  =DecToBcd(p_date->Date.Week);
    pDevDate->Date.Hour =DecToBcd(p_date->Date.Hour);
    pDevDate->Date.Min  =DecToBcd(p_date->Date.Min);
    pDevDate->Date.Sec  =DecToBcd(p_date->Date.Sec);
    ret_code = SetRtcDate(pDevDate,sizeof(_DevDate));
    if(ret_code != TRUE) TraceErr("SetSysDate 1");
    return ret_code;  
}



//
//check time of controller is time out or not
//
Bool CheckDateTimeOut(PRtcDate p_date)
{
    bool ret_code=FALSE;
    _DevDate rtc_date;
    rtc_date.StartAddr=0;
    GetRtcDate(&rtc_date,sizeof(_DevDate));

    return ret_code;
    
}
//
//Calculate Week
//
uint16 CalculateWeek( uint16 year , uint16 month, uint16 day )
{
    int c,y,week;
    if (month == 1 || month == 2)
    year--, month += 12;
    c = year / 100;
    y = year - c * 100;
    week = y + y / 4 + c / 4 - 2 * c + 26 * (month + 1) / 10 + day - 1;
    while (week < 0)
    week += 7; week %= 7; 
    if(week == 0) week = 7;
   // TraceDec1("week ", week);
    return week;
}

//
//
//
uint8 BcdToDec(uint8 bcd)
{
    uint16 dec_h, dec_l;
    dec_h = ((bcd &(0x0F << 4))>>4);
    dec_l = bcd&0x0F;
    return dec_h*10 + dec_l;
}

//
//y=y/10*16+y%10
//
uint8 DecToBcd(uint8 dec)
{
    return ((dec/10)<<4)+(dec%10);
}
//
//set RTC time
//
bool SetRtcDate(PDevDate p_dev_date,uint16 data_len)
{
    bool ret_code=TRUE;
    I2C_TransferReturn_TypeDef ret;
    I2C_TransferSeq_TypeDef    i2c_seq;
    i2c_seq.addr  = BQ_ADDR;
    i2c_seq.flags = I2C_FLAG_WRITE;
    i2c_seq.buf[0].data = (PUCHAR)p_dev_date;//InitRtcTab;
    i2c_seq.buf[0].len = data_len;//sizeof(_DevDate);
    ret = I2CSPM_Transfer(BQ_I2C, &i2c_seq); 
    if(ret != i2cTransferDone){//TraceErr1("SetRtcDate",ret);
        ret_code = FALSE;
    }
    return ret_code;
}

//
//get RTC time
//
bool GetRtcDate(PDevDate p_dev_date,uint16 data_len)
{
    bool ret_code=TRUE;
    I2C_TransferReturn_TypeDef ret;
    I2C_TransferSeq_TypeDef    i2c_seq;
    BYTE  read_data[7];
    BYTE  write_data[1];
    memset(read_data,0,sizeof(read_data));
    i2c_seq.addr  = BQ_ADDR;
    i2c_seq.flags = I2C_FLAG_WRITE_READ;
    // Select command to issue 
    write_data[0] =p_dev_date->StartAddr; //0;
    i2c_seq.buf[0].data   = write_data;
    i2c_seq.buf[0].len    = 1;
    // Select location/length of data to be read 
    i2c_seq.buf[1].data = read_data;
    i2c_seq.buf[1].len  = data_len;//sizeof(read_data);
    ret = I2CSPM_Transfer(BQ_I2C, &i2c_seq); 
    if (ret == i2cTransferDone) {//TraceOk("GetRtcDate");
        memcpy(&p_dev_date->Date,read_data,sizeof(read_data));
    }else{TraceErr1("GetRtcDate",ret);
        ret_code = FALSE;
    }
    //PrintDataByte("GetRtcDate 2",(PUCHAR)&p_dev_date->Date,sizeof(read_data));
    return ret_code;
}



