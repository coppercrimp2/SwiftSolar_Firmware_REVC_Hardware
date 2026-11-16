/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : FreeRTOS applicative file
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
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define OS_DELAY
#define DEBUG_MESSAGES
#include <string.h>
#include <stdio.h>
#include "queue.h"
#include "fonts.h"
#include "ssd1306.h"
#include "event_groups.h"
#include "fatfs.h"
#include "AudioFiles.h"
#include "GeneralDefines.h"
#include "mdf.h"
#include "gpdma.h"
#include "RTC.h"
#include "sdmmc.h"
#include "i2c.h"
#include "linked_list.h"
#include "task.h"
#include "PowerStates.h"
#include "usart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define CENTER_BTN_SIGNAL       ( 1 << 1 )
#define LID_OPEN                ( 1 << 2 )
#define LID_CLOSED              ( 1 << 3 )
#define SDCARD_IN               ( 1 << 4 )
#define SDCARD_OUT              ( 1 << 5 )

#define ALARM_A_SIGNAL            ( 1 << 6 )
#define END_OF_DAY_SIGNAL         ( 1 << 7 )
#define WRITEBUFFER1_SIGNAL       ( 1 << 8 )
#define WRITEBUFFER2_SIGNAL       ( 1 << 9 )
#define HALLEFFECT_SIGNAL         ( 1 << 10 )
#define ERROR_SIGNAL              ( 1 << 11 )
#define NEW_VOICE_MEMO_SIGNAL     ( 1 << 12 )
#define LOW_BATTERY_SIGNAL        ( 1 << 13 )

#define REC_BUFF_SIZE        (16384)//(32764)
#define REC_BUFF_SIZE_DIV2   (REC_BUFF_SIZE / 2)

#define BTN_PRESSED (0)
#define BTN_NOT_PRESSED (1)

#define BTN_PRESSED_Pos                         0U
#define BTN_PRESSED_Msk                         (1UL << BTN_PRESSED_Pos)
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
static void SDMMC1_SD_Init(void);
SWIFT_ERRORS SwiftErrors;



