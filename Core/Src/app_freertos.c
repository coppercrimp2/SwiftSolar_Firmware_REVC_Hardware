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

#define REC_BUFF_SIZE        (32764)
#define REC_BUFF_SIZE_DIV2   (REC_BUFF_SIZE / 2)

#define BTN_PRESSED (0)
#define BTN_NOT_PRESSED (1)

#define BTN_PRESSED_Pos                         0U
#define BTN_PRESSED_Msk                         (1UL << BTN_PRESSED_Pos)
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

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
  .stack_size = 500 * 4
};
/* Definitions for Standby */
osThreadId_t StandbyHandle;
const osThreadAttr_t Standby_attributes = {
  .name = "Standby",
  .priority = (osPriority_t) osPriorityLow1,
  .stack_size = 500 * 4
};
/* Definitions for Error */
osThreadId_t ErrorHandle;
const osThreadAttr_t Error_attributes = {
  .name = "Error",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 4
};
/* Definitions for DisplayUpdate */
osThreadId_t DisplayUpdateHandle;
const osThreadAttr_t DisplayUpdate_attributes = {
  .name = "DisplayUpdate",
  .priority = (osPriority_t) osPriorityLow1,
  .stack_size = 256 * 4
};
/* Definitions for HallEffectSensor */
osThreadId_t HallEffectSensorHandle;
const osThreadAttr_t HallEffectSensor_attributes = {
  .name = "HallEffectSensor",
  .priority = (osPriority_t) osPriorityLow1,
  .stack_size = 500 * 4
};
/* Definitions for Initialize */
osThreadId_t InitializeHandle;
const osThreadAttr_t Initialize_attributes = {
  .name = "Initialize",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 600 * 4
};
/* Definitions for QueueDisplayMessage */
osMessageQueueId_t QueueDisplayMessageHandle;
const osMessageQueueAttr_t QueueDisplayMessage_attributes = {
  .name = "QueueDisplayMessage"
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
  bool LidClosed;
  bool CurrentlyRecording;
};

struct system_flags SystemFlags;

static int16_t RecBuff[REC_BUFF_SIZE] = {0};
__IO uint32_t  DmaRecHalfBuffCplt  = 0;
__IO uint32_t  DmaRecBuffCplt      = 0;

extern DMA_HandleTypeDef handle_GPDMA1_Channel11;

void WriteDebugFile(void);
void InitializeCodecAndDMA(void);
void AudioCodec_SleepMode(void);

uint32_t byteswritten;
FRESULT FatFsResult;

