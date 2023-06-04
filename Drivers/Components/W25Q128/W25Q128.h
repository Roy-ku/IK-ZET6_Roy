#ifndef __W25Q128_H
#define __W25Q128_H

#include "bsp.h"


void w25q128_CS_ENABLE(void);
void w25q128_CS_DISABLE(void);
void w25q128_ReadInfo(void);
void w25q128_BufferRead(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
uint8_t w25q128_BufferWrite(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void w25q128_EraseSector(uint32_t _uiSectorAddr);
#endif /* __W25Q128_H */
