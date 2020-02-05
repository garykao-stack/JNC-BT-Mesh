/*
 * bus.h
 *
 *  Created on: 2019/10/14
 *      Author: Richard
 */

#ifndef _AD7147_
#define _AD7147_

/*BANK 1 REGISTERS*/
#define PWR_CONTROL    0x000
#define STAGE_CAL_EN   0x001
#define AMB_COMP_CTRL0 0x002
#define AMB_COMP_CTRL1 0x003
#define AMB_COMP_CTRL2 0x004
#define STAGE_LOW_INT_ENABLE 0x005
#define STAGE_HIGH_INT_ENABLE 0x006
#define STAGE_COMPLETE_INT_ENABLE 0x007
#define STAGE_COMPLETE_INT_STATUS 0x00A
#define CDC_RESULT_S0 0x00B
#define CDC_RESULT_S1 0x00C
#define CDC_RESULT_S2 0x00D
#define CDC_RESULT_S3 0x00E
#define CDC_RESULT_S4 0x00F
#define CDC_RESULT_S5 0x010
#define CDC_RESULT_S6 0x011
#define CDC_RESULT_S7 0x012
#define CDC_RESULT_S8 0x013
#define CDC_RESULT_S9 0x014
#define CDC_RESULT_S10 0x015
#define CDC_RESULT_S11 0x016

/*
=================================================================
 BANK 1 REGISTERS Setup
=================================================================
*/
/*ADDR_PWR_CONTROL(0x000)
POWER MODE - 電源 */
#define PWCTL_FULL_POWER_MODE 0x00
#define PWCTL_FULL_SHOUTDOWN_MODE 0x01
#define PWCTL_LOW_POWER_MODE 0x02


/* ADDR_PWR_CONTROL(0x000)
LP_CONV_DELAY 省電採樣延遲 */
#define PWCTL_LP_CONV_DELAY_200MS 0x00
#define PWCTL_LP_CONV_DELAY_400MS (0x01<<2)
#define PWCTL_LP_CONV_DELAY_600MS (0x02<<2)
#define PWCTL_LP_CONV_DELAY_800MS (0x03<<2)

/* ADDR_PWR_CONTROL(0x000)
SEQUENCE_STATE_NUM 連線採樣數 */
#define PWCTL_SQUENCE_STATE_1 (0<<4)
#define PWCTL_SQUENCE_STATE_2 (1<<4)
#define PWCTL_SQUENCE_STATE_3 (2<<4)
#define PWCTL_SQUENCE_STATE_4 (3<<4)
#define PWCTL_SQUENCE_STATE_5 (4<<4) 
#define PWCTL_SQUENCE_STATE_6 (5<<4)
#define PWCTL_SQUENCE_STATE_7 (6<<4)
#define PWCTL_SQUENCE_STATE_8 (7<<4)
#define PWCTL_SQUENCE_STATE_9 (8<<4)
#define PWCTL_SQUENCE_STATE_10 (9<<4)
#define PWCTL_SQUENCE_STATE_11 (10<<4)
#define PWCTL_SQUENCE_STATE_12 (11<<4)

/* ADDR_PWR_CONTROL(0x000)*/
#define PWCTL_DECIMATION_256 (0x00<<8)
#define PWCTL_DECIMATION_128 (0x01<<8)
#define PWCTL_DECIMATION_64 (0x02<<8)
/*#define PWCTL_DECIMATION_64_1 (0x03<<8)*/

// ADDR_PWR_CONTROL(0x000)
#define PWCTL_SW_RESET (0x01<<10)

// ADDR_PWR_CONTROL(0x000) INT_POL 
#define PWCTL_INT_POOL_LOW 0
#define PWCTL_INT_POOL_HIGH (1<<11)

/* ADDR_PWR_CONTROL(0x000) EXT_SOURCE */
#define PWCTL_EXT_SOURCE_ENABLE 0
#define PWCTL_EXT_SOURCE_DISABLE (1<<12)

