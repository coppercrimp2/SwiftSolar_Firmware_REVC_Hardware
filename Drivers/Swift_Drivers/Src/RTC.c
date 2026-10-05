//Rob's Realtime Clock Code
#include <string.h>
#include <time.h>

#include "stm32u5xx_hal.h"
#include "main.h"
#include "RTC.h"
#include "GeneralDefines.h"

int yisleap(int);
int get_yday(int, int, int);

RTC_TimeTypeDef ReturnNextStartTime(Swift_Schedule *);
RTC_TimeTypeDef ReturnNextStopTime(Swift_Schedule *);

uint8_t RTC_CompareTimes(RTC_TimeTypeDef, RTC_TimeTypeDef);

RTC_HandleTypeDef hrtc;

void RTC_Init(void)
{
	RTC_PrivilegeStateTypeDef privilegeState = {0};
	  RTC_TimeTypeDef sTime = {0};
	  RTC_DateTypeDef sDate = {0};
	  //RTC_AlarmTypeDef sAlarm = {0};

	  hrtc.Instance = RTC;
	  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	  hrtc.Init.AsynchPrediv = 127;
	  hrtc.Init.SynchPrediv = 255;
	  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
	  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
	  hrtc.Init.BinMode = RTC_BINARY_NONE;
	  if (HAL_RTC_Init(&hrtc) != HAL_OK)
	  {
		Error_Handler();
	  }
	  privilegeState.rtcPrivilegeFull = RTC_PRIVILEGE_FULL_NO;
	  privilegeState.backupRegisterPrivZone = RTC_PRIVILEGE_BKUP_ZONE_NONE;
	  privilegeState.backupRegisterStartZone2 = RTC_BKP_DR0;
	  privilegeState.backupRegisterStartZone3 = RTC_BKP_DR0;
	  if (HAL_RTCEx_PrivilegeModeSet(&hrtc, &privilegeState) != HAL_OK)
	  {
		Error_Handler();
	  }
  
  if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != 0x32F2)
  {
	//HAL_PWREx_EnableBkupRAMRetention();
    //HAL_RTC_Init(&hrtc);
    sTime.Hours = 22;
    sTime.Minutes = 40;
    sTime.Seconds = 0;
    sTime.SubSeconds = 0;
    sTime.TimeFormat = RTC_HOURFORMAT12_AM;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    HAL_RTC_SetTime(&hrtc, &sTime, FORMAT_BCD);

    sDate.WeekDay = RTC_WEEKDAY_SATURDAY;
    sDate.Month = RTC_MONTH_APRIL;
    sDate.Date = 0x19;
    sDate.Year = 0x25;

    HAL_RTC_SetDate(&hrtc, &sDate, FORMAT_BCD);

    HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR0,0x32F2);
  }
  //else
  //{
    //HAL_RTC_MspInit(&hrtc);
  //}
  
  //HAL_PWR_DisableBkUpAccess();
}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();
    __HAL_RCC_RTCAPB_CLK_ENABLE();
    __HAL_RCC_RTCAPB_CLKAM_ENABLE();

    /* RTC interrupt Init */
    HAL_NVIC_SetPriority(RTC_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(RTC_IRQn);
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
    __HAL_RCC_RTCAPB_CLK_DISABLE();
    __HAL_RCC_RTCAPB_CLKAM_DISABLE();

    /* RTC interrupt Deinit */
    HAL_NVIC_DisableIRQ(RTC_IRQn);
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
}

RTC_TimeTypeDef ReturnNextStartTime(Swift_Schedule *swiftschedule)
{
  RTC_TimeTypeDef ScheduleTime;
  uint8_t tmpHours = 0;
  uint16_t tmpMinutes = 0;
  
  if(swiftschedule->ScheduleType == SCHEDULE_TYPE_DUTYCYCLED)
  {
    tmpHours = ((swiftschedule->StartStopTimes[0].RecordingStopTime.Hours) * (swiftschedule->CurrentRecordNumber));
    tmpMinutes = (uint16_t)((uint16_t)(swiftschedule->StartStopTimes[0].RecordingStopTime.Minutes)  * (uint16_t)(swiftschedule->CurrentRecordNumber));
    
    while (tmpMinutes > 59)
    {
      tmpHours = tmpHours + 1;
      tmpMinutes = tmpMinutes - 60;
    }
    
    ScheduleTime.Hours = tmpHours;
    ScheduleTime.Minutes = (uint8_t)tmpMinutes;
    
  }
  else //(swiftschedule->ScheduleType == SCHEDULE_TYPE_ARBITRARY)
  {
    ScheduleTime.Hours = (swiftschedule->StartStopTimes[swiftschedule->CurrentRecordNumber].RecordingStartTime.Hours);
    ScheduleTime.Minutes = (swiftschedule->StartStopTimes[swiftschedule->CurrentRecordNumber].RecordingStartTime.Minutes);
  }
  
  return ScheduleTime;
}

