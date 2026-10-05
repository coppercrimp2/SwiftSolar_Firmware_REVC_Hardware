#include "stm32u5xx_hal.h"
//#include "stm32f4xx_hal.h"
#include "WriteToFlash.h"
#include "GeneralDefines.h"


uint8_t WriteToFlash(uint8_t *buffer, uint32_t offset, uint16_t size)
{ 
  //***TRY AGAIN****************************************************************
  uint32_t Address = offset + SETTINGS_BASE_ADDRESS;
  static uint32_t* dataPointer;
  //uint16_t index;
  //uint8_t remainder;
  int i = 0;
  
  //index = size/8;
  //remainder = size%8;  
  dataPointer = (uint32_t *)buffer;
  
  HAL_ICACHE_Disable();

  HAL_FLASH_Unlock();
  
  for(i = 0; i < size; i++){ //FLASH_TYPEPROGRAM_QUADWORD
    //HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (Address + (8 * i)), *dataPointer);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, (Address + (16 * i)), (uint32_t)dataPointer);
	//***NOTE 4/2/2023 MADE A CHANGE FROM FLASH_TYPEPROGRAM_DOUBLEWORD to FLASH_TYPEPROGRAM_QUADWORD.
	//***Associated code will need to be updated for this to work!!
    dataPointer++;
    dataPointer++;
    dataPointer++;
    dataPointer++;
  }
  
  /*
  for(i = 0; i < index; i++){
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (Address + (8 * i)), *dataPointer);
    dataPointer++;
  }
  
  if(remainder){
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (Address + (8 * i)), *dataPointer);
  }
  */
  HAL_FLASH_Lock();
  
  HAL_ICACHE_Enable();

  return 1;
  //***END TRY AGAIN************************************************************
}

uint8_t ReadFromFlash(uint8_t *buffer, uint32_t offset, uint16_t size)
{
  uint32_t Address = offset + SETTINGS_BASE_ADDRESS;
  int i;
  
  for(i = 0; i < size; i++)
  {
    buffer[i] = *(__IO uint8_t*)(Address + i);
  }
  
  return 1;
}

uint8_t EraseFlashSector(void)
{
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t PAGEError = 0;
  
  //uint8_t buffer[8];
  HAL_ICACHE_Disable();

  //*********************************************************
  /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();

  /* Erase the user Flash area
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

  /* Clear OPTVERR bit set on virgin samples */
  //__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
  /* Clear PEMPTY bit set (as the code is executed from Flash which is not empty) */
  //if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_PEMPTY) != 0)
  //{
    //__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PEMPTY);
  //}
  
  if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_SR_ERRORS) != 0)
  {
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PGSERR);
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PGAERR);
  }

  
  /* Fill EraseInit structure*/
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.Banks       = FLASH_BANK_2;
  EraseInitStruct.Page        = 0;
  EraseInitStruct.NbPages     = 1;

  /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
     you have to make sure that these data are rewritten before they are accessed during code
     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
     DCRST and ICRST bits in the FLASH_CR register. */
  HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);
  //*********************************************************
  
  HAL_FLASH_Lock();
  
  HAL_ICACHE_Enable();

  //Write flash update key
//  buffer[0] = 0xF7;
//  buffer[1] = 0x33; //0x32
//  buffer[2] = 0x00;
//  buffer[3] = 0x00;
//  buffer[4] = 0xFF;
//  buffer[5] = 0xFF;
//  buffer[6] = 0xFF;
//  buffer[7] = 0xFF;
//  WriteToFlash(buffer, 0x00, 8);
  
  return 1;
}
