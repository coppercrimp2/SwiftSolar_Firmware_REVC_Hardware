/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins
     PC14-OSC32_IN (PC14)   ------> RCC_OSC32_IN
     PC15-OSC32_OUT (PC15)   ------> RCC_OSC32_OUT
     PH0-OSC_IN (PH0)   ------> RCC_OSC_IN
     PH1-OSC_OUT (PH1)   ------> RCC_OSC_OUT
     PA13 (JTMS/SWDIO)   ------> DEBUG_JTMS-SWDIO
     PA14 (JTCK/SWCLK)   ------> DEBUG_JTCK-SWCLK
*/


/* USER CODE BEGIN 2 */
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  //HAL_GPIO_WritePin(GPIOB, OLED_13V_EN_Pin|SDCard_En_Pin|SDCARD_PWR_EN_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(PWR_3p3V_Enable_GPIO_Port, PWR_3p3V_Enable_Pin, GPIO_PIN_RESET); //Disable 3.3V
  HAL_GPIO_WritePin(GPIOB, OLED_13V_EN_Pin, GPIO_PIN_RESET); //13V is off
  HAL_GPIO_WritePin(GPIOB, SDCard_En_Pin, GPIO_PIN_RESET); //SDCard_En is off
  HAL_GPIO_WritePin(GPIOB, SDCARD_PWR_EN_Pin, GPIO_PIN_SET); //SDCard Power Off

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, OLED_CS_Pin|OLED_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  //HAL_GPIO_WritePin(PWR_3p3V_Enable_GPIO_Port, PWR_3p3V_Enable_Pin, GPIO_PIN_RESET);



  /*Configure GPIO pins : PE2 PE3 PE4 PE5
                           PE6 PE7 PE8 PE10
                           PE11 PE12 PE13 PE14
                           PE15 PE0 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15|GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PC13 PC0 PC1 PC2
                           PC3 PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2
                          |GPIO_PIN_3|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3
                           PA4 PA5 PA6 PA8
                           PA10 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_8
                          |GPIO_PIN_10|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : HallSensor_Pin USB_VBUS_Pin */
  GPIO_InitStruct.Pin = HallSensor_Pin|USB_VBUS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : OLED_13V_EN_Pin SDCard_En_Pin SDCARD_PWR_EN_Pin */
  GPIO_InitStruct.Pin = OLED_13V_EN_Pin|SDCard_En_Pin|SDCARD_PWR_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB2 PB13 PB14 PB15
                           PB3 PB4 PB6 PB8 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15
                          |GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_6|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PUSHBUTTON1_Pin PUSHBUTTON2_Pin PUSHBUTTON3_Pin STATUS_0_Pin
                           STATUS_1_Pin STATUS_2_Pin */
  GPIO_InitStruct.Pin = PUSHBUTTON1_Pin|PUSHBUTTON2_Pin|PUSHBUTTON3_Pin|STATUS_0_Pin
                          |STATUS_1_Pin|STATUS_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : OLED_CS_Pin OLED_RST_Pin */
  GPIO_InitStruct.Pin = OLED_CS_Pin|OLED_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PWR_3p3V_Enable_Pin */
  GPIO_InitStruct.Pin = PWR_3p3V_Enable_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PWR_3p3V_Enable_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PD0 PD1 PD3 PD4
                           PD5 PD7 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_4
                          |GPIO_PIN_5|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : SDCARD_DETECT_Pin */
  GPIO_InitStruct.Pin = SDCARD_DETECT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SDCARD_DETECT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PH3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  HAL_GPIO_WritePin(PWR_3p3V_Enable_GPIO_Port, PWR_3p3V_Enable_Pin, GPIO_PIN_SET); //Disable 3.3V

  /* EXTI interrupt init*/
  //HAL_NVIC_SetPriority(EXTI5_IRQn, 5, 0);
  //HAL_NVIC_EnableIRQ(EXTI5_IRQn);

  HAL_NVIC_SetPriority(EXTI7_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI7_IRQn);

  HAL_NVIC_SetPriority(EXTI9_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_IRQn);

}
/* USER CODE END 2 */
