#include "stm32u5xx_hal.h"
#include "ff.h"
#include "ffconf.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"
#include "AudioFiles.h"
//#include "GeneralDefines.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "RTC.h"
#include "flash.h"

static char SD_Path[4]; 
static FATFS SDDISKFatFs; 
static FILINFO FileInfo;

DWORD clust2sect (FATFS* fs, DWORD clst);
DWORD get_fat (FATFS* fs, DWORD clst);
FRESULT put_fat (FATFS* fs, DWORD clst, DWORD val);

FIL TextFile;

//static char MyFileName[72]; //(_MAX_LFN + 1) * 2 bytes

uint8_t AudioFiles_MountSDCard(SWIFT_ERRORS* SwiftError)
{
  uint8_t retSD; 
  
  SwiftError->CallingFunction = FUNC_AudioFiles_MountSDCard;
  SwiftError->FatFsResult = FR_OK;
  
  retSD = FATFS_LinkDriver(&SD_Driver, SD_Path);  
  if(retSD != 0)
  {
    SwiftError->ErrorNumber = 1;
    return 1; //ERROR
  }

  SwiftError->FatFsResult = f_mount(&SDDISKFatFs, (TCHAR const*)SD_Path, 1);

  if(SwiftError->FatFsResult != FR_OK)
  {
    SwiftError->ErrorNumber = 2;
    return 1; //ERROR
  }  
  
  return 0; //No Error
}

/*
uint8_t AudioFiles_FormatSDCard(SWIFT_ERRORS* SwiftError)
{  
  SwiftError->CallingFunction = FUNC_AudioFiles_FormatSDCard;
  
  SwiftError->FatFsResult = f_mkfs((TCHAR const*)SD_Path, 0, 0);
  if(SwiftError->FatFsResult != FR_OK)
  {
    SwiftError->ErrorNumber = 1;
    return 1;
  }  
  
  return 0;
}
*/

uint8_t AudioFiles_NewRecordingDirectory(WAVFile_Attributes* RecordingDirectory)
{  
  int DirectoryIndex = 0;
  char DirectoryName[13];
  char strDirectoryIndex[4];
  FRESULT ReturnResult;
  
  f_chdir("0:/");
  
  if(RecordingDirectory->RecordingDirFlag) //Directory has already been created
  {
    ReturnResult = f_chdir((char const*)RecordingDirectory->RecordingDirName);
  }
  else
  {
    while(DirectoryIndex < 255){
      
      strcpy(DirectoryName, (char*) RecordingDirectory->Filename);

      if(DirectoryIndex < 10)
        strcat(DirectoryName, "00");
      else if(DirectoryIndex < 100)
        strcat(DirectoryName, "0");

      sprintf(strDirectoryIndex, "%d", DirectoryIndex);
      strcat(DirectoryName,strDirectoryIndex);
      
      ReturnResult = f_stat(DirectoryName, &FileInfo);
      
      if(ReturnResult == FR_NO_FILE)
              break;
      
      DirectoryIndex++;
    }
    
    if(DirectoryIndex == 255)
    {
      return 1; //ERROR
    }  
    
    ReturnResult = f_mkdir(DirectoryName);
    if (ReturnResult != FR_OK) 
    {
      return 1; //ERROR
    }    
    
    ReturnResult = f_chdir(DirectoryName);

    if (ReturnResult != FR_OK) 
    {
      return 1; //ERROR
    }
    
    RecordingDirectory->RecordingDirFlag = 1; //This indicates that the Recording Directory has been created
    
    RecordingDirectory->RecordingDirName[0] = '/';
    RecordingDirectory->RecordingDirName[1] = '\0';
    strcat((char*)RecordingDirectory->RecordingDirName, DirectoryName);
    
    //strcpy((char*) RecordingDirectory->RecordingDirName, DirectoryName);
    
    AudioFiles_NewConfigTextFile();  //Create new Configuration text file
  }

  return 0; //NO ERROR
}

uint8_t AudioFiles_NewDayDirectory(uint8_t* CustomLabel)
{  
  char DirectoryName[20];
  char temp[11];
  FRESULT ReturnResult;
  
  strcpy(DirectoryName, (char*) CustomLabel);
    
  RTC_ReturnDateString(temp);
  strcat(DirectoryName, temp);  
  
  //Check to see if directory has already been created
  ReturnResult = f_stat(DirectoryName, &FileInfo);      
  if(ReturnResult == FR_NO_FILE)
  {
    ReturnResult = f_mkdir(DirectoryName);
    if (ReturnResult != FR_OK) 
    {
      return 1; //ERROR
    }
  }

  ReturnResult = f_chdir(DirectoryName);

  if (ReturnResult != FR_OK) 
  {
    return 1; //ERROR
  }

  return 0; //NO ERROR 
} 

