/*
 * DAC7760.h
 *  Created on: 2019/07/11   Author: Richard
 */

#ifndef _DAC7760_
#define _DAC7760_


// DAC8760 Address Functions
#define DAC7_WRITE_NOP_REGISTER			         0x00
#define DAC7_WRITE_DATA_REGISTER			     0x01
#define DAC7_READ_REGISTER    			         0x02
#define DAC7_WRITE_CONTROL_REGISTER			     0x55
#define DAC7_WRITE_RESET_REGISTER		     	 0x56
#define DAC7_WRITE_CONFIGURATION_REGISTER		 0x57
#define DAC7_WRITE_GAIN_CALIBRATION_REGISTER     0x58
#define DAC7_WRITE_ZERO_CALIBRATION_REGISTER	 0x59
#define DAC7_WRITE_WATCHDOG_TIMER_RESET		     0x95
#if 0

// DAC8760 Register Read Address Functions
#define DAC7_READ_STATUS_REGISTER                0x00
#define DAC7_READ_DATA_REGISTER                  0x01
#define DAC7_READ_CONTROL_REGISTER               0x02
#define DAC7_READ_CONFIGURATION_REGISTER         0x0B
#define DAC7_READ_GAIN_CALIBRATION_REGISTER      0x13
#define DAC7_READ_ZERO_CALIBRATION_REGISTER      0x17

// Control Register Settings
#define DAC7_CLRSEL_SHIFT                         15
#define DAC7_CLRSEL                               (1 << DAC7_CLRSEL_SHIFT)
#define DAC7_OVR_SHIFT                            14
#define DAC7_OVR                                  (1 << DAC7_OVR_SHIFT)
#define DAC7_REXT_SHIFT                           13
#define DAC7_REXT                                 (1 << DAC7_REXT_SHIFT)
#define DAC7_OUTEN_SHIFT                          12
#define DAC7_OUTEN                                (1 << DAC7_OUTEN_SHIFT)
#define DAC7_SRCLK_SHIFT                          8
#define DAC7_UPDATE_FREQUENCY_258065              (0 << DAC7_SRCLK_SHIFT)
#define DAC7_UPDATE_FREQUENCY_200000              (1 << DAC7_SRCLK_SHIFT)
#define DAC7_UPDATE_FREQUENCY_153845              (2 << DAC7_SRCLK_SHIFT)
#define DAC7_UPDATE_FREQUENCY_131145              (3 << DAC7_SRCLK_SHIFT)
#define DAC7_UPDATE_FREQUENCY_115940              (4 << DAC7_SRCLK_SHIFT)
#define DAC7_UPDATE_FREQUENCY_69565               (5 << DAC7_SRCLK_SHIFT)
#define DAC7_UPDATE_FREQUENCY_37560               (6 << DAC7_SRCLK_SHIFT)
#define DAC7_UPDATE_FREQUENCY_25805               (7 << DAC7_SRCLK_SHIFT)
#define DAC7_UPDATE_FREQUENCY_20150               (8 << DAC7_SRCLK_SHIFT)
#define DAC7_UPDATE_FREQUENCY_16030               (9 << DAC7_SRCLK_SHIFT)
#define DAC7_UPDATE_FREQUENCY_10295               (10 << DAC7_SRCLK_SHIFT)
#define DAC7_UPDATE_FREQUENCY_8280                (11 << DAC7_SRCLK_SHIFT)
#define DAC7_UPDATE_FREQUENCY_6900                (12 << DAC7_SRCLK_SHIFT)
#define DAC7_UPDATE_FREQUENCY_5530                (13 << DAC7_SRCLK_SHIFT)
#define DAC7_UPDATE_FREQUENCY_4240                (14 << DAC7_SRCLK_SHIFT)
#define DAC7_UPDATE_FREQUENCY_3300                (15 << DAC7_SRCLK_SHIFT)
#define DAC7_SRSTEP_SHIFT                         5
#define DAC7_SLEW_RATE_STEP_SIZE_1                (0 << DAC7_SRSTEP_SHIFT)
#define DAC7_SLEW_RATE_STEP_SIZE_2                (1 << DAC7_SRSTEP_SHIFT)
#define DAC7_SLEW_RATE_STEP_SIZE_4                (2 << DAC7_SRSTEP_SHIFT)
#define DAC7_SLEW_RATE_STEP_SIZE_8                (3 << DAC7_SRSTEP_SHIFT)
#define DAC7_SLEW_RATE_STEP_SIZE_16               (4 << DAC7_SRSTEP_SHIFT)
#define DAC7_SLEW_RATE_STEP_SIZE_32               (5 << DAC7_SRSTEP_SHIFT)
#define DAC7_SLEW_RATE_STEP_SIZE_64               (6 << DAC7_SRSTEP_SHIFT)
#define DAC7_SLEW_RATE_STEP_SIZE_128              (7 << DAC7_SRSTEP_SHIFT)
#define DAC7_SREN_SHIFT                           4
#define DAC7_SLEW_RATE_ENABLE                     (1 << DAC7_SREN_SHIFT)
#define DAC7_DCEN_SHIFT                           3
#define DAC7_DAISY_CHAIN_ENABLE                   (1 << DAC7_DCEN_SHIFT)
#define DAC7_RANGE_SHIFT                          0
#define DAC7_RANGE_0V_5V                          (0 << DAC7_RANGE_SHIFT)
#define DAC7_RANGE_0V_10V                         (1 << DAC7_RANGE_SHIFT)
#define DAC7_RANGE_M5V_P5V                        (2 << DAC7_RANGE_SHIFT)
#define DAC7_RANGE_M10V_P10V                      (3 << DAC7_RANGE_SHIFT)
#define DAC7_RANGE_NOT_ALLOWED                    (4 << DAC7_RANGE_SHIFT)
#define DAC7_RANGE_4MA_20MA                       (5 << DAC7_RANGE_SHIFT)
#define DAC7_RANGE_0MA_20MA                       (6 << DAC7_RANGE_SHIFT)
#define DAC7_RANGE_0MA_24MA                       (7 << DAC7_RANGE_SHIFT)

