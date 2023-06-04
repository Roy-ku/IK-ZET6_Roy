#ifndef __PCF8574T_H
#define __PCF8574T_H

#include "bsp.h"

typedef enum
{
    P0,
    P1,
    P2,
    P3,
    P4,
    P5,
    P6,
    P7
} PCF8574_GPIOPin;

#define PCF8574T_ADDRESS 0x40 // 7-bit

uint8_t PCF8574T_DevAddr(void);
uint8_t PCF8574T_WritePin(PCF8574_GPIOPin pin, uint8_t pinstate);
uint8_t PCF8574T_TogglePin(PCF8574_GPIOPin pin);
uint8_t PCF8574T_ReadPin(PCF8574_GPIOPin pin);
uint8_t PCF8574T_WriteByte(uint8_t value);
uint8_t PCF8574T_ReadByte(uint8_t *value);
#endif /* __PCF8574T_H */
