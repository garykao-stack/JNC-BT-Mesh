#ifndef _BaseDef_H_
#define _BaseDef_H_



#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include "native_gecko.h"
#include "bg_types.h"


#define DISABLE         0
#define ENABLE          1
#define FAIL            0
#define PASS            1
#define NO              0
#define YES             1
#define ON              1
#define OFF             0
#define WRITE           0
#define READ            1
#define HIGH            1
#define LOW             0

#define SUCCESS         0
#ifdef NULL
#undef NULL
#endif
#define NULL            0
#define NULL_0          0
#define RESULT_OK       0
#define RESULT_FALSE    0xFF


/// Data type unsigned char
typedef unsigned char   UCHAR;
/// Data type unsigned char
typedef unsigned char   BYTE;
/// Data type unsigned int
typedef unsigned int    UINT;
/// Data type unsigned int
typedef unsigned short  WORD;    // 16bit
/// Data type unsigned long
typedef unsigned long   ULONG;
/// Data type unsigned long
typedef unsigned long   DWORD;


/// Data type unsigned char
typedef unsigned char   uchar;
/// Data type unsigned char
typedef unsigned char   byte;
/// Data type unsigned int
typedef unsigned int    uint;
/// Data type unsigned int
typedef unsigned short  word;    // 16bit
/// Data type unsigned long
typedef unsigned long   ulong;
/// Data type unsigned long
typedef unsigned long   dword;



/// Data type unsigned char
typedef unsigned char*   PUCHAR;
/// Data type unsigned int
typedef unsigned int*    PUINT;
/// Data type unsigned int
typedef unsigned long*   PULONG;


/// Data type unsigned char
typedef byte*   PBYTE;
/// Data type unsigned int
typedef word*    PWORD;
/// Data type unsigned int



/// Data type unsigned char
typedef char*   PCHAR;


/// Data type unsigned int
//typedef int*    PINT;
/// Data type unsigned int
typedef long*   PLONG;

/// Data type unsigned char
typedef uint8*  PUINT8;
/// Data type unsigned int
typedef uint16* PUINT16;
/// Data type unsigned int
typedef uint32* PUINT32;


// for volatile

typedef char volatile      VOLCHAR;
typedef uchar volatile     VOLUCHAR;
typedef int16 volatile     VOLINT16;
typedef int32 volatile     VOLINT32;
typedef uint16 volatile    VOLUINT16;
typedef uint32 volatile    VOLUINT32;


typedef char volatile*      PVOLCHAR;
typedef uchar volatile*     PVOLUCHAR;
typedef int16 volatile*     PVOLINT16;
typedef int32 volatile*     PVOLINT32;
typedef uint16 volatile*    PVOLUINT16;
typedef uint32 volatile*    PVOLUINT32;



#ifdef TRUE
typedef BOOL Bool;
#else
typedef enum bool_enum
{
	FALSE, TRUE
} Bool;
#endif

#ifndef RESET
typedef enum reset_enum
{ RETAIN,   RESET 
} BReset;
#endif

typedef struct 
{   WORD u16Reg;    BYTE u8Value;
} RegUnitType;

#define bit  U8

#define BIT0    0x0001
#define BIT1    0x0002
#define BIT2    0x0004
#define BIT3    0x0008
#define BIT4    0x0010
#define BIT5    0x0020
#define BIT6    0x0040
#define BIT7    0x0080
#define BIT8    0x0100
#define BIT9    0x0200
#define BIT10   0x0400
#define BIT11   0x0800
#define BIT12   0x1000
#define BIT13   0x2000
#define BIT14   0x4000
#define BIT15   0x8000
#define BIT16   0x00010000
#define BIT17   0x00020000
#define BIT18   0x00040000
#define BIT19   0x00080000
#define BIT20   0x00100000
#define BIT21   0x00200000
#define BIT22   0x00400000
#define BIT23   0x00800000
#define BIT24   0x01000000
#define BIT25   0x02000000
#define BIT26   0x04000000
#define BIT27   0x08000000
#define BIT28   0x10000000
#define BIT29   0x20000000
#define BIT30   0x40000000
#define BIT31   0x80000000



