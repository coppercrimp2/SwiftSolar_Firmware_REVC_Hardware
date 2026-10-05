#include "stm32u5xx_hal.h"
#include "SwiftSettings.h"
//#include "RTC.h"
#include "GeneralDefines.h"

/**
  * @brief Swift firmware version number V0.1.0.5 - INCLUDES NEW SAMPLE RATE IMPROVEMENTS
  * @brief Swift firmware version number V0.1.0.6 - INCLUDES ABILITY TO TURN MIC BIAS VOLTAGE ON/OFF THROUGH CONFIG UTILITY
  * @brief Swift firmware version number V0.1.0.7 - Includes lower power operation in standby and record mode, and fixes an 
            issue where the SDMMC1 clock was turned off when it wasnt supposed to. Also fixed the serial number reporting feature 11/5/2019
  * @brief Swift firmware version number V0.1.0.8 - Fixes issue with SwiftConfig.txt text file. This feature now works 
  * @brief Swift firmware version number V0.1.0.9 - Added code to check the battery voltage when the Swift boots up
  * @brief Latest version for SwiftOne REVB is now v0.1.1.4 - USB ProductID has been changed to 22353DEC (0x5751) to differentirate between original Swift
  */


uint8_t SwiftSettings_GetCodecVariables(Codec_Config *ConfigVariables)
{
  uint32_t address = SETTINGS_BASE_ADDRESS + CODEC_SETTINGS_OFFSET + 3;
  
  ConfigVariables->Codec_GAIN = *(__IO uint8_t*)(address); //Gain
  address++;
  
  ConfigVariables->Codec_PLL_PR = *(__IO uint8_t*)(address); //PLL P and R values
  address++;
  
  ConfigVariables->Codec_PLL_J = *(__IO uint8_t*)(address); //PLL J
  address++;
  
  ConfigVariables->Codec_PLL_D_MSB = *(__IO uint8_t*)(address); //PLL D MSB
  address++;
  ConfigVariables->Codec_PLL_D_LSB = *(__IO uint8_t*)(address); //PLL D LSB
  address++;
  
  ConfigVariables->Codec_NADC = *(__IO uint8_t*)(address); //NADC
  address++;
  ConfigVariables->Codec_MADC = *(__IO uint8_t*)(address); //MADC 
  address++;
  ConfigVariables->Codec_AOSR = *(__IO uint8_t*)(address); //AOSR
  address++;
  ConfigVariables->Codec_BDIV = *(__IO uint8_t*)(address); //BDIV
  address++;
  ConfigVariables->Codec_PRB = *(__IO uint8_t*)(address); //PRB
  address++;
  ConfigVariables->Codec_MIC_BIAS_CFG = *(__IO uint8_t*)(address); //Microphone Bias Voltage ON or OFF
  
  return 1;
}

//uint8_t SwiftSettings_GetDSTVariables(DST_Settings *ConfigVariables)
//{
//  uint8_t i = 0;
//  uint8_t tmp1, tmp2;
//  
//  uint32_t address = SETTINGS_BASE_ADDRESS + DST_OFFSET + 2;
//    
//  tmp1 = *(__IO uint8_t*)(address);
//  address++;
//  tmp2 = *(__IO uint8_t*)(address);
//  address++;
//  
//  ConfigVariables->DSTStart_DOY = ((tmp1 << 8) | tmp2);
//  
//  tmp1 = *(__IO uint8_t*)(address);
//  address++;
//  tmp2 = *(__IO uint8_t*)(address);
//  address++;
//  
//  ConfigVariables->DSTEnd_DOY = ((tmp1 << 8) | tmp2);
//  
//  ConfigVariables->DSTOffset_Hours = *(__IO uint8_t*)(address);
//  address++;
//  
//  ConfigVariables->DSTOffset_Minutes = *(__IO uint8_t*)(address);
//  address++;
//  
//  ConfigVariables->DSTStart_Suffix_1[0] = '_';
//    
//  for(i = 1; i < 6; i++)
//  {
//    ConfigVariables->DSTStart_Suffix_1[i] = (*(__IO uint8_t*)(address));
//    address++;
//  }
//  
//  ConfigVariables->DSTStart_Suffix_1[i] = 0;
//  
//  ConfigVariables->DSTEnd_Suffix_2[0] = '_';
//  
//  for(i = 1; i < 6; i++)
//  {
//    ConfigVariables->DSTEnd_Suffix_2[i] = (*(__IO uint8_t*)(address));
//    address++;
//  }
//  
//  ConfigVariables->DSTEnd_Suffix_2[i] = 0;
//  
//  return 1;
//}

