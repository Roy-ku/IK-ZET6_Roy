/* 溫濕度傳感器 */
#include "AHT10.h"

AHT_DrvTypeDef aht10_drv = {
    .ChipName = "AHT10",
    AHT10_DevAddr,
    AHT10_Reset,
    AHT10_Correction,
    AHT10_ReadTEMPHUM,
};

/**
 * @brief 讀取設備地址
 * @return uint8_t 設備地址
 */
uint8_t AHT10_DevAddr()
{
    return AHT10_ADDRESS;
}

uint8_t AHT10_Reset(void)
{
    uint8_t _cmddata[1] = {0xBA};
    return bsp_I2C_AHT_Reset(AHT10_DevAddr(), _cmddata);
}

/**
 * @brief 校正AHT
 * @return uint8_t status
 */
uint8_t AHT10_Correction(void)
{
    uint8_t _cmddata[3] = {0xE1, 0x08, 0x00};
    return bsp_I2C_AHT_Correction(AHT10_DevAddr(), _cmddata);
}

/**
 * @brief 讀取溫濕度
 * @param humidity 濕度
 * @param temperature 溫度
 * @return uint8_t status
 */
uint8_t AHT10_ReadTEMPHUM(float *humidity, float *temperature)
{
    uint8_t _cmddata[3] = {0xAC, 0x33, 0x00};
    return bsp_I2C_AHT_ReadTEMPHUM(AHT10_DevAddr(), _cmddata, humidity, temperature);
}
