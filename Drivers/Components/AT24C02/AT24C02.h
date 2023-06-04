#ifndef __AT24C02_H
#define __AT24C02_H

#include "bsp.h"

/*
 * AT24C02 2kb = 2048bit = 2048/8 B = 256 B
 * 32 pages of 8 bytes each
 *
 * Device Address
 * 1 0 1 0 A2 A1 A0 R/W
 * 1 0 1 0 0  0  0  0 = 0XA0
 * 1 0 1 0 0  0  0  1 = 0XA1
 */
#define AT24C02_ADDRESS 0xA0 // 7-bit
#define AT24C02_PAGESIZE 8

uint8_t at24c02_DevAddr(void);
uint8_t at24c02_PageSize(void);
uint8_t at24c02_ByteWrite(uint8_t value, uint16_t WriteAddr);
uint8_t at24c02_BufferRead(uint8_t *pBuffer, uint16_t ReadAddr, uint16_t NumByteToRead);
uint8_t at24c02_BufferWrite(uint8_t *pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite);
#endif /* __AT24C02_H */