Message DisplayMessage;

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
  xEventGroup = xEventGroupCreate();

  //** Get WAV File Attributes ***
  SwiftSettings_GetWAVFileAttributes(&WAVFile);
  //** Get schedule attributes ***

  MX_GPDMA1_Init();
  __HAL_LINKDMA(&MdfHandle0, hdma, handle_GPDMA1_Channel11);
  MX_MDF1_Init();

  MX_MDFQueue_Config();
  HAL_DMAEx_List_LinkQ(&handle_GPDMA1_Channel11, &MDFQueue);

  MDF_DMAConfig();

  DeInit_MDF1_Interface();
  //ReInit_MDF1_Interface();

  osKernelInitialize();
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

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of Record */
  RecordHandle = osThreadNew(RecordTask, NULL, &Record_attributes);

  /* creation of Standby */
  StandbyHandle = osThreadNew(StandbyTask, NULL, &Standby_attributes);

  /* creation of Error */
  ErrorHandle = osThreadNew(ErrorTask, NULL, &Error_attributes);

  /* creation of DisplayUpdate */
  DisplayUpdateHandle = osThreadNew(DisplayUpdateTask, NULL, &DisplayUpdate_attributes);

  /* creation of HallEffectSensor */
  HallEffectSensorHandle = osThreadNew(HallEffectSensorTask, NULL, &HallEffectSensor_attributes);

  /* creation of Initialize */
  InitializeHandle = osThreadNew(InitializeTask, NULL, &Initialize_attributes);

  /* creation of ScanButtons */
  ScanButtonsHandle = osThreadNew(ScanButtonsTask, NULL, &ScanButtons_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  ScanButtonsHandle = osThreadNew(ScanButtonsTask, NULL, &ScanButtons_attributes);
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
  //for(;;)
  //{
    //osDelay(1);
  //}
  /* USER CODE END defaultTask */
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

      /* USER CODE BEGIN RecordLoop */
      /* Infinite loop */
      for(;;)
      {
        __HAL_RCC_SDMMC1_CLK_DISABLE();
        uxBits = xEventGroupWaitBits(xEventGroup, (WRITEBUFFER1_SIGNAL | WRITEBUFFER2_SIGNAL | CENTER_BTN_SIGNAL | END_OF_DAY_SIGNAL | ALARM_A_SIGNAL | NEW_VOICE_MEMO_SIGNAL | LOW_BATTERY_SIGNAL), pdTRUE, pdFALSE, osWaitForever);
        __HAL_RCC_SDMMC1_CLK_ENABLE();


        if(uxBits & WRITEBUFFER1_SIGNAL) //Buffer is half full
        {
          //__HAL_RCC_SDMMC1_CLK_ENABLE();
          FatFsResult = f_write(&FsFile, &RecBuff[0], (UINT)((REC_BUFF_SIZE_DIV2 - 1) * 2), (void *)&byteswritten);
          if(FatFsResult != FR_OK)
          {
            xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
            osThreadSuspend(RecordHandle); //Terminate current task
          }
        }

        if(uxBits & WRITEBUFFER2_SIGNAL)//Buffer is completely full
        {
          FatFsResult = f_write(&FsFile, &RecBuff[REC_BUFF_SIZE_DIV2], (UINT)(REC_BUFF_SIZE_DIV2 * 2), (void *)&byteswritten);
          if(FatFsResult != FR_OK)
          {
            xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
          }

          if(--BufferWritesPerFile == 0)
          {
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
            if(AudioFiles_NewAudioFile(WAVFile.Filename, &FsFile))
            {
              xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
              osThreadSuspend(RecordHandle); //Terminate current task
            }

            BufferWritesPerFile = WAVFile.FileSize;
          }
        }


        if(uxBits & (CENTER_BTN_SIGNAL | NEW_VOICE_MEMO_SIGNAL | LOW_BATTERY_SIGNAL))
        {
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

          if(uxBits & CENTER_BTN_SIGNAL)
          {
            //Update stop date to show that the schedule recording now starts in the past
            SwiftSchedule.StartDate.Year = 0;
            SwiftSchedule.StartDate.Date = 1;
            SwiftSchedule.StartDate.Month = 1;
            SwiftSchedule.StartDate.WeekDay = 1;
            SwiftSchedule.StopDate.Year = 1;
            SwiftSchedule.StopDate.Date = 1;
            SwiftSchedule.StopDate.Month = 1;
            SwiftSchedule.StopDate.WeekDay = 1;

            WAVFile.RecordingDirFlag = 0;

            strcpy(DisplayMessage.Line1, "Recording stopped");
            strcpy(DisplayMessage.Line2, "");
            strcpy(DisplayMessage.Line3, "");
            strcpy(DisplayMessage.Line4, "");
            DisplayMessage.LineCount = 4;
            if(SystemFlags.LidClosed == false)
            {
              xQueueSend(QueueDisplayMessageHandle, ( void * ) &DisplayMessage, osWaitForever);
              StandbyWithLidOpen();
            }
            else
            {
              StandbyWithLidClosed();
            }

            SystemFlags.CurrentlyRecording = false;

            //** Put CODEC into Low Power Mode ***
            osThreadResume(StandbyHandle); //Resume standby thread
          }
          else if(uxBits & NEW_VOICE_MEMO_SIGNAL)
          {
            //osThreadResume(VoiceMemoHandle);
          }
          else
          {
            //osThreadResume(LowBatteryHandle);
            //LOW BATTERY THREAD DOES NOT EXIST YET WE'LL NEED TO ADD THIS
          }

          osThreadSuspend(RecordHandle); //Suspend this task
        }

        else if(uxBits & END_OF_DAY_SIGNAL)
        {
          cndtrRegister = (handle_GPDMA1_Channel11.Instance->CBR1 / 2);
          MemoryArrayIndex = REC_BUFF_SIZE - cndtrRegister - 1;

          DeInit_MDF1_Interface();

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
          BufferWritesPerFile = WAVFile.FileSize;

          if((SwiftSchedule.StartsAtMidnightFlag) && (RTC_CheckIfBetweenDates(SwiftSchedule.StartDate, SwiftSchedule.StopDate)))
          {
            //f_chdir("..");//back up a directory <- THIS DOESNT WORK WITH EXFAT FILESYSTEM!!!
            f_chdir((char const*)WAVFile.RecordingDirName); // <- THIS REPLACES THE ABOVE LINE

            if(AudioFiles_NewDayDirectory(WAVFile.Filename))
            {
              xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
              osThreadSuspend(RecordHandle); //Terminate this task
            }

            if(AudioFiles_NewAudioFile(WAVFile.Filename, &FsFile))
            {
              xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
              osThreadSuspend(RecordHandle); //Terminate current task
            }

            //ReInit_MDF1_Interface();

            uxBits = xEventGroupWaitBits(xEventGroup, (WRITEBUFFER1_SIGNAL | WRITEBUFFER2_SIGNAL), pdTRUE, pdFALSE, osWaitForever);

            if(uxBits & WRITEBUFFER1_SIGNAL)
            {
              DataSize = (REC_BUFF_SIZE_DIV2 - (MemoryArrayIndex + 1)) * 2;
              FatFsResult = f_write(&FsFile, &RecBuff[(MemoryArrayIndex * 2)], (UINT)(DataSize), (void *)&byteswritten);
              if(FatFsResult != FR_OK)
              {
                xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
                osThreadSuspend(RecordHandle); //Terminate current task
              }
            }
            else if(uxBits & WRITEBUFFER2_SIGNAL)
            {
              DataSize = (REC_BUFF_SIZE - (MemoryArrayIndex + 1)) * 2;
              FatFsResult = f_write(&FsFile, &RecBuff[(MemoryArrayIndex * 2)], (UINT)(DataSize), (void *)&byteswritten);
              if(FatFsResult != FR_OK)
              {
                xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
                osThreadSuspend(RecordHandle); //Terminate current task
              }
            }
            SwiftSchedule.CurrentRecordNumber = 0;
            SetNextStopTime(&SwiftSchedule);
          }
          else
          {
            //** Power down DMA and I2S ***
            //HAL_DMA_Abort(&hdma_sai2_a);
            //__HAL_SAI_DISABLE(&hsai_BlockA2);
            //HAL_SAI_MspDeInit(&hsai_BlockA2);
            //hsai_BlockA2.State = HAL_SAI_STATE_RESET;
            if(SystemFlags.LidClosed == false)
            {
              StandbyWithLidOpen();
            }
            else
            {
              StandbyWithLidClosed();
            }
            SystemFlags.CurrentlyRecording = false;
            //** Set next start time ***
            SetNextStartTime(&SwiftSchedule);

            //xEventGroupSetBits(xEventGroup, BLU_LED_SIGNAL);
            //** Put CODEC into Low Power Mode ***

            osThreadResume(StandbyHandle); //Resume standby thread
            osThreadSuspend(RecordHandle); //Suspend this task
          }
        }
        else if(uxBits & ALARM_A_SIGNAL)
        {
          cndtrRegister = (handle_GPDMA1_Channel11.Instance->CBR1 / 2);
          DeInit_MDF1_Interface();

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
          BufferWritesPerFile = WAVFile.FileSize;

          //** Set next start time if this isnt the last start time in schedule. If it is the last one, then start time will be set in Standby Task***
          if((SwiftSchedule.CurrentRecordNumber + 1) != SwiftSchedule.NumberOfRecords)
            SetNextStartTime(&SwiftSchedule);

          strcpy(DisplayMessage.Line1, "");
          strcpy(DisplayMessage.Line2, "");
          strcpy(DisplayMessage.Line3, "Standby...");
          strcpy(DisplayMessage.Line4, "");
          DisplayMessage.LineCount = 4;
          if(SystemFlags.LidClosed == false)
          {
            xQueueSend(QueueDisplayMessageHandle, ( void * ) &DisplayMessage, osWaitForever);
            StandbyWithLidOpen();
          }
          else
          {
            StandbyWithLidClosed();
          }

          SystemFlags.CurrentlyRecording = false;

          osThreadResume(StandbyHandle); //Resume standby thread
          osThreadSuspend(RecordHandle); //Suspend this task
        }
        else //This must be NEW_VOICE_MEMO_SIGNAL
        {

        }
      }
  /* USER CODE END Record */
}

