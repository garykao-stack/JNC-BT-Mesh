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


/***************************************************************************//**
 * @addtogroup Sensor
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup TemperatureSensor
 * @{
 ******************************************************************************/

#define VALUE_IS_NOT_KNOWN  (0xFF) ///< Temperature value is not known

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
    snprintf(tmp, 21, "Temp: %3d.%1dC ", temperature / 2, (temperature * 5) % 10);
    DI_Print(tmp, DI_ROW_TEMPERATURE);
  }
}

/*******************************************************************************
 * Initialize the temperature sensor.
 ******************************************************************************/
void init_temperature_sensor()
{
  bool status = Si7013_Detect(I2C0, SI7021_ADDR, NULL);
  if (!status) {
    printf("I2C Error\r\n");
  }
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

  TraceDec2("Temp&Humi", tempRH, tempData);
    tempData = (((tempData * 2) + 499) / 1000);
    temperature = (temperature_8_t)tempData;
  }
#endif
  //TraceDec1("temperature12",temperature);
  display_temperature();
  return temperature;
}

/*
const int16 jnc_temp[16]={-555,-444,-333,-222,-111,00,111,333,555,777,999,101,202,303,404,505};
const int16 jnc_rh[16]  ={000,102,103,104,104,105,106,107,108,109,111,112,113,114,115,116};
*/
uchar temp_rh_index=0;
//
//return tempature and relative humidity
//
uint32  GetTempAndRh()
{
    
    uint32 ret_code=0;
    ret_code = GetTempHumi();
    //TraceDec2("GetTempAndRh: Tempature & Humidity", (int16)HIWORD(ret_code),(int16)LOWORD(ret_code));

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
