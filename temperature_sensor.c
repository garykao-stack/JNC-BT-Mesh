/***************************************************************************//**
 * @file  temperature_sensor.c
 * @brief Temperature sensor implementation
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
#include "bus_I2C.h"

#include <stdio.h>
#include "temperature_sensor.h"
#include "display_interface.h"
#include "si7013.h"

#ifdef ENABLE_LOGGING
#define log(...) printf(__VA_ARGS__)
#else
#define log(...)
#endif

/***************************************************************************//**
 * @addtogroup Sensor
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup TemperatureSensor
 * @{
 ******************************************************************************/


/// Temperature
static temperature_8_t temperature = VALUE_IS_NOT_KNOWN;

/***************************************************************************//**
 * Display temperature on display interface.
 ******************************************************************************/
void display_temperature(void)
{
  if ((temperature_8_t)VALUE_IS_NOT_KNOWN == temperature) {
    DI_Print("Temperature: UNKNOWN", DI_ROW_TEMPERATURE);
  } else {
    char tmp[21];
    snprintf(tmp, 21, "Temperature: %3d.%1dC ", temperature / 2, (temperature * 5) % 10);
    DI_Print(tmp, DI_ROW_TEMPERATURE);
  }
}

/*******************************************************************************
 * Initialize the temperature sensor.
 ******************************************************************************/
void init_temperature_sensor()
{
#ifdef FEATURE_I2C_SENSOR
  bool status = Si7013_Detect(I2C0, SI7021_ADDR, NULL);
  if (!status) {
    printf("Si7021 disconnect Error\r\n");
  }
#endif
  get_temperature();

}

/*******************************************************************************
 * Get the current temperature value measured by sensor.
 *
 * @return Current value of temperature.
 ******************************************************************************/
temperature_8_t get_temperature(void)
{
  int32_t tempData = 0;
#ifndef FEATURE_I2C_SENSOR
  static int32_t DummyValue = 0l;

  /* Use dummy counter for boards that does not support I2C sensor feature */
  tempData = DummyValue + 20000l;
  DummyValue = (DummyValue + 1000l) % 21000l;
  tempData = (((tempData * 2) + 499) / 1000);
  temperature = (temperature_8_t)tempData;
#else
  uint32_t tempRH = 0;

  /* Sensor relative humidity and temperature measurement returns 0 on success, nonzero otherwise */
  if (Si7013_MeasureRHAndTemp(I2C0, SI7021_ADDR, &tempRH, &tempData) != 0) {
    temperature = VALUE_IS_NOT_KNOWN;
  } else if ((tempData > 63500) || (tempData < -64000)) {
    temperature = VALUE_IS_NOT_KNOWN;
  } else {

  TraceDec2("Temp&Humi 1", tempRH, tempData);
    tempData = (((tempData * 2) + 499) / 1000);
  //TraceDec1("Temp 1",tempData);
    temperature = (temperature_8_t)tempData;
  }
#endif
  //TraceDec1("temperature12",temperature);
  display_temperature();
  return temperature;
}


//
// Get Temperture and Humidity
// 0: Tempature 1: Humidity
uint16 GetTempRH(uchar select)
{
    uint32  tempRH = 0; 
    int32_t tempData = 0;

    uint16 ret_code;
    // Sensor relative humidity and temperature measurement returns 0 on success, nonzero otherwise 
    
    if(Si7013_MeasureRHAndTemp(I2C0, SI7021_ADDR, &tempRH, &tempData) != 0) 
   // Si7013_StartNoHoldMeasureRHAndTemp(I2C0, SI7021_ADDR); Delay_ms(100);
   // if(Si7013_ReadNoHoldRHAndTemp(I2C0, SI7021_ADDR, &tempRH, &tempData) != 0) 
        {ret_code = VALUE_IS_NOT_KNOWN;}
    else if ((tempData > 63500) || (tempData < -64000)) 
        ret_code = VALUE_IS_NOT_KNOWN;

    if(ret_code != VALUE_IS_NOT_KNOWN)
        {
    
        tempRH /=100; if(tempRH > 1000) tempRH = 1000;
        tempData /=100;
        TraceDec2("Temp&Humi 2", tempRH, tempData);
    
        if(select == 0)  {return (int16)tempData;} //tempature
        else {return (int16)tempRH;} //humidity }
        }
    else {TraceErr("GetTempRH");}

    return ret_code;
}


//
// Get Temperture and Humidity
// 0: Tempature 1: Humidity
uint16 GetTempAndRH(int16 *Temp, uint16 *humidity)
{
    uint32  tempRH = 0; 
    int32_t tempData = 0;

    uint16 ret_code=0;
    
    if(Si7013_MeasureRHAndTemp(I2C0, SI7021_ADDR, &tempRH, &tempData) != 0) 
        {//TraceDec2("GetTempRH 1",tempData,tempRH);
        ret_code = VALUE_IS_NOT_KNOWN;
        }
    else if ((tempData > 63500) || (tempData < -64000))
        {//TraceDec2("GetTempRH 2",tempData,tempRH);
        ret_code = VALUE_IS_NOT_KNOWN;
        }
    else if ((tempRH > 100000))
        {//TraceDec2("GetTempRH 3",tempData,tempRH);
         tempRH = 100000;   
        }    
    else if((tempRH == 0) && (tempData == 0))
        {//TraceDec2("GetTempRH 4",tempData,tempRH);
        ret_code = VALUE_IS_NOT_KNOWN;;
        }


    
    if(ret_code != VALUE_IS_NOT_KNOWN)
        {//TraceDec2("GetTempRH Ok",tempData,tempRH);
        tempRH /=100; if(tempRH >= 1000) tempRH = 1000;
        *humidity =tempRH;
        tempData /=100; if(tempData < 800) *Temp = tempData;
        TraceDec3("Temp&Humi 3", tempRH, tempData,*Temp);
        }
    else 
        {
            
         ret_code = VALUE_IS_NOT_KNOWN;TraceDec2("GetTempRH 6",tempData,tempRH);
        }

    return ret_code;
}



uchar temp_rh_index=0;
//
//return tempature and relative humidity
//
uint32  GetTempAndRh()
{
    
    uint32 ret_code=0;
    ret_code = GetTempHumi();
/*
    uint16 temp_value,rh_value;
    temp_value = (uint16)jnc_temp[temp_rh_index];
    rh_value = (uint16)jnc_rh[temp_rh_index];;
    ret_code = MAKEDWORD(temp_value,rh_value);

    Printf("Temp %d Humidity %d\r\n",(int16)temp_value,(int16)rh_value);
    //Trace16_2(temp_value, rh_value);
    
    if(++temp_rh_index >= 16) temp_rh_index=0;
*/    
    return ret_code;
    
}






/** @} (end addtogroup TemperatureSensor) */
/** @} (end addtogroup Sensor) */