/* ADDR_PWR_CONTROL(0x000) CDC_BIAS */
#define PWCTL_BIAS_0 0
#define PWCTL_BIAS_20 (1<<14)
#define PWCTL_BIAS_35 (2<<14)
#define PWCTL_BIAS_50 (3<<14)

/* STAGE_CAL_EN(0x001) STAGE_CAL_EN enable calibration */
#define STAGE0_CAL_EN 0x0001
#define STAGE1_CAL_EN 0x0002
#define STAGE2_CAL_EN 0x0004
#define STAGE3_CAL_EN 0x0008
#define STAGE4_CAL_EN 0x0010
#define STAGE5_CAL_EN 0x0020
#define STAGE6_CAL_EN 0x0040
#define STAGE7_CAL_EN 0x0080
#define STAGE8_CAL_EN 0x0100
#define STAGE9_CAL_EN 0x0200
#define STAGE10_CAL_EN 0x0400
#define STAGE11_CAL_EN 0x0800

// STAGE_CAL_EN(0x001) Full Power mode skip control 平均筆數(應該是)-全載模式 
#define AVG_FP_SKIP_3 (0x00<<12)
#define AVG_FP_SKIP_7 (0x01<<12)
#define AVG_FP_SKIP_15 (0x02<<12)
#define AVG_FP_SKIP_31 (0x03<<12)

// STAGE_CAL_EN(0x001) Low Power mode skip control 平均筆數(也許設)-低功耗模式 
#define AVG_LP_SKIP_ALL (0x00<<14)
#define AVG_LP_SKIP_1 (0x01<<14)
#define AVG_LP_SKIP_2 (0x02<<14)
#define AVG_LP_SKIP_3 (0x03<<14)

// AMB_COMP_CTRL0(0x002) FF_SKIP_CNT(頻道數) 快速過濾控制
#define FF_SKIP_CNT(cnt) (cnt+1)

// AMB_COMP_CTRL0(0x002) FP_PROXIMITY_CNT(?) 偵錯到接近物體時停止校正功能的時間-全載模式
#define FP_PROXIMITY_CNT(cnt) (cnt<<4)

// AMB_COMP_CTRL0(0x002) LP_PROXIMITY_CNT(?) 偵錯到接近物體時停止校正功能的時間-低功模式
#define LP_PROXIMITY_CNT(cnt) (cnt<<8)

/* AMB_COMP_CTRL0(0x002)
PWR_DOWN_TIMEOUT
Full power to low power mode timeout control
00 = 1.25 × (FP_PROXIMITY_CNT)
01 = 1.50 × (FP_PROXIMITY_CNT)
10 = 1.75 × (FP_PROXIMITY_CNT)
11 = 2.00 × (FP_PROXIMITY_CNT)*/
#define AMBCTL0_PWR_DOWN_TIMEOUT_1_25 (0x00<<12)
#define AMBCTL0_PWR_DOWN_TIMEOUT_1_50 (0x01<<12)
#define AMBCTL0_PWR_DOWN_TIMEOUT_1_75 (0x02<<12)
#define AMBCTL0_PWR_DOWN_TIMEOUT_2_00 (0x03<<12)

/* AMB_COMP_CTRL0(0x002)FORCED_CAL 強制校正 */
#define AMBCTL0_FORCED_CAL (1<<14)

/* AMB_COMP_CTRL0(0x002) CONV_RESET 重置校正佇列到STAGE0 */
#define AMBCTL0_CONV_RESET (1<<15)

/* AMB_COMP_CTRL1(0x003) PROXIMITY_RECAL_LVL 重新校正等級*/
#define AMBCTL1_PROXIMITY_RECAL_LVL(lvl) (lvl&0xff)

/* AMB_COMP_CTRL1(0x003) PROXIMITY_DETECTION_RAGE 接近偵測率(*16)*/
#define AMBCTL1_PROXIMITY_DETECTION_RAGE(rate) ((rate&0x3f)<<8)

/* AMB_COMP_CTRL1(0x003) SLOW_FILTER_UPDATE_LVL 低速過濾更新等級(看不懂)*/
#define AMBCTL1_SLOW_FILTER_UPDATE_LVL(lvl) ((lvl&0x03)<<14)

