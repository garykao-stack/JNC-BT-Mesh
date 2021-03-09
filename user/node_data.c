#include "global.h"

//richard add
#include "em_device.h"
#include "em_chip.h"
#include "em_emu.h"
#include "em_msc.h"
#include "em_gpio.h"
#include "em_cmu.h"

//richard Add

#include "leds.h"
#include "sensor_client.h"
#include "bus_usart.h"
#include "Mesh_Node.h"

#include "node_data.h"



_Mesh_Node_Data MeshNodeData;                   /**< User data contents */
_PMesh_Node_Data pMeshNodeData;

_AdjustValue    AdjustValue;
_PAdjustValue   pAdjValue;


//const long Baudrate[]={2400,4800,9600,19200,38400,57600,115200};
//uint32 *pNodedata = (uint32*)0x0FE00000;  // pointer to the beginning of UD page
uint16 val1, val2;
word LevelData[12]={0,1,2,3,4,5,6,7,8,9,10,11};
word LevelDataRead[12];


void NodeDataInit()
{TraceProc();

    TraceDec1("MeshNodeData Size", sizeof(_Mesh_Node_Data));
    pMeshNodeData = &MeshNodeData;
    pAdjValue = &AdjustValue;
    ReadNodeData();
    //PrintDataByte("MeshNodeData", (PUCHAR)pMeshNodeData,NODE_DATA_SIZE);
   // PrintDataByte("Adjust Value", (PUCHAR)pAdjValue,ADJUST_VALUE_SIZE);
    if(pMeshNodeData->DataInitID != NODE_DATA_ID) MeshNodeDataReset();
    return;
}



void MeshNodeDataReset()
{TraceProc();

    memset(&MeshNodeData,0,NODE_DATA_SIZE);
    pMeshNodeData->DataInitID=NODE_DATA_ID;    
    pMeshNodeData->StructVer=100;
    pMeshNodeData->MeshNodeID = 0;
    pMeshNodeData->MeshNodeRole = NR_DEFAULT;
    pMeshNodeData->SensorClass = SENSOR_AIP; //SENSOR_CLASS_PT485;
    pMeshNodeData->BaudRate=USART_BAUDRATE_DEFAULT; //for 9600
    pMeshNodeData->TxPower=TX_POWER_HI;
    pMeshNodeData->CTune = BSP_CLK_HFXO_CTUNE;
    pMeshNodeData->TxGain = COMP_TX_POWER;
    pMeshNodeData->RxGain = COMP_RX_POWER;
    pMeshNodeData->SleepingTimer=TIMER_NODE_SLEEPING;
    pMeshNodeData->TempDiff=0;
    pMeshNodeData->HumidityDiff=0;


    pAdjValue->TempGain = 1.0;
    pAdjValue->TempOffset = 0.0;
    pAdjValue->HumGain = 1.0;
    pAdjValue->HumOffset = 0.0;
    pAdjValue->UserTempGain = 1.0;
    pAdjValue->UserTempOffset = 0.0;
    pAdjValue->UserRhGain = 1.0;
    pAdjValue->UserRhOffset = 0.0;
 
    WriteNodeData();
   
}





#define WRITE_MESH_NODE_ALL         0
#define WRITE_MESH_NODE_DATA        1
#define WRITE_MESH_ADJ_VALUE        2


// write node data to PS key
// error code:0x018A
//
Result WriteNodeData()
{TraceProc();
    Result ret_code=RESULT_OK;

   ret_code = Cmd_flash_ps_save(PS_KEY_MESH_NODE_DATA,NODE_DATA_SIZE,(const uint8*)pMeshNodeData)->result;
   if(ret_code) {Trace1("Write Node Data Error %x",ret_code);}
   else Delay_ms(20);

    ret_code = Cmd_flash_ps_save(PS_KEY_ADJUST_VALUE,ADJUST_VALUE_SIZE,(const uint8*)pAdjValue)->result;
    if(ret_code) {Trace1("Write AdjustValue Error %x",ret_code);}
    else Delay_ms(20);

   
    return ret_code;
}


// Read node data to PS key
//
//
Result ReadNodeData()
{TraceProc();
  Result ret_code=RESULT_OK;

  struct gecko_msg_flash_ps_load_rsp_t* pRsp;
  pRsp = Cmd_flash_ps_load(PS_KEY_MESH_NODE_DATA);
  if(pRsp->result == RESULT_OK) memcpy((void *)pMeshNodeData, (void *)&(pRsp->value.data), pRsp->value.len);
  else {ret_code = pRsp->result; Trace1("ReadNodeData Error %x",pRsp->result);}

  pRsp = Cmd_flash_ps_load(PS_KEY_ADJUST_VALUE);
  if(pRsp->result == RESULT_OK) memcpy((void *)pAdjValue, (void *)&(pRsp->value.data), pRsp->value.len);
  else {ret_code = pRsp->result; Trace1("Read Adjust Value Error %x",pRsp->result);}

  return ret_code;
}





