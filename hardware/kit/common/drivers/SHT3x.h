#ifndef __SHT3x_H
#define __SHT3x_H

#include "em_device.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHT3x_ADDR      0x88

#define TrigTR_MSB      0x2C
#define TrigTR_LSB      0x06
#define SoftResetMSB    0x30
#define SoftResetLSB    0xA2


/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/
bool SHT3x_Measure(I2C_TypeDef *i2c, uint8_t addr, float *rhData, float *tData);

#ifdef __cplusplus
}
#endif

/** @} (end group Si7013) */
/** @} (end group kitdrv) */

#endif /* __SHT3x_H */
