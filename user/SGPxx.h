#ifndef __SGPXX_H__
#define __SGPXX_H__

#include <stdbool.h>
#include "em_device.h"

typedef enum
{
  SGPxx_IDLE        = 0,
  SGPxx_softreset   = 1,
  SGPxx__INIT       = 2,
  SGPxx_Measure     = 3,
  SGPxx_Value       = 4
}SGPxx_STATE;


static const uint8_t SGPxx_address = 0x58;
static const uint8_t SGPxx_soft_reset[2] = {0x00, 0x06};

typedef enum    //Address 0x58
{
  SGPxx_Write  = 0xB0,
  SGPxx_Read   = 0xB1
}SGPxx_Header;

//BMP280 Command
typedef enum
{
  SGPxx_Iaq_Init_msb            = 0x20,
  SGPxx_Iaq_Init_lsb            = 0x03,
  SGPxx_measure_iaq_msb         = 0x20,
  SGPxx_measure_iaq_lsb         = 0x08,
  SGPxx_get_iaq_baseline_msb    = 0x20,
  SGPxx_get_iaq_baseline_lsb    = 0x15,
  SGPxx_set_iaq_baseline_msb    = 0x20,
  SGPxx_set_iaq_baseline_lsb    = 0x1E,
  SGPxx_set_absolute_humi_msb   = 0x20,
  SGPxx_set_absolute_humi_lsb   = 0x61,
  SGPxx_measure_test_msb        = 0x20, //Datasheet不建議使用
  SGPxx_measure_test_lsb        = 0x32,
  SGPxx_get_feature_set_msb     = 0x20,
  SGPxx_get_feature_set_lsb     = 0x2F,
  SGPxx_measure_raw_msb         = 0x20,
  SGPxx_measure_raw_lsb         = 0x50,
  SGPxx_get_tvoc_baseline_msb   = 0x20,
  SGPxx_get_tvoc_baseline_lsb   = 0xB3,
  SGPxx_set_tvoc_baseline_msb   = 0x20,
  SGPxx_set_tvoc_baseline_lsb   = 0x77
}SGPxx_Command;

// extern int SGPxx_VOC_Value;
// extern void SGPxx_Load(void);

extern bool SGPxx_IsReady();
extern bool SGPxx_GetTvoc(uint16_t *value);

#endif
