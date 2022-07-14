#ifndef __DEBUG_PRINT_H
#define __DEBUG_PRINT_H
/*#undef TraceNum(str,num)
#undef GetMacroValue(var)
#undef TraceProc()
#undef TraceStep(str)
#undef Trace(str)
#undef TraceStr(str1,str2)
#undef TraceOk(str)
#undef TraceOk1(str,v1)
#undef TraceErr(str)
#undef TraceErr1(str,v1)
#undef TraceErr2(str,v1,v2)
#undef Trace1(str,v1)
#undef Trace2(str,v1,v2)
#undef Trace3(str,v1,v2,v3)
#undef TraceDec1(str,v1)
#undef TraceDec2(str,v1,v2)
#undef TraceDec3(str,v1,v2,v3)
#undef TraceDec4(str,v1,v2,v3,v4)
#undef Trace32_1(v1)
#undef Trace32_2(v1,v2)
#undef Trace32_3(v1,v2,v3)
#undef Trace32_4(v1,v2,v3,v4)
#undef Trace16_1(v1)
#undef Trace16_2(v1,v2)
#undef Trace16_3(v1,v2,v3)
#undef Trace16_4(v1,v2,v3,v4)
#undef Trace8_1(v1)
#undef Trace8_2(v1,v2)
#undef Trace8_3(v1,v2,v3)
#undef Trace8_4(v1,v2,v3,v4)
#undef Trace16Ptr_1(ptr,v1)
#undef Trace16Ptr_2(ptr,v1,v2)
#undef Trace16Ptr_3(ptr,v1,v2,v3)
#undef Trace16Ptr_4(ptr,v1,v2,v3,v4)
#undef Trace32Ptr_1(ptr,v1)
#undef Trace32Ptr_2(ptr,v1,v2)
#undef Trace32Ptr_3(ptr,v1,v2,v3)
#undef Trace32Ptr_4(ptr,v1,v2,v3,v4)*/
#undef TraceNum
#undef GetMacroValue
#undef TraceProc
#undef TraceStep
#undef Trace
#undef TraceStr
#undef TraceOk
#undef TraceOk1
#undef TraceErr
#undef TraceErr1
#undef TraceErr2
#undef Trace1
#undef Trace2
#undef Trace3
#undef TraceDec1
#undef TraceDec2
#undef TraceDec3
#undef TraceDec4
#undef Trace32_1
#undef Trace32_2
#undef Trace32_3
#undef Trace32_4
#undef Trace16_1
#undef Trace16_2
#undef Trace16_3
#undef Trace16_4
#undef Trace8_1
#undef Trace8_2
#undef Trace8_3
#undef Trace8_4
#undef Trace16Ptr_1
#undef Trace16Ptr_2
#undef Trace16Ptr_3
#undef Trace16Ptr_4
#undef Trace32Ptr_1
#undef Trace32Ptr_2
#undef Trace32Ptr_3
#undef Trace32Ptr_4
#ifndef DEBUG_FUNCTION
#define DEBUG_FUNCTION Printf
#endif
#endif

#define TraceNum(str,num) DEBUG_FUNCTION("%s => %d\r\n",str,num)