void SetNextStartTime(Swift_Schedule *swiftschedule)
{
  RTC_TimeTypeDef ScheduleTime;
  uint8_t tmpHours = 0;
  uint16_t tmpMinutes = 0;
  
  swiftschedule->CurrentRecordNumber++;
  
  if(swiftschedule->CurrentRecordNumber >= swiftschedule->NumberOfRecords)
  {
    swiftschedule->CurrentRecordNumber = 0;
  }
  if(swiftschedule->ScheduleType == SCHEDULE_TYPE_DUTYCYCLED)
  {
    if(swiftschedule->CurrentRecordNumber != 0) //Duty cycle schedules always start at midnight. We dont want AlarmA to start the recording because AlarmB will!
    {
      tmpHours = ((swiftschedule->StartStopTimes[0].RecordingStopTime.Hours) * (swiftschedule->CurrentRecordNumber));
      tmpMinutes = (uint16_t)((uint16_t)(swiftschedule->StartStopTimes[0].RecordingStopTime.Minutes)  * (uint16_t)(swiftschedule->CurrentRecordNumber));
      
      while (tmpMinutes > 59)
      {
        tmpHours = tmpHours + 1;
        tmpMinutes = tmpMinutes - 60;
      }
      
      ScheduleTime.Hours = tmpHours;
      ScheduleTime.Minutes = (uint8_t)tmpMinutes;
      
      RTC_SetAlarmA(ScheduleTime);
    }    
  }
  else if(swiftschedule->ScheduleType == SCHEDULE_TYPE_ARBITRARY)
  { //if CurrentRecordNumber = 0 AND StartsAtMidnightFlag == 1 then we dont want to do anything (because AlarmB handles this)
    if(!((swiftschedule->CurrentRecordNumber == 0) && (swiftschedule->StartsAtMidnightFlag == 1))) //<= THIS IS UNTESTED CODE!! MIGHT NEED TO BE DEBUGGED WHEN WE GET TO ARBITRARY SCHEDULE STUFF
    {
      ScheduleTime.Hours = (swiftschedule->StartStopTimes[swiftschedule->CurrentRecordNumber].RecordingStartTime.Hours);
      ScheduleTime.Minutes = (swiftschedule->StartStopTimes[swiftschedule->CurrentRecordNumber].RecordingStartTime.Minutes);
      RTC_SetAlarmA(ScheduleTime);
    }
  } 
}

RTC_TimeTypeDef ReturnNextStopTime(Swift_Schedule *swiftschedule)
{
  RTC_TimeTypeDef ScheduleTime;
  uint8_t tmpHours = 0;
  uint16_t tmpMinutes = 0;
  
  if(swiftschedule->ScheduleType == SCHEDULE_TYPE_DUTYCYCLED)
  {
    tmpHours = (swiftschedule->StartStopTimes[0].RecordingStopTime.Hours) * (swiftschedule->CurrentRecordNumber);
    tmpMinutes = (uint16_t)((uint16_t)(swiftschedule->StartStopTimes[0].RecordingStopTime.Minutes)  * (uint16_t)(swiftschedule->CurrentRecordNumber));

    tmpHours = tmpHours + (swiftschedule->StartStopTimes[0].RecordingStartTime.Hours);
    tmpMinutes = tmpMinutes + (uint16_t)(swiftschedule->StartStopTimes[0].RecordingStartTime.Minutes);
    
    while (tmpMinutes > 59)
    {
      tmpHours = tmpHours + 1;
      tmpMinutes = tmpMinutes - 60;
    }
    
    ScheduleTime.Hours = tmpHours;
    ScheduleTime.Minutes = (uint8_t)tmpMinutes;
  }
  else
  {
    ScheduleTime.Hours = (swiftschedule->StartStopTimes[swiftschedule->CurrentRecordNumber].RecordingStopTime.Hours);
    ScheduleTime.Minutes = (swiftschedule->StartStopTimes[swiftschedule->CurrentRecordNumber].RecordingStopTime.Minutes);
  }
  
  return ScheduleTime;
}

