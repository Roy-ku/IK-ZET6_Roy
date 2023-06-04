#ifndef __YH_BSP__I2CX_BUS_H
#define __YH_BSP__I2CX_BUS_H

#include "bsp.h"

#ifdef HAL_I2C_MODULE_ENABLED
/* Memory Mode */
HAL_StatusTypeDef bsp_I2Cx_bus_Write_8bit(I2C_HandleTypeDef *hi2c, uint16_t devaddress, uint8_t reg, uint8_t value);
HAL_StatusTypeDef bsp_I2Cx_bus_Write_16bit(I2C_HandleTypeDef *hi2c, uint16_t devaddress, uint16_t reg, uint8_t value);
HAL_StatusTypeDef bsp_I2Cx_bus_WriteMultiple_8bit(I2C_HandleTypeDef *hi2c, uint16_t dev_address, uint8_t reg, uint8_t *buffer, uint16_t length);
HAL_StatusTypeDef bsp_I2Cx_bus_WriteMultiple_16bit(I2C_HandleTypeDef *hi2c, uint16_t dev_address, uint16_t reg, uint8_t *buffer, uint16_t length);
uint8_t bsp_I2Cx_bus_Read_8bit(I2C_HandleTypeDef *hi2c, uint16_t devaddress, uint8_t reg);
uint8_t bsp_I2Cx_bus_Read_16bit(I2C_HandleTypeDef *hi2c, uint16_t devaddress, uint16_t reg);
HAL_StatusTypeDef bsp_I2Cx_bus_ReadMultiple_8bit(I2C_HandleTypeDef *hi2c, uint16_t devaddress, uint8_t reg, uint8_t *buffer, uint16_t length);
HAL_StatusTypeDef bsp_I2Cx_bus_ReadMultiple_16bit(I2C_HandleTypeDef *hi2c, uint16_t devaddress, uint16_t reg, uint8_t *buffer, uint16_t length);

/* Master Mode*/
HAL_StatusTypeDef bsp_I2Cx_bus_Master_Read(I2C_HandleTypeDef *hi2c, uint16_t devaddress, uint8_t *buffer, uint16_t length, uint32_t timeout);
HAL_StatusTypeDef bsp_I2Cx_bus_Master_Write(I2C_HandleTypeDef *hi2c, uint16_t devaddress, uint8_t *buffer, uint16_t length, uint32_t timeout);

HAL_StatusTypeDef bsp_I2Cx_bus_IsDeviceReady(I2C_HandleTypeDef *hi2c, uint16_t devaddress, uint32_t trials, uint32_t timeout);

#endif // HAL_I2C_MODULE_ENABLED
#endif /* __YH_BSP__I2CX_BUS_H */
