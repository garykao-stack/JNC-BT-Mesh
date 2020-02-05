/***************************************************************************//**
 * @file
 * @brief HAL configuration file
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef HAL_CONFIG_H
#define HAL_CONFIG_H

#include "board_features.h"
#include "hal-config-board.h"
#include "hal-config-app-common.h"

#if BOARD_SILICON_CLIENT || BOARD_SILICON_SERVER    //richard add
    #ifndef HAL_VCOM_ENABLE
      #define HAL_VCOM_ENABLE                   (1)
    #endif
#endif


#ifndef HAL_I2CSENSOR_ENABLE
#if defined(FEATURE_I2C_SENSOR)
#define HAL_I2CSENSOR_ENABLE              (1)
#else
#define HAL_I2CSENSOR_ENABLE              (0)
#endif
#endif

//richard Add
#ifdef FEATURE_LCD_SUPPORT
    #define HAL_SPIDISPLAY_ENABLE             (1)
#else
    #define HAL_SPIDISPLAY_ENABLE             (0)
#endif



/*
#ifndef HAL_SPIDISPLAY_ENABLE
#define HAL_SPIDISPLAY_ENABLE             (1)
#endif
*/
#define HAL_SPIDISPLAY_EXTCOMIN_CALLBACK
#if defined(FEATURE_IOEXPANDER)
#define HAL_SPIDISPLAY_EXTMODE_EXTCOMIN               (0)
#else
#define HAL_SPIDISPLAY_EXTMODE_EXTCOMIN               (1)
#endif
#define HAL_SPIDISPLAY_EXTMODE_SPI                    (0)
#define HAL_SPIDISPLAY_EXTCOMIN_USE_PRS               (0)
#define HAL_SPIDISPLAY_EXTCOMIN_USE_CALLBACK          (0)
#define HAL_SPIDISPLAY_FREQUENCY                      (1000000)

/*
// Richard Add for WiFi Coexistence
// Enable 802.11 co-ex feature in gecko_init
#define HAL_COEX_ENABLE            0 //1

// Request window in microseconds
// How many us before the TX/RX request signal is enabled
#define HAL_COEX_REQ_WINDOW        500

// Grant signal polarity, pin and port
#define BSP_COEX_GNT_ASSERT_LEVEL  0
#define BSP_COEX_GNT_PIN           3
#define BSP_COEX_GNT_PORT          gpioPortF

// Priority signal polarity, pin and port
#define BSP_COEX_PRI_ASSERT_LEVEL  0
#define BSP_COEX_PRI_PIN           4
#define BSP_COEX_PRI_PORT          gpioPortF

// Request signal polarity, pin and port
#define BSP_COEX_REQ_ASSERT_LEVEL  0
#define BSP_COEX_REQ_PIN           5
#define BSP_COEX_REQ_PORT          gpioPortF

// Shared request in case multiple EFR32 radios are used
#define HAL_COEX_REQ_SHARED        0

// Enable priority signal (set both to 1)
#define HAL_COEX_TX_HIPRI          1
#define HAL_COEX_RX_HIPRI          1

// Abort TX if grant is denied during ongoing TX operation
#define HAL_COEX_TX_ABORT          1
*/

#endif
