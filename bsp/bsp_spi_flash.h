#ifndef _BSP_SPI_FLASH_H
#define _BSP_SPI_FLASH_H
#include "bsp.h"

#define SPIFlash_BUF_SIZE 1024

typedef enum
{
	SFLASH_FAILED = 0,
	SFLASH_PASSED = 1,
	SFLASH_SAME = 2,
	SFLASH_DIFFERENT = 3
} SFLASH_Status;

enum
{
	SST25VF016B_ID = 0xBF2541,
	MX25L1606E_ID = 0xC22015,
	W25Q64_ID = 0xEF4017, /* BV, JV, FV */
	W25Q128_ID = 0xEF4018
};

typedef struct
{
	uint32_t ChipID;	 /* Flash ID */
	uint8_t DeviceID;	 /* Flash ID  */
	char ChipName[16];	 /* Flash型號字符串，主要用於顯示 */
	uint32_t TotalSize;	 /* 總容量 */
	uint16_t SectorSize; /* 扇區大小 */
} SFLASH_T;
extern SFLASH_T sf_w25q128_info;
extern SFLASH_T sf_w25q64_info;

typedef struct
{
	SFLASH_T *tSF;
	void (*CS_ENABLE)(void);
	void (*CS_DISABLE)(void);
	void (*ReadInfo)(void);
	void (*BufferRead)(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
	uint8_t (*BufferWrite)(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
	void (*EraseSector)(uint32_t _uiSectorAddr);

} SPIFLASH_DrvTypeDef;
extern SPIFLASH_DrvTypeDef w25q128_drv;
extern SPIFLASH_DrvTypeDef w25q64_drv;

extern uint8_t SPIFlash_buf[SPIFlash_BUF_SIZE];

void bsp_SFlash_Init(void);
void sf_EraseChip(void);
void sf_EraseSector(uint32_t _uiSectorAddr);
SFLASH_Status sf_BufferWrite(uint8_t *_pBuf, uint32_t _uiWriteAddr, uint32_t _usWriteSize);
void sf_BufferRead(uint8_t *_pBuf, uint32_t _uiReadAddr, uint32_t _uiSize);
void sf_ReadInfo(void);
void sfReadTest(void);
void sfWriteTest(void);

#endif

/***************************** (END OF FILE) *********************************/
