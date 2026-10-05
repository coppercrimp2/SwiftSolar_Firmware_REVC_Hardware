//WriteToFlash.h

#ifndef __WriteToFlash_H
#define __WriteToFlash_H

uint8_t WriteToFlash(uint8_t*, uint32_t, uint16_t);
uint8_t ReadFromFlash(uint8_t*, uint32_t, uint16_t);
uint8_t EraseFlashSector(void);
uint8_t CheckFlashDefaults(void);

#endif /* __WriteToFlash_H */