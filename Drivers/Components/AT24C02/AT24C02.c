#include "AT24C02.h"

EEPROM_DrvTypeDef at24c02_drv = {
    .ChipName = "AT24C02",
    .MemaddrSize = 1,
    at24c02_DevAddr,
    at24c02_PageSize,
    at24c02_ByteWrite,
    at24c02_BufferRead,
    at24c02_BufferWrite,
};

/**
 * @brief 讀取設備地址
 * @return uint8_t 設備地址
 */
uint8_t at24c02_DevAddr()
{
    return AT24C02_ADDRESS;
}

/**
 * @brief PageSize
 * @return uint8_t PageSize
 */
uint8_t at24c02_PageSize()
{
    return AT24C02_PAGESIZE;
}

/**
 * @brief Write a Byte
 * @param value data
 * @param WriteAddr 內部地址
 * @return uint8_t 狀態
 */
uint8_t at24c02_ByteWrite(uint8_t value, uint16_t WriteAddr)
{
    return bsp_I2C_EEPROM_ByteWrite(at24c02_DevAddr(), value, WriteAddr);
}

/**
 * @brief 從EEPROM裡面讀取一塊數據
 * @param pBuffer 存放從EEPROM讀取的數據的緩衝區指針
 * @param ReadAddr 接收數據的EEPROM的地址
 * @param NumByteToRead 要從EEPROM讀取的字節數
 * @return uint8_t 0:OK 1:ERROR
 */
uint8_t at24c02_BufferRead(uint8_t *pBuffer, uint16_t ReadAddr, uint16_t NumByteToRead)
{
    return bsp_I2C_EEPROM_BufferRead(at24c02_DevAddr(), pBuffer, ReadAddr, NumByteToRead);
}

/**
 * @brief 在EEPROM的一個寫循環中可以寫多個字節，但一次寫入的字節數
 *        不能超過EEPROM頁的大小，AT24C02每頁有8個字節
 * @param pBuffer 緩衝區指針
 * @param WriteAddr 寫地址
 * @param NumByteToWrite 寫的字節數
 * @return uint8_t  0:OK 1:ERROR
 */
uint8_t at24c02_BufferWrite(uint8_t *pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite)
{
    return bsp_I2C_EEPROM_BufferWrite(at24c02_DevAddr(), pBuffer, WriteAddr, NumByteToWrite);
}
