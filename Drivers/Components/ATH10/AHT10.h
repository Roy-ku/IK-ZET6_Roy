#ifndef __AHT10_H
#define __AHT10_H

#include "bsp.h"

#define AHT10_ADDRESS 0x70 // 7-bit


uint8_t AHT10_DevAddr(void);
uint8_t AHT10_Reset(void);
uint8_t AHT10_Correction(void);
uint8_t AHT10_ReadTEMPHUM(float *humidity, float *temperature);
#endif /* __AHT10_H */
