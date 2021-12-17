#include "global.h"
#include "sensor_server.h"
#include "mesh_event.h"
#include "device_bus.h"
#include "com_port.h"
#include "bus_usart.h" 
#include "sensor_client.h"
#include "sensor_server.h"
#include "Mesh_node.h"
#include "spidrv.h"
#include "DAC7760.h"

extern SPIDRV_Handle_t pG6SpiMaster;

const uint8 InitDacNOP[]={0x00,0x00,0x00};  // NOP

const uint8 InitDac1[]={0x56,0x00,0x01}; //reset A
const uint8 InitDac2[]={0x55,0x10,0x09}; // Daisy Enable
const uint8 InitDac3[]={0x56,0x00,0x01,0x00,0x00,0x00}; //B reset
const uint8 InitDac4[]={0x55,0x10,0x01,0x00,0x00,0x00}; //B Daisy Disable
const uint8 InitDac5[]={0x57,0x00,0x20,0x57,0x00,0x20}; //DAC7_WRITE_CONFIGURATION_REGISTER
const uint8 InitDac6[]={0x58,0x80,0x00,0x58,0x80,0x00}; //DAC7_WRITE_GAIN_CALIBRATION_REGISTER
const uint8 InitDac7[]={0x59,0x00,0x20,0x59,0x00,0x20}; //DAC7_WRITE_ZERO_CALIBRATION_REGISTER

const uint8 InitDac2_1[]={0x01,0x80,0x00,0x01,0x80,0x00}; // 
const uint8 InitDac2_2[]={0x01,0x86,0x70,0x00,0x00,0x00}; // for B vol

uint16 VoOutPercent=50; //default 50% +5V-out


void InitDac7760()
{Trace("InitDac7760");
  // init DAC A and B
  Ecode_t ret_code=0;  
  ret_code = SPIDRV_MTransmitB(pG6SpiMaster, InitDac1, sizeof(InitDac1));
  if(ret_code != ECODE_OK) {TraceDec1("InitDac7760 - 1",ret_code);}  
  ret_code = SPIDRV_MTransmitB(pG6SpiMaster, InitDac2, sizeof(InitDac2));
  ret_code = SPIDRV_MTransmitB(pG6SpiMaster, InitDac3, sizeof(InitDac3));
  ret_code = SPIDRV_MTransmitB(pG6SpiMaster, InitDac4, sizeof(InitDac4));   
  ret_code = SPIDRV_MTransmitB(pG6SpiMaster, InitDac5, sizeof(InitDac5));
  ret_code = SPIDRV_MTransmitB(pG6SpiMaster, InitDac6, sizeof(InitDac6));    
  ret_code = SPIDRV_MTransmitB(pG6SpiMaster, InitDac7, sizeof(InitDac7));
  DacSetVol(VoOutPercent);
  
}


void DacSetVol(uint16 percent)
{Trace("DacSetVol");
    uint16 vol_data;
    Ecode_t ret_code;
    DAC_REG dac_reg;
    dac_reg.AddrA = DAC7_WRITE_DATA_REGISTER;
    dac_reg.AddrB = DAC7_WRITE_DATA_REGISTER;

    if(percent >VOL_PERCENT_MAX) percent = VOL_PERCENT_MAX;
    vol_data = (((0xfff*percent)/VOL_PERCENT_MAX))<<4; 
    dac_reg.ValueA=WordSwap(vol_data);

    if(percent > AB_DAC_DIFF) percent -= AB_DAC_DIFF;     // A-20% ==B
    else percent = VOL_PERCENT_MIN;
     
    vol_data = (((0xfff*percent)/VOL_PERCENT_MAX))<<4;
    dac_reg.ValueB=WordSwap(vol_data);
    
    ret_code = SPIDRV_MTransmitB(pG6SpiMaster, (uint8_t *)&dac_reg, sizeof(dac_reg));
    if(ret_code != ECODE_OK) Trace1("DacSetVol = %X - 1",ret_code);
}

//
//
//
void SetVolDecInc(uint16 status)
{TraceDec1("SetVolDecInc",VoOutPercent);
  if(status == 1)   
    {if(VoOutPercent <100) VoOutPercent++;}
  else 
    { if(VoOutPercent >0) VoOutPercent--;}
  
 DacSetVol(VoOutPercent);
}





