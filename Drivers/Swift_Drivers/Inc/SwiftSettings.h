//#include "SwiftErrors.h"

#ifndef __SwiftSettings_H
#define __SwiftSettings_H

#define __SWIFT_FIRMWARE_VERSION_MAIN   (0x01) /*!< [31:24] main version */
#define __SWIFT_FIRMWARE_VERSION_SUB1   (0x00) /*!< [23:16] sub1 version */
#define __SWIFT_FIRMWARE_VERSION_SUB2   (0x02) /*!< [15:8]  sub2 version */
#define __SWIFT_FIRMWARE_VERSION_RC     (0x06) /*!< [7:0]  release candidate */ 
#define __SWIFT_FIRMWARE_VERSION         ((__SWIFT_FIRMWARE_VERSION_MAIN << 24U)\
                                        |(__SWIFT_FIRMWARE_VERSION_SUB1 << 16U)\
                                        |(__SWIFT_FIRMWARE_VERSION_SUB2 << 8U )\
                                        |(__SWIFT_FIRMWARE_VERSION_RC))

/**
  * @brief Swift firmware version number V0.1.0.5 - INCLUDES NEW SAMPLE RATE IMPROVEMENTS
  * @brief Swift firmware version number V0.1.0.6 - INCLUDES ABILITY TO TURN MIC BIAS VOLTAGE ON/OFF THROUGH CONFIG UTILITY
  * @brief Swift firmware version number V0.1.0.7 - Includes lower power operation in standby and record mode, and fixes an 
            issue where the SDMMC1 clock was turned off when it wasnt supposed to. Also fixed the serial number reporting feature 11/5/2019
  * @brief Swift firmware version number V0.1.0.8 - Fixes issue with SwiftConfig.txt text file. This feature now works 
  * @brief Swift firmware version number V0.1.0.9 - Added code to check the battery voltage when the Swift boots up
  * @brief Latest version for SwiftOne REVB is now v0.1.1.4 - USB ProductID has been changed to 22353DEC (0x5751) to differentirate between original Swift
  * @brief Swift firmware version number V1.0.0.1 - Changed the SAI initialization code to more closely follow I2S protocol. Changed FreeRTOS xPortSysTickHandler
            to fix bug I found with xExpectedIdleTime variable. Changed PLLN variable from 11 to 12 to increase clock rate. Initial tests in Sapsucker Woods resulted
            in what I believe might be buffer overrun errors, so the hope is that increasing the clock rate will solve this issue. The codec now does a software reset
            to go into a low power mode. This results in a audible pop at the beginning of the files but also decreases the standby power quite a bit
  */

typedef struct
{
  uint8_t MCU_HSE_M_Divider;
  uint16_t MCU_PLL_N_Multiplier;
  uint8_t MCU_PLL_Q_Divider;
  uint8_t MCU_PLL_P_Divider;
  uint8_t MCU_AHB_Prescaler;
  uint8_t MCU_APB1_Prescaler;
  uint8_t MCU_APB2_Prescaler;
  uint8_t MCU_FLASH_Latency;
  uint8_t PLLSAI2M_Divider;
  uint16_t PLLSAI2N_Multiplier;
  uint8_t PLLSAI2P_Divider;
  uint8_t MCU_I2S_SampleRate;
}STM32_SysClock;

typedef struct
{
  uint8_t Codec_GAIN;
  uint8_t Codec_NADC;
  uint8_t Codec_MADC;
  uint8_t Codec_AOSR;
  uint8_t Codec_BDIV;
  uint8_t Codec_PRB;
  uint8_t Codec_PLL_J;  
  uint8_t Codec_PLL_D_MSB;
  uint8_t Codec_PLL_D_LSB;
  uint8_t Codec_PLL_PR;
  uint8_t Codec_MIC_BIAS_CFG;
}Codec_Config;

typedef struct
{
  uint32_t FileSize;
  uint8_t Filename[10];
  uint8_t FilenameLength;  
  uint8_t IsFileOpen; //If '1' a file is currently open
  uint8_t DirectoryLevel; //0=root dir, 1=recording dir, 2=day dir
  uint32_t SampleRate;
  uint8_t RecordingDirFlag; //if '0' then recording directory has not been created, if '1' it has
  uint8_t RecordingDirName[14]; //This holds the name of the current Recording Directory so we can return to it if necessary
}WAVFile_Attributes;

typedef struct
{
  RTC_TimeTypeDef RecordingStartTime;
  RTC_TimeTypeDef RecordingStopTime;
}Schedule_Times;

typedef struct
{
  uint8_t ScheduleType;
  uint8_t StartsAtMidnightFlag;
  uint8_t EndsAtMidnightFlag;
  uint8_t NumberOfRecords;
  uint8_t CurrentRecordNumber;
  RTC_DateTypeDef StartDate;
  RTC_DateTypeDef StopDate;
  Schedule_Times StartStopTimes[20];
}Swift_Schedule;

uint8_t SwiftSettings_GetCodecVariables(Codec_Config *);

uint8_t SwiftSettings_GetSTM32ClkDivVariables(STM32_SysClock *);

uint8_t SwiftSettings_GetWAVFileAttributes(WAVFile_Attributes *);

uint8_t SwiftSettings_GetSwiftSchedule(Swift_Schedule *);

uint8_t SwiftSettings_GetSTM32VersionInfo(uint8_t *);

uint8_t SwiftSettings_GetUniqueID(uint8_t *);

//uint8_t SwiftSettings_GetDSTVariables(DST_Settings *);

//uint8_t SwiftSettings_GetLatLon(uint8_t *);

uint8_t SwiftSettings_GetConfigTextFromFlash(uint8_t *, uint8_t);

//uint8_t* SwiftSettings_ReadErrorFromFlash(void);

#endif /* __Settings_H */