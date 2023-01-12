#include "global.h"
#include "em_device.h"
#include "em_chip.h"
#include "em_emu.h"
#include "em_msc.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "leds.h"
#include "sensor_client.h"
#include "bus_usart.h"
#include "Mesh_Node.h"
#include "node_data.h"



_Mesh_Node_Data MeshNodeData;                   /**< User data contents */
_PMesh_Node_Data pMeshNodeData;

_AdjustValue    AdjustValue;
_PAdjustValue   pAdjValue;

void NodeDataInit()
{//TraceProc();
    pMeshNodeData = &MeshNodeData;
    pAdjValue = &AdjustValue;   
    ReadNodeData();
    if(pMeshNodeData->DataInitID != NODE_DATA_ID) MeshNodeDataReset();
    //MeshNodeDataReset(); while(1);
    pMeshNodeData->SegPPercent[0]=0;   
    pMeshNodeData->SegPPercent[1]=36;   
    pMeshNodeData->SegPPercent[2]=45;  
    pMeshNodeData->SegPPercent[3]=52;
    pMeshNodeData->SegPPercent[4]=76;
    return;
}



void MeshNodeDataReset()
{//TraceProc();
    float temp_gain,hum_gain,temp_offset,hum_offset;

    temp_gain = pAdjValue->TempGain; hum_gain = pAdjValue->HumGain;
    temp_offset = pAdjValue->TempOffset;hum_offset = pAdjValue->HumOffset;
    if(temp_gain == 0) temp_gain = 1.0;
    if(hum_gain == 0) hum_gain = 1.0;
    
    memset(&MeshNodeData,0,NODE_DATA_SIZE);
    memset(&AdjustValue,0,ADJUST_VALUE_SIZE);
    pMeshNodeData->DataInitID=NODE_DATA_ID;    
    pMeshNodeData->StructVer=FW_VER;
    pMeshNodeData->MeshNodeID = 0;
    pMeshNodeData->MeshNodeRole = NR_DEFAULT;
    pMeshNodeData->SensorClass = SENSOR_AUTO_SCAN; 
    pMeshNodeData->BaudRate = USART_BAUDRATE_DEFAULT; //for 9600
    pMeshNodeData->TxPower = TX_POWER_HI;
    pMeshNodeData->SleepingTimer=TIMER_NODE_SLEEPING;
    pMeshNodeData->WorkingTimer = TIMER_DEFAULT_WORKING;
    pMeshNodeData->RebootForRs485IdelSecnods=0;
    pAdjValue->TempGain = temp_gain;
    pAdjValue->HumGain = hum_gain;
    pAdjValue->TempOffset = temp_offset;
    pAdjValue->HumOffset = hum_offset;    
    pAdjValue->RS485TransmitterData.Rs485ClientBuffTimeoutMs=300;
    pAdjValue->RS485TransmitterData.Rs485ServerDelayBeforeSleep=10;
    // Printf("TempGain=%f hum_gain=%f temp_offset=%f hum_offset=%f ",temp_gain,hum_gain,temp_offset,hum_offset);
    BtmG6Reset();
    WriteNodeData();
   
}


//
//
//
void MeshNodeSetupReset()
{
    pMeshNodeData->SensorClass = SENSOR_AUTO_SCAN; 
    pMeshNodeData->BaudRate = USART_BAUDRATE_DEFAULT; //for 9600
    pMeshNodeData->WorkingTimer = TIMER_DEFAULT_WORKING;
    pMeshNodeData->RebootForRs485IdelSecnods=0;
    pAdjValue->TempGain = 1.0;
    pAdjValue->TempOffset = 0.0;
    pAdjValue->HumGain = 1.0;
    pAdjValue->HumOffset = 0.0;    
    pAdjValue->RS485TransmitterData.Rs485ClientBuffTimeoutMs=300;
    pAdjValue->RS485TransmitterData.Rs485ServerDelayBeforeSleep=10;
    WriteNodeData();
   
}



