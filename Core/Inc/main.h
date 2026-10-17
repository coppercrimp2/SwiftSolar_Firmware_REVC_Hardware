/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u5xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void SystemClock_Config(void);

typedef struct Message {
  char Line1[20];
  char Line2[20];
  char Line3[20];
  char Line4[20];
  char Line5[20];
  int LineCount;
} Message;

extern DMA_QListTypeDef MDFQueue;
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define HallSensor_Pin GPIO_PIN_1
#define HallSensor_GPIO_Port GPIOA
#define OLED_13V_EN_Pin GPIO_PIN_0
#define OLED_13V_EN_GPIO_Port GPIOB
#define BTN_INT_Pin GPIO_PIN_15
#define BTN_INT_GPIO_Port GPIOB
#define BTN_LEFT_Pin GPIO_PIN_8
#define BTN_LEFT_GPIO_Port GPIOD
#define BTN_PBOUT_Pin GPIO_PIN_9
#define BTN_PBOUT_GPIO_Port GPIOD
#define BTN_PSHOLD_Pin GPIO_PIN_10
#define BTN_PSHOLD_GPIO_Port GPIOD
#define OLED_CS_Pin GPIO_PIN_11
#define OLED_CS_GPIO_Port GPIOD
#define OLED_RST_Pin GPIO_PIN_12
#define OLED_RST_GPIO_Port GPIOD
#define STATUS_0_Pin GPIO_PIN_13
#define STATUS_0_GPIO_Port GPIOD
#define STATUS_1_Pin GPIO_PIN_14
#define STATUS_1_GPIO_Port GPIOD
#define STATUS_2_Pin GPIO_PIN_15
#define STATUS_2_GPIO_Port GPIOD
#define BTN_ENTER_Pin GPIO_PIN_7
#define BTN_ENTER_GPIO_Port GPIOC
#define BTN_MEMO_Pin GPIO_PIN_8
#define BTN_MEMO_GPIO_Port GPIOA
#define USB_VBUS_Pin GPIO_PIN_9
#define USB_VBUS_GPIO_Port GPIOA
#define USB_VBUS_EXTI_IRQn EXTI9_IRQn
#define BTN_TIME_Pin GPIO_PIN_4
#define BTN_TIME_GPIO_Port GPIOD
#define BTN_RIGHT_Pin GPIO_PIN_5
#define BTN_RIGHT_GPIO_Port GPIOD
#define SDCARD_DETECT_Pin GPIO_PIN_5
#define SDCARD_DETECT_GPIO_Port GPIOB
#define SDCard_En_Pin GPIO_PIN_7
#define SDCard_En_GPIO_Port GPIOB
#define SDCARD_PWR_EN_Pin GPIO_PIN_9
#define SDCARD_PWR_EN_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
//#define REC_BUFF_SIZE        (32764)//(16382)//(2048U)//(1024U)
//#define REC_BUFF_SIZE_DIV2  (REC_BUFF_SIZE / 2)
/* USER CODE END Private defines */



#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
