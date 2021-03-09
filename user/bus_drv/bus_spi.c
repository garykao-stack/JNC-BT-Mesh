
#include <stdbool.h>
#include "global.h"
#include "spidrv.h"
#include "AD7147.h"
#include "device_bus.h"

#include "bus_spi.h"


#if DEVICE_SPI_ENABLE

uint SpiPinCS;


SPIDRV_HandleData_t SpiHandleDataMaster;
SPIDRV_Handle_t SpiHandleMaster = &SpiHandleDataMaster;


void SpiInit(void) //void initUSART1 (void)
{TraceProc();
    
    Ecode_t error_code;
    if(!GetMeshNodeStatus(STATUS_SPI_ENABLE)) return;

    // SpiPinCS = SPI_CS_PIN;
    // Configure GPIO mode for master
    GPIO_PinModeSet(SPI_SCLK_PORT,    SPI_SCLK_PIN,   gpioModePushPull, 1); // US1_CLK is push pull
    GPIO_PinModeSet(SPI_CS_PORT,      SPI_CS_PIN,     gpioModePushPull, 1); // US1_CS is push pull
    GPIO_PinModeSet(SPI_MOSI_PORT,    SPI_MOSI_PIN,   gpioModePushPull, 1); // US1_TX (MOSI) is push pull
    GPIO_PinModeSet(SPI_MISO_PORT,    SPI_MISO_PIN,   gpioModeInput,    1); // US1_RX (MISO) is input

    
    GPIO_PinModeSet(SPI_TEST_PORT, SPI_TEST_PIN, gpioModePushPull, 1); // US1_CS is push pull
            
    // PC6 - MOSI    // PC7 - MISO  // PC8 - CLK  // PC9 - CS
    SPIDRV_Init_t SpiMasterInit = SPIDRV_MASTER_USART1;
    SpiMasterInit.clockMode = spidrvClockMode3;
    SpiMasterInit.csControl = spidrvCsControlApplication;
     // Initialize the USART1 a SPI Master
     error_code = SPIDRV_Init( SpiHandleMaster, &SpiMasterInit );
     if(error_code) TraceErr("SPIDRV_Init");

     // Transmit data using a blocking transmit function
}

void SpiDeInit(void)
{
    Ecode_t error_code;
    //if(SpiHandleMaster)
    error_code = SPIDRV_DeInit(SpiHandleMaster);
    if(error_code) TraceErr("SPIDRV_DeInit");
}



//uchar SpiActiveDev;

// open and set active SPI device
void SpiDevOpen()
{
   //SpiActiveDev = device_id; 
}

// close spi device
void SpiDevClose()
{
    //if(SpiActiveDev != SPI_ACTIVE_DEVICE_OFF)
    //SpiActiveDev = SPI_ACTIVE_DEVICE_OFF; 
}


#define CS_SILICON_GPIO

// status = ON: ==> device enable
// status = OFF ==> device disable
void SpiSetCS(uchar status)
{
    GPIO_Port_TypeDef port;
    uint16  pin;
    uchar   device_id;
if(!GetMeshNodeStatus(STATUS_SPI_ENABLE)) return;    
#if AUTO_CS    
    return;
#endif

#ifdef CS_SILICON_GPIO
    // use BT Mesh GPIO
    port = pActDevSpi->port; pin=pActDevSpi->pin; device_id = pActDevSpi->DeviceID;
    if( device_id >= DEV_AD7147_0 && device_id <= DEV_AD7147_7)
        {
        if(status == HIGH) {GPIO_PinOutSet(port,pin);}// Trace("CS Hi");}
                            
        else {GPIO_PinOutClear(port,pin);}// Trace("CS Low");}
        }
    else TraceErr1("Other SPI Device CS",device_id); 
#else   // for CS_I2C_GPIO

#endif


    
}

// Set all of the CS of IC ON/OFF
void SpiSetAllCS(uchar status)
{//TraceProc();
    _PDeviceSpi p_deviceCs;
    p_deviceCs = DeviceSpi;
    if(!GetMeshNodeStatus(STATUS_SPI_ENABLE)) return;

    while(p_deviceCs->DeviceID != END_TBL)
        {
          SpiSetCS(status);  p_deviceCs++; 
        };
    SpiDevOpen(pActDevSpi->DeviceID);
}



bool SpiWrite(PUCHAR tx_buff,uchar size)
{//TraceProc();
    Ecode_t err_code; 
    bool ret_code=false;
    if(!GetMeshNodeStatus(STATUS_SPI_ENABLE)) return FALSE;
    
      // Transmit data using a blocking transmit function
    SpiSetCS(LOW);
    err_code = SPIDRV_MTransmitB(SpiHandleMaster, tx_buff, (int)size);
    SpiSetCS(HIGH);
    if(err_code) TraceErr1("spi_write", err_code);
   else ret_code = true;

    return ret_code;
}

bool SpiRead(PUCHAR cmd_buff, int cmd_size,PUCHAR rx_buff,int rx_size)
{//TraceProc();

    bool ret_code=false;
    Ecode_t err_code;
    if(!GetMeshNodeStatus(STATUS_SPI_ENABLE)) return FALSE;
    
    SpiSetCS(LOW); //Trace("DMA 1-1");
    err_code = SPIDRV_MTransmitB(SpiHandleMaster, cmd_buff, cmd_size); // send cmd
    if(err_code) {ret_code=false;TraceErr1("spi_write 4", err_code);}
    else ret_code = true;
    
    err_code = SPIDRV_MReceiveB(SpiHandleMaster, rx_buff, rx_size);    // receive data from slave
    if(err_code) {ret_code=false;TraceErr1("spi_Read 4", err_code);}
    else ret_code = true;
    
    SpiSetCS(HIGH);
    return ret_code;
}

#else
void SpiInit(void) //void initUSART1 (void)
{TraceProc();
}


#endif

