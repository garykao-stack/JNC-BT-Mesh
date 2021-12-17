

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
#ifdef  G6_BT_MESH

SPIDRV_HandleData_t G6SpiMaster;
SPIDRV_Handle_t pG6SpiMaster = &G6SpiMaster;

void G6SpiInit(void) //void initUSART1 (void)
{
    
    Ecode_t error_code;
    SPIDRV_Init_t SpiMasterInit = SPIDRV_MASTER_USART1;
    SpiMasterInit.portLocationCs = _USART_ROUTELOC0_CSLOC_LOC2;//_USART_ROUTELOC0_CSLOC_LOC2 for PA5
     error_code = SPIDRV_Init( pG6SpiMaster, &SpiMasterInit );
     if(error_code) {TraceErr("G6SpiInit");} else {TraceOk("G6SpiInit");}

     GPIO_PinModeSet(BSP_G6_SPEED_PORT, BSP_G6_SPEED_PIN, gpioModeInputPull, 1); //init speed button
}

void G6BtMeshInit()
{
    G6SpiInit();
    InitDac7760();
    InitBQ3200();
    TraceDec1("pAdjValue->G6Speed", pAdjValue->G6Speed);
    if(pAdjValue->G6Speed >= VOL_PERCENT_MIN && pAdjValue->G6Speed <= VOL_PERCENT_MAX)
      G6SetVol(pAdjValue->G6Speed);
    else G6SetVol(VOL_DEFAULT);
}
// G6S ==> G6 Stage
#define G6S_INFO_INIT       0x00
#define G6S_WAITING         0x10    // waiting message from host
#define G6S_SET_VOL         0x11    
#define G6S_GET_VOL         0x12
#define G6S_SET_DATE        0x13
#define G6S_GET_DATE        0x14
#define G6S_SCHEDULE        0x15
#define G6S_OTHER2          0x16
#define G6S_OTHER3          0x17
#define G6S_OK              0x18
#define G6S_ERROR           0x19



//
//Control G6
//
void G6ControlProc()
{
    if(GetNodeStatus(NS_G6_READY) != TRUE) return;
    pStageInfo = GetNodeStageInfo(SENSOR_BTM_G6_PROC);
    //
    switch(ActiveStage())
        {
            
            case G6S_INFO_INIT:
                ToNextStage(G6S_WAITING);  
                break;
            case G6S_WAITING: 
                ToNextStage(CheckG6Status()); 
                break;
            case G6S_GET_VOL: 
                ToNextStage(G6S_SET_VOL);
                break;
            case G6S_SET_VOL: 
                ToNextStage(G6S_GET_DATE);
                break;            
            case G6S_GET_DATE: 
                ToNextStage(G6S_SET_DATE);
                break;;
            case G6S_SET_DATE:
                ToNextStage(G6S_OK);
                break;
            case G6S_OK: Trace("G6S_OK");
                ToNextStage(G6S_WAITING);
                break;
            case G6S_ERROR:
                ToNextStage(G6S_WAITING);
                break;                
        };                
}

//
// return G6 whether change or not
//
uint8 CheckG6Status()
{
    uint8 stage=G6S_WAITING;

    return stage;
}

//
// 
//
uint8 CheckG6PowerOn(PDevDate p_date)
{
    uint8 stage=OFF;

    return stage;
}


//
// Input: 0% ~ 100%
//
Bool G6SetVol(uint16 percent)
{
    Bool ret_code=TRUE;
    DacSetVol(percent);
    if(pAdjValue->G6Speed != percent){Trace("Save Vol Data");
        pAdjValue->G6Speed = percent;
        WriteAdjValue();
      }
    
    return ret_code; 
}

//
// 0% ~ 100%
//
uint16 G6GetVol(uint16 value)
{
    Bool ret_code=TRUE;
    //value transform to voltage call G6SendCmd()

    return ret_code; 
}
#endif