void SetNextStopTime(Swift_Schedule *swiftschedule)
{
  RTC_TimeTypeDef ScheduleTime;
  uint8_t tmpHours = 0;
  uint16_t tmpMinutes = 0;
  
  //HAL_PWR_EnableBkUpAccess();
  __HAL_RTC_ALARM_CLEAR_FLAG(&hrtc, RTC_FLAG_ALRAF);
  
  if(swiftschedule->ScheduleType == SCHEDULE_TYPE_DUTYCYCLED)
  {
    tmpHours = (swiftschedule->StartStopTimes[0].RecordingStopTime.Hours) * (swiftschedule->CurrentRecordNumber);
    tmpMinutes = (uint16_t)((uint16_t)(swiftschedule->StartStopTimes[0].RecordingStopTime.Minutes)  * (uint16_t)(swiftschedule->CurrentRecordNumber));

    tmpHours = tmpHours + (swiftschedule->StartStopTimes[0].RecordingStartTime.Hours);
    tmpMinutes = tmpMinutes + (uint16_t)(swiftschedule->StartStopTimes[0].RecordingStartTime.Minutes);
    
    while (tmpMinutes > 59)
    {
      tmpHours = tmpHours + 1;
      tmpMinutes = tmpMinutes - 60;
    }
    
    ScheduleTime.Hours = tmpHours;
    ScheduleTime.Minutes = (uint8_t)tmpMinutes;
    
    RTC_SetAlarmA(ScheduleTime);
  }
  else if(swiftschedule->ScheduleType == SCHEDULE_TYPE_ARBITRARY)
  {//if (CurrentRecordNumber = swiftschedule->NumberOfRecords) AND (EndsAtMidnightFlag == 1) then we dont want to do anything (because AlarmB handles this)
    if(!(((swiftschedule->CurrentRecordNumber+1) == swiftschedule->NumberOfRecords) && (swiftschedule->EndsAtMidnightFlag == 1))) //<= THIS IS UNTESTED CODE!! MIGHT NEED TO BE DEBUGGED WHEN WE GET TO ARBITRARY SCHEDULE STUFF
    {
      ScheduleTime.Hours = (swiftschedule->StartStopTimes[swiftschedule->CurrentRecordNumber].RecordingStopTime.Hours);
      ScheduleTime.Minutes = (swiftschedule->StartStopTimes[swiftschedule->CurrentRecordNumber].RecordingStopTime.Minutes);
      RTC_SetAlarmA(ScheduleTime);
    }
  }
  else //This will be continuous recording option
  {
    //Continuous recording stop time will always be 23:59:00 for version 1.0.1.0
    ScheduleTime.Hours = 23;
    ScheduleTime.Minutes = 59;
    RTC_SetAlarmA(ScheduleTime);
  }
}

