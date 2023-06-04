#include "PCF8574T.h"

#define PCF8574T_CLEAR_BIT(x, bit) (x &= ~(1 << bit))      /* 清零第bit位 */
#define PCF8574T_SET_BIT(x, bit) (x |= (1 << bit))         /* 置位第bit位 */
#define PCF8574T_TOGGLE_BIT(x, bit) (x ^= (1 << bit))      /* 反轉第bit位 */
#define PCF8574T_GET_BIT(x, bit) ((x & (1 << bit)) >> bit) /* 獲取第bit位 */

IOExpand_DrvTypeDef pcf8574Tt_drv = {
    .ChipName = "PCF8574T",
    PCF8574T_DevAddr,
    PCF8574T_WritePin,
    PCF8574T_ReadPin,
    PCF8574T_TogglePin,
    PCF8574T_WriteByte,
    PCF8574T_ReadByte,
};

/**
 * @brief 讀取設備地址
 * @return uint8_t 設備地址
 */
uint8_t PCF8574T_DevAddr()
{
    return PCF8574T_ADDRESS;
}

/**
 * @brief 寫入IO的狀態(bit)
 * @param pin PCF8574_GPIO Pin
 * @param pinstate 0 or 1
 * @return uint8_t status
 */
uint8_t PCF8574T_WritePin(PCF8574_GPIOPin pin, uint8_t pinstate)
{
    uint8_t value = 0;

    PCF8574T_ReadByte(&value);
    if (pinstate == 0)
    {
        PCF8574T_CLEAR_BIT(value, pin);
        return bsp_I2C_IOEXPAND_Write(PCF8574T_DevAddr(), value);
    }
    else if (pinstate == 1)
    {
        PCF8574T_SET_BIT(value, pin);
        return bsp_I2C_IOEXPAND_Write(PCF8574T_DevAddr(), value);
    }

    return bsp_FAILED;
}

/**
 * @brief 讀取IO的狀態(bit)
 * @param pin PCF8574_GPIO Pin
 * @return uint8_t IO狀態
 */
uint8_t PCF8574T_ReadPin(PCF8574_GPIOPin pin)
{
    uint8_t value = 0;
    if(PCF8574T_ReadByte(&value) != bsp_PASSED)
    {
        bsp_Log_Error("PCF8574T_ReadPin happen error");
        return 0;
    }
    return  PCF8574T_GET_BIT(value,pin);
}

/**
 * @brief 翻轉IO的狀態(bit)
 * @param pin PCF8574_GPIO Pin
 * @return uint8_t status
 */
uint8_t PCF8574T_TogglePin(PCF8574_GPIOPin pin)
{
    uint8_t value = 0;

    PCF8574T_ReadByte(&value);
    PCF8574T_TOGGLE_BIT(value, pin);
    return bsp_I2C_IOEXPAND_Write(PCF8574T_DevAddr(), value);
}

/**
 * @brief 寫入IO的狀態(Byte)
 * @param value reg
 * @return uint8_t status
 */
uint8_t PCF8574T_WriteByte(uint8_t value)
{
    return bsp_I2C_IOEXPAND_Write(PCF8574T_DevAddr(), value);
}

/**
 * @brief 讀取IO的狀態(Byte)
 * @param value reg
 * @return uint8_t status
 */
uint8_t PCF8574T_ReadByte(uint8_t *value)
{
    return bsp_I2C_IOEXPAND_Read(PCF8574T_DevAddr(), value);
}
