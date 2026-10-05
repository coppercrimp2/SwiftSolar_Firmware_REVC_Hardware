//General Defines

#ifndef __GeneralDefines_H
#define __GeneralDefines_H

#define CODEC_I2C_ADDRESS       (0x18 << 1)

//******************************************************************************
//Defines associated with Swift operating states
#define STANDBY 0
#define RECORDING  1
#define ERROR   2

//******************************************************************************
//Defines associated with writing Swift settings to flash
#define SETTINGS_BASE_ADDRESS     ((uint32_t)0x08080000)//((uint32_t)0x080FF000) //ADDR_FLASH_PAGE_16

//#define FLASH_WRITE_KEY_OFFSET      ((uint32_t)0)
#define CODEC_SETTINGS_OFFSET       ((uint32_t)0)//((uint32_t)4)
#define STM32_CLOCKDIV_OFFSET       ((uint32_t)32)//((uint32_t)70)
#define WAVFILE_ATTRIBUTES_OFFSET   ((uint32_t)64)//((uint32_t)89)
#define SCHEDULE_STARTTIMES_OFFSET  ((uint32_t)96)//((uint32_t)110)
#define SCHEDULE_STOPTIMES_OFFSET   ((uint32_t)144)//((uint32_t)157)
#define LATLONG_OFFSET              ((uint32_t)192)//((uint32_t)157)
#define DST_OFFSET                  ((uint32_t)240)//((uint32_t)157)
#define CONFIG_TEXTFILE_OFFSET      ((uint32_t)272)//((uint32_t)204)

//#define ERROR_REPORT_OFFSET         ((uint32_t)24)//((uint32_t)22)

#define SCHEDULE_TYPE_ARBITRARY     ((uint8_t)0x01)
#define SCHEDULE_TYPE_DUTYCYCLED    ((uint8_t)0x02)
#define SCHEDULE_TYPE_CONTINUOUS    ((uint8_t)0x03)

//******************************************************************************
//Default Swift settings for uninitialized flash
#define DEFAULT_SAMPLERATE      ((uint8_t)0x20) //32kHz
#define DEFAULT_GAIN            ((uint8_t)0x42)  //
#define DEFAULT_CODEC_PLL_PR            ((uint8_t)0x91)//
#define DEFAULT_CODEC_PLL_J             ((uint8_t)0x04)//
#define DEFAULT_CODEC_PLL_D_MSB         ((uint8_t)0x23)//
#define DEFAULT_CODEC_PLL_D_LSB         ((uint8_t)0xC0)//
#define DEFAULT_CODEC_NADC              ((uint8_t)0x03)//
#define DEFAULT_CODEC_MADC              ((uint8_t)0x05)//
#define DEFAULT_CODEC_AOSR              ((uint8_t)0x80)//
#define DEFAULT_CODEC_BDIV                    ((uint8_t)0x14)//
#define DEFAULT_CODEC_PRB                     ((uint8_t)0x01)//

#define DEFAULT_MCU_HSE_M_Divider             ((uint8_t)0x04)
#define DEFAULT_MCU_PLL_N_Multiplier_MSB      ((uint8_t)0x00)
#define DEFAULT_MCU_PLL_N_Multiplier_LSB      ((uint8_t)0x32) //0x32 -> 50dec
#define DEFAULT_MCU_PLL_Q_Divider             ((uint8_t)0x06)
#define DEFAULT_MCU_PLL_P_Divider             ((uint8_t)0x04)
#define DEFAULT_MCU_AHB_Prescaler             ((uint8_t)0x04)
#define DEFAULT_MCU_APB1_Prescaler            ((uint8_t)0x02)
#define DEFAULT_MCU_APB2_Prescaler            ((uint8_t)0x02)
#define DEFAULT_MCU_RCC_MCO_Divider           ((uint8_t)0x02)
#define DEFAULT_MCU_PLLI2S_N_Multiplier_MSB   ((uint8_t)0x00)
#define DEFAULT_MCU_PLLI2S_N_Multiplier_LSB   ((uint8_t)0x32)
#define DEFAULT_MCU_PLLI2S_R_Divider          ((uint8_t)0x04)
#define DEFAULT_MCU_I2S_SampleRate            ((uint8_t)0x20) //32kHz

#define DEFAULT_FILESIZE_BYTE1       ((uint8_t)0xE0)//((uint8_t)0xD0)
#define DEFAULT_FILESIZE_BYTE2       ((uint8_t)0x6D)//((uint8_t)0xA4)
#define DEFAULT_FILESIZE_BYTE3       ((uint8_t)0x00)//((uint8_t)0x00)
#define DEFAULT_FILESIZE_BYTE4       ((uint8_t)0x00)//((uint8_t)0x00)

#endif /* __GeneralDefines_H */