uint8_t RTC_InitializeScheduleAlarms(Swift_Schedule *swiftschedule)
{
  /****************************************************************************/
  //If we return 0 go into standby state
  //If we return 1 go to record state
  /****************************************************************************/
  
  RTC_TimeTypeDef CurrentTime;//, ScheduleTime;
  volatile RTC_TimeTypeDef ScheduleTime;
  volatile RTC_DateTypeDef ScheduleDate;
  volatile uint8_t returnval;
  
  //HAL_PWR_EnableBkUpAccess();
  __HAL_RTC_ALARM_CLEAR_FLAG(&hrtc, RTC_FLAG_ALRAF);
    
  if(swiftschedule->NumberOfRecords == 0)
  {
    //BOOTING WITH NO VALID SCHEDULE
    HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A); //Disable AlarmA
    HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_B); //Disable AlarmB
    swiftschedule->CurrentRecordNumber = 0;
    //HAL_PWR_DisableBkUpAccess();
    return 0;
  } 
  
  //** GET SCHEDULE STOP DATE***************************
  //Convert BCD values to binary
  //ScheduleDate.Year = (RTC_Bcd2ToByte(swiftschedule->StopDate.Year) + 2000) - 1900;
  ScheduleDate.Year = RTC_Bcd2ToByte(swiftschedule->StopDate.Year);
  ScheduleDate.Month = RTC_Bcd2ToByte(swiftschedule->StopDate.Month);
  ScheduleDate.Date = RTC_Bcd2ToByte(swiftschedule->StopDate.Date);
  
  //** IS STOP DATE IN THE PAST?**********************
  returnval = RTC_CompareDates(ScheduleDate);
  if((returnval == 2) || (returnval == 0)) //Was if(returnval == 2) **Changed this 5/9/2020 so the recording will stop on end date instead of day after
  {
    //BOOTING WITH NO VALID SCHEDULE (SCHEDULE IS IN THE PAST)
    HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
    HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_B);
    swiftschedule->CurrentRecordNumber = 0;    
    //HAL_PWR_DisableBkUpAccess();
    return 0;
  }
  
  //** GET SCHEDULE START DATE***************************
  //Convert BCD values to binary
  //ScheduleDate.Year = (RTC_Bcd2ToByte(swiftschedule->StartDate.Year) + 2000) - 1900;
  ScheduleDate.Year = RTC_Bcd2ToByte(swiftschedule->StartDate.Year);
  ScheduleDate.Month = RTC_Bcd2ToByte(swiftschedule->StartDate.Month);
  ScheduleDate.Date = RTC_Bcd2ToByte(swiftschedule->StartDate.Date);
  
  //** IS START DATE IN THE FUTURE?**********************
  returnval = RTC_CompareDates(ScheduleDate);
  if(returnval == 1)
  {
    //BOOTING BEFORE A DUTY-CYCLED OR CONTINUOUS RECORDING SCHEDULE
    HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
    RTC_EnableAlarmB();
    swiftschedule->CurrentRecordNumber = 0;
    //HAL_PWR_DisableBkUpAccess();
    return 0;
  }
  
  //If we get here, the start date is either in the past or equal to the current date. Also the stop date is in the future
  //This means we are either in the middle of a continuous recording schedule, or a duty-cycled recording schedule
  
  //** IS THIS A CONTINUOUS RECORDING?*******************
  //** NOTE: I AM COMMENTING THIS OUT FOR VERSION 1.0.1.0 (12/28/2020) ***
  //if(swiftschedule->ScheduleType == 3)
  //{
    //BOOTING IN THE MIDDLE OF A CONTINUOUS RECORDING SCHEDULE
    //RTC_DeactivateAlarm(RTC_CR_ALRAE); //Deactivate Alarm A
    //RTC_EnableAlarmB();
    //swiftschedule->CurrentRecordNumber = 0;
    //return 1; //Go directly into record mode
  //}
  
  //If we get here, we are in the middle of a duty-cycled recording schedule.
  //Now we need to check the schedule times to see if we are in the middle
  //of a recording period
  
  //** GET CURRENT TIME**********************************
  HAL_RTC_GetTime(&hrtc, &CurrentTime, RTC_FORMAT_BIN);
  
  for(int i = 0; i < swiftschedule->NumberOfRecords; i++)
  {
    swiftschedule->CurrentRecordNumber = i;
    
    ScheduleTime = ReturnNextStartTime(swiftschedule);
    //** IS THIS SCHEDULE START TIME IN THE FUTURE? I.E ScheduleTime > CurrentTime?*****
    returnval = RTC_CompareTimes(ScheduleTime, CurrentTime);
    if(returnval == 1)
    {
      //BOOTING IN MIDDLE OF DUTY-CYCLED RECORDING SCHEDULE - SCENARIO 1
      //We are in standby period between records
      RTC_SetAlarmA(ScheduleTime);
      RTC_EnableAlarmB();
      //swiftschedule->CurrentRecordNumber = i;    
      //HAL_PWR_DisableBkUpAccess();
      return 2;
    }
    
    //** IS CURRENT TIME LESS THEN NEXT STOP TIME IN SCHEDULE ARRAY?**************
    ScheduleTime = ReturnNextStopTime(swiftschedule);
    returnval = RTC_CompareTimes(ScheduleTime, CurrentTime);
    if(returnval == 1)
    {
      //BOOTING IN MIDDLE OF DUTY-CYCLED RECORDING SCHEDULE - SCENARIO 2
      //We are in the middle of a record interval
      SetNextStopTime(swiftschedule);//RTC_SetAlarmA(ScheduleTime);
      RTC_EnableAlarmB();
      //swiftschedule->CurrentRecordNumber = i;//(i + 1);      
      //HAL_PWR_DisableBkUpAccess();
      return 1;
    }
  }
  
  //**The time schedule is over for the day
  HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
  RTC_EnableAlarmB();
  swiftschedule->CurrentRecordNumber = 0;
  //HAL_PWR_DisableBkUpAccess();
  return 0;
}

