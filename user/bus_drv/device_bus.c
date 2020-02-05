/* Device initialization header */
#include "global.h"

// IC bus initialize
#include "bus_i2c.h"
#include "bus_spi.h"
#include "bus_rs485.h"
#include "bus_usart.h"

// device initialize
#include "AD7147.h"
#include "water_level_mesh.h"
#include "mod_bus.h"
#include "com_port.h"
#include "device_bus.h"

uchar  SpiStatus,I2cStatus;

//uchar SpiTxBuffer[TX_BUFFER_SIZE];
//uchar SpiRxBuffer[RX_BUFFER_SIZE];

//uint8_t TxRxBuffer[TXRX_BUFFER_SIZE] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xA};


void DeviceInit() 
{TraceProc();
    DeviceBusInit();    // must first
    DeviceAllInit();
}

void DeviceBusInit()
{TraceProc();
    SpiStatus = I2cStatus = 0;

    UsartInit();
    Rs485Init();
    I2CInit();
    SpiInit();
}

void DeviceAllInit()
{TraceProc();
    //AD7147Init(); 
    WaterLevelMeshInit();
    ComPortInit();
    
}

PUCHAR GetTxBuff()
{
    return SpiTxBuffer;
}

PUCHAR GetRxBuff()
{
    return SpiRxBuffer;
}


//
// Device enable
//
bool DeviceOn(uchar dev_id)
{
    bool ret_code=true;

    return ret_code;
}

//
// Device disable
//
bool DeviceOff(uchar dev_id)
{
    bool ret_code=true;

    return ret_code;
}

//
// Device sleeping
//
bool DeviceSleep(uchar dev_id)
{
    bool ret_code=true;

    return ret_code;
}

//
// Device get data
//
bool DeviceGetData(uchar dev_id)
{
    bool ret_code=true;

    return ret_code;
}
//
// Device send cmd
//
bool DeviceSendCmd(uchar dev_id,PUCHAR pCmd,uchar size)
{
    bool ret_code=true;

    return ret_code;
}

//
// Device send cmd and get data
//
bool DeviceAccess(uchar dev_id,PUCHAR pCmd,uchar size)
{
    bool ret_code=true;

    return ret_code;
}



