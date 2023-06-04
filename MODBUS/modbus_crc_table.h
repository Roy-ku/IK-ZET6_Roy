#ifndef __BSP_USER_LIB_H
#define __BSP_USER_LIB_H

uint16_t BEBufToUint16(uint8_t *_pBuf);
uint16_t LEBufToUint16(uint8_t *_pBuf);
uint32_t BEBufToUint32(uint8_t *_pBuf);
uint32_t LEBufToUint32(uint8_t *_pBuf);
uint16_t CRC16_Modbus(uint8_t *_pBuf, uint16_t _usLen);

#endif
/***************************** (END OF FILE) *********************************/