uint8_t RTC_CompareTimes(RTC_TimeTypeDef ScheduleTime, RTC_TimeTypeDef CurrentTime)
{  
  //return 0 if ScheduleTime equals CurrentTime
  //return 1 if ScheduleTime is greater then CurrentTime (ScheduleTime is in the future)
  //return 2 if ScheduleTime is less then CurrentTime (ScheduleTime is in the past)
  //return 3 if error
  
  uint8_t ReturnVal;
  
  if(ScheduleTime.Hours > CurrentTime.Hours) //ScheduleTime is in the future
  {    
    //return 1;
    ReturnVal = 1;
  }
  else if(ScheduleTime.Hours < CurrentTime.Hours) //ScheduleTime is in the past
  {    
    //return 2;
    ReturnVal = 2;
  }
  else //ScheduleTime.Hours equals CurrentTime.Hours
  {
    if(ScheduleTime.Minutes > CurrentTime.Minutes) //ScheduleTime is in the future
    {
      //return 1;
      ReturnVal = 1;
    }
    else if(ScheduleTime.Minutes < CurrentTime.Minutes) //ScheduleTime is in the past
    {
      //return 2;
      ReturnVal = 2;
    }
    else //ScheduleTime is equal to Current Time
    {
      //return 0;
      ReturnVal = 0;
    }
  }
  
  return ReturnVal; //Something went wrong, you should never get here
}

uint8_t RTC_CompareDates(RTC_DateTypeDef ScheduleDate)
{  
  //return 0 if ScheduleDate equals CurrentDate
  //return 1 if ScheduleDate is greater then CurrentDate (ScheduleDate is in the future)
  //return 2 if ScheduleDate is less then CurrentDate (ScheduleDate is in the past)
  //return 3 if error
  
  uint8_t ReturnVal;
  RTC_DateTypeDef CurrentDate;

  HAL_RTC_GetDate(&hrtc, &CurrentDate, RTC_FORMAT_BIN); 
  
  if(ScheduleDate.Year > CurrentDate.Year) //ScheduleDate is in the future
  {    
    //return 1;
    ReturnVal = 1;
  }
  else if(ScheduleDate.Year < CurrentDate.Year) //ScheduleDate is in the past
  {    
    //return 2;
    ReturnVal = 2;
  }
  else //ScheduleDate.Year equals CurrentDate.Year
  {
    if(ScheduleDate.Month > CurrentDate.Month)//ScheduleDate is in the future
    {
      //return 1;
      ReturnVal = 1;
    }
    else if(ScheduleDate.Month < CurrentDate.Month)//ScheduleDate is in the past
    {
      //return 2;
      ReturnVal = 2;
    }
    else //ScheduleDate.Month equals CurrentDate.Month
    {
      if(ScheduleDate.Date > CurrentDate.Date)//ScheduleDate is in the future
      {
        //return 1;
        ReturnVal = 1;
      }
      else if(ScheduleDate.Date < CurrentDate.Date)//ScheduleDate is in the past
      {
        //return 2;
        ReturnVal = 2;
      }
      else //ScheduleDate equals CurrentDate. Return zero
      {
        //return 0;
        ReturnVal = 0;
      }      
    }
  }
  
  return ReturnVal; //Something went wrong, you should never get here
}