void ReInit_MDF1_Interface(void);
void DeInit_MDF1_Interface(void);
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osThreadId_t ScanButtonsHandle;
const osThreadAttr_t ScanButtons_attributes = {
  .name = "ScanButtons",
  .priority = (osPriority_t) osPriorityAboveNormal,
  .stack_size = 256 * 4
};
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for Record */
osThreadId_t RecordHandle;
const osThreadAttr_t Record_attributes = {
  .name = "Record",
  .priority = (osPriority_t) osPriorityLow1,
  .stack_size = 512 * 4
};
/* Definitions for DisplayUpdate */
osThreadId_t DisplayUpdateHandle;
const osThreadAttr_t DisplayUpdate_attributes = {
  .name = "DisplayUpdate",
  .priority = (osPriority_t) osPriorityLow1,
  .stack_size = 256 * 4
};
/* Definitions for Initialize */
osThreadId_t InitializeHandle;
const osThreadAttr_t Initialize_attributes = {
  .name = "Initialize",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for Scheduler */
osThreadId_t SchedulerHandle;
const osThreadAttr_t Scheduler_attributes = {
  .name = "Scheduler",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 256 * 4
};
/* Definitions for QueueDisplayMessage */
osMessageQueueId_t QueueDisplayMessageHandle;
const osMessageQueueAttr_t QueueDisplayMessage_attributes = {
  .name = "QueueDisplayMessage"
};
/* Definitions for QueueRecordMessage */
osMessageQueueId_t QueueRecordMessageHandle;
const osMessageQueueAttr_t QueueRecordMessage_attributes = {
  .name = "QueueRecordMessage"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
WAVFile_Attributes WAVFile;
Swift_Schedule SwiftSchedule;
FIL FsFile;

FIL VoiceMemoFile;

MDF_DmaConfigTypeDef         DMAConfig;
static void MDF_DMAConfig(void);

unsigned int BufferWritesPerFile;

struct system_flags{
  bool LidClosed = 0;
  bool CurrentlyRecording = 0;
};

struct system_flags SystemFlags;


static int16_t RecBuff[REC_BUFF_SIZE] = {0}; //Lives in .BSS / RAM
__IO uint32_t  DmaRecHalfBuffCplt  = 0;
__IO uint32_t  DmaRecBuffCplt      = 0;

extern DMA_HandleTypeDef handle_GPDMA1_Channel11;

void WriteDebugFile(void);
void InitializeCodecAndDMA(void);
void AudioCodec_SleepMode(void);

uint32_t byteswritten;
FRESULT FatFsResult;

Message DisplayMessage;
Record_Task_Message RecordTaskMessage;

EventGroupHandle_t xEventGroup;
/* USER CODE END FunctionPrototypes */

/* USER CODE BEGIN PREPOSTSLEEP */
/*
 * TIM2 - system tick used by HAL I think
 * SYSTICK - Used by FreeRTOS
 */

__weak void PreSleepProcessing(uint32_t ulExpectedIdleTime)
{
/* place for user code */
  ulExpectedIdleTime = 0;

  HAL_SuspendTick(); //TIM2 Disable

  if(SystemFlags.LidClosed == false)  // LID IS OPEN
  {
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
  }
  else  // LID IS CLOSED
  {
    //printf("EnteringStop1\n\r");
    HAL_PWREx_EnterSTOP1Mode(PWR_STOPENTRY_WFI);
  }
}

__weak void PostSleepProcessing(uint32_t ulExpectedIdleTime)
{
/* place for user code */
  (void) ulExpectedIdleTime;

  if(SystemFlags.LidClosed == true)
  {
    SystemClock_Config();
    //printf("ExitingStop1\n\r");
    //MX_GPDMA1_Init();
    //MX_MDF1_Init();
  }
  HAL_ResumeTick(); //TIM2 Enable

}
/* USER CODE END PREPOSTSLEEP */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  printf("\r\n\r\n\r\nStarting MX_FREERTOS_Init function (233)\r\n");
  xEventGroup = xEventGroupCreate();

  //** Get WAV File Attributes ***
  SwiftSettings_GetWAVFileAttributes(&WAVFile);
  //** Get schedule attributes ***

  //MX_USART1_UART_Init();

  MX_GPDMA1_Init();
  __HAL_LINKDMA(&MdfHandle0, hdma, handle_GPDMA1_Channel11);
  MX_MDF1_Init();

  printf("MX_FREERTOS_Init function (246)\r\n");

  MX_MDFQueue_Config();
  HAL_DMAEx_List_LinkQ(&handle_GPDMA1_Channel11, &MDFQueue);

  MDF_DMAConfig();

  DeInit_MDF1_Interface();
  //ReInit_MDF1_Interface();

  osKernelInitialize();

  printf("\r\nMX_FREERTOS_Init function (258)\r\n");
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */
  /* creation of QueueDisplayMessage */
  QueueDisplayMessageHandle = osMessageQueueNew (100, sizeof(uint16_t), &QueueDisplayMessage_attributes);
  /* creation of QueueRecordMessage */
  QueueRecordMessageHandle = osMessageQueueNew (100, sizeof(uint16_t), &QueueRecordMessage_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of Record */
  RecordHandle = osThreadNew(RecordTask, NULL, &Record_attributes);

  /* creation of DisplayUpdate */
  DisplayUpdateHandle = osThreadNew(DisplayUpdateTask, NULL, &DisplayUpdate_attributes);

  /* creation of Initialize */
  InitializeHandle = osThreadNew(InitializeTask, NULL, &Initialize_attributes);

  /* creation of Scheduler */
  SchedulerHandle = osThreadNew(SchedulerTask, NULL, &Scheduler_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  printf("\r\nStarting OS Kernel (301)\r\n");
  /* add threads, ... */
  osKernelStart();
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}
/* USER CODE BEGIN Header_StartDefaultTask */
/**
* @brief Function implementing the defaultTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN defaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END defaultTask */
}

/* USER CODE BEGIN Header_SchedulerTask */
/**
* @brief Function implementing the Scheduler thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_SchedulerTask */
void SchedulerTask(void *argument)
{
  /* USER CODE BEGIN Scheduler */
  /* Infinite loop */
  for(;;)
  {
    uxBits = xEventGroupWaitBits(xEventGroup, (END_OF_DAY_SIGNAL | ALARM_A_SIGNAL), pdTRUE, pdFALSE, osWaitForever);

    if( SystemFlags.CurrentlyRecording == pdTRUE )
    {
      //Stop RecordingTask
      RecordTaskMessage.StartOrStopRecording = pdFalse;
      xQueueSend(QueueRecordMessageHandle, ( void * ) &DisplayMessage, osWaitForever);

      SystemFlags.CurrentlyRecording = FALSE;

    }


    if(uxBits & WRITEBUFFER1_SIGNAL) //Buffer is half full
  }
  /* USER CODE END Scheduler */
}

/* USER CODE BEGIN Header_RecordTask */
/**
* @brief Function implementing the Record thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_RecordTask */
void RecordTask(void *argument)
{
  /* USER CODE BEGIN Record */
  /* Infinite loop */
  EventBits_t uxBits;
      volatile uint32_t cndtrRegister = 0;
      volatile uint32_t MemoryArrayIndex = 0;
      volatile uint32_t DataSize;

      printf("\r\nInitializing RecordTask (328)\r\n");

      /* USER CODE BEGIN RecordLoop */
      /* Infinite loop */
      for(;;)
      {
        xQueueReceive(RecordMessageQueue, (void *)RecordPath, osWaitForever);

        printf("%s\r\n", RecordPath);

        if(AudioFiles_NewRecordingDirectory(&WAVFile))
        {
          xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
          osThreadTerminate(NULL); //Terminate this task
        }
        //** Create new Swift configuration text file ***
        //AudioFiles_NewConfigTextFile(&TextFile);
        //** Create new day directory ***
        if(AudioFiles_NewDayDirectory(WAVFile.Filename))
        {
          xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
          osThreadTerminate(NULL); //Terminate this task
        }
        //** Create new audio file ***
        if(AudioFiles_NewAudioFile(WAVFile.Filename, &FsFile))
        {
          xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
          osThreadTerminate(NULL); //Terminate this task
        }

        while(1)
        {
          if(xTaskNotifyWait(WRITE_BUFFER_1 | WRITE_BUFFER_2 |  STOP_RECORDING, 0, &ulNotifiedValue, osWaitForever ) == pdTRUE)
          {
            if(ulNotifiedValue == WRITE_BUFFER_1) //Buffer is half full
            {
              //__HAL_RCC_SDMMC1_CLK_ENABLE();
              FatFsResult = f_write(&FsFile, &RecBuff[0], (UINT)((REC_BUFF_SIZE_DIV2 - 1) * 2), (void *)&byteswritten);
              if(FatFsResult != FR_OK)
              {
                xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
                osThreadSuspend(RecordHandle); //Terminate current task
              }
            }

            else if(ulNotifiedValue == WRITE_BUFFER_2)//Buffer is completely full
            {
              FatFsResult = f_write(&FsFile, &RecBuff[REC_BUFF_SIZE_DIV2], (UINT)(REC_BUFF_SIZE_DIV2 * 2), (void *)&byteswritten);
              if(FatFsResult != FR_OK)
              {
                xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
              }
            }
            else if(ulNotifiedValue == STOP_RECORDING)
            {

            }

          }
        {
      }


        __HAL_RCC_SDMMC1_CLK_DISABLE();
        if(xTaskNotifyWait(WRITE_BUFFER_1 | WRITE_BUFFER_2 |  STOP_RECORDING, 0, &ulNotifiedValue, osWaitForever ) == pdTRUE)
        {
          __HAL_RCC_SDMMC1_CLK_ENABLE();

          if(ulNotifiedValue == WRITE_BUFFER_1) //Buffer is half full
          {
            //__HAL_RCC_SDMMC1_CLK_ENABLE();
            FatFsResult = f_write(&FsFile, &RecBuff[0], (UINT)((REC_BUFF_SIZE_DIV2 - 1) * 2), (void *)&byteswritten);
            if(FatFsResult != FR_OK)
            {
              xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
              osThreadSuspend(RecordHandle); //Terminate current task
            }
          }

          if(ulNotifiedValue == WRITE_BUFFER_2)//Buffer is completely full
          {
            FatFsResult = f_write(&FsFile, &RecBuff[REC_BUFF_SIZE_DIV2], (UINT)(REC_BUFF_SIZE_DIV2 * 2), (void *)&byteswritten);
            if(FatFsResult != FR_OK)
            {
              xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
            }
          }


          if(ulNotifiedValue == STOP_RECORDING)
          {
            printf("\r\nEvent on STOP_RECORDING of RecordTask %d\r\n",__LINE__);
            cndtrRegister = (handle_GPDMA1_Channel11.Instance->CBR1 / 2);

            DeInit_MDF1_Interface();

            RTC_DeactivateAlarm(RTC_ALARM_A); //Deactivate Alarm A
            RTC_DeactivateAlarm(RTC_ALARM_B); //Deactivate Alarm B

            BufferWritesPerFile = WAVFile.FileSize;

            MemoryArrayIndex = REC_BUFF_SIZE - cndtrRegister - 1;

            if(MemoryArrayIndex >= REC_BUFF_SIZE_DIV2)
            {
              DataSize = ((REC_BUFF_SIZE_DIV2 - cndtrRegister)) * 2;
              FatFsResult = f_write(&FsFile, &RecBuff[REC_BUFF_SIZE_DIV2], (UINT)(DataSize), (void *)&byteswritten);
              if(FatFsResult != FR_OK)
              {
                xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
                osThreadSuspend(RecordHandle);
              }
            }
            else
            {
              DataSize = (MemoryArrayIndex + 1) * 2;
              //FatFsResult = f_write(&FsFile, &RecBuff[0], (UINT)(DataSize * 2U), (void *)&byteswritten);
              FatFsResult = f_write(&FsFile, &RecBuff[0], (UINT)(DataSize), (void *)&byteswritten);
              if(FatFsResult != FR_OK)
              {
                xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
                osThreadSuspend(RecordHandle); //Terminate current task
              }
            }

            if(AudioFiles_WriteHeader(&SwiftErrors, &FsFile))
            {
              xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
              osThreadSuspend(RecordHandle);
            }

            if(AudioFiles_CloseFile(&SwiftErrors, &FsFile))
            {
              xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
              osThreadSuspend(RecordHandle); //Terminate current task
            }

            osThreadSuspend(RecordHandle); //Suspend this task
          }
      }
  /* USER CODE END Record */
}

/* USER CODE BEGIN Header_DisplayUpdateTask */
/**
* @brief Function implementing the DisplayUpdate thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_DisplayUpdateTask */
void DisplayUpdateTask(void *argument)
{
  /* USER CODE BEGIN DisplayUpdate */
  /* Infinite loop */
  //UBaseType_t uxHighWaterMark = 0;
  //char strHighWaterMark[4];
  //char strFullString[10];

  for(;;)
    {
      xQueueReceive(QueueDisplayMessageHandle, (void *)&DisplayMessage, osWaitForever);
      SSD1306_Clear();
      SSD1306_GotoXY (0,0);
      SSD1306_Puts (DisplayMessage.Line1, &Font_7x10, 1);
      if(DisplayMessage.LineCount == 2)
      {
        SSD1306_GotoXY (0,12);
        SSD1306_Puts (DisplayMessage.Line2, &Font_7x10, 1);
      }
      else if(DisplayMessage.LineCount == 3)
      {
        SSD1306_GotoXY (0,12);
        SSD1306_Puts (DisplayMessage.Line2, &Font_7x10, 1);
        SSD1306_GotoXY (0,24);
        SSD1306_Puts (DisplayMessage.Line3, &Font_7x10, 1);
      }
      else if(DisplayMessage.LineCount == 4)
      {
        SSD1306_GotoXY (0,12);
        SSD1306_Puts (DisplayMessage.Line2, &Font_7x10, 1);
        SSD1306_GotoXY (0,24);
        SSD1306_Puts (DisplayMessage.Line3, &Font_7x10, 1);
        SSD1306_GotoXY (0,36);
        SSD1306_Puts (DisplayMessage.Line4, &Font_7x10, 1);
      }
      else if(DisplayMessage.LineCount == 5)
      {
        SSD1306_GotoXY (0,12);
        SSD1306_Puts (DisplayMessage.Line2, &Font_7x10, 1);
        SSD1306_GotoXY (0,24);
        SSD1306_Puts (DisplayMessage.Line3, &Font_7x10, 1);
        SSD1306_GotoXY (0,36);
        SSD1306_Puts (DisplayMessage.Line4, &Font_7x10, 1);
        SSD1306_GotoXY (0,48);
        SSD1306_Puts (DisplayMessage.Line5, &Font_7x10, 1);
      }
      SSD1306_UpdateScreen();

      osDelay(5);

      /*
      osDelay(500);

      strcpy(strFullString, "REC: ");
      uxHighWaterMark = uxTaskGetStackHighWaterMark( RecordHandle );
      sprintf(strHighWaterMark, "%d", (int)uxHighWaterMark);
      strcat(strFullString, strHighWaterMark);
      SSD1306_Clear();
      SSD1306_GotoXY (0,0);
      SSD1306_Puts (strFullString, &Font_7x10, 1);
      SSD1306_UpdateScreen();

      strcpy(strFullString, "STBY: ");
      uxHighWaterMark = uxTaskGetStackHighWaterMark( StandbyHandle );
      sprintf(strHighWaterMark, "%d", (int)uxHighWaterMark);
      strcat(strFullString, strHighWaterMark);
      SSD1306_GotoXY (0,12);
      SSD1306_Puts (strFullString, &Font_7x10, 1);
      SSD1306_UpdateScreen();
       */
      /*strcpy(strFullString, "INIT: ");
      uxHighWaterMark = uxTaskGetStackHighWaterMark( InitializeHandle );
      sprintf(strHighWaterMark, "%d", (int)uxHighWaterMark);
      strcat(strFullString, strHighWaterMark);
      SSD1306_GotoXY (0,24);
      SSD1306_Puts (strFullString, &Font_7x10, 1);
      SSD1306_UpdateScreen();*/
      /*
      strcpy(strFullString, "HALL: ");
      uxHighWaterMark = uxTaskGetStackHighWaterMark( HallEffectSensorHandle );
      sprintf(strHighWaterMark, "%d", (int)uxHighWaterMark);
      strcat(strFullString, strHighWaterMark);
      SSD1306_GotoXY (0,24);
      SSD1306_Puts (strFullString, &Font_7x10, 1);
      SSD1306_UpdateScreen();
      */

    }
  /* USER CODE END DisplayUpdate */
}

/* USER CODE BEGIN Header_InitializeTask */
/**
* @brief Function implementing the Initialize thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_InitializeTask */
void InitializeTask(void *argument)
{
  /* USER CODE BEGIN Initialize */
  /* Infinite loop */
    EventBits_t uxBits;
    uint8_t InitialScheduleState;

    printf("\r\nInitializeTask initialized (1135)\r\n");

    //FRESULT ReturnResult;
    //FIL MyFile;
    /* Infinite loop */
    for(;;)
    {
      osThreadSuspend(StandbyHandle);
      osThreadSuspend(RecordHandle);

      //Check whether lid is open or closed
      if(HAL_GPIO_ReadPin(HallSensor_GPIO_Port, HallSensor_Pin) == GPIO_PIN_RESET)
      {
        SystemFlags.LidClosed = true;
        StandbyWithLidClosed();
        printf("\r\nLid is closed in InitializeTask (1151)\r\n");
      }
      else
      {
        SystemFlags.LidClosed = false;
        StandbyWithLidOpen();

        SSD1306_Init();
        SSD1306_Clear();
        SSD1306_GotoXY (0,0);
        SSD1306_Puts ("Initializing...", &Font_7x10, 1);
        SSD1306_UpdateScreen();
        printf("\r\nLid is open in InitializeTask (1163)\r\n");
        osDelay(700);
      }
      SystemFlags.CurrentlyRecording = false;

      //HAL_SD_InitCard(&hsd1);
      SDMMC1_SD_Init();
      printf("\r\nSDMMC1_SD_Init successful in InitializeTask (1170)\r\n");

      if(BSP_SD_Init() == MSD_OK)
      {
        printf("\r\nBSP_SD_Init successful in InitializeTask (1174)\r\n");
        //HAL_SD_InitCard(&hsd1); //Commented this out on 9/6/2025. Doenst seem to be needed and screws up SDCard interface if called after ejecting SD Card
        AudioFiles_MountSDCard(&SwiftErrors);

        if(SystemFlags.LidClosed == false)
        {
          SSD1306_GotoXY (0,12);
          SSD1306_Puts ("SD Card Loaded", &Font_7x10, 1);

          SSD1306_UpdateScreen();

          osDelay(700);

          SSD1306_GotoXY (0,24);
          SSD1306_Puts ("Checking schedule", &Font_7x10, 1);

          SSD1306_UpdateScreen();

          osDelay(400);
        }

        SwiftSettings_GetSwiftSchedule(&SwiftSchedule);

        if(RTC_InitializeScheduleAlarms(&SwiftSchedule) == 1)
        {
          //We are in the middle of a record period so go directly into record mode
          __HAL_RCC_SDMMC1_CLK_ENABLE();
          //** Create new recording directory ***
          if(AudioFiles_NewRecordingDirectory(&WAVFile))
          {
            xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
            osThreadTerminate(NULL); //Terminate this task
          }
          //** Create new Swift configuration text file ***
          //AudioFiles_NewConfigTextFile(&TextFile);
          //** Create new day directory ***
          if(AudioFiles_NewDayDirectory(WAVFile.Filename))
          {
            xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
            osThreadTerminate(NULL); //Terminate this task
          }
          //** Create new audio file ***
          if(AudioFiles_NewAudioFile(WAVFile.Filename, &FsFile))
          {
            xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
            osThreadTerminate(NULL); //Terminate this task
          }
        }
        //** Initialize record schedule
        InitialScheduleState = RTC_InitializeScheduleAlarms(&SwiftSchedule);

      switch (InitialScheduleState)
      {
      case 1: //We are in the middle of a record period so go directly into record mode

        printf("\r\nInitializeTask case 1: We are in the middle of a record period (1203)\r\n");

        __HAL_RCC_SDMMC1_CLK_ENABLE();
        //** Create new recording directory ***
        if(AudioFiles_NewRecordingDirectory(&WAVFile))
        {
          xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
          osThreadTerminate(NULL); //Terminate this task
        }
        //** Create new Swift configuration text file ***
        //AudioFiles_NewConfigTextFile(&TextFile);
        //** Create new day directory ***
        if(AudioFiles_NewDayDirectory(WAVFile.Filename))
        {
          xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
          osThreadTerminate(NULL); //Terminate this task
        }
        //** Create new audio file ***
        if(AudioFiles_NewAudioFile(WAVFile.Filename, &FsFile))
        {
          xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
          osThreadTerminate(NULL); //Terminate this task
        }

        printf("\r\nInitializeTask case 1. Files created successfully (1227)\r\n");

        WAVFile.DirectoryLevel = 2;
        //** Initialize number of buffer writes per file ***
        BufferWritesPerFile = WAVFile.FileSize;

        //ReInit_MDF1_Interface();
        if(SystemFlags.LidClosed == false)
        {
          RecordingWithLidOpen();
        }
        else
        {
          RecordingWithLidClosed();
        }

        SystemFlags.CurrentlyRecording = true;

        HAL_MDF_AcqStart_DMA(&MdfHandle0, &MdfFilterConfig0, &DMAConfig);

          //HAL_PWREx_ConfigSRDDomain(PWR_SRD_DOMAIN_RUN);

        strcpy(DisplayMessage.Line1, "");
        strcpy(DisplayMessage.Line2, "");
        strcpy(DisplayMessage.Line3, "Recording...");
        strcpy(DisplayMessage.Line4, "");
        DisplayMessage.LineCount = 4;
        if(SystemFlags.LidClosed == false)
        {
          xQueueSend(QueueDisplayMessageHandle, ( void * ) &DisplayMessage, osWaitForever);
        }

        printf("\r\nInitializeTask case 1. About to go into RecordTask (1259)\r\n");

        HAL_MDF_AcqStart_IT(&MdfHandle1, &MdfFilterConfig1);

        //** Blink green LED ***
        //xEventGroupSetBits(xEventGroup, GRN_LED_SIGNAL);

        //** Start RecordLoopTask ***
        osThreadResume(RecordHandle);
        //osThreadTerminate(NULL); //Terminate this task
        //osThreadSuspend(NULL);

        break;

      case 2: // We booted in standby period between record periods. Wait for ALARM_A then start recording
        //** Blink blue LED ***
        //xEventGroupSetBits(xEventGroup, BLU_LED_SIGNAL);
        printf("\r\nInitializeTask case 2: We are in standby period between record periods (1276)\r\n");
        strcpy(DisplayMessage.Line1, "");
        strcpy(DisplayMessage.Line2, "");
        strcpy(DisplayMessage.Line3, "Standby...");
        strcpy(DisplayMessage.Line4, "");
        DisplayMessage.LineCount = 4;
        if(SystemFlags.LidClosed == false)
        {
          xQueueSend(QueueDisplayMessageHandle, ( void * ) &DisplayMessage, osWaitForever);
        }
        SystemFlags.CurrentlyRecording = false;
        //** Wait for pushbutton or Alarm A ***
        uxBits = xEventGroupWaitBits(xEventGroup, CENTER_BTN_SIGNAL | ALARM_A_SIGNAL | NEW_VOICE_MEMO_SIGNAL, pdTRUE, pdFALSE, osWaitForever);
        printf("\r\nInitializeTask case 2: CENTER_BTN_SIGNAL | ALARM_A_SIGNAL | NEW_VOICE_MEMO_SIGNAL event occurred (1289)\r\n");
        __HAL_RCC_SDMMC1_CLK_ENABLE();

        if(uxBits & (CENTER_BTN_SIGNAL | ALARM_A_SIGNAL))
        {
          printf("\r\nInitializeTask case 2: CENTER_BTN_SIGNAL | ALARM_A_SIGNAL event occurred (1294)\r\n");
          //StatusLED_SolidGreenLED();
          strcpy(DisplayMessage.Line1, "");
          strcpy(DisplayMessage.Line2, "");
          strcpy(DisplayMessage.Line3, "Recording...");
          strcpy(DisplayMessage.Line4, "");
          DisplayMessage.LineCount = 4;
          if(SystemFlags.LidClosed == false)
          {
            xQueueSend(QueueDisplayMessageHandle, ( void * ) &DisplayMessage, osWaitForever);
            RecordingWithLidOpen();
          }
          else
          {
            RecordingWithLidClosed();
          }

          //** Create new recording directory ***
          if(AudioFiles_NewRecordingDirectory(&WAVFile))
          {
            xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
            osThreadSuspend(InitializeHandle); //Suspend this task
          }
          //** Create new day directory ***
          if(AudioFiles_NewDayDirectory(WAVFile.Filename))
          {
            xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
            //osThreadTerminate(InitScheduleHandle); //Terminate this task
          }
          //** Create new audio file ***
          if(AudioFiles_NewAudioFile(WAVFile.Filename, &FsFile))
          {
            xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
            osThreadSuspend(InitializeHandle); //Suspend this task
          }
          printf("\r\nInitializeTask case 2: Files created successfully (1329)\r\n");
          //** Initialize number of buffer writes per file ***
          BufferWritesPerFile = WAVFile.FileSize;
          if(uxBits & CENTER_BTN_SIGNAL)
          {
            printf("\r\nInitializeTask case 2: CENTER_BTN_SIGNAL occurred (1334)\r\n");
            SwiftSchedule.EndsAtMidnightFlag = 0;
            SwiftSchedule.StartsAtMidnightFlag = 1;
            SwiftSchedule.CurrentRecordNumber = 0;
            SwiftSchedule.NumberOfRecords = 1;
            SwiftSchedule.ScheduleType = SCHEDULE_TYPE_CONTINUOUS;
            SwiftSchedule.StartDate.Year = 0;
            SwiftSchedule.StartDate.Date = 1;
            SwiftSchedule.StartDate.Month = 1;
            SwiftSchedule.StartDate.WeekDay = 1;
            SwiftSchedule.StopDate.Year = 99;
            SwiftSchedule.StopDate.Date = 1;
            SwiftSchedule.StopDate.Month = 1;
            SwiftSchedule.StopDate.WeekDay = 1;
            SwiftSchedule.StartStopTimes[0].RecordingStartTime.Hours = 0x00;
            SwiftSchedule.StartStopTimes[0].RecordingStartTime.Minutes = 0x00;
            SwiftSchedule.StartStopTimes[0].RecordingStopTime.Hours = 0x23;
            SwiftSchedule.StartStopTimes[0].RecordingStopTime.Minutes = 0x59;
            SetNextStopTime(&SwiftSchedule);
            //RTC_DeactivateAlarm(RTC_ALARM_A); //Deactivate Alarm A
            RTC_EnableAlarmB();
          }
          else//Must be ALARM_A_SIGNAL
          {
            printf("\r\nInitializeTask case 2: ALARM_A_SIGNAL occurred (1358)\r\n");
            //Set next stop time
            SetNextStopTime(&SwiftSchedule);
          }

          SystemFlags.CurrentlyRecording = true;

          //InitializeCodecAndDMA();
          //ReInit_MDF1_Interface();
          printf("\r\nInitializeTask case 2: About to go into RecordTask (1367)\r\n");

          HAL_MDF_AcqStart_DMA(&MdfHandle0, &MdfFilterConfig0, &DMAConfig);

          HAL_MDF_AcqStart_IT(&MdfHandle1, &MdfFilterConfig1);

          //** Blink green led ***
          //xEventGroupSetBits(xEventGroup, GRN_LED_SIGNAL);

          //** Start RecordLoopTask ***
          osThreadResume(RecordHandle);
        }
        else //This must be NEW_VOICE_MEMO_SIGNAL
        {
          //VoiceMemoHandle = osThreadNew(NewVoiceMemoTask, NULL, &VoiceMemo_attributes);
        }

        osThreadSuspend(InitializeHandle); //Suspend this task
        break;

      default:
        printf("\r\nInitializeTask default case: Set recorder up for standby mode (1388)\r\n");
        //** Blink blue LED ***
        strcpy(DisplayMessage.Line1, "");
        strcpy(DisplayMessage.Line2, "");
        strcpy(DisplayMessage.Line3, "Standby...");
        strcpy(DisplayMessage.Line4, "");
        DisplayMessage.LineCount = 4;

        if(SystemFlags.LidClosed == false)
        {
          StandbyWithLidOpen();
          xQueueSend(QueueDisplayMessageHandle, ( void * ) &DisplayMessage, osWaitForever);
        }
        else
        {
          StandbyWithLidClosed();
        }

        SystemFlags.CurrentlyRecording = false;

        //** Put CODEC into Low Power Mode ***
        //AudioCodec_SleepMode();
        //HAL_MDF_MspDeInit(MDF_HandleTypeDef* mdfHandle)

        //DeInit_MDF1_Interface();

        //HAL_I2C_MspDeInit(&hi2c1);
        //** Start standby thread ***
        printf("\r\nInitializeTask default case: Go to StandbyTask (1416)\r\n");
        osThreadResume(StandbyHandle);
        //osThreadTerminate(NULL); //Terminate this task
        osThreadSuspend(InitializeHandle);
      }
      }
      else
      {
        printf("\r\nInitializeTask SD Card Error (1424)\r\n");

        HAL_SD_DeInit(&hsd1);

        strcpy(DisplayMessage.Line1, "SD Card Error");
        strcpy(DisplayMessage.Line2, "");
        strcpy(DisplayMessage.Line3, "");
        strcpy(DisplayMessage.Line4, "");
        DisplayMessage.LineCount = 4;

        if(SystemFlags.LidClosed == false)
        {
          StandbyWithLidOpen();
          xQueueSend(QueueDisplayMessageHandle, ( void * ) &DisplayMessage, osWaitForever);
        }
        else
        {
          StandbyWithLidClosed();
        }
        SystemFlags.CurrentlyRecording = false;

      }
      osThreadSuspend(InitializeHandle);
    }
  /* USER CODE END Initialize */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
static void SDMMC1_SD_Init(void)
{
  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
  hsd1.Init.ClockDiv = 2;
  hsd1.Init.TranceiverPresent = SDMMC_TRANSCEIVER_NOT_PRESENT;
  /*if (HAL_SD_Init(&hsd1) != HAL_OK)
  {
    return 0;
  }
  else
  {
    return 1;
  }*/
}

void ReInit_MDF1_Interface()
{
  //MX_GPDMA1_Init();
  //__HAL_RCC_GPDMA1_CLK_ENABLE();
  //HAL_NVIC_EnableIRQ(GPDMA1_Channel11_IRQn);

  MX_GPDMA1_Init();

  //HAL_MDF_MspInit(&MdfHandle0);
  //HAL_MDF_MspInit(&MdfHandle1);

  MX_MDF1_Init();
}

void DeInit_MDF1_Interface()
{
  HAL_MDF_DeInit(&MdfHandle0);
  HAL_MDF_DeInit(&MdfHandle1);

  //MX_GPDMA1_DeInit();
  //HAL_MDF_DeInit(&MdfHandle0);
  MX_GPDMA1_DeInit();
}

static void MDF_DMAConfig(void)
{
  /* Initialize DMA configuration parameters */
  DMAConfig.Address                              = (uint32_t)&RecBuff[0];
  //DMAConfig.DataLength                           = (REC_BUFF_SIZE * 4U);
  DMAConfig.DataLength                           = (REC_BUFF_SIZE * 2U);
  DMAConfig.MsbOnly                              = ENABLE;
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtclock)
{
  //Alarm A is used to keep track of on/off times during the day
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xEventGroupSetBitsFromISR(xEventGroup, ALARM_A_SIGNAL, &xHigherPriorityTaskWoken);
}

void HAL_RTCEx_AlarmBEventCallback(RTC_HandleTypeDef *hrtclock)
{
  //Alarm B will be used for 24 hour end-of-day alarm.
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xEventGroupSetBitsFromISR(xEventGroup, END_OF_DAY_SIGNAL, &xHigherPriorityTaskWoken);
}

void HAL_MDF_AcqHalfCpltCallback(MDF_HandleTypeDef *hmdf)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  //xEventGroupSetBitsFromISR(xEventGroup, WRITEBUFFER1_SIGNAL, &xHigherPriorityTaskWoken);
  xTaskNotifyFromISR(RecordHandle, WRITEBUFFER1_SIGNAL, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
  //xTaskNotifyFromISR( xTaskToNotify, ulValue, eAction, pxHigherPriorityTaskWoken )
  //BaseType_t xEventGroupSetBitsFromISR( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToSet, BaseType_t * pxHigherPriorityTaskWoken )
}

void HAL_MDF_AcqCpltCallback(MDF_HandleTypeDef *hmdf)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  //xEventGroupSetBitsFromISR(xEventGroup, WRITEBUFFER2_SIGNAL, &xHigherPriorityTaskWoken);
  xTaskNotifyFromISR(RecordHandle, WRITEBUFFER2_SIGNAL, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if(GPIO_Pin == HallSensor_Pin)
  {
    xEventGroupSetBitsFromISR(xEventGroup, LID_OPEN, &xHigherPriorityTaskWoken);
  }
  else if(GPIO_Pin == USB_VBUS_Pin)
  {
    NVIC_SystemReset();
  }
  //else if(GPIO_Pin == SDCARD_DETECT_Pin)
  //{
    //xEventGroupSetBitsFromISR(xEventGroup, SDCARD_OUT, &xHigherPriorityTaskWoken);
  //}
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if(GPIO_Pin == HallSensor_Pin)
  {
    xEventGroupSetBitsFromISR(xEventGroup, LID_CLOSED, &xHigherPriorityTaskWoken);
  }
  else if(GPIO_Pin == USB_VBUS_Pin)
  {
    NVIC_SystemReset();
  }
 // else if(GPIO_Pin == SDCARD_DETECT_Pin)
  //{
    //xEventGroupSetBitsFromISR(xEventGroup, SDCARD_IN, &xHigherPriorityTaskWoken);
  //}
  //else if(GPIO_Pin == PUSHBUTTON1_Pin)
  //{
    //HAL_NVIC_DisableIRQ(EXTI6_IRQn);
    //xEventGroupSetBitsFromISR(xEventGroup, LEFT_BTN_SIGNAL, &xHigherPriorityTaskWoken);
  //}

}

PUTCHAR_PROTOTYPE
{
 /* Place your implementation of fputc here */
 /* e.g. write a character to the USART1 and Loop until the end of transmission */
 HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
 return ch;
}

/* USER CODE END Application */