/* AMB_COMP_CTRL2(0x004) FP_PROXIMITY_RECAL 
Full power mode proximity recalibration time control(全載模式接近偵錯重新校正時間?)
n: 預設&最大:0x3ff*/
#define AMBCTL2_FP_PROXIMITY_RECAL(n) (n&0x3ff)


/* AMB_COMP_CTRL2(0x004)
LP_PROXIMITY_RECAL
Low power mode proximity recalibration time control(低功耗模式接近偵錯重新校正時間?)
n: 預設&最大:0x3f*/
#define AMBCTL2_LP_PROXIMITY_RECAL(n) ((n&0x3f)<<10)

/* STAGE_LOW_INT_ENABLE(0x005)
STAGEx_LOW_INT_ENABLE 閾值超限中斷 */
#define STAGE0_LOW_INT_ENABLE 0x0001
#define STAGE1_LOW_INT_ENABLE 0x0002
#define STAGE2_LOW_INT_ENABLE 0x0004
#define STAGE3_LOW_INT_ENABLE 0x0008
#define STAGE4_LOW_INT_ENABLE 0x0010
#define STAGE5_LOW_INT_ENABLE 0x0020
#define STAGE6_LOW_INT_ENABLE 0x0040
#define STAGE7_LOW_INT_ENABLE 0x0080
#define STAGE8_LOW_INT_ENABLE 0x0100
#define STAGE9_LOW_INT_ENABLE 0x0200
#define STAGE10_LOW_INT_ENABLE 0x0400
#define STAGE11_LOW_INT_ENABLE 0x0800

/* STAGE_LOW_INT_ENABLE(0x005) GPIO_SETUP 閾值超限中斷 */
#define GPIO_SETUP_DISABLE 0x00
#define GPIO_SETUP_AS_INPUT (0x01<<12)
#define GPIO_SETUP_ACT_LO_OUTPUT (0x02<<12)
#define GPIO_SETUP_ACT_HI_OUTPUT (0x03<<12)

/* STAGE_LOW_INT_ENABLE(0x005) GPIO_INPUT_CONFIG 輸入觸發設定*/
#define GPIO_INPUT_NEG_LVL (0x00<<14)
#define GPIO_INPUT_POS_EDG (0x01<<14)
#define GPIO_INPUT_NEG_EDG (0x02<<14)
#define GPIO_INPUT_POS_LVL (0x03<<14)


/* STAGE_HIGH_INT_ENABLE(0x006) STAGEx_HIGH_INT_ENABLE 閾值超限中斷-下限 */
#define STAGE0_HIGH_INT_ENABLE 0x0001
#define STAGE1_HIGH_INT_ENABLE 0x0002
#define STAGE2_HIGH_INT_ENABLE 0x0004
#define STAGE3_HIGH_INT_ENABLE 0x0008
#define STAGE4_HIGH_INT_ENABLE 0x0010
#define STAGE5_HIGH_INT_ENABLE 0x0020
#define STAGE6_HIGH_INT_ENABLE 0x0040
#define STAGE7_HIGH_INT_ENABLE 0x0080
#define STAGE8_HIGH_INT_ENABLE 0x0100
#define STAGE9_HIGH_INT_ENABLE 0x0200
#define STAGE10_HIGH_INT_ENABLE 0x0400
#define STAGE11_HIGH_INT_ENABLE 0x0800

/* STAGE_COMPLETE_INT_ENABLE(0x007) STAGEx_COMPLETE_INT_ENABLE 採樣完成中斷 */
#define STAGE0_COMPLETE_INT_ENABLE 0x0001
#define STAGE1_COMPLETE_INT_ENABLE 0x0002
#define STAGE2_COMPLETE_INT_ENABLE 0x0004
#define STAGE3_COMPLETE_INT_ENABLE 0x0008
#define STAGE4_COMPLETE_INT_ENABLE 0x0010
#define STAGE5_COMPLETE_INT_ENABLE 0x0020
#define STAGE6_COMPLETE_INT_ENABLE 0x0040
#define STAGE7_COMPLETE_INT_ENABLE 0x0080
#define STAGE8_COMPLETE_INT_ENABLE 0x0100
#define STAGE9_COMPLETE_INT_ENABLE 0x0200
#define STAGE10_COMPLETE_INT_ENABLE 0x0400
#define STAGE11_COMPLETE_INT_ENABLE 0x0800