/* USER CODE BEGIN Header_StandbyTask */
/**
* @brief Function implementing the Standby thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StandbyTask */
void StandbyTask(void *argument)
{
  /* USER CODE BEGIN Standby */
  /* Infinite loop */
  EventBits_t uxBits;

      for(;;)
      {
        __HAL_RCC_SDMMC1_CLK_DISABLE();

        uxBits = xEventGroupWaitBits(xEventGroup, CENTER_BTN_SIGNAL | ALARM_A_SIGNAL | END_OF_DAY_SIGNAL | NEW_VOICE_MEMO_SIGNAL | LOW_BATTERY_SIGNAL, pdTRUE, pdFALSE, osWaitForever);

        /*__HAL_RCC_SDMMC1_CLK_ENABLE();

        BufferWritesPerFile = WAVFile.FileSize;
        if(SystemFlags.LidClosed == false)
        {
          RecordingWithLidOpen();
        }
        else
        {
          RecordingWithLidClosed();
        }

        osDelay(50);
        */

        //BSP_SD_Init();
        //AudioFiles_MountSDCard(&SwiftErrors);

        switch (uxBits)
        {
          case (END_OF_DAY_SIGNAL):
            if(RTC_CheckIfBetweenDates(SwiftSchedule.StartDate, SwiftSchedule.StopDate))//Is current date between StartDate and StopDate?
            {
              //if(WAVFile.DirectoryLevel == 2)
              //{
                //f_chdir("..");//back up a directory
              //}
              if(AudioFiles_NewRecordingDirectory(&WAVFile))
              {
                xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
                osThreadSuspend(StandbyHandle); //Terminate this task
              }
              //** Create new day directory ***
              if(AudioFiles_NewDayDirectory(WAVFile.Filename))
              {
                xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
                osThreadSuspend(StandbyHandle); //Terminate this task
              }
              WAVFile.DirectoryLevel = 2;
              if(SwiftSchedule.StartsAtMidnightFlag == 1)
              {
                strcpy(DisplayMessage.Line1, "");
                strcpy(DisplayMessage.Line2, "");
                strcpy(DisplayMessage.Line3, "Recording");
                strcpy(DisplayMessage.Line4, "");
                DisplayMessage.LineCount = 4;
                if(SystemFlags.LidClosed == false)
                {
                  RecordingWithLidOpen();
                  xQueueSend(QueueDisplayMessageHandle, ( void * ) &DisplayMessage, osWaitForever);
                }
                else
                {
                  RecordingWithLidClosed();
                }

                SystemFlags.CurrentlyRecording = true;

                //** Create new audio file ***
                if(AudioFiles_NewAudioFile(WAVFile.Filename, &FsFile))
                {
                  xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
                  osThreadSuspend(StandbyHandle); //Terminate this task
                }

                SwiftSchedule.CurrentRecordNumber = 0;
                SetNextStopTime(&SwiftSchedule);

                //InitializeCodecAndDMA();
                //ReInit_MDF1_Interface();
                HAL_MDF_AcqStart_DMA(&MdfHandle0, &MdfFilterConfig0, &DMAConfig);
                HAL_MDF_AcqStart_IT(&MdfHandle1, &MdfFilterConfig1);

                //** Start RecordLoopTask ***
                osThreadResume(RecordHandle);
                osThreadSuspend(StandbyHandle); //Suspend this task
              }
             else
              {// HERE LIES THE PROBLEM!!!!
                SwiftSchedule.CurrentRecordNumber = SwiftSchedule.NumberOfRecords;
                SetNextStartTime(&SwiftSchedule);
              }
            }
            break;

          case (CENTER_BTN_SIGNAL):

            strcpy(DisplayMessage.Line1, "Starting new");
            strcpy(DisplayMessage.Line2, "continuous");
            strcpy(DisplayMessage.Line3, "recording");
            strcpy(DisplayMessage.Line4, "");
            DisplayMessage.LineCount = 4;
            if(SystemFlags.LidClosed == false)
            {
              xQueueSend(QueueDisplayMessageHandle, ( void * ) &DisplayMessage, osWaitForever);
            }

            osDelay(700);

            //** Create new recording directory ***
            if(AudioFiles_NewRecordingDirectory(&WAVFile))
            {
              xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
              osThreadSuspend(StandbyHandle); //Terminate this task
            }
            //** Create new day directory ***
            if(AudioFiles_NewDayDirectory(WAVFile.Filename))
            {
              xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
              osThreadSuspend(StandbyHandle); //Terminate this task
            }
            WAVFile.DirectoryLevel = 2;
            //** Create new audio file ***
            if(AudioFiles_NewAudioFile(WAVFile.Filename, &FsFile))
            {
              xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
              osThreadSuspend(StandbyHandle); //Terminate this task
            }
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

            //RTC_DeactivateAlarm(RTC_ALARM_A); //Deactivate Alarm A
            SetNextStopTime(&SwiftSchedule);
            RTC_EnableAlarmB();

            //ReInit_MDF1_Interface();

            strcpy(DisplayMessage.Line1, "");
            strcpy(DisplayMessage.Line2, "");
            strcpy(DisplayMessage.Line3, "Recording...");
            strcpy(DisplayMessage.Line4, "");
            DisplayMessage.LineCount = 4;
            if(SystemFlags.LidClosed == false)
            {
              //RecordingWithLidOpen();
              xQueueSend(QueueDisplayMessageHandle, ( void * ) &DisplayMessage, osWaitForever);
            }
            //else
            //{
              //RecordingWithLidClosed();
            //}

            SystemFlags.CurrentlyRecording = true;

            HAL_MDF_AcqStart_DMA(&MdfHandle0, &MdfFilterConfig0, &DMAConfig);

            HAL_MDF_AcqStart_IT(&MdfHandle1, &MdfFilterConfig1);

            //** Start RecordLoopTask ***
            osThreadResume(RecordHandle);
            osThreadSuspend(StandbyHandle); //Suspend this task
            break;

          case (ALARM_A_SIGNAL):

            strcpy(DisplayMessage.Line1, "");
            strcpy(DisplayMessage.Line2, "");
            strcpy(DisplayMessage.Line3, "Recording...");
            strcpy(DisplayMessage.Line4, "");
            DisplayMessage.LineCount = 4;
            if(SystemFlags.LidClosed == false)
            {
              xQueueSend(QueueDisplayMessageHandle, ( void * ) &DisplayMessage, osWaitForever);
            }

            __HAL_RCC_SDMMC1_CLK_ENABLE();

            BufferWritesPerFile = WAVFile.FileSize;
            if(SystemFlags.LidClosed == false)
            {
              RecordingWithLidOpen();
            }
            else
            {
              RecordingWithLidClosed();
            }

            //osDelay(50);

            if(AudioFiles_NewAudioFile(WAVFile.Filename, &FsFile))
            {
              xEventGroupSetBits(xEventGroup, ERROR_SIGNAL);
              osThreadSuspend(StandbyHandle); //Terminate this task
            }

            SetNextStopTime(&SwiftSchedule);

            SystemFlags.CurrentlyRecording = true;

            HAL_MDF_AcqStart_DMA(&MdfHandle0, &MdfFilterConfig0, &DMAConfig);
            HAL_MDF_AcqStart_IT(&MdfHandle1, &MdfFilterConfig1);

            //** Start RecordLoopTask ***
            osThreadResume(RecordHandle);
            osThreadSuspend(StandbyHandle); //Suspend this task
            break;

          case (LOW_BATTERY_SIGNAL):
            //osThreadResume(LowBatteryHandle);
            osThreadSuspend(StandbyHandle);
            break;

          case (NEW_VOICE_MEMO_SIGNAL):
            //osThreadDef(VoiceMemo, NewVoiceMemoTask, osPriorityNormal, 0, 512);
            //VoiceMemoHandle = osThreadCreate(osThread(VoiceMemo), NULL);

            //osThreadResume(VoiceMemoHandle);
            osThreadSuspend(StandbyHandle);
            break;
        }
      }
  /* USER CODE END Standby */
}