#define _bit0_(val)                 ((bit)(val & BIT0))
#define _bit1_(val)                 ((bit)(val & BIT1))
#define _bit2_(val)                 ((bit)(val & BIT2))
#define _bit3_(val)                 ((bit)(val & BIT3))
#define _bit4_(val)                 ((bit)(val & BIT4))
#define _bit5_(val)                 ((bit)(val & BIT5))
#define _bit6_(val)                 ((bit)(val & BIT6))
#define _bit7_(val)                 ((bit)(val & BIT7))
#define _bit8_(val)                 ((bit)(val & BIT8))
#define _bit9_(val)                 ((bit)(val & BIT9))
#define _bit10_(val)                ((bit)(val & BIT10))
#define _bit11_(val)                ((bit)(val & BIT11))
#define _bit12_(val)                ((bit)(val & BIT12))
#define _bit13_(val)                ((bit)(val & BIT13))
#define _bit14_(val)                ((bit)(val & BIT14))
#define _bit15_(val)                ((bit)(val & BIT15))


//typedef U8      BOOLEAN;   ///< BOOLEAN
#define BOOL    bool

#if !_MSC_VER   // richard modify
#define HINIBBLE(value)           ((value) >> 4)
#define LONIBBLE(value)           ((value) & 0x0f)
//#define HIBYTE(value)             ((BYTE)((value) / 0x100))
//#define LOBYTE(value)             ((BYTE)(value))
#define LOBYTE(LO) (BYTE)(((LO)&0x00FF)<<8)
#define HIBYTE(HI) (BYTE)(((HI)&0xFF00) >> 8)
#define LOWORD(l) ((WORD)(l))
//#define HIWORD(l) ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define HIWORD(l) (*(((PWORD)&l)+1))


#define MAKEWORD(value1, value2)  ((((WORD)(value1)) * 0x100) + (value2))
#define MAKEDWORD(Hi_Word, Lo_Word)  ((uint32)Hi_Word<<16 | (uint32)Lo_Word)

#endif
// NOTE. these have problem with long integer (32-bit) on C51
#define MAX(a, b)        (((a) > (b)) ? (a) : (b))
#define MIN(a ,b)        (((a) < (b)) ? (a) : (b))


#define _MAX( a, b )        (((a) >= (b)) * (a) + ((b) > (a)) * (b))
#define _MIN( a, b )        (((a) <= (b)) * (a) + ((b) < (a)) * (b))

#define _CONCAT( a, b )     a##b
#define CONCAT( a, b )      _CONCAT( a, b )
#define UNUSED( var )       ((void)(var))

#define WORD_LO(LO) (((LO)&0x00FF)<<8)
#define WORD_HI(HI) (((HI)&0xFF00) >> 8)
#define WORD_SWAP(x) (WORD_LO(x)+ WORD_HI(x))

//#define LOBYTE(w) ((BYTE)(w))
//#define HIBYTE(w) ((BYTE)(((WORD)(w) >> 8) & 0xFF))

#define END_TBL     0xFF
#define END_TBL_0   (0)

// size of array size

#define sizeof_array(Arr1)   (sizeof(Arr1)/sizeof(Arr1[0]))

 
typedef int     (*PFunVoid)(void);
typedef Bool    (*PFunSendData)(PUCHAR buff,int size);
typedef uint32  (*PFunGetData)(PUCHAR buff,int size);
typedef uint32  (*PFunGetStatus)(void);
typedef void    (*PTimerTask)(void);
typedef void    (*PTimerTask16)(uint16);
typedef void    (*PTimerTask32)(uint32);



#endif  //_BaseDef_H_

