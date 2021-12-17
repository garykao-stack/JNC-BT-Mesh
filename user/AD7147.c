


#include <stdbool.h>
#include "global.h"
#include "device_bus.h"
#include "bus_spi.h"
#include "AD7147.h"

#if AD7147_ENABLE

const uint16 AD7147_Bank1Tbl[BANK1_INIT_TBL_SIZE]={
  WORD_SWAP(SPI_7147_WR|0x0000), //start address
  REG_VALUE_PWR_CONTROL,        //0x000 Power control register
  REG_VALUE_CALIBRATION,        //0x001 calibration status STAGE0_CAL_EN ~ STAGE11_CAL_EN
  REG_VALUE_AMB_COMP_CTRL0,     //0x002 AMB_COMP_CTRL0: -0
  REG_VALUE_AMB_COMP_CTRL1,     //0x003 AMB_COMP_CTRL1: -1
  REG_VALUE_AMB_COMP_CTRL2,     //0x004 AMB_COMP_CTRL2: -2
  0,//0x005
  0,//0x006 
  REG_VALUE_STAGE11_COMPLETE    //0x007 STAGE_COMPLETE_INT_ENABLE
};
  
const uint16 AD7147_Bank2Tbl[BANK2_INIT_TBL_SIZE]=
{
WORD_SWAP(SPI_7147_WR|0x0080), //start address
//CIN0 ~ CON6
SETUP_CIN0,   //for CIN0 setup
SETUP_CIN1,   //for CIN1 setup
SETUP_CIN2,   //for CIN2 setup
SETUP_CIN3,   //for CIN3 setup
SETUP_CIN4,   //for CIN4 setup
SETUP_CIN5,   //for CIN5 setup
SETUP_CIN6,   //for CIN6 setup
//CIN7 ~ CON12
SETUP_CIN7,   //for CIN7 setup
SETUP_CIN8,   //for CIN8 setup
SETUP_CIN9,   //for CIN9 setup
SETUP_CIN10,  //for CIN10 setup
SETUP_CIN11,  //for CIN11 setup
//SETUP_CIN12,  //for CIN12 setup     
};

_DeviceSpi DeviceSpi[AD7147_MAX_NUM+1]=
{
    {DEV_AD7147_0, &CdcValue[0*IC_CIN_NUM], SPI_TEST_PORT,SPI_TEST_PIN},//CS0
    {DEV_AD7147_1, &CdcValue[1*IC_CIN_NUM], SPI_CS_PORT, SPI_CS_PIN},   //CS1
    
    {DEV_AD7147_2, &CdcValue[2*IC_CIN_NUM], SPI_TEST_PORT,SPI_TEST_PIN},//CS0
    {DEV_AD7147_3, &CdcValue[3*IC_CIN_NUM], SPI_CS_PORT, SPI_CS_PIN},   //CS1
    
    {DEV_AD7147_4, &CdcValue[4*IC_CIN_NUM], SPI_TEST_PORT,SPI_TEST_PIN},//CS0
    {DEV_AD7147_5, &CdcValue[5*IC_CIN_NUM], SPI_CS_PORT, SPI_CS_PIN},   //CS1
    
    {DEV_AD7147_6, &CdcValue[6*IC_CIN_NUM], SPI_TEST_PORT,SPI_TEST_PIN},//CS0
    {DEV_AD7147_7, &CdcValue[7*IC_CIN_NUM], SPI_CS_PORT, SPI_CS_PIN},   //CS1
    {END_TBL,NULL,NULL,NULL}
};

_PDeviceSpi pActDevSpi;   // point to active SPI device




