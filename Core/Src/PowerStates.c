/*
 * PowerStates.c
 *
 *  Created on: May 2, 2026
 *      Author: rak287
 */

#include "PowerStates.h"
#include "main.h"
#include "i2c.h"
#include "mdf.h"
#include "gpdma.h"
#include "linked_list.h"

void RecordingWithLidClosed()
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /*
  SDMMC1 - ENABLED
  SDCARD_DETECT - DISABLED
  SDCard_En - ENABLED
  SDCARD_PWR_EN - ENABLED and HIGH
  HallSensor - ENABLED
  OLED_13V_EN - DISABLED
  MDF1 - ENABLED
  I2C2 - DISABLED (Display off)
  PUSHBUTTON1, PUSHBUTTON2, PUSHBUTTON3 - DISABLED
  OLED_CS - DISABLED
  OLED_RST - DISALBED
  STATUS_0, STATUS_1, STATUS_2 - ENABLED
  PWR_3p3V_Enable - ENABLED and HIGH
  USB_VBUS - DISABLED
  USB_OTG pins - DISABLED
  */

  //OLED 13V Power Enable
  if((OLED_13V_EN_GPIO_Port->ODR & OLED_13V_EN_Pin) == OLED_13V_EN_Pin)
  {
    HAL_GPIO_WritePin(OLED_13V_EN_GPIO_Port, OLED_13V_EN_Pin, GPIO_PIN_RESET); //Set LOW to turn 13V OFF
  }

  //ENABLE MDF1 INTERFACE
  if((RCC->AHB1ENR & RCC_AHB1ENR_GPDMA1EN) != RCC_AHB1ENR_GPDMA1EN) //Test to see if GPDMA1 clock is enabled. If it is, we'll take this as an indication that MDF is already activated
  {
    //MX_GPDMA1_Init();
    //MX_MDF1_Init();
    MX_GPDMA1_Init();
     __HAL_LINKDMA(&MdfHandle0, hdma, handle_GPDMA1_Channel11);
     MX_MDF1_Init();

     MX_MDFQueue_Config();
     HAL_DMAEx_List_LinkQ(&handle_GPDMA1_Channel11, &MDFQueue);

     //MDF_DMAConfig();
  }

  //DISABLE I2C2 INTERFACE
  if((RCC->APB1ENR1 & RCC_APB1ENR1_I2C2EN) == RCC_APB1ENR1_I2C2EN)
  {
    HAL_I2C_MspDeInit(&hi2c2);
  }

/*Pushbutton inputs: BTN_TIME BTN_RIGHT BTN_LEFT BTN_ENTER BTN_MEMO BTN_PBOUT
#define BTN_TIME_Pin GPIO_PIN_4
#define BTN_TIME_GPIO_Port GPIOD
#define BTN_RIGHT_Pin GPIO_PIN_5
#define BTN_RIGHT_GPIO_Port GPIOD
#define BTN_LEFT_Pin GPIO_PIN_8
#define BTN_LEFT_GPIO_Port GPIOD
#define BTN_ENTER_Pin GPIO_PIN_7
#define BTN_ENTER_GPIO_Port GPIOC
#define BTN_MEMO_Pin GPIO_PIN_8
#define BTN_MEMO_GPIO_Port GPIOA
#define BTN_PBOUT_Pin GPIO_PIN_9
#define BTN_PBOUT_GPIO_Port GPIOD
 */
  if((BTN_TIME_GPIO_Port->MODER & 3145728) != 3145728)
  {
    GPIO_InitStruct.Pin = BTN_TIME_Pin|BTN_RIGHT_Pin|BTN_LEFT_Pin|BTN_PBOUT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(BTN_TIME_GPIO_Port, &GPIO_InitStruct);
  }

  if((BTN_ENTER_GPIO_Port->MODER & 3145728) != 3145728)
  {
    GPIO_InitStruct.Pin = BTN_ENTER_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(BTN_ENTER_GPIO_Port, &GPIO_InitStruct);
  }

  if((BTN_MEMO_GPIO_Port->MODER & 3145728) != 3145728)
  {
    GPIO_InitStruct.Pin = BTN_MEMO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(BTN_MEMO_GPIO_Port, &GPIO_InitStruct);
  }

  if((OLED_CS_GPIO_Port->ODR & OLED_CS_Pin) != OLED_CS_Pin)
  {
  //OLED_CS - DISABLED
    HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET); //CS high disables OLED communication
  }

  //OLED_RST
  if((OLED_RST_GPIO_Port->ODR & OLED_RST_Pin) == OLED_RST_Pin)
  {
    HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET); //Hold reset low
  }

}

