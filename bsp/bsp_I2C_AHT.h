#ifndef __I2C_AHT_H
#define __I2C_AHT_H

#include "bsp.h"

/*********************************************************************************************************/
/* 軟體IIC */
#define AHT_SOFTIIC 1
/*********************************************************************************************************/

typedef struct
{
    char ChipName[8];
    uint8_t (*DrvAddr)(void);
    uint8_t (*Reset)(void);
    uint8_t (*Correction)(void);
    uint8_t (*ReadTEMPHUM)(float *humidity, float *temperatur);
} AHT_DrvTypeDef;

extern AHT_DrvTypeDef aht10_drv;
extern float AHT10_Humidity, AHT10_Temperature;
void bsp_AHT_Init(void);
bsp_Status bsp_I2C_AHT_Reset(uint8_t drvaddr, uint8_t *_cmddata);
bsp_Status bsp_I2C_AHT_Correction(uint8_t drvaddr,uint8_t *_cmddata);
bsp_Status bsp_I2C_AHT_ReadTEMPHUM(uint8_t drvaddr, uint8_t *_cmddata,float *_humidity, float *_temperature);
bsp_Status bsp_ATH_Test(void);
void bsp_ATH_Show(void);
#endif /* __I2C_AHT10_H */
