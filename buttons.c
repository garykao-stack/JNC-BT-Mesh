/***************************************************************************//**
 * @file  buttons.c
 * @brief Buttons implementation file
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
#include "native_gecko.h"
#include <gpiointerrupt.h>
#include "buttons.h"

uint32 TimerPB0,TimerPB1,TimerPB2;


/***************************************************************************//**
 * @addtogroup Buttons
 * @{
 ******************************************************************************/

/*******************************************************************************
 * Button initialization. Configure pushbuttons PB0, PB1 as inputs.
 ******************************************************************************/
void button_init(void)
{
  // configure pushbutton PB0 and PB1 as inputs, with pull-up enabled
  GPIO_PinModeSet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN, gpioModeInputPull, 1);
  GPIO_PinModeSet(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN, gpioModeInputPull, 1);
  TimerPB0 = TimerPB1 = 0;
}

#define PB_SPEED_UP_TIMER               TIMER_MS_2_TICKS(1000)  // 1 sec
#define PB_TIMER_FILTER1                TIMER_MS_2_TICKS(1000)  // 1 sec
#define PB_TIMER_FILTER2                TIMER_MS_2_TICKS(1000)  // 1 sec

#define PB_TIMER_FILTER_OVER            TIMER_MS_2_TICKS(50)  //
#define PB_TIMER_RESET_OVER             TIMER_MS_2_TICKS(2000)  // 2 sec


uchar   PbStatus=0;

/***************************************************************************//**
 * This is a callback function that is invoked each time a GPIO interrupt
 * in one of the pushbutton inputs occurs. Pin number is passed as parameter.
 *
 * @param[in] pin  Pin number where interrupt occurs
 *
 * @note This function is called from ISR context and therefore it is
 *       not possible to call any BGAPI functions directly. The button state
 *       change is signaled to the application using gecko_external_signal()
 *       that will generate an event gecko_evt_system_external_signal_id
 *       which is then handled in the main loop.
 ******************************************************************************/
void button_interrupt(uint8_t pin)
{
    uint32  diff;
  if(pin == BSP_BUTTON0_PIN){
        if(GPIO_PinInGet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN) == HIGH){
                diff = RTCC_CounterGet() - TimerPB0;
                if(diff > PB_SPEED_UP_TIMER)
                    gecko_external_signal(PB_SPEED_5SEC);
                else  
                    gecko_external_signal(PB_SPEED_NORMAL);
            }
        else{TimerPB0 = RTCC_CounterGet(); }
    } 
  else if(pin == BSP_BUTTON1_PIN){
        if(GPIO_PinInGet(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN) == HIGH) 
            gecko_external_signal(PB1_PRESS_OFF);
        else gecko_external_signal(PB1_PRESS_ON);
    }
  else if(pin == BSP_G6_SPEED_PIN){
      if(GPIO_PinInGet(BSP_G6_SPEED_PORT, BSP_G6_SPEED_PIN) == HIGH)
        gecko_external_signal(PB_SPEED_KEY);
      //else gecko_external_signal(PB_SPEED_ON);
    }
  
  else if(pin == BSP_G6_RESET_PIN){ 
       
      if(GPIO_PinInGet(BSP_G6_RESET_PORT, BSP_G6_RESET_PIN) == HIGH){
             TimerPB2 = RTCC_CounterGet(); Trace("PB_G6_RESET 1");
        }
      else{
           diff = RTCC_CounterGet() - TimerPB2;
            if(diff < PB_TIMER_RESET_OVER) { Trace("PB_G6_RESET 2");
            return;}
            gecko_external_signal(PB_G6_RESET); Trace("PB_G6_RESET 3");
        }
    }
  
  else if(pin == BSP_FILTER_PIN){
       diff = RTCC_CounterGet() - TimerPB1;
       if(diff < PB_TIMER_FILTER_OVER) { //TraceErr("NO Effective 1");
        return;}
       else TimerPB1 = RTCC_CounterGet();
        
        if(GPIO_PinInGet(BSP_FILTER_PORT, BSP_FILTER_PIN) == HIGH){
            gecko_external_signal(PB_DOOR_OPEN);    
        }
        else{
            gecko_external_signal(PB_DOOR_CLOSE);
        }
    }

}

/*******************************************************************************
 * Enable button interrupts for PB0, PB1. Both GPIOs are configured to trigger
 * an interrupt on the rising edge (button released).
 ******************************************************************************/
void enable_button_interrupts(void)
{
  GPIOINT_Init();

  /* configure interrupt for PB0 and PB1, rising edges */
  GPIO_ExtIntConfig(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN, BSP_BUTTON0_PIN, true, true, true);
  GPIO_ExtIntConfig(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN, BSP_BUTTON1_PIN,true, true, true);
  GPIO_ExtIntConfig(BSP_G6_SPEED_PORT, BSP_G6_SPEED_PIN, BSP_G6_SPEED_PIN,true, true, true);
  GPIO_ExtIntConfig(BSP_FILTER_PORT, BSP_FILTER_PIN, BSP_FILTER_PIN,true, true, true);
  GPIO_ExtIntConfig(BSP_G6_RESET_PORT, BSP_G6_RESET_PIN, BSP_G6_RESET_PIN,true, true, true);  

  /* register the callback function that is invoked when interrupt occurs */
  GPIOINT_CallbackRegister(BSP_BUTTON0_PIN, button_interrupt);
  GPIOINT_CallbackRegister(BSP_BUTTON1_PIN, button_interrupt);
  GPIOINT_CallbackRegister(BSP_G6_SPEED_PIN, button_interrupt);
  GPIOINT_CallbackRegister(BSP_FILTER_PIN, button_interrupt);
  GPIOINT_CallbackRegister(BSP_G6_RESET_PIN, button_interrupt);
}

/** @} (end addtogroup Buttons) */
