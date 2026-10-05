/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_custom_hid_if.c
  * @version        : v2.0_Cube
  * @brief          : USB Device Custom HID interface file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "usbd_custom_hid_if.h"
#include "GeneralDefines.h"
#include "WriteToFlash.h"
#include "RTC.h"
#include "SwiftSettings.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device.
  * @{
  */

/** @addtogroup USBD_CUSTOM_HID
  * @{
  */

/** @defgroup USBD_CUSTOM_HID_Private_TypesDefinitions USBD_CUSTOM_HID_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Defines USBD_CUSTOM_HID_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */

/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Macros USBD_CUSTOM_HID_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Variables USBD_CUSTOM_HID_Private_Variables
  * @brief Private variables.
  * @{
  */

/** Usb HID report descriptor. */
__ALIGN_BEGIN static uint8_t CUSTOM_HID_ReportDesc_FS[USBD_CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END =
{
	0x06, 0xFF, 0x00,
	0x09, 0x01,
	0xa1, 0x01,            /* COLLECTION (Application)         */
	/* 7 */

	//**BEGIN REPORT ID's ASSOCIATED WITH AUDIO CODEC SETTINGS********************
	0x85, 0x01,            /*     REPORT_ID (1)		           */
	0x09, 0x01,            /*     USAGE (1)	                 */
	0x15, 0x00,            /*     LOGICAL_MINIMUM (0)        */
	0x26, 0xFF, 0x00,      /*     LOGICAL_MAXIMUM (255)      */
	0x75, 0x08,            /*     REPORT_SIZE (8)            */
	0x95, 0x20,            /*     REPORT_COUNT (32)          */
	0x81, 0x82,            /*     INPUT                      */
	/* 22 */

	0x85, 0x02,            /*     REPORT_ID (2)		           */
	0x09, 0x01,            /*     USAGE (1)	                 */
	0x91, 0x82,            /*     OUTPUT                     */
	/* 28 */
	//**END REPORT ID's ASSSOCIATED WITH AUDIO CODEC SETTINGS*********************

	//**BEGIN REPORT ID's ASSOCIATED WITH RTC DATE AND TIME SETTINGS**************
	0x85, 0x03,            /*     REPORT_ID (3)		           */
	0x09, 0x02,            /*     USAGE (2)	                 */
	0x15, 0x00,            /*     LOGICAL_MINIMUM (0)        */
	0x26, 0xFF, 0x00,      /*     LOGICAL_MAXIMUM (255)      */
	0x75, 0x08,            /*     REPORT_SIZE (8)            */
	0x95, 0x0D,            /*     REPORT_COUNT (13)          */
	0x81, 0x82,            /*     INPUT                      */
	/* 43 */

	0x85, 0x04,            /*     REPORT_ID (4)		           */
	0x09, 0x02,            /*     USAGE (2)	                 */
	0x91, 0x82,            /*     OUTPUT                     */
	/* 49 */
	//**END REPORT ID's ASSOCIATED WITH RTC DATE AND TIME SETTINGS****************

	//**BEGIN REPORT ID's ASSOCIATED WITH STM32 FIRMWARE VERSION (HAL AND SWIFT)**
	0x85, 0x06,            /*     REPORT_ID (6)		           */
	0x09, 0x04,            /*     USAGE (4)	                 */
	0x15, 0x00,            /*     LOGICAL_MINIMUM (0)        */
	0x26, 0xFF, 0x00,      /*     LOGICAL_MAXIMUM (255)      */
	0x75, 0x08,            /*     REPORT_SIZE (8)            */
	0x95, 0x0C,            /*     REPORT_COUNT (12)          */
	0x81, 0x82,            /*     INPUT                      */
	/* 64 */
	//**END REPORT ID's ASSOCIATED WITH STM32 VERSION INFO************************

	//**BEGIN REPORT ID's ASSOCIATED WITH STM32 UNIQUE IDENTIFIER*****************
	0x85, 0x07,            /*     REPORT_ID (7)		           */
	0x09, 0x05,            /*     USAGE (5)	                 */
	0x15, 0x00,            /*     LOGICAL_MINIMUM (0)        */
	0x26, 0xFF, 0x00,      /*     LOGICAL_MAXIMUM (255)      */
	0x75, 0x08,            /*     REPORT_SIZE (8)            */
	0x95, 0x0E,            /*     REPORT_COUNT (14)          */
	0x81, 0x82,            /*     INPUT                      */
	/* 79 */
	//**END REPORT ID's ASSOCIATED WITH STM32 UNIQUE IDENTIFIER*******************

	//**BEGIN REPORT ID's ASSOCIATED WITH READING BATTERY VOLTAGE*****************
	0x85, 0x08,            /*     REPORT_ID (8)		           */
	0x09, 0x06,            /*     USAGE (6)	                 */
	0x15, 0x00,            /*     LOGICAL_MINIMUM (0)        */
	0x26, 0xFF, 0x00,      /*     LOGICAL_MAXIMUM (255)      */
	0x75, 0x08,            /*     REPORT_SIZE (8)            */
	0x95, 0x03,            /*     REPORT_COUNT (3)           */
	0x81, 0x82,            /*     INPUT                      */
	/* 94 */
	//**END REPORT ID's ASSOCIATED WITH READING BATTERY VOLTAGE*******************

	//**BEGIN REPORT ID's ASSOCIATED WITH ACTIVATING BOOTLOADER*******************
	0x85, 0x09,            /*     REPORT_ID (9)		           */
	0x09, 0x07,            /*     USAGE (7)	                 */
	0x15, 0x00,            /*     LOGICAL_MINIMUM (0)        */
	0x26, 0xFF, 0x00,      /*     LOGICAL_MAXIMUM (255)      */
	0x75, 0x08,            /*     REPORT_SIZE (8)            */
	0x95, 0x08,            /*     REPORT_COUNT (8)           */
	0x91, 0x82,            /*     OUTPUT                     */
	/* 109 */
	//**END REPORT ID's ASSOCIATED WITH ACTIVATING BOOTLOADER*********************

	//**BEGIN REPORT ID's ASSOCIATED WITH STM32 CLOCK DIVIDERS********************
	0x85, 0x0B,            /*     REPORT_ID (11)		         */
	0x09, 0x09,            /*     USAGE (9)	                 */
	0x15, 0x00,            /*     LOGICAL_MINIMUM (0)        */
	0x26, 0xFF, 0x00,      /*     LOGICAL_MAXIMUM (255)      */
	0x75, 0x08,            /*     REPORT_SIZE (8)            */
	0x95, 0x20,            /*     REPORT_COUNT (32)           */
	0x81, 0x82,            /*     INPUT                      */
	/* 124 */

	0x85, 0x0C,            /*     REPORT_ID (12)		           */
	0x09, 0x09,            /*     USAGE (9)	                 */
	0x91, 0x82,            /*     OUTPUT                     */
	/* 130 */
	//**END REPORT ID's ASSOCIATED WITH STM32 CLOCK DIVIDERS**********************

	//**BEGIN REPORT ID's ASSOCIATED WITH WAV FILENAME AND SIZE*******************
	0x85, 0x0D,            /*     REPORT_ID (13)		         */
	0x09, 0x0A,            /*     USAGE (10)	               */
	0x15, 0x00,            /*     LOGICAL_MINIMUM (0)        */
	0x26, 0xFF, 0x00,      /*     LOGICAL_MAXIMUM (255)      */
	0x75, 0x08,            /*     REPORT_SIZE (8)            */
	0x95, 0x20,            /*     REPORT_COUNT (32)          */
	0x81, 0x82,            /*     INPUT                      */
	/* 145 */

	0x85, 0x0E,            /*     REPORT_ID (14)		         */
	0x09, 0x0A,            /*     USAGE (10)	               */
	0x91, 0x82,            /*     OUTPUT                     */
	/* 151 */
	//**END REPORT ID's ASSOCIATED WITH WAV FILENAME AND SIZE*********************

	//**BEGIN REPORT ID's ASSOCIATED WITH RECORDING SCHEDULE START TIMES**********
	0x85, 0x0F,            /*     REPORT_ID (15)		         */
	0x09, 0x0B,            /*     USAGE (11)	               */
	0x15, 0x00,            /*     LOGICAL_MINIMUM (0)        */
	0x26, 0xFF, 0x00,      /*     LOGICAL_MAXIMUM (255)      */
	0x75, 0x08,            /*     REPORT_SIZE (8)            */
	0x95, 0x30,            /*     REPORT_COUNT (48)          */
	0x81, 0x82,            /*     INPUT                      */
	/* 166 */

	0x85, 0x10,            /*     REPORT_ID (16)		         */
	0x09, 0x0B,            /*     USAGE (11)	               */
	0x91, 0x82,            /*     OUTPUT                     */
	/* 172 */
	//**END REPORT ID's ASSOCIATED WITH RECORDING SCHEDULE START TIMES************

	//**BEGIN REPORT ID's ASSOCIATED WITH RECORDING SCHEDULE STOP TIMES***********
	0x85, 0x11,            /*     REPORT_ID (17)		         */
	0x09, 0x0C,            /*     USAGE (12)	                 */
	0x15, 0x00,            /*     LOGICAL_MINIMUM (0)        */
	0x26, 0xFF, 0x00,      /*     LOGICAL_MAXIMUM (255)      */
	0x75, 0x08,            /*     REPORT_SIZE (8)            */
	0x95, 0x30,            /*     REPORT_COUNT (48)          */
	0x81, 0x82,            /*     INPUT                      */
	/* 187 */

	0x85, 0x12,            /*     REPORT_ID (18)		         */
	0x09, 0x0C,            /*     USAGE (12)	                 */
	0x91, 0x82,            /*     OUTPUT                     */
	/* 193 */
	//**END REPORT ID's ASSOCIATED WITH RECORDING SCHEDULE STOP TIMES*************

	//**BEGIN REPORT ID's ASSOCIATED WITH CONFIGURATION TEXT FILE*****************
	0x85, 0x13,            /*     REPORT_ID (19)		         */
	0x09, 0x0D,            /*     USAGE (13)	               */
	0x15, 0x00,            /*     LOGICAL_MINIMUM (0)        */
	0x26, 0xFF, 0x00,      /*     LOGICAL_MAXIMUM (255)      */
	0x75, 0x08,            /*     REPORT_SIZE (8)            */
	0x95, 0x30,            /*     REPORT_COUNT (48)          */
	0x81, 0x82,            /*     INPUT                      */
	/* 208 */

	0x85, 0x14,            /*     REPORT_ID (20)		         */
	0x09, 0x0D,            /*     USAGE (13)	               */
	0x91, 0x82,            /*     OUTPUT                     */
	/* 214 */
	//**END REPORT ID's ASSOCIATED WITH CONFIGURATION TEXT FILE*******************

	0xC0 	                 /*     END_COLLECTION	           */
};

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Exported_Variables USBD_CUSTOM_HID_Exported_Variables
  * @brief Public variables.
  * @{
  */
extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */
/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_FunctionPrototypes USBD_CUSTOM_HID_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CUSTOM_HID_Init_FS(void);
static int8_t CUSTOM_HID_DeInit_FS(void);
static int8_t CUSTOM_HID_OutEvent_FS(uint8_t *buffer);

/**
  * @}
  */

USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_FS =
{
  CUSTOM_HID_ReportDesc_FS,
  CUSTOM_HID_Init_FS,
  CUSTOM_HID_DeInit_FS,
  CUSTOM_HID_OutEvent_FS
};

/** @defgroup USBD_CUSTOM_HID_Private_Functions USBD_CUSTOM_HID_Private_Functions
  * @brief Private functions.
  * @{
  */

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes the CUSTOM HID media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_Init_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  DeInitializes the CUSTOM HID media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_DeInit_FS(void)
{
  /* USER CODE BEGIN 5 */
  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Manage the CUSTOM HID class events
  * @param  event_idx: Event index
  * @param  state: Event state
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_OutEvent_FS(uint8_t *Buffer)
{
	switch (Buffer[0])
	  {
	    case 2: //Codec Settings Variables
	      //SwiftSettings_WriteToFlash(Buffer);
	      EraseFlashSector();
	      WriteToFlash(Buffer, CODEC_SETTINGS_OFFSET, 2);
	      break;

	    case 4: //RTC Date and Time
	      RTC_SetDateTime(Buffer);
	      break;

	    case 9: //Jump to bootloader and reset
	      HAL_RCC_DeInit();
	      SysTick->CTRL = 0;
	      SysTick->LOAD = 0;
	      SysTick->VAL = 0;
	      __set_PRIMASK(1);

	      *((unsigned long *)(0x2001FFF0)) = 0x0055AAFF;  // indicate that bootloader should be started
	      HAL_NVIC_SystemReset(); // force system reset; __low_level_int() function will start bootloader
	      break;

	    case 12: //STM32 Clock Divider variables
	      WriteToFlash(Buffer, STM32_CLOCKDIV_OFFSET, 2);
	      break;

	    case 14: //WAV File Variables (File size and file name)
	      WriteToFlash(Buffer, WAVFILE_ATTRIBUTES_OFFSET, 2);
	      break;

	    case 16: //Recording schedule start times
	      WriteToFlash(Buffer, SCHEDULE_STARTTIMES_OFFSET, 3);
	      break;

	    case 18: //Recording schedule stop times
	      WriteToFlash(Buffer, SCHEDULE_STOPTIMES_OFFSET, 3);
	      break;

	    case 20: //Configuration Text File
	      //WritePacketToFlash(Buffer, CONFIG_TEXTFILE_OFFSET, 47);
	      WriteToFlash(Buffer, (CONFIG_TEXTFILE_OFFSET + (Buffer[1] * 48)), 3);
	      break;

	    default:
	      break;
	  }

	  return (0);
}

/* USER CODE BEGIN 7 */
/**
  * @brief  Send the report to the Host
  * @param  report: The report to be sent
  * @param  len: The report length
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
/*
static int8_t USBD_CUSTOM_HID_SendReport_FS(uint8_t *report, uint16_t len)
{
  return USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, report, len);
}
*/
/* USER CODE END 7 */

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

