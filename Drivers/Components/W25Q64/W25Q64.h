#ifndef __W25Q64_H
#define __W25Q64_H

#include "bsp.h"

void w25q64_CS_ENABLE(void);
void w25q64_CS_DISABLE(void);
void w25q64_ReadInfo(void);
void w25q64_BufferRead(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
uint8_t w25q64_BufferWrite(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void w25q64_EraseSector(uint32_t _uiSectorAddr);
#endif /* __w25q64_H */