void RecordingWithLidOpen()
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};


  //OLED 13V Power Enable
  if((OLED_13V_EN_GPIO_Port->ODR & OLED_13V_EN_Pin) != OLED_13V_EN_Pin)
  {
    HAL_GPIO_WritePin(OLED_13V_EN_GPIO_Port, OLED_13V_EN_Pin, GPIO_PIN_SET); //Set HIGH to turn 13V on
  }

  //ENABLE MDF1 INTERFACE
  if((RCC->AHB1ENR & RCC_AHB1ENR_GPDMA1EN) != RCC_AHB1ENR_GPDMA1EN) //Test to see if GPDMA1 clock is enabled. If it is, we'll take this as an indication that MDF is already activated
  {
    MX_GPDMA1_Init();
    MX_MDF1_Init();
  }

  //ENABLE I2C2 INTERFACE
  if((RCC->APB1ENR1 & RCC_APB1ENR1_I2C2EN) != RCC_APB1ENR1_I2C2EN)
  {
    MX_I2C2_Init();
  }

  /*
  Pushbutton inputs: BTN_TIME BTN_RIGHT BTN_LEFT BTN_ENTER BTN_MEMO BTN_PBOUT
  #define BTN_TIME_Pin GPIO_PIN_4
  #define BTN_TIME_GPIO_Port GPIOD
  #define BTN_RIGHT_Pin GPIO_PIN_5
  #define BTN_RIGHT_GPIO_Port GPIOD
  #define BTN_LEFT_Pin GPIO_PIN_8
  #define BTN_LEFT_GPIO_Port GPIOD
  #define BTN_ENTER_Pin GPIO_PIN_7
  #define BTN_ENTER_GPIO_Port GPIOC
  #define BTN_MEMO_Pin GPIO_PIN_8
  #define BTN_MEMO_GPIO_Port GPIOA
  #define BTN_PBOUT_Pin GPIO_PIN_9
  #define BTN_PBOUT_GPIO_Port GPIOD
  */
    if((BTN_TIME_GPIO_Port->MODER & 3145728) != 3145728)
    {
      GPIO_InitStruct.Pin = BTN_TIME_Pin|BTN_RIGHT_Pin|BTN_LEFT_Pin|BTN_PBOUT_Pin;
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      HAL_GPIO_Init(BTN_TIME_GPIO_Port, &GPIO_InitStruct);
    }

    if((BTN_ENTER_GPIO_Port->MODER & 3145728) != 3145728)
    {
      GPIO_InitStruct.Pin = BTN_ENTER_Pin;
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      HAL_GPIO_Init(BTN_ENTER_GPIO_Port, &GPIO_InitStruct);
    }

    if((BTN_MEMO_GPIO_Port->MODER & 3145728) != 3145728)
    {
      GPIO_InitStruct.Pin = BTN_MEMO_Pin;
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      HAL_GPIO_Init(BTN_MEMO_GPIO_Port, &GPIO_InitStruct);
    }

  if((OLED_CS_GPIO_Port->ODR & OLED_CS_Pin) == OLED_CS_Pin)
  {
  //OLED_CS - DISABLED
    HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET); //CS low enables OLED communication
  }

  //OLED_RST
  if((OLED_RST_GPIO_Port->ODR & OLED_RST_Pin) != OLED_RST_Pin)
  {
    HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET); //Set reset high
  }

}

void StandbyWithLidClosed()
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  //OLED 13V Power Enable
  if((OLED_13V_EN_GPIO_Port->ODR & OLED_13V_EN_Pin) == OLED_13V_EN_Pin)
  {
    HAL_GPIO_WritePin(OLED_13V_EN_GPIO_Port, OLED_13V_EN_Pin, GPIO_PIN_RESET); //Set LOW to turn 13V OFF
  }

  //ENABLE MDF1 INTERFACE
  if((RCC->AHB1ENR & RCC_AHB1ENR_GPDMA1EN) == RCC_AHB1ENR_GPDMA1EN) //Test to see if GPDMA1 clock is enabled. If it is, we'll take this as an indication that MDF is already activated
  {
    HAL_MDF_DeInit(&MdfHandle0);
    HAL_MDF_DeInit(&MdfHandle1);
    MX_GPDMA1_DeInit();
  }

  //DISABLE I2C2 INTERFACE
  if((RCC->APB1ENR1 & RCC_APB1ENR1_I2C2EN) == RCC_APB1ENR1_I2C2EN)
  {
    HAL_I2C_MspDeInit(&hi2c2);
  }

  /*Pushbutton inputs: BTN_TIME BTN_RIGHT BTN_LEFT BTN_ENTER BTN_MEMO BTN_PBOUT
  #define BTN_TIME_Pin GPIO_PIN_4
  #define BTN_TIME_GPIO_Port GPIOD
  #define BTN_RIGHT_Pin GPIO_PIN_5
  #define BTN_RIGHT_GPIO_Port GPIOD
  #define BTN_LEFT_Pin GPIO_PIN_8
  #define BTN_LEFT_GPIO_Port GPIOD
  #define BTN_ENTER_Pin GPIO_PIN_7
  #define BTN_ENTER_GPIO_Port GPIOC
  #define BTN_MEMO_Pin GPIO_PIN_8
  #define BTN_MEMO_GPIO_Port GPIOA
  #define BTN_PBOUT_Pin GPIO_PIN_9
  #define BTN_PBOUT_GPIO_Port GPIOD
   */
    if((BTN_TIME_GPIO_Port->MODER & 3145728) != 3145728)
    {
      GPIO_InitStruct.Pin = BTN_TIME_Pin|BTN_RIGHT_Pin|BTN_LEFT_Pin|BTN_PBOUT_Pin;
      GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      HAL_GPIO_Init(BTN_TIME_GPIO_Port, &GPIO_InitStruct);
    }

    if((BTN_ENTER_GPIO_Port->MODER & 3145728) != 3145728)
    {
      GPIO_InitStruct.Pin = BTN_ENTER_Pin;
      GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      HAL_GPIO_Init(BTN_ENTER_GPIO_Port, &GPIO_InitStruct);
    }

    if((BTN_MEMO_GPIO_Port->MODER & 3145728) != 3145728)
    {
      GPIO_InitStruct.Pin = BTN_MEMO_Pin;
      GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      HAL_GPIO_Init(BTN_MEMO_GPIO_Port, &GPIO_InitStruct);
    }

  if((OLED_CS_GPIO_Port->ODR & OLED_CS_Pin) != OLED_CS_Pin)
  {
  //OLED_CS - DISABLED
    HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET); //CS high disables OLED communication
  }

  //OLED_RST
  if((OLED_RST_GPIO_Port->ODR & OLED_RST_Pin) == OLED_RST_Pin)
  {
    HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET); //Hold reset low
  }

}

