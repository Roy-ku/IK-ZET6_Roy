#ifndef _BSP_I2C_SW_H
#define _BSP_I2C_SW_H

#include "bsp.h"

#define I2C_WR	0		/* 寫控制bit */
#define I2C_RD	1		/* 讀控制bit */

void bsp_SoftI2C_init(void);
void i2c_Start(void);
void i2c_Stop(void);
void i2c_SendByte(uint8_t _ucByte);
uint8_t i2c_ReadByte(void);
uint8_t i2c_WaitAck(void);
void i2c_Ack(void);
void i2c_NAck(void);
uint8_t i2c_CheckDevice(uint8_t _Address);

#endif //_BSP_I2C_SW_H
