/*
 * Global.c
 *
 *  Created on: 2019/07/11
 *      Author: richard.huang
 */

//#include "native_gecko.h"
#include "Global.h"

uint32  TraceProcCount;
uint32  MeshNodeStatus;


#if(DEBUG_PORT == SWO_PORT)
    #include "retargetswo.h"
#else
    #include "retargetserial.h"
#endif  

void GlobalInitial()
{
#if(DEBUG_PORT == SWO_PORT)
    RETARGET_SwoInit(); // Richard: for USB print debug message
#else
    RETARGET_SerialInit();
#endif
}

#if(DEBUG_PORT == SWO_PORT) 
//Richard: For SWO function
int _write(int file, const char*ptr, int len)
{
   int x;
   for (x = 0; x < len; x++)   RETARGET_WriteChar(*ptr++);
  
    return (len);
}
#endif


void WordSwapBuff(PUINT16 pBuff,uchar size)
{
    uchar loop;
    for(loop=0; loop < size; loop++)
        {*pBuff = WordSwap(*pBuff); pBuff++;}
}

//
//Hi Low Byte swap
uint16 WordSwap(uint16 value)
{
    uchar hi_byte;
    hi_byte = *(((PUCHAR)&value)+1);
    value <<=8; 
    value+=(uint16)hi_byte;

    return value;
}

//
//
void DWordSwap(PUCHAR p_value)
{
    uchar value;
    PUCHAR p_temp = p_value+3;
    value = *p_value;  *p_value = *p_temp; *p_temp = value;
    p_value++; p_temp--;
    value = *p_value;  *p_value = *p_temp; *p_temp = value;
}

//
// NUM: toal Dword number
//
void DWordSwapN(PUCHAR p_value, uint16 num)
{
  uint16 loop;

  for(loop=0; loop<num; loop++)
    {
     DWordSwap(p_value);
     p_value +=4;
    }
  
}


// for float
uint32_t* f_to_u32(float *f)
{
  return (uint32_t*)f;
}
uint16_t f_to_u16L(float f)
{
  return WordSwap(*f_to_u32(&f) & 0xffff);
}
uint16_t f_to_u16H(float f)
{
  return WordSwap(*f_to_u32(&f) >> 16);
}

#define TEMO_OFFSET_VALUE   300
#define TEMO_MUL_VALUE      3


uchar WordToByte(word value)
{
    uchar ret_code=0;
    if(value % 0x8000)
        {
            ret_code = 66; //-10 degree
        }
    else
        {
            value = value/100 + TEMO_OFFSET_VALUE;           
            ret_code = ret_code/TEMO_MUL_VALUE;
        }

    return ret_code;
}

word ByteToWord(uchar value)
{
    word ret_code=0;

    return ret_code;
}



void SetStatusOn(uint32 status) 
{ MeshNodeStatus |= status;
}
void SetStatusOff(uint32 status) 
{ MeshNodeStatus &= ~status;
}
void SetMeshNodeStatus(uint32 status,uchar on_off) 
{ 
    if(on_off == ON) MeshNodeStatus |= status;
    else MeshNodeStatus &= ~status;
}


bool GetMeshNodeStatus(uint32 status)
{
    bool ret_code = FALSE;
    if(MeshNodeStatus & status) ret_code = TRUE;
  return ret_code;
}

////////////////////////////////// for Debug Function ///////////////////////////////////////////////

// Show data for debug
//
#ifdef DEBUG_PRINT  
void PrintData(PCHAR pTitle,PUINT16 pbuff, UINT len)
{
    UINT x,y;
    if(pTitle != NULL) Printf("%s ==> Length = %d\r\n",pTitle,len);
    y=0;
    while(len)
        {
            for(x=0 ; x < 8 && len > 0 ; x++)
                {
                  Printf("[%02d:%04Xh ",y,*pbuff);
                  y++;pbuff++; len--;
                }
            Printf("\r\n");
        }; 
        
}


//
//
void PrintDataLen(PCHAR pTitle,PUINT16 pbuff,uint16 size, uchar len)
{    
    uint16 loop,y;
    if(pTitle) Trace(pTitle);

    y=0;
    while(size)
        {
            
            for(loop=0; loop < len && size > 0; loop++)
                {Printf("[%02d:%04Xh ",y++,*pbuff++); size--;}
            Printf("\r\n");
        };
}

//
void PrintDataLenDec(PCHAR pTitle,PUINT16 pbuff,uint16 size, uchar len)
{    
    uint16 loop,y;
    if(pTitle) Trace(pTitle);

    y=0;
    while(size)
        {
            
            for(loop=0; loop < len && size > 0; loop++)
                {Printf("[%02d:%04d ",y++,*pbuff++); size--;}
            Printf("\r\n");
        };
}