void AD7147Init()
{
    uchar loop;
    //PrintData("AD7147_Bank1Tbl_1", (PUINT16)AD7147_Bank1Tbl, BANK1_INIT_TBL_SIZE);
    //PrintData("AD7147_Bank1Tbl_2", (PUINT16)AD7147_Bank2Tbl, BANK2_INIT_TBL_SIZE);

    //for(loop=0; loop<10; loop++)  {AD7147AllCsStatus(LOW); Delay_ms(20); AD7147AllCsStatus(HIGH);}

    ResetGetCinInfo();
    //AD7147Num = GetAD7147Num(); //get total AD7147 number    

    Trace("Initial AD7147 Register");
    for(loop=0; loop<AD7147Num; loop++){//initial AD7147
        //Trace2("Device ID",pActDevSpi->DeviceID,loop);
        AD7147Open(); 
        AD7147WriteMultiReg(PWR_CONTROL,(PUINT16)&AD7147_Bank1Tbl,BANK1_INIT_TBL_SIZE);
        AD7147WriteMultiReg(STAGE0_BASE,(PUINT16)&AD7147_Bank2Tbl,BANK2_INIT_TBL_SIZE);
        AD7147Close();
        }    
    ResetGetCinInfo();
    
    
}



uchar  GetCinInfoStage;
uchar   GetCinDelayTimer;
extern word SoftTimer10ms[];


uchar TCounterGetCin,TCounterErrCin;

uchar ScanCinStatus;



void ResetScanCinStatus()
{
    TCounterGetCin = TCounterErrCin = 0;
    ScanCinStatus = SCAN_CIN_STATUS_PENDING;
    GetCinInfoStage = GET_CIN_STAGE_PENDING;
    SetEventTaskTimer(TD_TASK_GET_CIN_VALUE, CYCLE_GET_CIN_PENDING, TIMER_EVENT_REPEAT);
}


void StartScanCinValue()
{
    ScanCinStatus = SCAN_CIN_STATUS_ON_GOING;
    GetCinInfoStage = GET_CIN_STAGE_DATA_START;
    SetEventTaskTimer(TD_TASK_GET_CIN_VALUE, CYCLE_GET_CIN, TIMER_EVENT_REPEAT);
}

uchar GetScanCinStatus()
{
    return ScanCinStatus;
}

uchar GetScanCinStage()
{
    return GetCinInfoStage;
}


//
// To Get one sensor CINx value
void GetAD7147CinxProc()
{
    bool temp;
    switch(GetCinInfoStage)
        {
            case GET_CIN_STAGE_PENDING:// Trace("GET_CIN_PENDING");
                
                break;
            case GET_CIN_STAGE_DATA_START: Trace("GET_CIN_DATA_START");
                ScanCinStatus = SCAN_CIN_STATUS_ON_GOING;
                AD7147Open();
                TCounterGetCin = 0;
                GetCinInfoStage = GET_CIN_STAGE_DATA_WAITING;            
                break;
            case GET_CIN_STAGE_DATA_WAITING: //Trace("GET_CIN_DATA_WAITING");
                if(TCounterGetCin >= WAITING_GET_CIN_VALUE) GetCinInfoStage = GET_CIN_STAGE_DATA;
                TCounterGetCin++;
                break;
            case GET_CIN_STAGE_DATA:      //Trace("GET_CIN_DATA ");
                //temp = GetCinValue(pActDevSpi->CinDataBuff,IC_CIN_NUM);
                temp = GetCinValue();
                if(temp) {// Trace("GET_CIN_DATA 2");
                    GetCinInfoStage = GET_CIN_STAGE_DATA_COMPLETE;   // get CIN data OK
                    ScanCinStatus = SCAN_CIN_STATUS_COMPLETE;
                    }
                else {//Trace("GET_CIN_DATA Error");
                        GetCinInfoStage = GET_CIN_STAGE_ERROR;
                     }                
                break;

            case GET_CIN_STAGE_ERROR: Trace("GET_CIN_ERROR");
                if(TCounterErrCin >= ERROR_CIN_COUNTER)
                    { TraceErr1("TCounterErrCin",TCounterErrCin);
                     GetCinInfoStage = GET_CIN_STAGE_DATA_COMPLETE;
                     ScanCinStatus = SCAN_CIN_STATUS_ERROR;
                    }
                else
                    {
                       GetCinInfoStage = GET_CIN_STAGE_DATA_START; // get cin data again
                       
                    }
                TCounterErrCin++;
                break;
            case GET_CIN_STAGE_DATA_COMPLETE: Trace("GET_CIN_DATA_COMPLETE"); // to next
                AD7147Close();
                TCounterErrCin = 0;
                GetCinInfoStage = GET_CIN_STAGE_PENDING;
                SetEventTaskTimer(TD_TASK_GET_CIN_VALUE, CYCLE_GET_CIN_PENDING, TIMER_EVENT_REPEAT);
                break;
            default: TraceErr("GetAllCinInfo");  
        };
}



