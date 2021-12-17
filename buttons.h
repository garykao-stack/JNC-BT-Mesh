/***************************************************************************//**
 * @file  buttons.h
 * @brief Buttons header file
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

#ifndef BUTTONS_H
#define BUTTONS_H

/***************************************************************************//**
 * @defgroup Buttons Buttons Module
 * @brief Buttons Module Implementation
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup Buttons
 * @{
 ******************************************************************************/

/*******************************************************************************
 * External signal definitions. These are used to signal button press events
 * from GPIO interrupt handler to application.
 ******************************************************************************/
#define PB_SPEED_5SEC           0x01    //
#define PB_SPEED_NORMAL         0x02    // System to normal status
 

#define PB1_PRESS_OFF           0x11
#define PB1_PRESS_ON            0x12

#define PB_SPEED_ON             0x20
#define PB_SPEED_OFF            0x21



#define PB0_MEDIUM_PRESS        0x10



/***************************************************************************//**
 * Button initialization. Configure pushbuttons PB0, PB1 as inputs.
 ******************************************************************************/
void button_init(void);

/***************************************************************************//**
 * Enable button interrupts for PB0, PB1. Both GPIOs are configured to trigger
 * an interrupt on the rising edge (button released).
 ******************************************************************************/
void enable_button_interrupts(void);

/** @} (end addtogroup Buttons) */

#endif /* BUTTONS_H */