uint8_t AudioFiles_ChangeDirectory(void)
{ 
  f_chdir("0:/");
  
  return 1;
}

uint8_t AudioFiles_NewAudioFile(uint8_t* CustomLabel, FIL* MyFile)
{	
  FRESULT ReturnResult;
  char DateTimeString[20]; 
  char NewFileName[29];
    
  strcpy(NewFileName, (char*) CustomLabel);
  
  RTC_ReturnDateTimeFileName(DateTimeString);
  strcat(NewFileName, (char*) DateTimeString);
  
  ReturnResult = f_open(MyFile, NewFileName, FA_CREATE_ALWAYS | FA_WRITE);
  
  if (ReturnResult != FR_OK) 
  {
    return 1;
  }

  //Make room for header information
  ReturnResult  = f_lseek(MyFile, 46);
  if (ReturnResult != FR_OK) 
  {
    return 1;
  }  
  else
  {
    return 0; //NO ERROR
  }

}

void AudioFiles_GetDiskInfoString(Message* message)
{
  FATFS *fs;
  DWORD fre_clust, fre_sect, tot_sect;
  
  f_getfree((TCHAR const*)SD_Path, &fre_clust, &fs);
  
  /* Get total sectors and free sectors */
  tot_sect = (fs->n_fatent - 2) * fs->csize;
  fre_sect = fre_clust * fs->csize;
  
  if(SDDISKFatFs.fs_type == 3) //FS_FAT32
  {
    //ReturnVal = sprintf(DiskInfoString, "Drive filesystem: FAT32\r\nTotal drive space: %lu KB\r\nAvailable drive space: %lu KB\r\n\r\n", tot_sect / 2, fre_sect / 2);
	  strcpy(message->Line1, "Filesystem: FAT32");
	  //strcpy(message->Line2, (char *)(sprintf("Total space: %lu", tot_sect / 2)));
	  //sprintf(message->Line2, "Total space: %lu KB", tot_sect / 2);
	  //sprintf(message->Line3, "Total space: %lu KB", fre_sect / 2);
  }
  else if(SDDISKFatFs.fs_type == 4) //EX_FAT
  {
    //ReturnVal = sprintf(DiskInfoString, "Drive filesystem: exFAT\r\nTotal drive space: %lu KB\r\nAvailable drive space: %lu KB\r\n\r\n", tot_sect / 2, fre_sect / 2);
	  strcpy(message->Line1, "EXFAT");
	  sprintf(message->Line2, "Total space: %luGB", (tot_sect / 2)/1000000);
	  sprintf(message->Line3, "Free space: %lu KB", fre_sect / 2);
  }
  else //UNKNOWN FAT TYPE
  {
    //ReturnVal = sprintf(DiskInfoString, "Drive filesystem: Unknown\r\nTotal drive space: %lu KB\r\nAvailable drive space: %lu KB\r\n\r\n", tot_sect / 2, fre_sect / 2);
	  strcpy(message->Line1, "UNKNOWN");
  }  
  
  //sprintf(DiskInfoString, "%10lu KB total drive space.\r\n%10lu KB available.\r\n", tot_sect / 2, fre_sect / 2);

}

uint8_t AudioFiles_NewConfigTextFile()
{	
  FRESULT ReturnResult;
  uint8_t ConfigurationText[48];
  UINT bw;
  uint8_t i = 0;
  //FIL TextFile;
  
  ReturnResult = f_open(&TextFile, "SwiftConfig.txt", FA_CREATE_ALWAYS | FA_WRITE);
   
  do{
    SwiftSettings_GetConfigTextFromFlash(ConfigurationText, i); 
    f_write(&TextFile, &ConfigurationText[4], ConfigurationText[3], &bw); //ConfigurationText[4] is where text begins, ConfigurationText[3] is number of bytes
    i++;
  }while(i <= ConfigurationText[2]);  
  
  f_close(&TextFile);
  
  if (ReturnResult != FR_OK) 
  {
    return 1;
  }
  else
  {
    return 0; //NO ERROR
  }
}