//
//write calibration dat to PS
//
Result WriteWlCalData()
{TraceProc();
    Result ret_code=RESULT_OK;
    
    return ret_code;
}


// Read calibration data from PS
//
//
Result ReadWlCalData()
{TraceProc();
    Result ret_code=RESULT_OK;
    
    return ret_code;

}

//
// return len of key
//
uchar GetMeshNodeData(uint16 key, PUCHAR pnode_data)
{
    uchar ret_code=0;
    struct gecko_msg_flash_ps_load_rsp_t* pRsp;
    pRsp = Cmd_flash_ps_load(key);
    if(!pRsp->result){
      memcpy((void *)pnode_data, (void *)&(pRsp->value.data), pRsp->value.len);
      }
    else {ret_code = 0; Trace1("ReadNodeData Error %x",result);}

    return ret_code;
    
}

//
//write data to PS key
//
Result SetMeshNodeData(uint16 key, PUCHAR pnode_data, uchar size)
{
    Result ret_code=RESULT_OK;
    
    ret_code = Cmd_flash_ps_save(key,size,(const uint8*)pnode_data)->result;
    if(result) {ret_code = result; Trace1("WriteNodeData Error %x",result);}

    return ret_code;
}



//
//
//
bool EraseCalibrationData()
{TraceProc();
    bool ret_code=TRUE;
    MSC_Status_TypeDef rsp_code;
    dword ctune=0; 
    
    ctune = *((dword*)CTUNE_UD_ADDR);     //save CTUNE value
    Trace1("CTUNE = ", ctune);
    
    MSC_Init(); // Initialize the MSC for writing    
    rsp_code = MSC_ErasePage((uint32_t *) CAL_DATA_ADDR);
    if (rsp_code != mscReturnOk){ret_code = FALSE; UD_Flash_Error(rsp_code);}
    
    MSC_WriteWord((uint32_t *)CTUNE_UD_ADDR,(void *)&ctune, 4);   //restore CTUNE value
    MSC_Deinit();
    return ret_code;
}


// index = ic numder
// pBuff = point to buffer of data
// write data to UD
MSC_Status_TypeDef WriteCalibrationData(byte index,void* pBuff)
{TraceProc();
  MSC_Status_TypeDef ret_code=mscReturnOk;
  uint32_t * ptr_user_data;
  ptr_user_data = (uint32_t*)(CAL_DATA_ADDR+(index*ONE_LEVEL_BUFF_SIZE));
  
  MSC_Init(); // Initialize the MSC for writing
  
  ret_code = MSC_WriteWord((uint32_t *)ptr_user_data,pBuff, ONE_LEVEL_BUFF_SIZE);
  if(ret_code != mscReturnOk) {UD_Flash_Error(ret_code);} //MSC_Deinit();
  else { Trace("Flash Save Ok");}
  
  MSC_Deinit();



  return ret_code;
}


// index = ic numder
// pBuff = point to buffer of data
// read data from UD
int ReadCalibrationData(byte index,void* pBuff)
{TraceProc();
    int ret_code=TRUE;
    void* ptr_user_data;
    ptr_user_data = (void*)(CAL_DATA_ADDR+(index*ONE_LEVEL_BUFF_SIZE));
    memcpy(pBuff,ptr_user_data,ONE_LEVEL_BUFF_SIZE);
    return ret_code;
}




// index = ic numder
// pBuff = point to buffer of data
// write data to UD for backup
int BackupCalibrationData()
{
  int ret_code=TRUE;
  

  return ret_code;
}




/* Put the error message to the console */
void UD_Flash_Error(msc_Return_TypeDef err)
{
	switch (err)
	    {
	    case mscReturnInvalidAddr:  Trace("ERROR: INVALID ADDRESS");
	      break;
	    case mscReturnLocked:	    Trace("ERROR: USER PAGE IS LOCKED");
	      break;
	    case mscReturnTimeOut:	    Trace("ERROR: TIMEOUT OCCURED");
	      break;
	    case mscReturnUnaligned:    Trace("ERROR: UNALIGNED ACCESS");
	      break;
	    default: Trace("ERROR: UNKNOW");
	    break;
	    }
    //err = 0;
}