//
// Get all of the value of CINx
bool GetCinValue()
{
    bool ret_code;
    uint16 cmd_get_cin_value=WORD_SWAP(GET_CIN_VALUE);
    ret_code = GetCinStatus();
    if(ret_code)
        {// to get cin value from AD7147
        SpiRead((PUCHAR)&cmd_get_cin_value,2,(PUCHAR)pActDevSpi->CinDataBuff,CIN_BUFF_BYTE_SIZE);
        WordSwapBuff(pActDevSpi->CinDataBuff,IC_CIN_NUM);
        ret_code = TRUE;
        }
    else { //TraceErr("CIN Number Error 1"); 
            ret_code = FALSE; }
    return ret_code;
    
}

bool GetCinStatus()
{
    bool ret_code=FALSE;
    uint16 get_ciin_status=WORD_SWAP(GET_CIN_STATUS);
    uint16 cin_status; 
    SpiRead((PUCHAR)&get_ciin_status,2,(PUCHAR)&cin_status,2);
    cin_status = WordSwap(cin_status);
    //if(cin_status == GET_CIN_VALUE_OK) {ret_code=TRUE;Trace1("CIN Ok",cin_status);}
    if(cin_status & GET_CIN_VALUE_OK) {Trace16_1(cin_status);
            ret_code=TRUE;}
    else {//TraceErr1("CIN Number Error 2", cin_status);
            ret_code=FALSE;}
        
    return ret_code;
}



// Reset stage of GetOneAD7147Status
//
void ResetGetCinInfo()
{
    pActDevSpi = NULL; //pActSpiCinBuff=NULL;
    ResetScanCinStatus();
}


// open spi device and initial spi pointer
void AD7147Open()
{
    SetDeviceSpi();
    SpiDevOpen();
    AD7147Power(FULL_POWER);    // full power status
}

// AD7147 close device and go to next spi device
void AD7147Close()
{
    AD7147Power(LOW_POWER); // low power status
    SpiDevClose();
    
}
//
// To Next SPI device
_PDeviceSpi SetDeviceSpi()
{
    _PDeviceSpi p_dev_spi;
    if(pActDevSpi == NULL || pActDevSpi->CinDataBuff == NULL) 
        p_dev_spi = DeviceSpi;
    else p_dev_spi = pActDevSpi+1;

    pActDevSpi = p_dev_spi;    
    return p_dev_spi;
    
}


//const uchar AD71447CmdOn[4]={0xE0,0x00,0x00,0xB0};
//const uchar AD71447CmdOff[4]={0xE0,0x00,0x00,0x01};

// control power of AD7147
//
void AD7147Power(uchar status)
{
    if(status == LOW_POWER) { AD7147WriteReg(PWR_CONTROL,0x001);}
    else { 
           AD7147WriteReg(AMB_COMP_CTRL0,CIN_STAGE_RESET); // resets the conversion sequence to STAGE0 
           AD7147WriteReg(PWR_CONTROL,0x0B0); // full power
           //Delay_ms(2); //Full Power model: must delay
        } 
}

// all spi device status = ON/OFF
//
void AD7147AllCsStatus(uchar status)
{
    SpiSetAllCS(status);
}



// write one registaer
//
bool AD7147WriteReg(uint16 reg_addr,uint16 reg_data)
{
    bool  ret_code=true;
    AD7147Reg ad7147_reg;
    
    ad7147_reg.addr = WordSwap(reg_addr|SPI_7147_WR);
    ad7147_reg.data = WordSwap(reg_data);
    SpiWrite((PUCHAR)&ad7147_reg,4); 
    return ret_code;
}


//
// size: word number
//
bool AD7147WriteMultiReg(uint16 start_addr,uint16* p_reg_data, uchar size)
{
    bool  ret_code=true;
    SpiWrite((PUCHAR)p_reg_data,size*2); 
    return ret_code;
}
#else
void AD7147Init() {return;}


#endif


