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

#include "node_data.h"


extern Result result;

Node_Data MeshNodeData;                   /**< User data contents */
//const long Baudrate[]={2400,4800,9600,19200,38400,57600,115200};
//uint32 *pNodedata = (uint32*)0x0FE00000;  // pointer to the beginning of UD page
uint16 val1, val2;
word LevelData[12]={0,1,2,3,4,5,6,7,8,9,10,11};
word LevelDataRead[12];


void NodeDataInit()
{TraceProc();

    return;
#if 0  
    byte loop;

    MeshNodeData.EepromID=0xDDDD;
    MeshNodeData.StructVer=0x5678;
    MeshNodeData.BleNodeID = 12;
    MeshNodeData.NodeBehavior = 11;
    MeshNodeData.BaudRate=6; //for 115200
    MeshNodeData.Sleeping=0;
    MeshNodeData.DeviceKind=0;
    MeshNodeData.DeviceLength=24;
    MeshNodeData.Reserve[2]=0xEEEEEEEE;
    // MSC_Init(); 
    TraceDec1("NodeUserData size", NODE_DATA_SIZE);
    WriteNodeData(&MeshNodeData);
     PrintDataByte("Write User data ", (BYTE*)&MeshNodeData,NODE_DATA_SIZE);
    //PrintDataByte("Write User data ", (BYTE*)&MeshNodeData,NODE_DATA_SIZE);
    //*pNodedata=0xABCD4567;

    memset((void *) &MeshNodeData, 0xFF, sizeof(Node_Data));
    PrintDataByte("Reset 0 User data", (BYTE*)&MeshNodeData, NODE_DATA_SIZE);

    ReadNodeData(&MeshNodeData);
    PrintDataByte("Read User data ", (BYTE*)&MeshNodeData, NODE_DATA_SIZE);

///////////////////////////////////////////////////////////////////////////////////////////
    EraseCalibrationData();

    for(loop=0; loop<8; loop++)  WriteCalibrationData(loop,(void*)LevelData);

     for(loop=0; loop<8; loop++){
    ReadCalibrationData(loop,(void*)LevelDataRead); 
    PrintDataDec("Read User data ", LevelDataRead, 12);
        }
#endif     

}

// write node data to PS key
// error code:0x018A
//
int WriteNodeData(PNode_Data pNode_data)
{TraceProc();
    int ret_code=TRUE;

   result = Cmd_flash_ps_save(PS_KEY_MESH_NODE_DATA,NODE_DATA_SIZE,(const uint8*)pNode_data)->result;
   if(result) {ret_code = result; Trace1("WriteNodeData Error %x",result);}
    return ret_code;
}


// Read node data to PS key
//
//
int ReadNodeData(PNode_Data pNode_data)
{TraceProc();
  int ret_code=TRUE;
  struct gecko_msg_flash_ps_load_rsp_t* pRsp;
  pRsp = Cmd_flash_ps_load(PS_KEY_MESH_NODE_DATA);
  if(!pRsp->result){
    memcpy((void *)pNode_data, (void *)&(pRsp->value.data), pRsp->value.len);
    }
  else {ret_code = result; Trace1("ReadNodeData Error %x",result);}

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
static void UD_Flash_Error(msc_Return_TypeDef err)
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


