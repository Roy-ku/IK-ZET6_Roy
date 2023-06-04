#ifndef __I2C_EE_H
#define __I2C_EE_H

#include "bsp.h"

/*********************************************************************************************************/
/* 軟體IIC */
#define EEPROM_SOFTIIC 1
/*********************************************************************************************************/

typedef struct
{
    char ChipName[10];
    uint8_t MemaddrSize;
    uint8_t (*DrvAddr)(void);
    uint8_t (*PageSize)(void);
    uint8_t (*ByteWrite)(uint8_t value, uint16_t WriteAddr);
    uint8_t (*BufferRead)(uint8_t *pBuffer, uint16_t ReadAddr, uint16_t NumByteToRead);
    uint8_t (*BufferWrite)(uint8_t *pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite);

} EEPROM_DrvTypeDef;
extern EEPROM_DrvTypeDef at24c02_drv;

void bsp_EEPROM_init(void);
bsp_Status bsp_I2C_EEPROM_BufferWrite(uint16_t drvaddr, uint8_t *pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite);
bsp_Status bsp_I2C_EEPROM_ByteWrite(uint16_t drvaddr, uint8_t value, uint16_t WriteAddr);
bsp_Status bsp_I2C_EEPROM_BufferRead(uint16_t drvaddr, uint8_t *pBuffer, uint16_t ReadAddr, uint16_t NumByteToRead);
bsp_Status bsp_I2C_EEPROM_Test(void);

#endif /* __I2C_EE_H */
