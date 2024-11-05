/* Device initialization header */
#include "global.h"

// IC bus initialize
#include "bus_i2c.h"
#include "bus_spi.h"
#include "bus_rs485.h"
#include "bus_usart.h"

// device initialize
#include "AD7147.h"
#include "G6_BT_Mesh.h"
#include "water_level_mesh.h"
#include "mod_bus.h"
#include "com_port.h"
#include "device_bus.h"

uchar  SpiStatus,I2cStatus;

//uchar SpiTxBuffer[TX_BUFFER_SIZE];
//uchar SpiRxBuffer[RX_BUFFER_SIZE];

//uint8_t TxRxBuffer[TXRX_BUFFER_SIZE] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xA};


void DeviceInit() 
{
    NodeDataInit();
    DeviceBusInit();    // must first
    DeviceAllInit();
}

void DeviceBusInit()
{
    SpiStatus = I2cStatus = 0;

    UsartInit();
    Rs485Init();
    I2CInit();
    SpiInit();
#ifdef  BT_MESH_G6    
    G6BtMeshInit();
#endif
}

void DIDO_init()
{
    // DI configure as inputs, with pull-up enabled
    GPIO_PinModeSet(BSP_DI_PORT, BSP_DI_PIN, gpioModeInputPull, 1);
    // DO
    GPIO_PinModeSet(BSP_DO_PORT, BSP_DO_PIN, gpioModePushPull, 0);
}

void DeviceAllInit()
{
    //AD7147Init(); 
    //WaterLevelMeshInit();
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