uint8_t RTC_CheckIfBetweenDates(RTC_DateTypeDef StartDate, RTC_DateTypeDef StopDate)
{
  //This function checks to see if todays date is between StartDate and StopDate
  //Return '1' if current date is between StartDate and StopDate, else return '0'
  RTC_DateTypeDef BinaryFormatDate;
  uint8_t CompareResult;
  uint8_t ReturnVal = 0;
  
  BinaryFormatDate.Year = RTC_Bcd2ToByte(StartDate.Year);
  BinaryFormatDate.Month = RTC_Bcd2ToByte(StartDate.Month);
  BinaryFormatDate.Date = RTC_Bcd2ToByte(StartDate.Date);
    
  CompareResult = RTC_CompareDates(BinaryFormatDate);
  
  //** IS START DATE TODAY OR IN THE PAST?*************
  if((CompareResult == 0) || (CompareResult == 2))
  {
    BinaryFormatDate.Year = RTC_Bcd2ToByte(StopDate.Year);
    BinaryFormatDate.Month = RTC_Bcd2ToByte(StopDate.Month);
    BinaryFormatDate.Date = RTC_Bcd2ToByte(StopDate.Date);
    CompareResult = RTC_CompareDates(BinaryFormatDate);
    //** IS STOP DATE TODAY OR IN THE FUTURE?*************
    if(CompareResult == 1)//if(CompareResult < 2) **Changed this 5/9/2020 so the recording will stop on end date instead of day after
    {
      ReturnVal = 1;
    }
  }
  
  return ReturnVal;
}

uint8_t RTC_SetAlarmA(RTC_TimeTypeDef TimeOfAlarm)
{  
  //This function only configures Alarm A. Alarm A will just keep track of on/off
  //times during the day (no start or stop dates)
  
  RTC_AlarmTypeDef sAlarm;
  
  sAlarm.AlarmTime.Hours = TimeOfAlarm.Hours; //StartTime->Hours;
  sAlarm.AlarmTime.Minutes = TimeOfAlarm.Minutes;
  sAlarm.AlarmTime.Seconds = 0x00;//TimeOfAlarm->Seconds;
  sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY; //Do not trigger alarm on date or weekday //RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 0x00;//StartDate->Date;
  sAlarm.Alarm = RTC_ALARM_A;
  
  //HAL_PWR_EnableBkUpAccess();
  
  //HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, FORMAT_BCD); OLD CODE USED BCD FORMAT BUT CHANGED THE FORMAT AROUND 12/31/2017
  HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN);
  
  //HAL_PWR_DisableBkUpAccess();
  
  return 1;
}

uint8_t RTC_EnableAlarmB(void)
{
  //Alarm B will be used for 24 hour end-of-day alarm. 
  //This alarm should also check date, month, and year in interrupt subroutine
  
  RTC_AlarmTypeDef sAlarm;
  
  sAlarm.AlarmTime.Hours = 0x00;
  sAlarm.AlarmTime.Minutes = 0x00;
  sAlarm.AlarmTime.Seconds = 0x00;
  sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY; //Do not trigger alarm on date or weekday
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 0x00;
  sAlarm.Alarm = RTC_ALARM_B;
 
  //HAL_PWR_EnableBkUpAccess();
  HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, FORMAT_BCD);
  //HAL_PWR_DisableBkUpAccess();
  
  return 1;
}

uint8_t RTC_DeactivateAlarm(uint32_t WhichAlarm)
{
  //HAL_PWR_EnableBkUpAccess();
  HAL_RTC_DeactivateAlarm(&hrtc, WhichAlarm);
  //HAL_PWR_DisableBkUpAccess();
  
  return 1;
}

