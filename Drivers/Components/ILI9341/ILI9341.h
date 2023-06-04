#ifndef __ILI9341_H
#define __ILI9341_H

#include "bsp.h"

#define LCDID_UNKNOWN 0
#define LCDID_ILI9341 0x9341
#define LCDID_ST7789V 0x8552

void ILI9341_REG_Config(void);
void ILI9341_RESET(void);
uint16_t ILI9341_ReadID(void);
void ILI9341_GramScan(uint8_t ucOption);
void ILI9341_DisplayOn(void);
void ILI9341_DisplayOff(void);
void ILI9341_SetDisplayWindow(uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight);
void ILI9341_SetCursor(uint16_t usX, uint16_t usY);
uint16_t ILI9341_GetLcdPixelWidth(void);
uint16_t ILI9341_GetLcdPixelHeight(void);
#endif /* __ILI9341_H */
