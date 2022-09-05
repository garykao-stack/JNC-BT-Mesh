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
  // configure LED0 and LED_BLUE as outputs
  GPIO_PinModeSet(BSP_LED0_PORT, BSP_LED0_PIN, gpioModePushPull, LED_DEFAULT_STATE);
  GPIO_PinModeSet(BSP_LED1_PORT, BSP_LED1_PIN, gpioModePushPull, LED_DEFAULT_STATE);
  GPIO_PinModeSet(BSP_LED2_PORT, BSP_LED2_PIN, gpioModePushPull, LED_DEFAULT_STATE);
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
      TURN_LED_OFF(BSP_LED2_PORT, BSP_LED2_PIN);
      break;

    case LED_STATE_ON:
      TURN_LED_ON(BSP_LED0_PORT, BSP_LED0_PIN);
      TURN_LED_ON(BSP_LED1_PORT, BSP_LED1_PIN);
      TURN_LED_ON(BSP_LED2_PORT, BSP_LED2_PIN);
      break;

    case LED_STATE_PROV:
      GPIO_PinOutToggle(BSP_LED0_PORT, BSP_LED0_PIN);
      GPIO_PinOutToggle(BSP_LED1_PORT, BSP_LED1_PIN);
      break;

    default:
      break;
  }
}

static long LedStatus=0;
static uint8 tick=0;
void LedTick(){
	if (tick<=100)tick++;
	else tick=0;
	if (tick==50 || !tick){ //0.5 sec
		//dprint("0.5sec !!!!!\r\n");
		if(LedStatus & LED_STATE_MESH_ERROR) SetLedToggle(LED_BLUE);
	}
}



//
//
//
void SetLedStatus(uint16 status)
{
   switch(status) 
    {
        case LED_STATUS_OFF: // Alll of the LED OFF
            SetLed(LED_RED,OFF);   SetLed(LED_BLUE,OFF); SetLed(LED_GREEN,OFF);break;
        case LED_STATUS_ON: // Alll of the LED ON
            SetLed(LED_RED,ON);   SetLed(LED_BLUE,ON); SetLed(LED_GREEN,ON);  break;
        case LED_STATUS_SLEEP:  SetLed(LED_SLEEP,OFF);  break;
        case LED_STATUS_ACTIVE: SetLed(LED_SLEEP,ON);   break;
        
        case LED_STATUS_UNPROV:     SetLedToggle(LED_RED);SetLedToggle(LED_BLUE); break;
        case LED_STATUS_START_PROV: SetLed(LED_RED,OFF);SetLed(LED_BLUE,ON); break;            
        case LED_STATUS_PROVING:    SetLedToggle(LED_RED);SetLedToggle(LED_BLUE); break;
            
        case LED_STATUS_IVI_UPDATE_ON:  SetLedToggle(LED_IVI_UPDATE); break;
        case LED_STATUS_IVI_UPDATE_OFF: SetLed(LED_IVI_UPDATE,OFF); break;
        
        case LED_STATUS_SERVER_TO_CLIENT:// SetLedToggle(LED_RED); break;
             SetLed(LED_RED,ON); Delay_ms(500); SetLed(LED_RED,OFF);break;
        
        case LED_STATUS_SERVER_IO_CHANGE:   //from Built-in or RS-485
             SetLed(LED_GREEN,ON); Delay_ms(300); SetLed(LED_GREEN,OFF);Delay_ms(300);
             SetLed(LED_GREEN,ON); Delay_ms(300); SetLed(LED_GREEN,OFF);Delay_ms(300);
             SetLed(LED_GREEN,ON); Delay_ms(300); SetLed(LED_GREEN,OFF);
             break;

        case LED_STATUS_SERVER_TO_RS485:   /*Trace("LED_STATUS_SERVER_TO_RS485");//from Built-in or RS-485
             SetLed(LED_GREEN,ON);*/        break;
        case LED_UART_RX_ON: SetLed(LED_GREEN,ON); break;
        case LED_UART_RX_OFF: SetLed(LED_GREEN,OFF); break;
        case LED_SEND_NODE_INFO_ERROR_ON: SetLed(LED_BLUE,ON); LedStatus|=LED_STATE_MESH_ERROR; break;
        case LED_SEND_NODE_INFO_ERROR_OFF: SetLed(LED_BLUE,OFF); LedStatus&=~LED_STATE_MESH_ERROR; break;
        case LED_STATUS_SERVER_TO_SILICON:  Trace("LED_STATUS_SERVER_TO_SILICON"); //from Built-in or RS-485
             SetLed(LED_GREEN,OFF);
             break;


             
        default: SetLed(LED_SLEEP,OFF); SetLed(LED_ACTIVE,OFF); break;    
    };
}

//
//
//
void SetLed(uchar led_num, uchar status)
{
   // return; // debug
    
    if(led_num == LED_RED && status == ON)       
        LED_ON_FUN(BSP_LED0_PORT,  BSP_LED0_PIN);
    else if(led_num == LED_RED && status == OFF)   
        LED_OFF_FUN(BSP_LED0_PORT, BSP_LED0_PIN);
    else if(led_num == LED_BLUE && status == ON)    
        LED_ON_FUN(BSP_LED1_PORT,  BSP_LED1_PIN);
    else if(led_num == LED_BLUE && status == OFF)   
        LED_OFF_FUN(BSP_LED1_PORT, BSP_LED1_PIN);
   else if(led_num == LED_GREEN && status == ON)    
       LED_ON_FUN(BSP_LED2_PORT,  BSP_LED2_PIN);
   else if(led_num == LED_GREEN && status == OFF)   
       LED_OFF_FUN(BSP_LED2_PORT, BSP_LED2_PIN);
}

void SetLedToggle(uchar led)
{
    if(led == LED_RED)
        {
            GPIO_PinOutToggle(BSP_LED0_PORT,BSP_LED0_PIN);
        }
    else if(led == LED_BLUE)
        {
            GPIO_PinOutToggle(BSP_LED1_PORT,BSP_LED1_PIN);
        }
    
    else if(led == LED_GREEN)
        {
            GPIO_PinOutToggle(BSP_LED2_PORT,BSP_LED2_PIN);
        }
}


/** @} (end addtogroup Leds) */