void RTC_SetDateTime(uint8_t *DateTimeArray)
{  
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;

  sDate.Year = DateTimeArray[1];
  sDate.Month = DateTimeArray[2];
  sDate.Date = DateTimeArray[3];
  sDate.WeekDay = DateTimeArray[4];
  sTime.Hours = DateTimeArray[5];
  sTime.Minutes = DateTimeArray[6];
  sTime.Seconds = DateTimeArray[7];
  sTime.SubSeconds = 0;
  
  sTime.TimeFormat = RTC_HOURFORMAT12_AM;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  
  //HAL_PWR_EnableBkUpAccess();
  HAL_RTC_SetTime(&hrtc, &sTime, FORMAT_BCD);
  HAL_RTC_SetDate(&hrtc, &sDate, FORMAT_BCD);
  //HAL_PWR_DisableBkUpAccess();
  
  //Now figure out what day of the year it is, and write that to battery backed SRAM
  //int day = get_yday(sDate.Month, sDate.Date, sDate.Year);
}

int yisleap(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int get_yday(int mon, int day, int year)
{
    static const int days[2][13] = {
        {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
        {0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}
    };
    int leap = yisleap(year);

    return days[leap][mon] + day;
}

/*
uint8_t* RTC_ReadDateTime(uint32_t BCD_Or_Bin_Format) //0x01 = BCD, 0x00 = BIN
{
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;

  static uint8_t buffer[8];
  
  HAL_RTC_GetTime(&hrtc, &sTime, BCD_Or_Bin_Format);
  HAL_RTC_GetDate(&hrtc, &sDate, BCD_Or_Bin_Format);
  
  buffer[1] = sDate.Year;
  buffer[2] = sDate.Month;
  buffer[3] = sDate.Date;
  buffer[4] = sDate.WeekDay;
  buffer[5] = sTime.Hours;
  buffer[6] = sTime.Minutes;
  buffer[7] = sTime.Seconds;
  
  return buffer;
}
*/

void RTC_ReadDateTime(uint8_t* buffer, uint32_t BCD_Or_Bin_Format) //0x01 = BCD, 0x00 = BIN
{
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;
  
  HAL_RTC_GetTime(&hrtc, &sTime, BCD_Or_Bin_Format);
  HAL_RTC_GetDate(&hrtc, &sDate, BCD_Or_Bin_Format);
  
  buffer[1] = sDate.Year;
  buffer[2] = sDate.Month;
  buffer[3] = sDate.Date;
  buffer[4] = sDate.WeekDay;
  buffer[5] = sTime.Hours;
  buffer[6] = sTime.Minutes;
  buffer[7] = sTime.Seconds;
}

void RTC_AddTimeAndSetAlarmA()
{
  time_t timeNow;
  struct tm time_info;
  struct tm *converted_time;
  RTC_TimeTypeDef sTimez;
  RTC_DateTypeDef sDatez;
  RTC_AlarmTypeDef sAlarm;
  
  HAL_RTC_GetTime(&hrtc, &sTimez, FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDatez, FORMAT_BIN);
  
  time_info.tm_year = (sDatez.Year + 2000) - 1900;
  time_info.tm_mon = sDatez.Month - 1;
  time_info.tm_mday = sDatez.Date;
  time_info.tm_hour = sTimez.Hours + 1; //<--This is weird that I have to add a 1 here. Not quite sure if this is legit
  time_info.tm_min = sTimez.Minutes;
  time_info.tm_sec = sTimez.Seconds;
  time_info.tm_isdst = sTimez.DayLightSaving;
  
  timeNow = mktime(&time_info);
  timeNow += 120;
  
  converted_time = localtime(&timeNow);
  
  //sTimez.Hours = time_info->tm_hour;
  //sTimez.Minutes = time_info->tm_min;
  //sTimez.Seconds = time_info->tm_sec;
  
  sAlarm.AlarmTime.Hours = converted_time->tm_hour;
  sAlarm.AlarmTime.Minutes = converted_time->tm_min;
  sAlarm.AlarmTime.Seconds = converted_time->tm_sec;
  sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY; //Do not trigger alarm on date or weekday //RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 0x00;//StartDate->Date;
  sAlarm.Alarm = RTC_ALARM_A;

  //HAL_PWR_EnableBkUpAccess();
  HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN);
  //HAL_PWR_DisableBkUpAccess();
}

void RTC_ReturnDateTimeString(char *buff)
{
  RTC_TimeTypeDef sTimez;
  RTC_DateTypeDef sDatez;
  
  struct tm now;
  
  HAL_RTC_GetTime(&hrtc, &sTimez, FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDatez, FORMAT_BIN);
  
  now.tm_year = (sDatez.Year + 2000) - 1900;
  now.tm_mon = sDatez.Month - 1;
  now.tm_mday = sDatez.Date;
  now.tm_hour = sTimez.Hours;
  now.tm_min = sTimez.Minutes;
  now.tm_sec = sTimez.Seconds;
  now.tm_isdst = sTimez.DayLightSaving;
  
  strftime(buff, 32, "Date-time: %Y/%m/%d %H:%M:%S\r\n", &now); 
}

void RTC_ReturnDateTimeFileName(char *buff)
{
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;
  
  struct tm now;
  
  HAL_RTC_GetTime(&hrtc, &sTime, FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDate, FORMAT_BIN);
  
  now.tm_year = (sDate.Year + 2000) - 1900;
  now.tm_mon = sDate.Month - 1;
  now.tm_mday = sDate.Date;
  now.tm_hour = sTime.Hours;
  now.tm_min = sTime.Minutes;
  now.tm_sec = sTime.Seconds;
  now.tm_isdst = sTime.DayLightSaving;

  strftime(buff, 20, "%Y%m%d_%H%M%S.wav", &now);
}

void RTC_ReturnDateString(char *buff)
{
  //RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;
  
  struct tm now;
  
  HAL_RTC_GetDate(&hrtc, &sDate, FORMAT_BIN);
  
  now.tm_year = (sDate.Year + 2000) - 1900;
  now.tm_mon = sDate.Month - 1;
  now.tm_mday = sDate.Date;
  now.tm_hour = 0;
  now.tm_min = 0;
  now.tm_sec = 0;
  now.tm_isdst = 0;

  strftime(buff, 11, "%Y-%m-%d", &now);
}

/*
void RTC_ReturnDateTimeString(char *buff)
{
  static int Year = 0;
  static int Month = 0;
  static int Day = 0;
  static int Hour = 0;
  static int Minute = 0;
  static int Second = 0;

  char YearString[5];
  char MonthString[3];
  char DayString[3]; //"DD"
  char HourString[3];
  char MinuteString[3];
  char SecondString[3];
  
  sprintf(YearString, "%04d", Year);
}
*/
//char* RTC_ReturnDateTimeString(char StringFormat)
//{
  //-- StringFormat --------------------------------------------------
  //-- If StringFormat = 0, this function returns "YYYYMMDD_HHMMSS.wav"
  //-- If StringFormat = 1, this function returns "MM/DD/YYYY HH:MM:SS"
  //------------------------------------------------------------------
/*  static char CompleteDateString[20];
  static char YearString[5];
  static char MonthString[3];
  static char DayString[3]; //"DD"
  static char HourString[3];
  static char MinuteString[3];
  static char SecondString[3];
  
  HAL_RTC_GetTime(&hrtc, &sTime, FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDate, FORMAT_BIN);
  
  sprintf(YearString, "%04d", (sDate.Year + 2000));
  sprintf(MonthString, "%02d", sDate.Month);
  sprintf(DayString, "%02d", sDate.Date);
  
  sprintf(HourString, "%02d", sTime.Hours);
  sprintf(MinuteString, "%02d", sTime.Minutes);
  sprintf(SecondString, "%02d", sTime.Seconds);
  
  CompleteDateString[0] = '\0';
  
  if(StringFormat == 1)
  {
    strcat(CompleteDateString,MonthString);
    strcat(CompleteDateString,"/");
    strcat(CompleteDateString,DayString);
    strcat(CompleteDateString,"/");
    strcat(CompleteDateString,YearString);
    strcat(CompleteDateString," ");
    strcat(CompleteDateString,HourString);
    strcat(CompleteDateString,":");
    strcat(CompleteDateString,MinuteString);
    strcat(CompleteDateString,":");
    strcat(CompleteDateString,SecondString);
  }
  else
  {
    strcat(CompleteDateString,YearString);
    strcat(CompleteDateString,MonthString);
    strcat(CompleteDateString,DayString);
    strcat(CompleteDateString,"_");
    strcat(CompleteDateString,HourString);
    strcat(CompleteDateString,MinuteString);
    strcat(CompleteDateString,SecondString);
    strcat(CompleteDateString,".wav");
  }
  
  return CompleteDateString;
}
*/