/* STAGE_COMPLETE_INT_ENABLE(0x007) GPIO_INT_ENABLE 啟用GPIO中斷*/
#define GPIO_INT_ENABLE (1<<12)

/*
=================================================================
                           Bank2 (取樣設定)
=================================================================
*/

/*STAGE起始位置，共12組*/
#define STAGE0_BASE 0x080
#define STAGE1_BASE 0x088
#define STAGE2_BASE 0x090
#define STAGE3_BASE 0x098
#define STAGE4_BASE 0x0A0
#define STAGE5_BASE 0x0A8
#define STAGE6_BASE 0x0B0
#define STAGE7_BASE 0x0B8
#define STAGE8_BASE 0x0C0
#define STAGE9_BASE 0x0C8
#define STAGE10_BASE 0x0D0
#define STAGE11_BASE 0x0D8


/*STAGE暫存器相對位移*/
#define STAGE_CONNECTION1 0x00
#define STAGE_CONNECTION2 0x01
#define STAGE_AFE_OFFSET  0x02
#define STAGE_SENSITIVITY 0x03
#define STAGE_OFFSET_LOW 0x04
#define STAGE_OFFSET_HIGH 0x05
#define STAGE_OFFSET_HIGH_CLAMP 0x06
#define STAGE_OFFSET_LOW_CLAMP 0x07

/* 使用方式
範例：
  在第0組的STAGE_CONNECTION1位址寫入0xffff
  > WriteRegister(STAGE0_BASE+STAGE_CONNECTION1,0xffff)
*/

/* 快速設定(使用變數定義)
STAGE_ADDR(n,func) 取得STAGE參數位置
  n   ：第(n)組STAGE
  func :相對功能位址

範例：
  在第0組的STAGE_CONNECTION1位址寫入0xffff
  > WriteRegister(STAGE_ADDR(0,STAGE_CONNECTION1),0xffff)
*/
#define STAGE_ADDR(n,func) (0x080+(n*8)+func)


/* STAGEx_CONNECTION1 (0x080,0x088,0x090,....,0x0D8): CIN0~CIN6專用
CINx Connection Setup 指定連接項目-1*/
#define CONNECTION1_CIN_ALL_BIAS 0x3fff /*CIN0~CIN6都接到BIAS中(沒錯，這裡是單數共7組)*/


#define CIN_FLOTING 0x00  /*指定CIN浮接*/
#define CIN_NEGATIVE 0x01 /*CIN連接到CDC Negative*/
#define CIN_POSITIVE 0x02 /*CIN連接到CDC Position*/
#define CIN_BIAS 0x03     /*CIN連接到BIAS(不清楚，可能是正負兩端的0v準位)*/
  /* CDC_SETUP(n,setup)
    範例：
      在STAGE2中，設定CIN4連接到CDC正端
      > WriteRegister(STAGE_ADDR(2,CIN_SETUP1(4,CIN_POSITIVE))
  */
#define CIN_SETUP1(n,setup) ((CONNECTION1_CIN_ALL_BIAS & ~(0x3<<(2*(n%7)))) | (setup<<(2*(n%7))) )
  /* 補充說明
    STAGE_CONNECTION1 n=0~6
    STAGE_CONNECTION2 n=7~11，與第一組相同因此共用：(n%7)
    清除對應bit(每組兩bit): ((CONNECTION1_CIN_ALL_BIAS & ~(0x3<<(2*(n%7))))
    指定對應bit(每組兩bit): | (setup<<(2*(n%7)))
  */