#define GetMacroValue(var)  \
        DEBUG_FUNCTION("#define %s_Hex 0x%08lX\r\n",#var,(uint32)var)
#define TraceProc() \
        DEBUG_FUNCTION("\r\n //***   %s %ld   ***//\r\n",__func__,TraceProcCount++)
#define TraceStep(str) \
        DEBUG_FUNCTION("-- %s\r\n",str)

#define Trace(str) \
        DEBUG_FUNCTION("%s\r\n",str)
#define TraceStr(str1,str2) \
        DEBUG_FUNCTION("%s = %s\r\n",str1,str2)
#define TraceOk(str)    DEBUG_FUNCTION("\r\nOK! ** %s **\r\n",str)
#define TraceOk1(str,v1)    DEBUG_FUNCTION("\r\nOK!** %s Vaule=%Xh **\r\n",str,v1)


#define TraceErr(str) \
        DEBUG_FUNCTION("\r\n\r\nError! ==**** %s ****==\r\n\r\n",str)
#define TraceErr1(str,v1) \
        DEBUG_FUNCTION("\r\n\r\nError! ==**** %s Code= %Xh ****==\r\n\r\n",str,v1)
#define TraceErr2(str,v1,v2) \
        DEBUG_FUNCTION("\r\n\r\nError! ==**** %s Code1=%Xh Code2=%Xh ****==\r\n\r\n",str,v1,v2)

#define Trace1(str,v1) \
        DEBUG_FUNCTION("%s = 0x%08lX\r\n",str,(uint32)v1)
#define Trace2(str,v1,v2) \
        DEBUG_FUNCTION("%s = 0x%08lX %s=0x%08lX\r\n",str,(uint32)v1,#v2,(uint32)v2)
#define Trace3(str,v1,v2,v3) \
        DEBUG_FUNCTION("%s: %s = 0x%08lX %s=0x%08lX %s=0x%08lX\r\n",str,#v1,(uint32)v1,#v2,(uint32)v2,#v3,(uint32)v3)

#define TraceDec1(str,v1) \
        DEBUG_FUNCTION("%s = %ld\r\n",str,(uint32)v1)
#define TraceDec2(str,v1,v2) \
        DEBUG_FUNCTION("%s %s= %ld %s=%ld\r\n",str,#v1,(uint32)v1,#v2,(uint32)v2)
#define TraceDec3(str,v1,v2,v3) \
        DEBUG_FUNCTION("%s: %s = %ld %s=%ld %s=%ld\r\n",str,#v1,(uint32)v1,#v2,(uint32)v2,#v3,(uint32)v3)
#define TraceDec4(str,v1,v2,v3,v4) \
        DEBUG_FUNCTION("%s: %s = %ld %s=%ld %s=%ld %s=%ld\r\n",str,#v1,(uint32)v1,#v2,(uint32)v2,#v3,(uint32)v3,#v4,(uint32)v4)


#define Trace32_1(v1) \
        DEBUG_FUNCTION("=> %s = 0x%08lX \r\n",#v1,(uint32)v1);
#define Trace32_2(v1,v2) \
        DEBUG_FUNCTION("=> %s = 0x%08lX %s=0x%08lX\r\n",#v1,(uint32)v1,#v2,(uint32)v2)
#define Trace32_3(v1,v2,v3) \
        DEBUG_FUNCTION("=> %s = 0x%08lX %s=0x%08lX %s=0x%08lX\r\n",#v1,(uint32)v1,#v2,(uint32)v2,#v3,(uint32)v3)
#define Trace32_4(v1,v2,v3,v4) \
        DEBUG_FUNCTION("=> %s = 0x%04X %s=0x%04X \r\n%s=0x%04X %s=0x%04X\r\n",#v1,(uint16)v1,#v2,(uint16)v2,#v3,(uint16)v3,#v4,(uint16)v4)


#define Trace16_1(v1) \
        DEBUG_FUNCTION("=> %s = 0x%04X \r\n",#v1,(uint16)v1);
#define Trace16_2(v1,v2) \
        DEBUG_FUNCTION("=> %s = 0x%04X %s=0x%04X\r\n",#v1,(uint16)v1,#v2,(uint16)v2)
#define Trace16_3(v1,v2,v3) \
        DEBUG_FUNCTION("=> %s = 0x%04X %s=0x%04X %s=0x%04X\r\n",#v1,(uint16)v1,#v2,(uint16)v2,#v3,(uint16)v3)
#define Trace16_4(v1,v2,v3,v4) \
        DEBUG_FUNCTION("=> %s = 0x%04X %s=0x%04X \r\n %s=0x%04X %s=0x%04X\r\n",#v1,(uint16)v1,#v2,(uint16)v2,#v3,(uint16)v3,#v4,(uint16)v4)



#define Trace8_1(v1) \
        DEBUG_FUNCTION("=> %s = 0x%02X \r\n",#v1,(uint8)v1);
#define Trace8_2(v1,v2) \
        DEBUG_FUNCTION("=> %s = 0x%02X %s=0x%02X\r\n",#v1,(uint8)v1,#v2,(uint8)v2)
#define Trace8_3(v1,v2,v3) \
        DEBUG_FUNCTION("=> %s = 0x%02X %s=0x%02X %s=0x%02X\r\n",#v1,(uint8)v1,#v2,(uint8)v2,#v3,(uint8)v3)
#define Trace8_4(v1,v2,v3,v4) \
        DEBUG_FUNCTION("=> %s = 0x%02X %s=0x%02X \r\n%s=0x%02X %s=0x%02X\r\n",#v1,(uint8)v1,#v2,(uint8)v2,#v3,(uint8)v3,#v4,(uint8)v4)

#define Trace16Ptr_1(ptr,v1) \
        DEBUG_FUNCTION("%s=%04Xh \r\n",#v1,ptr->v1)
#define Trace16Ptr_2(ptr,v1,v2) \
        DEBUG_FUNCTION("%s=%04Xh %s=%04Xh \r\n",#v1,ptr->v1,#v2,ptr->v2)
#define Trace16Ptr_3(ptr,v1,v2,v3) \
        DEBUG_FUNCTION("%s=%04Xh %s=%04Xh %s=%04Xh \r\n",#v1,ptr->v1,#v2,ptr->v2,#v3,ptr->v3)
#define Trace16Ptr_4(ptr,v1,v2,v3,v4) \
                DEBUG_FUNCTION("%s=%04Xh %s=%04Xh \r\n %s=%04Xh %s=%04Xh \r\n",#v1,ptr->v1,#v2,ptr->v2,#v3,ptr->v3,#v4,ptr->v4)

#define Trace32Ptr_1(ptr,v1) \
        DEBUG_FUNCTION("%s=%08lXh \r\n",#v1,(uint32)ptr->v1)
#define Trace32Ptr_2(ptr,v1,v2) \
        DEBUG_FUNCTION("%s=%08lXh %s=%08lXh \r\n",#v1,(uint32)ptr->v1,#v2,(uint32)ptr->v2)
#define Trace32Ptr_3(ptr,v1,v2,v3) \
        DEBUG_FUNCTION("%s=%08lXh %s=%08lXh %s=%08lXh \r\n",#v1,(uint32)ptr->v1,#v2,(uint32)ptr->v2,#v3,(uint32)ptr->v3)
#define Trace32Ptr_4(ptr,v1,v2,v3,v4) \
        DEBUG_FUNCTION("%s=%08lXh %s=%08lXh \r\n %s=%08lXh %s=%08lXh \r\n",#v1,(uint32)ptr->v1,#v2,(uint32)ptr->v2,#v3,(uint32)ptr->v3,#v4,(uint32)ptr->v4)


//#undef DEBUG_FUNCTION
