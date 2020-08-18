/***************************************************************************//**
 * @file  leds.c
 * @brief Leds implementation file
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
#include "global.h"
#include "hal-config.h"
#include "leds.h"

/***************************************************************************//**
 * @addtogroup Leds
 * @{
 ******************************************************************************/

/*******************************************************************************
 *  These defines are needed to support radio boards with active-low and
 *  active-high LED configuration
 ******************************************************************************/
#ifdef FEATURE_LED_BUTTON_ON_SAME_PIN
/* LED GPIO is active-low */
#define TURN_LED_OFF   GPIO_PinOutSet
#define TURN_LED_ON    GPIO_PinOutClear
#define LED_DEFAULT_STATE  1
#else
/* LED GPIO is active-high */
#define TURN_LED_OFF   GPIO_PinOutClear
#define TURN_LED_ON    GPIO_PinOutSet
#define LED_DEFAULT_STATE  0
#endif

/*******************************************************************************
 * LED initialization. Configure LED pins as outputs.
 ******************************************************************************/
void led_init(void)
{
  // configure LED0 and LED1 as outputs
  GPIO_PinModeSet(BSP_LED0_PORT, BSP_LED0_PIN, gpioModePushPull, LED_DEFAULT_STATE);
  GPIO_PinModeSet(BSP_LED1_PORT, BSP_LED1_PIN, gpioModePushPull, LED_DEFAULT_STATE);
  SetLedStatus(LED_STATUS_OFF);
    
}

/*******************************************************************************
 * Update the state of LEDs.
 *
 * @param[in] state  New state defined as LED_STATE_xxx.
 ******************************************************************************/
void led_set_state(uint8_t state)
{
  switch (state) {
    case LED_STATE_OFF:
      TURN_LED_OFF(BSP_LED0_PORT, BSP_LED0_PIN);
      TURN_LED_OFF(BSP_LED1_PORT, BSP_LED1_PIN);
      break;

    case LED_STATE_ON:
      TURN_LED_ON(BSP_LED0_PORT, BSP_LED0_PIN);
      TURN_LED_ON(BSP_LED1_PORT, BSP_LED1_PIN);
      break;

    case LED_STATE_PROV:
      GPIO_PinOutToggle(BSP_LED0_PORT, BSP_LED0_PIN);
      GPIO_PinOutToggle(BSP_LED1_PORT, BSP_LED1_PIN);
      break;

    default:
      break;
  }
}



//
//
//
void SetLedStatus(uchar status)
{
   switch(status) 
    {
        case LED_STATUS_OFF: // Alll of the LED OFF
            SetLed(LED_SLEEP,OFF);   SetLed(LED_ACTIVE,OFF); break;
        case LED_STATUS_ON: // Alll of the LED ON
            SetLed(LED_SLEEP,ON);   SetLed(LED_ACTIVE,ON);   break;
        case LED_STATUS_SLEEP:  SetLed(LED_SLEEP,OFF);  break;
        case LED_STATUS_ACTIVE: SetLed(LED_SLEEP,ON);   break;
        
        case LED_STATUS_UNPROV:     SetLedToggle(LED0);SetLedToggle(LED1); break;
        case LED_STATUS_START_PROV: SetLed(LED0,OFF);SetLed(LED1,ON); break;            
        case LED_STATUS_PROVING:    SetLedToggle(LED0);SetLedToggle(LED1); break;
            
        case LED_STATUS_IVI_UPDATE_ON:  SetLedToggle(LED_IVI_UPDATE); break;
        case LED_STATUS_IVI_UPDATE_OFF: SetLed(LED_IVI_UPDATE,OFF); break;
        
        case LED_STATUS_SERVER_TO_CLIENT:// SetLedToggle(LED0); break;
             SetLed(LED0,ON); Delay_ms(500); SetLed(LED0,OFF);break;
            
        default: SetLed(LED_SLEEP,OFF); SetLed(LED_ACTIVE,OFF); break;    
    };
}

//
//
//
void SetLed(uchar led_num, uchar status)
{
   // return; // debug
    
    if(led_num == LED0 && status == ON)       
        LED_ON_FUN(BSP_LED0_PORT,  BSP_LED0_PIN);
    else if(led_num == LED0 && status == OFF)   
        LED_OFF_FUN(BSP_LED0_PORT, BSP_LED0_PIN);
    else if(led_num == LED1 && status == ON)    
        LED_ON_FUN(BSP_LED1_PORT,  BSP_LED1_PIN);
    else if(led_num == LED1 && status == OFF)   
        LED_OFF_FUN(BSP_LED1_PORT, BSP_LED1_PIN);
    
   //  LED_ON_FUN(BSP_LED0_PORT,  BSP_LED0_PIN);
   //  LED_ON_FUN(BSP_LED1_PORT,  BSP_LED1_PIN);
}

void SetLedToggle(uchar led)
{
    if(led == LED0)
        {
            GPIO_PinOutToggle(BSP_LED0_PORT,BSP_LED0_PIN);
        }
    else
        {
            GPIO_PinOutToggle(BSP_LED1_PORT,BSP_LED1_PIN);
          
        }
}


/** @} (end addtogroup Leds) */