void BtmG6Reset()
{
    uint8 loop;
    memset(&(pAdjValue->G6Schedule),0,sizeof(pAdjValue->G6Schedule));
    for(loop=0; loop<G6_SCHEDULE_NUM; loop++){
        pAdjValue->G6Schedule[loop].ScheduleID=loop;
        }
     pMeshNodeData->FilterTime1=1500; //Hour
     pMeshNodeData->FilterTime2=3000;
     pMeshNodeData->FilterAllTime1=0;
     pMeshNodeData->FilterAllTime2=0;    
     pMeshNodeData->G6Status = 0;
     pMeshNodeData->G6ActPercent = 0;
     pMeshNodeData->SegPPercent[0]=0;    //for 0%
     pMeshNodeData->SegPPercent[1]=25;   //for 25%
     pMeshNodeData->SegPPercent[2]=50;   //for 50%
     pMeshNodeData->SegPPercent[3]=75;
     pMeshNodeData->SegPPercent[4]=100;
     pMeshNodeData->SegPPercent[5]=pMeshNodeData->SegPPercent[0];
    WriteNodeData();
}






#define WRITE_MESH_NODE_ALL         0
#define WRITE_MESH_NODE_DATA        1
#define WRITE_MESH_ADJ_VALUE        2

//
Result WriteNodeData()
{
    Result ret_code=RESULT_OK;

   ret_code = Cmd_flash_ps_save(PS_KEY_MESH_NODE_DATA,NODE_DATA_SIZE,(const uint8*)pMeshNodeData)->result;
   Delay_ms(10);

   ret_code = Cmd_flash_ps_save(PS_KEY_ADJUST_VALUE,ADJUST_VALUE_SIZE,(const uint8*)pAdjValue)->result;
   Delay_ms(10);

   
    return ret_code;
}

Result WriteMeshNodeData()
{
     Result ret_code=RESULT_OK;
    
    ret_code = Cmd_flash_ps_save(PS_KEY_MESH_NODE_DATA,NODE_DATA_SIZE,(const uint8*)pMeshNodeData)->result;
    Delay_ms(10);
    return ret_code;

}

Result WriteAdjValue()
{
    Result ret_code=RESULT_OK;

    ret_code = Cmd_flash_ps_save(PS_KEY_ADJUST_VALUE,ADJUST_VALUE_SIZE,(const uint8*)pAdjValue)->result;
    Delay_ms(10);
    return ret_code;


}



// Read node data to PS key
//
//
Result ReadNodeData()
{
  Result ret_code=RESULT_OK;
  uint8  len;

  struct gecko_msg_flash_ps_load_rsp_t* pRsp;
  pRsp = Cmd_flash_ps_load(PS_KEY_MESH_NODE_DATA);
  if(pRsp->result == RESULT_OK){
    if(pRsp->value.len > sizeof(_Mesh_Node_Data))
    	len = sizeof(_Mesh_Node_Data);
    else
    	len = pRsp->value.len;
    memcpy((void *)pMeshNodeData, (void *)&(pRsp->value.data),len);
  }else {
	  ret_code = pRsp->result;
  }


  pRsp = Cmd_flash_ps_load(PS_KEY_ADJUST_VALUE);
  if(pRsp->result == RESULT_OK) {
     if(pRsp->value.len > sizeof(_AdjustValue) )
    	 len = sizeof(_AdjustValue);
     else
    	 len = pRsp->value.len;
     memcpy((void *)pAdjValue, (void *)&(pRsp->value.data), len);
  }else {
	  ret_code = pRsp->result; Trace1("Read Adjust Value Error %x",pRsp->result);
  }


  return ret_code;
}





//
//write calibration dat to PS
//
Result WriteWlCalData()
{
    Result ret_code=RESULT_OK;
    
    return ret_code;
}


// Read calibration data from PS
//
//
Result ReadWlCalData()
{
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
{
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
{
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
{
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