uint8_t AudioFiles_WriteHeader(SWIFT_ERRORS* SwiftError, FIL* MyFile)
{
  UINT bw;
  DWORD tmpFileEndPointer;
  //volatile uint8_t SampleRateByte;
  //volatile uint32_t SampleRateWord;

  uint8_t WAVFileHeader[44] = {
      // ChunkID: "RIFF"
      0x52, 0x49, 0x46, 0x46,
      // ChunkSize: 199992 (file size - 8)
      0xF8, 0x07, 0x03, 0x00,
      // Format: "WAVE"
      0x57, 0x41, 0x56, 0x45,
      // Subchunk1ID: "fmt "
      0x66, 0x6D, 0x74, 0x20,
      // Subchunk1Size: 16
      0x10, 0x00, 0x00, 0x00,
      // AudioFormat: PCM = 1
      0x01, 0x00,
      // NumChannels: 2
      0x02, 0x00,
      // SampleRate: 16000
      0x80, 0x3E, 0x00, 0x00,
      // ByteRate: 64000
      0x00, 0xFA, 0x00, 0x00,
      // BlockAlign: 4
      0x04, 0x00,
      // BitsPerSample: 16
      0x10, 0x00,
      // Subchunk2ID: "data"
      0x64, 0x61, 0x74, 0x61,
      // Subchunk2Size: 199956
      0xD4, 0x07, 0x03, 0x00
  };
  
  SwiftError->CallingFunction = FUNC_AudioFiles_WriteHeader;
  
  //SampleRateByte = *(__IO uint8_t*)((uint32_t)0x080FE002);
  //SampleRateByte = *(__IO uint8_t*)((uint32_t)SETTINGS_BASE_ADDRESS);
  //SampleRateWord = SampleRateByte * 1000;
  
  WAVFileHeader[4] = (BYTE)((MyFile->obj.objsize - 8) & 0xFF);
  WAVFileHeader[5] = (BYTE)(((MyFile->obj.objsize - 8) >> 8) & 0xFF);
  WAVFileHeader[6] = (BYTE)(((MyFile->obj.objsize - 8) >> 16) & 0xFF);
  WAVFileHeader[7] = (BYTE)(((MyFile->obj.objsize - 8) >> 24) & 0xFF);
  
  /*
  WAVFileHeader[24] = (BYTE)((SampleRateWord) & 0xFF);
  WAVFileHeader[25] = (BYTE)(((SampleRateWord) >> 8) & 0xFF);
  WAVFileHeader[26] = (BYTE)(((SampleRateWord) >> 16) & 0xFF);
  WAVFileHeader[27] = (BYTE)(((SampleRateWord) >> 24) & 0xFF);
  
  SampleRateWord = ((SampleRateWord * 16) >> 3); //Shifting Right by three is the same as dividing by 8
  
  WAVFileHeader[28] = (BYTE)((SampleRateWord) & 0xFF);
  WAVFileHeader[29] = (BYTE)(((SampleRateWord) >> 8) & 0xFF);
  WAVFileHeader[30] = (BYTE)(((SampleRateWord) >> 16) & 0xFF);
  WAVFileHeader[31] = (BYTE)(((SampleRateWord) >> 24) & 0xFF);
  */

  WAVFileHeader[42] = (BYTE)((MyFile->obj.objsize - 46) & 0xFF);
  WAVFileHeader[43] = (BYTE)(((MyFile->obj.objsize - 46) >> 8) & 0xFF);
  WAVFileHeader[44] = (BYTE)(((MyFile->obj.objsize - 46) >> 16) & 0xFF);
  WAVFileHeader[45] = (BYTE)(((MyFile->obj.objsize - 46) >> 24) & 0xFF);

  tmpFileEndPointer = MyFile->obj.objsize;
  
  SwiftError->FatFsResult = f_lseek(MyFile, 0);
  if (SwiftError->FatFsResult != FR_OK) 
  {
    SwiftError->ErrorNumber = 1;
    return 1; //ERROR
  }  

  SwiftError->FatFsResult = f_write(MyFile, WAVFileHeader, 44, &bw);
  if (SwiftError->FatFsResult != FR_OK) 
  {
    SwiftError->ErrorNumber = 3;
    return 1; //ERROR
  } 
  
  SwiftError->FatFsResult = f_lseek(MyFile, tmpFileEndPointer);
  if (SwiftError->FatFsResult != FR_OK) 
  {
    SwiftError->ErrorNumber = 1;
    return 1; //ERROR
  } 

  return 0;

}

uint8_t AudioFiles_CloseFile(SWIFT_ERRORS* SwiftError, FIL* MyFile)
{
  SwiftError->CallingFunction = FUNC_AudioFiles_CloseFile;
  /*
  SwiftError->FatFsResult = f_sync(MyFile);
  if (SwiftError->FatFsResult != FR_OK) 
  {
    SwiftError->ErrorNumber = 1;
    return 1; //ERROR
  }  
  */
  SwiftError->FatFsResult = f_truncate(MyFile);
  if (SwiftError->FatFsResult != FR_OK) 
  {
    SwiftError->ErrorNumber = 2;
    return 1; //ERROR
  }  
    
  SwiftError->FatFsResult = f_close(MyFile);
  if (SwiftError->FatFsResult != FR_OK) 
  {
    SwiftError->ErrorNumber = 3;
    return 1; //ERROR
  }  
  
  return 0;
}
