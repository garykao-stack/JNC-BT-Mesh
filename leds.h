/***************************************************************************//**
 * @file  leds.h
 * @brief Leds header file
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

#ifndef LEDS_H
#define LEDS_H

/***************************************************************************//**
 * @defgroup Leds Leds Module
 * @brief Leds Module Implementation
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup Leds
 * @{
 ******************************************************************************/

/*******************************************************************************
 *  State of the LEDs is updated by calling LED_set_state().
 *  The new state is passed as parameter, possible values are defined below.
 ******************************************************************************/
#define LED_STATE_OFF    0   ///< light off (both LEDs turned off)
#define LED_STATE_ON     1   ///< light on (both LEDs turned on)
#define LED_STATE_PROV   3   ///< provisioning (LEDs blinking)

/***************************************************************************//**
 * LED initialization. Configure LED pins as outputs.
 ******************************************************************************/
void led_init(void);

/***************************************************************************//**
 * Update the state of LEDs.
 *
 * @param[in] state  New state defined as LED_STATE_xxx.
 ******************************************************************************/
void led_set_state(uint8_t state);


// Richard Add
////////// LED define ///////////////////////////////////////

#define LED0        0
#define LED1        1

#define LED_RED     0
#define LED_BLUE    1


#if BOARD_T3_SIMULATION || BOARD_T3
#define LED_ON_FUN      GPIO_PinOutSet
#define LED_OFF_FUN     GPIO_PinOutClear
#define LED_ON          HIGH
#define LED_OFF         LOW

#else
#define LED_ON_FUN      GPIO_PinOutClear
#define LED_OFF_FUN     GPIO_PinOutSet
#define LED_ON          LOW
#define LED_OFF         HIGH

#endif



#define LED_SERVER          LED0
#define LED_CLIENT          LED1
#define LED_GET_INFO        LED0
#define LED_BT_TEMP_HUM     LED0
#define LED_OTHER_TEMP_HUM  LED1
#define LED_IVI_UPDATE      LED1


#define LedOnClient()       SetLed(LED_CLIENT,LED_ON)
#define LedOffClient()      SetLed(LED_CLIENT,LED_OFF)
#define LedOnServer()       SetLed(LED_SERVER,LED_ON)
#define LedOffServer()      SetLed(LED_SERVER,LED_OFF)
#define LedOnGetReg()       SetLed(LED_GET_INFO,LED_ON)
#define LedOffGetReg()      SetLed(LED_GET_INFO,LED_OFF)

#define LedOnBtTempHum()    SetLed(LED_BT_TEMP_HUM,LED_ON)
#define LedOffBtTempHum()   SetLed(LED_BT_TEMP_HUM,LED_OFF)
#define LedOnOtherTempHum()  SetLed(LED_OTHER_TEMP_HUM,LED_ON)
#define LedOffOtherTempHum() SetLed(LED_OTHER_TEMP_HUM,LED_OFF)



#define LedClientSendCmd()      {LedOffClient(); Delay_ms(500); LedOnClient();}
#define LedClientGetRegInfo()   {LedOnGetReg();  Delay_ms(10); LedOffGetReg();}

#define LedBtTempHum()          {LedOnBtTempHum();  Delay_ms(500); LedOffBtTempHum();Delay_ms(500); \
                                 LedOnBtTempHum();  Delay_ms(500); LedOffBtTempHum();Delay_ms(500); \
                                 LedOnBtTempHum();  Delay_ms(500); LedOffBtTempHum();}
#define LedDevTempHum()         {LedOnOtherTempHum(); Delay_ms(500); LedOffOtherTempHum();Delay_ms(500);\
                                 LedOnOtherTempHum(); Delay_ms(500); LedOffOtherTempHum();Delay_ms(500);\
                                 LedOnOtherTempHum(); Delay_ms(500); LedOffOtherTempHum();}



#define LED_SLEEP           LED0
#define LED_ACTIVE          LED1

#define LED_STATUS_OFF              0   //All OFF
#define LED_STATUS_ON               1   //ALL ON

#define LED_STATUS_SLEEP            2
#define LED_STATUS_ACTIVE           3
#define LED_STATUS_CLIENT_SEND      4
#define LED_STATUS_CLIENT_RECEIVE   5
#define LED_STATUS_TOGGLE           6

#define LED_STATUS_UNPROV           7
#define LED_STATUS_START_PROV       8
#define LED_STATUS_PROVING          9

#define LED_STATUS_IVI_UPDATE_ON    10
#define LED_STATUS_IVI_UPDATE_OFF   11
#define LED_STATUS_SERVER_TO_CLIENT   12







void SetLed(uchar led_num, uchar status);
void SetLedStatus(uchar status);
void SetLedToggle(uchar led);



/** @} (end addtogroup Leds) */

#endif /* LEDS_H */
