/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "i2c_driver.h"
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

I2C_HandleTypeDef hi2c1;

uint8_t buf[12];
int16_t val;
static const uint8_t SENS_ADDR = 0x48 << 1; // Use 8-bit address


/* ----------------------------------FUNCTIONS--------------------------------*/

/**I2C1 GPIO Configuration
PB6     ------> I2C1_SCL
PB7     ------> I2C1_SDA
*/

extern void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
}

// ****************************************************************************
extern int16_t I2C_read_temp_sensor()
{

	  *buf=0;

	  if (HAL_I2C_Master_Transmit(&hi2c1, SENS_ADDR, buf, 1, HAL_MAX_DELAY) != HAL_OK)
	  {
		  printf("\r\nNo sensor connected!\r\n");
		  Error_Handler();
	  }

	  if (HAL_I2C_Master_Receive(&hi2c1, SENS_ADDR, buf, 2, HAL_MAX_DELAY) != HAL_OK)
	  {
		  printf("\r\nNo data from sensor received!\r\n");
		  Error_Handler();
	  }

	  val = ((int16_t)buf[0] << 4) | (buf[1] >> 4);

	  if ( val > 0x7FF ) {
        val |= 0xF000;
      }

	  val=val*0.0625;

	  return val;
}

// ****************************************************************************