/* USER CODE BEGIN Header_ErrorTask */
/**
* @brief Function implementing the Error thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ErrorTask */
void ErrorTask(void *argument)
{
  /* USER CODE BEGIN Error */
  /* Infinite loop */
  for(;;)
  {
    xEventGroupWaitBits(xEventGroup, ERROR_SIGNAL, pdTRUE, pdTRUE, osWaitForever);

    //osThreadDef(BlinkRedLED, BlinkRedLEDTask, osPriorityNormal, 0, 128);
    //BlinkRedLEDHandle = osThreadCreate(osThread(BlinkRedLED), NULL);
    //xEventGroupSetBits(xEventGroup, RED_LED_SIGNAL);

    //osThreadTerminate(ErrorHandle);
  }
  /* USER CODE END Error */
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

/* USER CODE BEGIN Header_HallEffectSensorTask */
/**
* @brief Function implementing the HallEffectSensor thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_HallEffectSensorTask */
void HallEffectSensorTask(void *argument)
{
  /* USER CODE BEGIN HallEffectSensor */
  /* Infinite loop */
  EventBits_t uxBits;

  for(;;)
  {
  uxBits = xEventGroupWaitBits(xEventGroup, LID_OPEN | LID_CLOSED, pdTRUE, pdFALSE, osWaitForever);

  if(uxBits & LID_OPEN)
  {
    //HAL_GPIO_WritePin(OLED_13V_EN_GPIO_Port, OLED_13V_EN_Pin, GPIO_PIN_SET);
    //MX_I2C3_Init();
    //HAL_I2C_MspInit(&hi2c3);
    //HAL_I2C_Init(&hi2c3);
    //__HAL_RCC_I2C3_CLK_ENABLE();
    //HAL_GPIO_DeInit(GPIOC, GPIO_PIN_1);
    //HAL_GPIO_DeInit(GPIOA, GPIO_PIN_7);
    //__HAL_RCC_I2C2_CLK_ENABLE();
    if(SystemFlags.CurrentlyRecording == true)
      RecordingWithLidOpen();
    else
      StandbyWithLidOpen();

    SSD1306_Init(); //<=ADD THIS BACK
    SystemFlags.LidClosed = false;
    xQueueSend(QueueDisplayMessageHandle, ( void * ) &DisplayMessage, osWaitForever);

    if(eTaskGetState(ScanButtonsHandle)==eSuspended)
      osThreadResume(ScanButtonsHandle);

    }
    else
    {
      SSD1306_OFF();

      if(SystemFlags.CurrentlyRecording == true)
        RecordingWithLidClosed();
      else
        StandbyWithLidClosed();
      //HAL_I2C_MspDeInit(&hi2c3);
      //__HAL_RCC_I2C2_CLK_DISABLE();
      //HAL_GPIO_WritePin(OLED_13V_EN_GPIO_Port, OLED_13V_EN_Pin, GPIO_PIN_RESET);
      SystemFlags.LidClosed = true;

      if(eTaskGetState(ScanButtonsHandle)==eRunning || eTaskGetState(ScanButtonsHandle)==eBlocked)
        osThreadSuspend(ScanButtonsHandle);

    }
  //osDelay(1);
  }
  /* USER CODE END HallEffectSensor */
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

        osDelay(700);
      }
      SystemFlags.CurrentlyRecording = false;

      //HAL_SD_InitCard(&hsd1);
      SDMMC1_SD_Init();

      if(BSP_SD_Init() == MSD_OK)
      {
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
        //** Initialize record schedule
        InitialScheduleState = RTC_InitializeScheduleAlarms(&SwiftSchedule);

      switch (InitialScheduleState)
      {
      case 1: //We are in the middle of a record period so go directly into record mode

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

        __HAL_RCC_SDMMC1_CLK_ENABLE();

        if(uxBits & (CENTER_BTN_SIGNAL | ALARM_A_SIGNAL))
        {
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
        //** Initialize number of buffer writes per file ***
        BufferWritesPerFile = WAVFile.FileSize;
        if(uxBits & CENTER_BTN_SIGNAL)
        {
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
          //Set next stop time
          SetNextStopTime(&SwiftSchedule);
        }

        SystemFlags.CurrentlyRecording = true;

        //InitializeCodecAndDMA();
        //ReInit_MDF1_Interface();

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
        osThreadResume(StandbyHandle);
        //osThreadTerminate(NULL); //Terminate this task
        osThreadSuspend(InitializeHandle);
      }
      }
      else
      {
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

/* USER CODE BEGIN Header_ScanButtonsTask */
/**
* @brief Function implementing the ScanButtons thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ScanButtonsTask */
void ScanButtonsTask(void *argument)
{
  /* USER CODE BEGIN ScanButtons */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END ScanButtons */
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
  xEventGroupSetBitsFromISR(xEventGroup, WRITEBUFFER1_SIGNAL, &xHigherPriorityTaskWoken);
}

void HAL_MDF_AcqCpltCallback(MDF_HandleTypeDef *hmdf)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xEventGroupSetBitsFromISR(xEventGroup, WRITEBUFFER2_SIGNAL, &xHigherPriorityTaskWoken);
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
/* USER CODE END Application */

