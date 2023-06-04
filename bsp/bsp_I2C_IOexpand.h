#ifndef __I2C_IOEXPAND_H
#define __I2C_IOEXPAND_H

#include "PCF8574T.h"
#include "bsp.h"

/*********************************************************************************************************/
/* 軟體IIC */
#define IOEXPAND_SOFTIIC 1
/*********************************************************************************************************/

typedef struct
{
    char ChipName[10];
    uint8_t (*DrvAddr)(void);
    uint8_t (*WritePin)(uint8_t pin, uint8_t pinstate); /* pin : 請參考PCF8574T.h 中的 PCF8574_GPIOPin*/
    uint8_t (*ReadPin)(uint8_t pin);
    uint8_t (*TogglePin)(uint8_t pin);
    uint8_t (*WriteByte)(uint8_t value);
    uint8_t (*ReadByte)(uint8_t *value);
} IOExpand_DrvTypeDef;

extern IOExpand_DrvTypeDef pcf8574Tt_drv;

void bsp_IOEXPAND_Init(void);
bsp_Status bsp_I2C_IOEXPAND_Write(uint8_t drvaddr, uint8_t value);
bsp_Status bsp_I2C_IOEXPAND_Read(uint8_t drvaddr, uint8_t *value);
#endif /* __I2C_AHT10_H */
