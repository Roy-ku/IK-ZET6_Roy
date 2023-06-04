#include "bsp.h"

/**
 * @brief 根據地址計算頁首地址
 * @param Address 頁首地址
 * @return uint32_t 頁區號(0~255)
 */
uint32_t bsp_GetSector(uint32_t Address)
{
	uint32_t sector = 0;
	if ((Address <= FLASH_BANK1_END) && (Address >= CPU_FLASH_BASE_ADDR))
	{
		sector = Address / FLASH_PAGE_SIZE;
	}
	else
	{
		bsp_Log_Error("Address is out of range.(0x08008000~0x0807FFFF)");
	}
	return sector;
}

/**
 * @brief  從內部Flash讀取資料
 * @param _ulFlashAddr 讀取的起始地址
 * @param _ucpDst 資料緩衝區
 * @param _ulSize 資料長度(必須是32bit唯一個單位)
 * @return bsp_Status Status
 */
bsp_Status bsp_ReadCpuFlash(uint32_t _ulFlashAddr, uint32_t *_ucpDst, uint32_t _ulSize)
{
	uint32_t count;

	/* 如果偏移地址超過本身容量時退出 */
	if (_ulFlashAddr + _ulSize > CPU_FLASH_BASE_ADDR + CPU_FLASH_SIZE)
	{
		return bsp_FAILED;
	}

	/* 長度為0時退出  */
	if (_ulSize == 0)
	{
		return bsp_PASSED;
	}

	for (count = 0; count < _ulSize; count++)
	{
		*_ucpDst++ = (*(uint32_t *)_ulFlashAddr);
		_ulFlashAddr += 4;
	}

	return bsp_PASSED;
}

/**
 * @brief  擦除內部FLASH一頁的分區(單位:2KB)
 * @param _ulFlashAddr 擦除的起始地址
 * @return bsp_Status Status
 */
bsp_Status bsp_EraseCpuFlash(uint32_t _ulFlashAddr)
{
	// uint32_t FirstSector = 0;
	FLASH_EraseInitTypeDef EraseInitStruct = {0};
	uint32_t NbrOfPage = 0; //紀錄寫入多少頁
	uint32_t SECTORError = 0;

	/* Flash解鎖 */
	HAL_FLASH_Unlock();

	/* 獲取地址所在的頁分區 */
	// FirstSector = bsp_GetSector(_ulFlashAddr);

	/* 扇區擦除配置 */
	NbrOfPage = ((CPU_FLASH_END_ADDR + 1) - _ulFlashAddr) / FLASH_PAGE_SIZE;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Banks = FLASH_BANK_1;
	EraseInitStruct.NbPages = NbrOfPage;
	EraseInitStruct.PageAddress = _ulFlashAddr;

	/* 扇區擦除 */
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK)
	{
		goto err;
	}

	/* Flash上鎖，禁止寫入Flash */
	HAL_FLASH_Lock();
	return bsp_PASSED;
err:
	/* Flash上鎖，禁止寫入Flash */
	HAL_FLASH_Lock();
	bsp_Log_Error("EraseCpuFlash error.");
	return bsp_FAILED;
}

/**
 * @brief 把資料寫入到內部Flash。須按32bit倍數寫。不支持頁。頁大小2048KB。
 * 		  寫入之前需先擦除頁。
 * @param _ulFlashAddr 寫入的起始地址
 * @param _ucpSrc 資料緩衝區
 * @param _ulSize 資料長度(必須是32bit唯一個單位)
 * @return bsp_Status Status
 */
bsp_Status bsp_WriteCpuFlash(uint32_t _ulFlashAddr, uint32_t *_ucpSrc, uint32_t _ulSize)
{
	uint32_t len = 0;

	/* 如果偏移地址超過本身容量時退出 */
	if (_ulFlashAddr + _ulSize > CPU_FLASH_BASE_ADDR + CPU_FLASH_SIZE)
	{
		return bsp_FAILED;
	}

	/* 長度為0時退出  */
	if (_ulSize == 0)
	{
		return bsp_PASSED;
	}

	//__set_PRIMASK(1); /* 關中斷 */

	/* Flash解鎖 */
	HAL_FLASH_Unlock();

	while (len < _ulSize)
	{
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, _ulFlashAddr, (uint64_t)_ucpSrc[len]) == HAL_OK)
		{
			_ulFlashAddr = _ulFlashAddr + 4;
			len++;
		}
		else
		{
			/* 寫入出錯 */
			goto err;
		}
	}

	/* Flash上鎖，禁止寫入Flash */
	HAL_FLASH_Lock();
	//__set_PRIMASK(0); /* 開中斷 */
	return bsp_PASSED;

err:
	/* Flash上鎖，禁止寫入Flash */
	HAL_FLASH_Lock();
	//__set_PRIMASK(0); /* 開中斷 */
	bsp_Log_Error("WriteCpuFlash error.");
	return bsp_FAILED;
}

/**
 * @brief 測試內部Flash讀寫
 */
void bsp_CpuFlash_Test(void)
{
	uint32_t IFLASH_TEST = 0x0807F800;
	uint32_t ucTest[2] = {0xAC22, 0xBB445566};
	uint32_t ucTest_read[2] = {0};
	uint32_t *ptr32_1, *ptr32_2;
	bsp_Log_Info("bsp_EraseCpuFlash %d", bsp_EraseCpuFlash(IFLASH_TEST));
	bsp_Log_Info("bsp_WriteCpuFlash %d", bsp_WriteCpuFlash((uint32_t)IFLASH_TEST, ucTest, 2));
	ptr32_1 = (uint32_t *)(IFLASH_TEST + 0);
	ptr32_2 = (uint32_t *)(IFLASH_TEST + 4);
	bsp_Log_Info("Read data: ptr32_1 = %x,ptr32_2 = %x", *ptr32_1, *ptr32_2);
	bsp_ReadCpuFlash(IFLASH_TEST, ucTest_read, 2);
	bsp_Log_Info("Read data: ucTest_read[0] = %x,ucTest_read[1] = %x", ucTest_read[0], ucTest_read[1]);
}

/***************************** (END OF FILE) *********************************/