void PrintDataByte(char *pTitle,BYTE* pbuff, UINT len)
{
    UINT x,y;
        
    if(pTitle != NULL) Printf("%s ==> Length = %d\r\n",pTitle,len);
    y=0;
    while(len)
        {
            for(x=0 ; x < 8 && len > 0 ; x++)
                {
                  Printf("[%02d:%02Xh ",y,*pbuff);
                  y++;pbuff++; len--;
                }
            Printf("\r\n");
        };         
}

// Show data for debug
//
void PrintDataDec(char *pTitle,WORD* pbuff, UINT len)
{
    UINT x,y;
        
    if(pTitle != NULL) Printf("%s ==> Length = %d\r\n",pTitle,len);
    y=0;
    while(len)
        {
            for(x=0 ; x < 8 && len > 0 ; x++)
                {
                  Printf("[%02d:%04d ",y,*pbuff);
                  y++;pbuff++; len--;
                }
            Printf("\r\n");
        }; 
        
}



void PrintDataType(PCHAR pString, PUCHAR pBuff, int size, uchar type)
{
    int loop;
        
    if (pString != NULL) Printf("%s: ", pString);
    TraceDec1("Print DataType Size",size);
    switch (type)
    {
        case PRINT_TYPE_HEX:
            for(loop = 0; loop < size; loop++) 
            {  Printf("%02Xh ", *pBuff++);  if((loop&0x0F) == 0x0F) {Printf("\r\n");}  }
            Printf("\r\n");
            break;
        case PRINT_TYPE_ASCII:
            for (loop = 0; loop < size; loop++) Printf("%c", *pBuff++);
            Printf("\r\n");
            break;
        case PRINT_TYPE_HEX_ASCII:
            for(loop = 0; loop < size; loop++) 
            {  Printf("%02Xh ", *pBuff++);  if((loop&0x0F) == 0x0F) {Printf("\r\n");}  }
            Printf("\r\n");
            pBuff = pBuff - size;
            for (loop = 0; loop < size; loop++) Printf("%c", *pBuff++);
            Printf("\r\n");
            break;
        default: break;
    };
    
}

/*
void PrintData1(PUCHAR pBuff, uchar size)
{
    uchar loop;
    if(pBuff == NULL) return;
    TraceDec1("Print Data Size",size);
    for(loop = 0; loop < size; loop++) 
    {  Printf("%02Xh ", *pBuff++);  if((loop&0x0F) == 0x0F) {Printf("\r\n");}  }
    Printf("\r\n");
}
*/
void PrintArray8(uint8array *pArrayBuff,int type)
{
    uint8 loop;
    uint size;
    uint8* pBuff;
        
    if(pArrayBuff->len == 0 | pArrayBuff->data == NULL) return;
    
    size = pArrayBuff->len; 
    pBuff = (pArrayBuff->data);
    TraceDec1("Print Data Size",size);
    switch (type)
    {
        case PRINT_TYPE_HEX:
            for(loop = 0; loop < size; loop++) 
            {  Printf("%02Xh ", *pBuff++);  if((loop&0x0F) == 0x0F) {Printf("\r\n");}  }
            Printf("\r\n");
            break;
        case PRINT_TYPE_ASCII:
            for (loop = 0; loop < size; loop++) Printf("%c", *pBuff++);
            Printf("\r\n");
            break;
        case PRINT_TYPE_HEX_ASCII:
            for(loop = 0; loop < size; loop++) 
            {  Printf("%02Xh ", *pBuff++);  if((loop&0x0F) == 0x0F) {Printf("\r\n");}  }
            Printf("\r\n");
            pBuff = pBuff - size;
            for (loop = 0; loop < size; loop++) Printf("%c", *pBuff++);
            Printf("\r\n");
            break;
        default: break;
    };

}

uint16 ShowResult(char* pString, uint16 result )
{
    //char ret_code[21];
    
    if(result) 
    {// return error
        Printf("\r\nShowResult X-X-X-X ERROR %s = 0x%04X \r\n",pString, result);
    }
    else
    {
        //Printf("\r\nO-O-O-O OK %s = 0x%04X \r\n",pString, result);
    }
    
    return result;
}
#endif 


//
//
//
uint16 TwoValueDiff(uint16 value_a, uint16 value_b)
{
    int ret_code;
    if(value_a >= value_b) ret_code = value_a - value_b;
    else ret_code = value_b - value_a;

    return ret_code;
}



void Delay_ms(int ms)
{
	int loop;
	for(loop=0; loop<ms; loop++) UDELAY_Delay(990);
}

//debug interface enable/disable
void JtagStatus(uchar status)
{
    if(status == ON)
        {GPIO_DbgSWDClkEnable(true); GPIO_DbgSWDIOEnable(true) ;}
    else
        {GPIO_DbgSWDClkEnable(false); GPIO_DbgSWDIOEnable(false) ;}
}



