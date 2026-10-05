//Rob's Realtime Clock Code

#ifndef __RTC_H
#define __RTC_H
#include "SwiftSettings.h"

uint8_t RTC_InitializeScheduleAlarms(Swift_Schedule *);
void RTC_Init(void);
void RTC_SetDateTime(uint8_t *);
void RTC_ReadDateTime(uint8_t*, uint32_t);

uint8_t RTC_CheckIfBetweenDates(RTC_DateTypeDef, RTC_DateTypeDef);

void SetNextStartTime(Swift_Schedule *);
void SetNextStopTime(Swift_Schedule *);

uint8_t RTC_CompareDates(RTC_DateTypeDef);

void RTC_ReturnDateTimeString(char *);
void RTC_ReturnDateTimeFileName(char *);

void RTC_AddTimeAndSetAlarmA(void);

char* RTC_ReturnDateTimeStringWithCount(void);

void RTC_ReturnFileNumberString(char*);


uint8_t RTC_SetAlarmA(RTC_TimeTypeDef );
uint8_t RTC_EnableAlarmB(void);

uint8_t RTC_DeactivateAlarm(uint32_t);

void RTC_ReturnDateString(char *);

void RTC_TestFunction(void);

#endif /* __RTC_H */