uint8_t SwiftSettings_GetSTM32ClkDivVariables(STM32_SysClock *ConfigVariables)
{
  uint32_t address = SETTINGS_BASE_ADDRESS + STM32_CLOCKDIV_OFFSET + 2;
  uint8_t tmp1, tmp2;
  
  ConfigVariables->MCU_HSE_M_Divider = *(__IO uint8_t*)(address);
  address++;
  
  tmp1 = (*(__IO uint8_t*)(address));
  address++;
  tmp2 = (*(__IO uint8_t*)(address));  
  ConfigVariables->MCU_PLL_N_Multiplier = ((tmp1 << 8) | tmp2); //MCU_PLL_N_Multiplier
  address++;
  
  ConfigVariables->MCU_PLL_Q_Divider = *(__IO uint8_t*)(address);
  address++;
  
  ConfigVariables->MCU_PLL_P_Divider = *(__IO uint8_t*)(address);
  address++;
  
  ConfigVariables->MCU_AHB_Prescaler = *(__IO uint8_t*)(address);
  address++;
  
  ConfigVariables->MCU_APB1_Prescaler = *(__IO uint8_t*)(address);
  address++;
  
  ConfigVariables->MCU_APB2_Prescaler = *(__IO uint8_t*)(address);
  address++;
  
  ConfigVariables-> MCU_FLASH_Latency = *(__IO uint8_t*)(address); //FLASH LATENCY LOCATION
  address++;
  
  ConfigVariables->PLLSAI2M_Divider = *(__IO uint8_t*)(address);
  address++;

  ConfigVariables->PLLSAI2N_Multiplier = *(__IO uint8_t*)(address);
  address++;

  ConfigVariables->PLLSAI2P_Divider = *(__IO uint8_t*)(address);
  address++;
    
  ConfigVariables->MCU_I2S_SampleRate = *(__IO uint8_t*)(address); //MCU_I2S_SampleRate
  
  return 1;
}

uint8_t SwiftSettings_GetWAVFileAttributes(WAVFile_Attributes *WAVFile)
{
  uint8_t tmp1, tmp2, tmp3, tmp4;
  uint32_t address = SETTINGS_BASE_ADDRESS + WAVFILE_ATTRIBUTES_OFFSET + 2;
  int i;
  
  tmp1 = (*(__IO uint8_t*)(address));
  address++;
  tmp2 = (*(__IO uint8_t*)(address));
  address++;
  tmp3 = (*(__IO uint8_t*)(address));
  address++;
  tmp4 = (*(__IO uint8_t*)(address));    
  WAVFile->FileSize = (((tmp4 << 24) | (tmp3 << 16) | (tmp2 << 8) | (tmp1)) / 2);
  address++;
  
  //WAVFile->FileSize = 4;
    
  WAVFile->FilenameLength = (*(__IO uint8_t*)(address));
  address++;
  
  for(i = 0; i < WAVFile->FilenameLength; i++)
  {
    WAVFile->Filename[i] = (*(__IO uint8_t*)(address));
    address++;
  }
  
  WAVFile->Filename[i] = 0; //Null terminate the string
  
  WAVFile->SampleRate = (*(__IO uint8_t*)((uint32_t)0x0807E002)) * 1000;
    
  return 1;
}