/* STAGEx_CONNECTION2 (0x081,0x089,0x091,....,0x0D9): CIN7~CIN12專用
CINx Connection Setup 指定連接項目-2*/
#define CONNECTION2_CIN_ALL_BIAS 0x0fff  /*CIN7~CIN12都接到BIAS中(是的7~12，這裡有6組)*/
#define CIN_SETUP2(n,setup) ((CONNECTION2_CIN_ALL_BIAS & ~(0x3<<(2*(n%7)))) | (setup<<(2*(n%7))) ) /*說明同CIN_SETUP2*/
#define SE_CONNECTION_SETUP_UNUSED 0x0000
#define SE_CONNECTION_SETUP_POSITIVE 0x1000
#define SE_CONNECTION_SETUP_NEGATIVE 0x2000
#define SE_CONNECTION_SETUP_DIFFERNETIAL 0x3000
#define NEG_AFE_OFFSET_DISABLE 0x4000
#define NEG_AFE_OFFSET_ENABLE 0x0000
#define POS_AFE_OFFSET_DISABLE 0x8000
#define POS_AFE_OFFSET_ENABLE 0x0000

/* STAGEx_AFE_OFFSET (0x082,0x08A,0x092,....,0x0DA)
環境偏移校正
*/
#define NEG_AFE_OFFSET(n) (n&0x003f)      /*負端校正值 範圍0~63, 校正值=n*0.32pF(0~20.16) */
#define NEG_AFE_OFFSET_SWAP 0x0080        /*正負交換，將負端校正接到CDC正端*/
#define POS_AFE_OFFSET(n) ((n&0x003f)<<8) /*正端校正值 範圍0~63, 校正值=n*0.32pF(0~20.16) */
#define POS_AFE_OFFSET_SWAP 0x8000        /*正負交換，將正端校正接到CDC負端*/

/* STAGE_SENSITIVITY
靈敏度
*/
#define NEG_THRESHOLD_SENSITIVITY(n)  (n&0xf)       /*負閾值靈敏度(0~15)對應到(25%~95.32%)*/
#define NEG_PEAK_DETECT(n)            ((n&7)<<4)    /*負端峰值偵測(0~5)對應到(40%~90%)*/
#define POS_THRESHOLD_SENSITIVITY(n)  ((n&0xf)<<8)  /*正閾值靈敏度(0~15)對應到(25%~95.32%)*/
#define POS_PEAK_DETECT(n)            ((n&7)<<12)   /*正端峰值偵測(0~5)對應到(40%~90%)*/


/*Use IC Sequential sampling*/
#ifndef SAMPLE_BY_CHIP_LOOP
#define SAMPLE_BY_CHIP_LOOP 1
#endif


///////////////////////////////// Richard ////////////////////////////////////////////////


#define SPI_7147_WR         0xE000      //write
#define SPI_7147_RD         0xE400      //read
 
#define POS_AFE_VALUE       (0x00<<8)   //adjust Gain/Offset

//CINx connected to CDC positive input
#define CIN0_CDC_INPUT_POS  (0x02)
#define CIN1_CDC_INPUT_POS  (CIN0_CDC_INPUT_POS<<2)
#define CIN2_CDC_INPUT_POS  (CIN0_CDC_INPUT_POS<<4)
#define CIN3_CDC_INPUT_POS  (CIN0_CDC_INPUT_POS<<6)
#define CIN4_CDC_INPUT_POS  (CIN0_CDC_INPUT_POS<<8)
#define CIN5_CDC_INPUT_POS  (CIN0_CDC_INPUT_POS<<10)
#define CIN6_CDC_INPUT_POS  (CIN0_CDC_INPUT_POS<<12)


//// CIN0 ~ CIN11 setup define


