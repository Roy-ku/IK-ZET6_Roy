#include "W25Q64.h"

#define w25q64_CS_0() HAL_GPIO_WritePin(SPI2_NSS2_GPIO_Port, SPI2_NSS2_Pin, GPIO_PIN_RESET)
#define w25q64_CS_1() HAL_GPIO_WritePin(SPI2_NSS2_GPIO_Port, SPI2_NSS2_Pin, GPIO_PIN_SET)

SPIFLASH_DrvTypeDef w25q64_drv = {

    .tSF = &sf_w25q64_info,
    w25q64_CS_ENABLE,
    w25q64_CS_DISABLE,
    w25q64_ReadInfo,
    w25q64_BufferRead,
    w25q64_BufferWrite,
    w25q64_EraseSector,
};

void w25q64_CS_ENABLE(void)
{
    w25q64_CS_0();
}

void w25q64_CS_DISABLE(void)
{
    w25q64_CS_1();
}

void w25q64_ReadInfo(void)
{
    sf_ReadInfo();
}

void w25q64_BufferRead(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
    sf_BufferRead(pBuffer, ReadAddr, NumByteToRead);
}

uint8_t w25q64_BufferWrite(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    SFLASH_Status _status = SFLASH_PASSED;
    _status = sf_BufferWrite(pBuffer, WriteAddr, NumByteToWrite);
    return _status;
}

void w25q64_EraseSector(uint32_t _uiSectorAddr)
{
    sf_EraseSector(_uiSectorAddr);
}
