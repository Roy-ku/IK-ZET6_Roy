#include "W25Q128.h"

#define W25Q128_CS_0() HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_RESET)
#define W25Q128_CS_1() HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_SET)

SPIFLASH_DrvTypeDef w25q128_drv = {

    .tSF = &sf_w25q128_info,
    w25q128_CS_ENABLE,
    w25q128_CS_DISABLE,
    w25q128_ReadInfo,
    w25q128_BufferRead,
    w25q128_BufferWrite,
    w25q128_EraseSector,
};

void w25q128_CS_ENABLE(void)
{
    W25Q128_CS_0();
}

void w25q128_CS_DISABLE(void)
{
    W25Q128_CS_1();
}

void w25q128_ReadInfo(void)
{
    sf_ReadInfo();
}

void w25q128_BufferRead(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
    sf_BufferRead(pBuffer, ReadAddr, NumByteToRead);
}

uint8_t w25q128_BufferWrite(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    SFLASH_Status _status = SFLASH_PASSED;
    _status = sf_BufferWrite(pBuffer, WriteAddr, NumByteToWrite);
    return _status;
}

void w25q128_EraseSector(uint32_t _uiSectorAddr)
{
    sf_EraseSector(_uiSectorAddr);
}