#define REG_VALUE_PWR_CONTROL       WORD_SWAP((PWCTL_FULL_SHOUTDOWN_MODE|PWCTL_SQUENCE_STATE_12)) // into shoutdown model
#define REG_VALUE_CALIBRATION       WORD_SWAP(0xfff|AVG_FP_SKIP_31)  //STAGE0_CAL_EN ~ STAGE11_CAL_EN
#define REG_VALUE_AMB_COMP_CTRL0    WORD_SWAP((FF_SKIP_CNT(2)|FP_PROXIMITY_CNT(3)|LP_PROXIMITY_CNT(2)|AMBCTL0_PWR_DOWN_TIMEOUT_2_00))
#define REG_VALUE_AMB_COMP_CTRL1    WORD_SWAP(AMBCTL1_PROXIMITY_RECAL_LVL(0x19)|AMBCTL1_PROXIMITY_DETECTION_RAGE(4))
#define REG_VALUE_AMB_COMP_CTRL2    WORD_SWAP(AMBCTL2_FP_PROXIMITY_RECAL(0x32)|AMBCTL2_LP_PROXIMITY_RECAL(2))
#define REG_VALUE_STAGE11_COMPLETE  WORD_SWAP(STAGE11_COMPLETE_INT_ENABLE)


//CIN0 ~ CON6
#define SETUP_CIN0  WORD_SWAP(STAGEx_CONNECT&(~BIT0)), WORD_SWAP(POS_AFE_ENABLE), WORD_SWAP(POS_AFE_VALUE), 0,0,0,0,0
#define SETUP_CIN1  WORD_SWAP(STAGEx_CONNECT&(~BIT2)), WORD_SWAP(POS_AFE_ENABLE), WORD_SWAP(POS_AFE_VALUE), 0,0,0,0,0
#define SETUP_CIN2  WORD_SWAP(STAGEx_CONNECT&(~BIT4)), WORD_SWAP(POS_AFE_ENABLE), WORD_SWAP(POS_AFE_VALUE), 0,0,0,0,0
#define SETUP_CIN3  WORD_SWAP(STAGEx_CONNECT&(~BIT6)), WORD_SWAP(POS_AFE_ENABLE), WORD_SWAP(POS_AFE_VALUE), 0,0,0,0,0
#define SETUP_CIN4  WORD_SWAP(STAGEx_CONNECT&(~BIT8)), WORD_SWAP(POS_AFE_ENABLE), WORD_SWAP(POS_AFE_VALUE), 0,0,0,0,0
#define SETUP_CIN5  WORD_SWAP(STAGEx_CONNECT&(~BIT10)),WORD_SWAP(POS_AFE_ENABLE), WORD_SWAP(POS_AFE_VALUE), 0,0,0,0,0
#define SETUP_CIN6  WORD_SWAP(STAGEx_CONNECT&(~BIT12)),WORD_SWAP(POS_AFE_ENABLE), WORD_SWAP(POS_AFE_VALUE), 0,0,0,0,0

//CIN7 ~ CON12
#define SETUP_CIN7  WORD_SWAP(STAGEx_CONNECT), WORD_SWAP(POS_AFE_ENABLE&(~BIT0)), WORD_SWAP(POS_AFE_VALUE), 0,0,0,0,0
#define SETUP_CIN8  WORD_SWAP(STAGEx_CONNECT), WORD_SWAP(POS_AFE_ENABLE&(~BIT2)), WORD_SWAP(POS_AFE_VALUE), 0,0,0,0,0
#define SETUP_CIN9  WORD_SWAP(STAGEx_CONNECT), WORD_SWAP(POS_AFE_ENABLE&(~BIT4)), WORD_SWAP(POS_AFE_VALUE), 0,0,0,0,0
#define SETUP_CIN10 WORD_SWAP(STAGEx_CONNECT), WORD_SWAP(POS_AFE_ENABLE&(~BIT6)), WORD_SWAP(POS_AFE_VALUE), 0,0,0,0,0
#define SETUP_CIN11 WORD_SWAP(STAGEx_CONNECT), WORD_SWAP(POS_AFE_ENABLE&(~BIT8)), WORD_SWAP(POS_AFE_VALUE), 0,0,0,0,0
#define SETUP_CIN12 WORD_SWAP(STAGEx_CONNECT), WORD_SWAP(POS_AFE_ENABLE&(~BIT10)),WORD_SWAP(POS_AFE_VALUE), 0,0,0,0,0

///////////////////// Other define /////////////////////////////////////////////////////////////