void StandbyWithLidOpen()
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  //OLED 13V Power Enable
  if((OLED_13V_EN_GPIO_Port->ODR & OLED_13V_EN_Pin) != OLED_13V_EN_Pin)
  {
    HAL_GPIO_WritePin(OLED_13V_EN_GPIO_Port, OLED_13V_EN_Pin, GPIO_PIN_SET); //Set HIGH to turn 13V on
  }

  //ENABLE MDF1 INTERFACE
  if((RCC->AHB1ENR & RCC_AHB1ENR_GPDMA1EN) == RCC_AHB1ENR_GPDMA1EN) //Test to see if GPDMA1 clock is enabled. If it is, we'll take this as an indication that MDF is already activated
  {
    HAL_MDF_DeInit(&MdfHandle0);
    HAL_MDF_DeInit(&MdfHandle1);
    MX_GPDMA1_DeInit();
  }

  //ENABLE I2C2 INTERFACE
  if((RCC->APB1ENR1 & RCC_APB1ENR1_I2C2EN) != RCC_APB1ENR1_I2C2EN)
  {
    MX_I2C2_Init();
  }

  /*
  Pushbutton inputs: BTN_TIME BTN_RIGHT BTN_LEFT BTN_ENTER BTN_MEMO BTN_PBOUT
  #define BTN_TIME_Pin GPIO_PIN_4
  #define BTN_TIME_GPIO_Port GPIOD
  #define BTN_RIGHT_Pin GPIO_PIN_5
  #define BTN_RIGHT_GPIO_Port GPIOD
  #define BTN_LEFT_Pin GPIO_PIN_8
  #define BTN_LEFT_GPIO_Port GPIOD
  #define BTN_ENTER_Pin GPIO_PIN_7
  #define BTN_ENTER_GPIO_Port GPIOC
  #define BTN_MEMO_Pin GPIO_PIN_8
  #define BTN_MEMO_GPIO_Port GPIOA
  #define BTN_PBOUT_Pin GPIO_PIN_9
  #define BTN_PBOUT_GPIO_Port GPIOD
  */
    if((BTN_TIME_GPIO_Port->MODER & 3145728) != 3145728)
    {
      GPIO_InitStruct.Pin = BTN_TIME_Pin|BTN_RIGHT_Pin|BTN_LEFT_Pin|BTN_PBOUT_Pin;
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      HAL_GPIO_Init(BTN_TIME_GPIO_Port, &GPIO_InitStruct);
    }

    if((BTN_ENTER_GPIO_Port->MODER & 3145728) != 3145728)
    {
      GPIO_InitStruct.Pin = BTN_ENTER_Pin;
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      HAL_GPIO_Init(BTN_ENTER_GPIO_Port, &GPIO_InitStruct);
    }

    if((BTN_MEMO_GPIO_Port->MODER & 3145728) != 3145728)
    {
      GPIO_InitStruct.Pin = BTN_MEMO_Pin;
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      HAL_GPIO_Init(BTN_MEMO_GPIO_Port, &GPIO_InitStruct);
    }

  if((OLED_CS_GPIO_Port->ODR & OLED_CS_Pin) == OLED_CS_Pin)
  {
  //OLED_CS - DISABLED
    HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET); //CS low enables OLED communication
  }

  //OLED_RST
  if((OLED_RST_GPIO_Port->ODR & OLED_RST_Pin) != OLED_RST_Pin)
  {
    HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET); //Set reset high
  }

}