uint8_t SwiftSettings_GetSwiftSchedule(Swift_Schedule *Swiftschedule)
{
  uint32_t StartTimeAddress = SETTINGS_BASE_ADDRESS + SCHEDULE_STARTTIMES_OFFSET + 6;
  uint32_t StopTimeAddress = SETTINGS_BASE_ADDRESS + SCHEDULE_STOPTIMES_OFFSET + 7;
  int i = 0;
  
  Swiftschedule->CurrentRecordNumber = 0;
  Swiftschedule->StartsAtMidnightFlag = (*(__IO uint8_t*)(SETTINGS_BASE_ADDRESS + SCHEDULE_STARTTIMES_OFFSET + 5));  
  Swiftschedule->EndsAtMidnightFlag = (Swiftschedule->StartsAtMidnightFlag >> 1) & 0x01;
  Swiftschedule->StartsAtMidnightFlag = Swiftschedule->StartsAtMidnightFlag & 0x01;
  
  Swiftschedule->ScheduleType = (*(__IO uint8_t*)(SETTINGS_BASE_ADDRESS + SCHEDULE_STARTTIMES_OFFSET + 5));  
  Swiftschedule->ScheduleType = (Swiftschedule->ScheduleType >> 4) & 0x03;  
  
  Swiftschedule->StartDate.Year = (*(__IO uint8_t*)(SETTINGS_BASE_ADDRESS + SCHEDULE_STARTTIMES_OFFSET + 2));
  Swiftschedule->StartDate.Month = (*(__IO uint8_t*)(SETTINGS_BASE_ADDRESS + SCHEDULE_STARTTIMES_OFFSET + 3));
  Swiftschedule->StartDate.Date = (*(__IO uint8_t*)(SETTINGS_BASE_ADDRESS + SCHEDULE_STARTTIMES_OFFSET + 4));
  
  Swiftschedule->StopDate.Year = (*(__IO uint8_t*)(SETTINGS_BASE_ADDRESS + SCHEDULE_STOPTIMES_OFFSET + 2));
  Swiftschedule->StopDate.Month = (*(__IO uint8_t*)(SETTINGS_BASE_ADDRESS + SCHEDULE_STOPTIMES_OFFSET + 3));
  Swiftschedule->StopDate.Date = (*(__IO uint8_t*)(SETTINGS_BASE_ADDRESS + SCHEDULE_STOPTIMES_OFFSET + 4));
  
  Swiftschedule->NumberOfRecords = (*(__IO uint8_t*)(StartTimeAddress));  
  StartTimeAddress++;
  
  if((Swiftschedule->ScheduleType == SCHEDULE_TYPE_ARBITRARY) || (Swiftschedule->ScheduleType == SCHEDULE_TYPE_CONTINUOUS))
  {
    while(i < Swiftschedule->NumberOfRecords)
    {
      Swiftschedule->StartStopTimes[i].RecordingStartTime.Hours = (*(__IO uint8_t*)(StartTimeAddress));
      Swiftschedule->StartStopTimes[i].RecordingStopTime.Hours = (*(__IO uint8_t*)(StopTimeAddress));
      StartTimeAddress++;
      StopTimeAddress++;
      
      Swiftschedule->StartStopTimes[i].RecordingStartTime.Minutes = (*(__IO uint8_t*)(StartTimeAddress));
      Swiftschedule->StartStopTimes[i].RecordingStopTime.Minutes = (*(__IO uint8_t*)(StopTimeAddress));
      StartTimeAddress++;
      StopTimeAddress++;
      
      i++;
    }
  }
  else if(Swiftschedule->ScheduleType == SCHEDULE_TYPE_DUTYCYCLED)
  {
      Swiftschedule->StartStopTimes[i].RecordingStartTime.Hours = (*(__IO uint8_t*)(StartTimeAddress));
      Swiftschedule->StartStopTimes[i].RecordingStopTime.Hours = (*(__IO uint8_t*)(StopTimeAddress));
      //StartTimeAddress++;
      //StopTimeAddress++;
      
      Swiftschedule->StartStopTimes[i].RecordingStartTime.Minutes = (*(__IO uint8_t*)(StartTimeAddress + 1));
      Swiftschedule->StartStopTimes[i].RecordingStopTime.Minutes = (*(__IO uint8_t*)(StopTimeAddress + 1));
      //StartTimeAddress++;
      //StopTimeAddress++;
      i++;
  }
  
  
  
  while(i < 20)
  {
    Swiftschedule->StartStopTimes[i].RecordingStartTime.Hours = 0x00;
    Swiftschedule->StartStopTimes[i].RecordingStopTime.Hours = 0x00;
    
    Swiftschedule->StartStopTimes[i].RecordingStartTime.Minutes = 0x00;
    Swiftschedule->StartStopTimes[i].RecordingStopTime.Minutes = 0x00;
    
    i++;
  }
  
  return 1;
}
/*
uint8_t SwiftSettings_GetLatLong(uint8_t *LatLongString)
{
  uint32_t address = SETTINGS_BASE_ADDRESS + LATLONG_OFFSET + 2;
  
  LatLongString = (*(__IO uint8_t*)(address));
  
  return 1;
}
*/
uint8_t SwiftSettings_GetConfigTextFromFlash(uint8_t *SwiftConfigText, uint8_t PacketNumber)
{ 
  
  uint8_t i = 0;
  
  while(i < 48)
  {
    SwiftConfigText[i] = *(__IO uint8_t*)((SETTINGS_BASE_ADDRESS + CONFIG_TEXTFILE_OFFSET + (PacketNumber * 48)) + i);
    i++;
  }
    
  return 1;  
}

