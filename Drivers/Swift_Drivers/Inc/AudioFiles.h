/*
 * AudioFiles.h
 *
 *  Created on: Nov 23, 2014
 *      Author: rak287
 */
#include "integer.h"
#include "ff.h"
#include "SwiftErrors.h"
#include "SwiftSettings.h"

#ifndef AUDIOFILES_H_
#define AUDIOFILES_H_

uint8_t AudioFiles_MountSDCard(SWIFT_ERRORS*);
//uint8_t AudioFiles_CreateNextDirectory(SWIFT_ERRORS*);
//uint8_t AudioFiles_CreateNextFile(SWIFT_ERRORS*, FIL*, uint32_t);
uint8_t AudioFiles_WriteHeader(SWIFT_ERRORS*, FIL*);
uint8_t AudioFiles_CloseFile(SWIFT_ERRORS*, FIL*);

//uint8_t AudioFiles_GetDiskInfoString(char*);
void AudioFiles_GetDiskInfoString(Message*);

uint8_t AudioFiles_NewRecordingDirectory(WAVFile_Attributes*);
uint8_t AudioFiles_NewDayDirectory(uint8_t*);
uint8_t AudioFiles_NewAudioFile(uint8_t*, FIL*);

uint8_t AudioFiles_ChangeDirectory(void);

uint8_t AudioFiles_NewConfigTextFile(void);

#endif /* AUDIOFILES_H_ */