#define AD7147_MAX_NUM      8   // Total max AD7147 number
#define IC_CIN_NUM          12
#define CIN_CMD_NUM         (IC_CIN_NUM)
#define CIN_NUM_MAX         (IC_CIN_NUM*AD7147_MAX_NUM)
#define CIN_BUFF_BYTE_SIZE  (IC_CIN_NUM*2)
#define BANK1_INIT_TBL_SIZE      9
#define BANK2_INIT_TBL_SIZE ((IC_CIN_NUM+0)*8+1)


#define CMD_GET_CIN_VALUE   0x0BE4
#define GET_CIN_VALUE_OK    0x0800 
//#define GET_CIN_VALUE_OK    0x0FFF


#define STAGEx_CONNECT      0x3FFF
#define POS_AFE_ENABLE      0x5FFF



// AD7147 register cmd
#define REG_GET_CIN_STATUS  0x000A
#define REG_GET_CIN_VALUE   0x000B


#define GET_CIN_VALUE       (SPI_7147_RD|REG_GET_CIN_VALUE) //(0xE40B) 
#define GET_CIN_STATUS      (SPI_7147_RD|REG_GET_CIN_STATUS) //(0xE40A) 
#define CIN_STAGE_RESET     0xB233


#define SOFT_TIMER_VALUE_CIN_DELAY   2  //30ms
// 0: Low Power
// 1: Full Power
#define LOW_POWER       0
#define FULL_POWER      1



#define GET_CIN_STAGE_PENDING             0
#define GET_CIN_STAGE_DATA_START          1
#define GET_CIN_STAGE_DATA_WAITING        2   // whether ready or not
#define GET_CIN_STAGE_DATA                3   // get cin data
#define GET_CIN_STAGE_ERROR               4   // get cin data
#define GET_CIN_STAGE_DATA_COMPLETE       5   // complete


#define CYCLE_GET_CIN           TIMER_10MS
#define CYCLE_GET_CIN_PENDING   TIMER_100MS
#define CYCLE_GET_CIN_COMPLETE  TIMER_500MS

#define WAITING_GET_CIN_VALUE   TIMER_100MS
#define WAITING_ERROR_CIN       TIMER_1SEC
#define GET_CIN_COUNTER         (WAITING_GET_CIN_VALUE/CYCLE_GET_CIN)
#define ERROR_CIN_COUNTER       (WAITING_ERROR_CIN/CYCLE_GET_CIN)


#define SCAN_CIN_STATUS_PENDING     0
#define SCAN_CIN_STATUS_ON_GOING    1
#define SCAN_CIN_STATUS_ERROR       2
#define SCAN_CIN_STATUS_COMPLETE    3


typedef uint16 (*PActSpiCinBuff)[CIN_CMD_NUM];    // point to active CIN buffer


typedef struct{
  uint16  addr;
  uint16  data;
}AD7147Reg,*PAD7147Reg;

//
// define SPI device structure
typedef struct 
{
    uchar   DeviceID;
    PUINT16 CinDataBuff;
    GPIO_Port_TypeDef port;
    uint16  pin;
}_DeviceSpi,*_PDeviceSpi;


extern _DeviceSpi DeviceSpi[];
extern _PDeviceSpi pActDevSpi;
extern  uint16  CdcValue[];
extern  uchar   AD7147Num;

void AD7147Init(void);
bool AD7147WriteReg(uint16 reg_addr,uint16 reg_data);
bool AD7147WriteMultiReg(uint16 start_addr,uint16* p_reg_data, uchar size);

void AD7147Open(void);
void AD7147Close(void);
void AD7147CsOn(void);
void AD7147CsOff(void);

void AD7147Power(uchar status);

void AD7147AllCsStatus(uchar status);

bool GetCinStatus(void);
bool GetCinValue(void);


bool AD7147GetData(void);
bool GetOneAD7147Status(void);

_PDeviceSpi SetDeviceSpi(void);
void ResetGetCinInfo();
void GetAD7147CinxProc(void);
void StartScanCinValue(void);
uchar GetScanCinStatus(void);
uchar GetScanCinStage(void);


void ResetScanCinStatus(void);
void StartScanCinValue(void);




#endif