uint8_t SwiftSettings_GetSTM32VersionInfo(uint8_t* DeviceID)
{ 
  uint32_t temp;
  
  temp = HAL_GetDEVID();    
  DeviceID[0] = (uint8_t)(temp & 0xFF);
  DeviceID[1] = (uint8_t)((temp >> 8) & 0xFF);
  DeviceID[2] = (uint8_t)((temp >> 16) & 0xFF);
  DeviceID[3] = (uint8_t)((temp >> 24) & 0xFF);
  
  temp = HAL_GetREVID();  
  DeviceID[4] = (uint8_t)(temp & 0xFF);
  DeviceID[5] = (uint8_t)((temp >> 8) & 0xFF);
  DeviceID[6] = (uint8_t)((temp >> 16) & 0xFF);
  DeviceID[7] = (uint8_t)((temp >> 24) & 0xFF);
  
  temp = HAL_GetHalVersion();
  DeviceID[8] = (uint8_t)(temp & 0xFF);
  DeviceID[9] = (uint8_t)((temp >> 8) & 0xFF);
  DeviceID[10] = (uint8_t)((temp >> 16) & 0xFF);
  DeviceID[11] = (uint8_t)((temp >> 24) & 0xFF);
  
  DeviceID[12] = __SWIFT_FIRMWARE_VERSION_RC;
  DeviceID[13] = __SWIFT_FIRMWARE_VERSION_SUB2;
  DeviceID[14] = __SWIFT_FIRMWARE_VERSION_SUB1;
  DeviceID[15] = __SWIFT_FIRMWARE_VERSION_MAIN;
  
  return 16;
}

uint8_t SwiftSettings_GetUniqueID(uint8_t* UniqueID)
{   
  UniqueID[0] = 7; //Report ID
  UniqueID[1] = 12; //Array Size

  UniqueID[2] = *(__IO uint8_t*)(0x0BFA0700 + 0);
	UniqueID[3] = *(__IO uint8_t*)(0x0BFA0700 + 1);
	UniqueID[4] = *(__IO uint8_t*)(0x0BFA0700 + 2);
	UniqueID[5] = *(__IO uint8_t*)(0x0BFA0700 + 3);
	UniqueID[6] = *(__IO uint8_t*)(0x0BFA0700 + 4);
	UniqueID[7] = *(__IO uint8_t*)(0x0BFA0700 + 5);
	UniqueID[8] = *(__IO uint8_t*)(0x0BFA0700 + 6);
	UniqueID[9] = *(__IO uint8_t*)(0x0BFA0700 + 7);
	UniqueID[10] = *(__IO uint8_t*)(0x0BFA0700 + 8);
	UniqueID[11] = *(__IO uint8_t*)(0x0BFA0700 + 9);
	UniqueID[12] = *(__IO uint8_t*)(0x0BFA0700 + 10);
	UniqueID[13] = *(__IO uint8_t*)(0x0BFA0700 + 11);
  return 14;
}