// Configuration Register Settings
#define DAC7_IOUT_RANGE_SHIFT                     9
#define DAC7_IOUT_RANGE_DISABLED                  (0 << DAC7_IOUT_RANGE_SHIFT)
#define DAC7_IOUT_RANGE_4MA_20MA                  (1 << DAC7_IOUT_RANGE_SHIFT)
#define DAC7_IOUT_RANGE_0MA_20MA                  (2 << DAC7_IOUT_RANGE_SHIFT)
#define DAC7_IOUT_RANGE_0MA_24MA                  (3 << DAC7_IOUT_RANGE_SHIFT)
#define DAC7_DUAL_OUTEN_SHIFT                     8
#define DAC7_DUAL_OUT_ENABLE                      (1 << DAC7_DUAL_OUTEN_SHIFT)
#define DAC7_APD_SHIFT                            7
#define DAC7_APD                                  (1 << DAC7_APD_SHIFT)
#define DAC7_CALEN_SHIFT                          5
#define DAC7_CAL_ENABLE                           (1 << DAC7_CALEN_SHIFT)
#define DAC7_HARTEN_SHIFT                         4
#define DAC7_HART_ENABLE                          (1 << DAC7_HARTEN_SHIFT)
#define DAC7_CRCEN_SHIFT                          3
#define DAC7_CRC_ENABLE                           (1 << DAC7_CRCEN_SHIFT)
#define DAC7_WDEN_SHIFT                           2
#define DAC7_WD_ENABLE                            (1 << DAC7_WDEN_SHIFT)
#define DAC7_WDPD_SHIFT                           0
#define DAC7_WD_TIMEOUT_10MS                     (0 << DAC7_WDPD_SHIFT)
#define DAC7_WD_TIMEOUT_51MS                     (1 << DAC7_WDPD_SHIFT)
#define DAC7_WD_TIMEOUT_102MS                    (2 << DAC7_WDPD_SHIFT)
#define DAC7_WD_TIMEOUT_204MS                    (3 << DAC7_WDPD_SHIFT)

// Reset Register Settings
#define DAC7_RESET_SHIFT                         0
#define DAC7_RESET                               (1 << DAC7_RESET_SHIFT)

// STATUS Register Settings
#define DAC7_CRC_FLT_SHIFT                       4
#define DAC7_CRC_ERROR                           (1 << DAC7_CRC_FLT_SHIFT)
#define DAC7_WDT_FLT_SHIFT                       3
#define DAC7_WDT_TIMEOUT                         (1 << DAC7_WDT_FLT_SHIFT)
#define DAC7_I_FLT_SHIFT                         2
#define DAC7_IOUT_ERROR                          (1 << DAC7_I_FLT_SHIFT)
#define DAC7_SR_ON_SHIFT                         1
#define DAC7_SR_ON                               (1 << DAC7_SR_ON_SHIFT)
#define DAC7_T_FLT_SHIFT                         0
#define DAC7_TEMP_ERROR                          (1 << DAC7_T_FLT_SHIFT)

/**********************************************************************************************************************************/
/*                              PROTOTYPES                                                                                        */
/**********************************************************************************************************************************/
void DacWriteReg (uint16_t writeValues, uint8_t address);
void DacReadReg (uint8_t *readValues, uint8_t startReg);
void DacSetup (uint16_t controlReg, uint16_t configurationReg, uint16_t gainCalReg, uint16_t zeroCalReg);
void DacReset (void);
void DacNop (void);
void DacSetOutValue (uint16_t value);
uint8_t DacReadStatus (void);
#endif


#pragma pack(push) 
#pragma pack(1)     //mapping to one byte

typedef struct _DAC_REG_
{
    uint8 AddrB;
    uint16  ValueB;    
    uint8 AddrA;
    uint16  ValueA;
}DAC_REG,*PDAC_REG;
#pragma pack(pop)




#define VOL_PERCENT_MIN     0
#define VOL_PERCENT_MAX     100
#define VOL_DEFAULT         25
#define AB_DAC_DIFF         20  //20%

void InitDac7760();
void DacSetVol(uint16 percent);



#endif

