#include "bsp_spi_flash.h"

extern SPI_HandleTypeDef hspi2;
#define hspix hspi2

/*******************************************************************************************************/
/* 命令 */
#define CMD_AAI 0xAD	/* AAI 連續編程指令(FOR SST25VF016B) */
#define CMD_DISWR 0x04	/* 禁止寫, 退出AAI狀態 */
#define CMD_EWRSR 0x50	/* 允許寫狀態寄存器的命令 */
#define CMD_WRSR 0x01	/* 寫狀態寄存器命令 */
#define CMD_WREN 0x06	/* 寫使能命令 */
#define CMD_PAGEWR 0x02 /* 頁编程 Page Program */
#define CMD_READ 0x03	/* 讀數據區命令 */
#define CMD_RDSR 0x05	/* 讀狀態寄存器命令 */
#define CMD_RDID 0x9F	/* 讀器件ID命令 */

#define CMD_EnterPowerdown 0xB9 /* Enter Powerdown */
#define CMD_ExitPowerdown 0xAB	/* Exit Powerdown */

#define CMD_SE 0x20		/* 擦除扇區命令 */
#define CMD_BE 0xC7		/* 批量擦除命令 */
#define DUMMY_BYTE 0x00 /* 啞命令，可以為任意值，用於讀操作 */

#define WIP_FLAG 0x01 /* 狀態寄存器中的正在編程標誌（WIP) */

/* 測試 */
#define TEST_ADDR 0 /* 測試用地址 */
/*******************************************************************************************************/

SFLASH_T sf_w25q128_info;
SFLASH_T sf_w25q64_info;
SPIFLASH_DrvTypeDef *SPIFLASHDrv;
uint8_t SPIFlash_buf[SPIFlash_BUF_SIZE];

#if 0
static void sf_WriteStatus(uint8_t _ucValue);
#endif

static uint32_t sf_ReadID(void);
static void sf_SetCS(uint8_t _Level);
static void sf_WriteEnable(void);
static void sf_WaitForWriteEnd(void);
static void sf_PageWrite(uint8_t *_pBuf, uint32_t _uiWriteAddr, uint16_t _usSize);
static uint8_t sf_NeedErase(uint8_t *_ucpOldBuf, uint8_t *_ucpNewBuf, uint16_t _uiLen);
static SFLASH_Status sf_CmpData(uint32_t _uiSrcAddr, uint8_t *_ucpTar, uint32_t _uiSize);
static uint8_t sf_AutoWriteSector(uint8_t *_ucpSrc, uint32_t _uiWrAddr, uint16_t _usWrLen);
static uint8_t sf_ExitPowerdown(void);
static uint8_t sf_EnterPowerdown(void);
static uint8_t s_spiBuf[4 * 1024]; /* 用於寫函數，先讀出整個扇區，修改緩衝區後，再整個扇區回寫 */

/**
 * @brief SPI Falsh初始化。讀取ID。
 */
void bsp_SFlash_Init(void)
{
	SPIFLASHDrv = &w25q128_drv;
	/* 讀取 SPIFlash ID */
	SPIFLASHDrv->ReadInfo();
	bsp_Log_Info("\r\nSPI FLASH info\r\nChipName : %s \r\nChipID : 0x%x \r\nDeviceID : 0x%x\r\nTotalSize : %d\r\n", SPIFLASHDrv->tSF->ChipName, SPIFLASHDrv->tSF->ChipID, SPIFLASHDrv->tSF->DeviceID, SPIFLASHDrv->tSF->TotalSize);
	// sfReadTest();
	// SPIFLASHDrv = &w25q64_drv;
	// SPIFLASHDrv->ReadInfo();
	// bsp_Log_Info("SPI FLASH info\r\nChipName : %s \r\nChipID : 0x%x \r\nDeviceID : 0x%x\r\nTotalSize : %d\r\n", SPIFLASHDrv->tSF->ChipName, SPIFLASHDrv->tSF->ChipID, SPIFLASHDrv->tSF->DeviceID, SPIFLASHDrv->tSF->TotalSize);
	// sfReadTest();
}

/**
 * @brief SPIFlash 片選控制函數
 * @param _Level 0: 選用/1: 釋放 
 */
static void sf_SetCS(uint8_t _Level)
{
	if (_Level == 0)
	{
		bsp_SpiBusEnter();
		SPIFLASHDrv->CS_ENABLE();
	}
	else
	{
		SPIFLASHDrv->CS_DISABLE();
		bsp_SpiBusExit();
	}
}

/**
 * @brief 擦除指定的扇區
 * @param _uiSectorAddr 扇區地址
 */
void sf_EraseSector(uint32_t _uiSectorAddr)
{
	sf_WriteEnable(); /* 發送寫使能命令 */

	/* 擦除扇區操作 */
	sf_SetCS(0); /* 使能片選 */
	g_spiLen = 0;
	g_spiBuf[g_spiLen++] = CMD_SE;							   /* 發送擦除命令 */
	g_spiBuf[g_spiLen++] = ((_uiSectorAddr & 0xFF0000) >> 16); /* 發送扇區地址高8bit */
	g_spiBuf[g_spiLen++] = ((_uiSectorAddr & 0xFF00) >> 8);	   /* 發送扇區地址中間8bit */
	g_spiBuf[g_spiLen++] = (_uiSectorAddr & 0xFF);			   /* 發送扇區地址低8bit */
	bsp_spiTransfer(&hspix);
	sf_SetCS(1); /* 禁能片選 */

	sf_WaitForWriteEnd(); /* 等待串行Flash内部寫操作完成 */
}

/**
 * @brief 擦除整片Flash
 */
void sf_EraseChip(void)
{
	sf_WriteEnable(); /* 發送寫使能命令 */

	/* 擦除扇區操作 */
	sf_SetCS(0); /* 使能片選 */
	g_spiLen = 0;
	g_spiBuf[g_spiLen++] = CMD_BE; /* 發送整片擦除命令 */
	bsp_spiTransfer(&hspix);
	sf_SetCS(1); /* 禁能片選 */

	sf_WaitForWriteEnd(); /* 等待串行Flash内部寫操作完成 */
}

/**
 * @brief 頁寫入
 * @param _pBuf 資料緩衝區
 * @param _uiWriteAddr 寫目標首地址
 * @param _usSize 資料個數，頁大小的整倍數(256Byte的整數倍)
 */
static void sf_PageWrite(uint8_t *_pBuf, uint32_t _uiWriteAddr, uint16_t _usSize)
{
	uint32_t i, j;

	if (SPIFLASHDrv->tSF->ChipID == SST25VF016B_ID)
	{
		/* AAI指令要求傳入的資料個數是偶數 */
		if ((_usSize < 2) && (_usSize % 2))
		{
			return;
		}

		sf_WriteEnable(); /* 發送寫使能命令 */

		sf_SetCS(0); /* 使能片選 */
		g_spiLen = 0;
		g_spiBuf[g_spiLen++] = CMD_AAI;							  /* 發送AAI命令(地址自動增加编程) */
		g_spiBuf[g_spiLen++] = ((_uiWriteAddr & 0xFF0000) >> 16); /* 發送扇區地址高8bit */
		g_spiBuf[g_spiLen++] = ((_uiWriteAddr & 0xFF00) >> 8);	  /* 發送扇區地址中間8bit */
		g_spiBuf[g_spiLen++] = (_uiWriteAddr & 0xFF);			  /* 發送扇區地址低8bit */
		g_spiBuf[g_spiLen++] = (*_pBuf++);						  /* 發送第1個資料 */
		g_spiBuf[g_spiLen++] = (*_pBuf++);						  /* 發送第2個資料 */
		bsp_spiTransfer(&hspix);
		sf_SetCS(1); /* 禁能片選 */

		sf_WaitForWriteEnd(); /* 等待串行Flash内部寫操作完成 */

		_usSize -= 2; /*  */

		for (i = 0; i < _usSize / 2; i++)
		{
			sf_SetCS(0); /* 使能片選 */
			g_spiLen = 0;
			g_spiBuf[g_spiLen++] = (CMD_AAI);  /* 發送AAI命令(地址自動增加编程) */
			g_spiBuf[g_spiLen++] = (*_pBuf++); /* 發送資料 */
			g_spiBuf[g_spiLen++] = (*_pBuf++); /* 發送資料 */
			bsp_spiTransfer(&hspix);
			sf_SetCS(1);		  /* 禁能片選 */
			sf_WaitForWriteEnd(); /* 等待串行Flash内部寫操作完成 */
		}

		sf_SetCS(0);
		g_spiLen = 0;
		g_spiBuf[g_spiLen++] = (CMD_DISWR);
		bsp_spiTransfer(&hspix);
		sf_SetCS(1);

		sf_WaitForWriteEnd(); /* 等待串行Flash内部寫操作完成 */
	}
	else /* for MX25L1606E 、 W25Q64BV */
	{
		for (j = 0; j < _usSize / 256; j++)
		{
			sf_WriteEnable(); /* 發送寫使能命令 */

			sf_SetCS(0); /* 使能片選 */
			g_spiLen = 0;
			g_spiBuf[g_spiLen++] = (CMD_PAGEWR);					  /* 發送AAI命令(地址自動增加编程) */
			g_spiBuf[g_spiLen++] = ((_uiWriteAddr & 0xFF0000) >> 16); /* 發送扇區地址高8bit */
			g_spiBuf[g_spiLen++] = ((_uiWriteAddr & 0xFF00) >> 8);	  /* 發送扇區地址中間8bit */
			g_spiBuf[g_spiLen++] = (_uiWriteAddr & 0xFF);			  /* 發送扇區地址低8bit */
			for (i = 0; i < 256; i++)
			{
				g_spiBuf[g_spiLen++] = (*_pBuf++); /* 發送資料 */
			}
			bsp_spiTransfer(&hspix);
			sf_SetCS(1); /* 禁止片選 */

			sf_WaitForWriteEnd(); /* 等待串行Flash内部寫操作完成 */

			_uiWriteAddr += 256;
		}

		sf_SetCS(0);
		g_spiLen = 0;
		g_spiBuf[g_spiLen++] = (CMD_DISWR);
		bsp_spiTransfer(&hspix);
		sf_SetCS(1);

		sf_WaitForWriteEnd(); /* 等待串行Flash内部寫操作完成 */
	}
}

/**
 * @brief 功能說明
 * @param _pBuf 資料緩衝區
 * @param _uiReadAddr 讀目標首地址
 * @param _uiSize 資料個數, 不能超出Flash總容量
 */
void sf_BufferRead(uint8_t *_pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
	uint16_t rem;
	uint16_t i;
	/* 如果讀取的資料長度為0或者超出串行Flash地址空間，則直接返回 */
	if ((_uiSize == 0) || (_uiReadAddr + _uiSize) > SPIFLASHDrv->tSF->TotalSize)
	{
		return;
	}
	/* 擦除扇區操作 */
	sf_SetCS(0); /* 使能片選 */
	g_spiLen = 0;
	g_spiBuf[g_spiLen++] = (CMD_READ);						 /* 發送讀命令 */
	g_spiBuf[g_spiLen++] = ((_uiReadAddr & 0xFF0000) >> 16); /* 發送扇區地址高8bit */
	g_spiBuf[g_spiLen++] = ((_uiReadAddr & 0xFF00) >> 8);	 /* 發送扇區地址中間8bit */
	g_spiBuf[g_spiLen++] = (_uiReadAddr & 0xFF);			 /* 發送扇區地址低8bit */
	bsp_spiTransfer(&hspix);

	/* 开始讀資料，因為底層DMA Buff有限，必須分包讀 */
	for (i = 0; i < _uiSize / SPI_BUFFER_SIZE; i++)
	{
		g_spiLen = SPI_BUFFER_SIZE;
		bsp_spiTransfer(&hspix);

		memcpy(_pBuf, g_spiBuf, SPI_BUFFER_SIZE);
		_pBuf += SPI_BUFFER_SIZE;
	}

	rem = _uiSize % SPI_BUFFER_SIZE; /* 剩餘Byte */
	if (rem > 0)
	{
		g_spiLen = rem;
		bsp_spiTransfer(&hspix);

		memcpy(_pBuf, g_spiBuf, rem);
	}
	sf_SetCS(1); /* 禁能片選 */
}

/**
 * @brief 比較Flash的資料
 * @param _uiSrcAddr ：Flash地址
 * @param _ucpTar 資料緩衝區
 * @param _uiSize 資料個數, 不能超出Flash總容量
 * @return SFLASH_Status SFLASH_SAME = 相等, SFLASH_DIFFERENT = 不等
 */
static SFLASH_Status sf_CmpData(uint32_t _uiSrcAddr, uint8_t *_ucpTar, uint32_t _uiSize)
{
	uint16_t i, j;
	uint16_t rem;

	/* 如果讀取的資料長度為0或者超出Flash地址空間，則直接返回 */
	if ((_uiSrcAddr + _uiSize) > SPIFLASHDrv->tSF->TotalSize)
	{
		return SFLASH_DIFFERENT;
	}

	if (_uiSize == 0)
	{
		return SFLASH_SAME; /* 相等 */
	}

	sf_SetCS(0); /* 使能片選 */
	g_spiLen = 0;
	g_spiBuf[g_spiLen++] = (CMD_READ);						/* 發送讀命令 */
	g_spiBuf[g_spiLen++] = ((_uiSrcAddr & 0xFF0000) >> 16); /* 發送扇區地址高8bit */
	g_spiBuf[g_spiLen++] = ((_uiSrcAddr & 0xFF00) >> 8);	/* 發送扇區地址中間8bit */
	g_spiBuf[g_spiLen++] = (_uiSrcAddr & 0xFF);				/* 發送扇區地址低8bit */
	bsp_spiTransfer(&hspix);

	/* 开始讀資料，因為底层DMA緩衝區有限，必须分包讀 */
	for (i = 0; i < _uiSize / SPI_BUFFER_SIZE; i++)
	{
		g_spiLen = SPI_BUFFER_SIZE;
		bsp_spiTransfer(&hspix);

		for (j = 0; j < SPI_BUFFER_SIZE; j++)
		{
			if (g_spiBuf[j] != *_ucpTar++)
			{
				goto NOTEQ; /* 不相等 */
			}
		}
	}

	rem = _uiSize % SPI_BUFFER_SIZE; /* 剩餘Byte */
	if (rem > 0)
	{
		g_spiLen = rem;
		bsp_spiTransfer(&hspix);

		for (j = 0; j < rem; j++)
		{
			if (g_spiBuf[j] != *_ucpTar++)
			{
				goto NOTEQ; /* 不相等 */
			}
		}
	}
	sf_SetCS(1);
	return SFLASH_SAME; /* 相等 */

NOTEQ:
	sf_SetCS(1); /* 不相等 */
	return SFLASH_DIFFERENT;
}

/**
 * @brief 判斷寫PAGE前是否需要先擦除。
 * @param _ucpOldBuf 舊資料
 * @param _ucpNewBuf 新資料。
 * @param _usLen ：資料個數。
 * @return uint8_t 0 : 不需要擦除， 1 ：需要擦除
 */
static uint8_t sf_NeedErase(uint8_t *_ucpOldBuf, uint8_t *_ucpNewBuf, uint16_t _usLen)
{
	uint16_t i;
	uint8_t ucOld;

	/*
	算法第1步：old 求反, new 不變
		  old    new
		  1101   0101
	~     1111
		= 0010   0101

	算法第2步: old 求反的结果& new 位&
		  0010   old
	&	  0101   new
		 =0000

	算法第3步: 结果為0,則表示無需擦除. 否則表示需要擦除
	*/

	for (i = 0; i < _usLen; i++)
	{
		ucOld = *_ucpOldBuf++;
		ucOld = ~ucOld;

		/* 注意錯誤的寫法: if (ucOld & (*_ucpNewBuf++) != 0) */
		if ((ucOld & (*_ucpNewBuf++)) != 0)
		{
			return SFLASH_PASSED; // 需要擦除
		}
	}
	return SFLASH_FAILED; // 不需要擦除
}

/**
 * @brief 寫1個扇區并校驗,如果不正確則再重寫兩次，本函數自動完成擦除操作。
 * @param _ucpSrc 資料源緩衝區；
 * @param _uiWrAddr ：目標區域首地址
 * @param _usWrLen 資料個數，不能超過扇區大小。
 * @return uint8_t 0 : 錯誤， 1 ： 成功
 */
static uint8_t sf_AutoWriteSector(uint8_t *_ucpSrc, uint32_t _uiWrAddr, uint16_t _usWrLen)
{
	uint16_t i;
	uint16_t j;			  /* 用餘延遲 */
	uint32_t uiFirstAddr; /* 扇區首址 */
	uint8_t ucNeedErase;  /* 1表示需要擦除 */
	uint8_t cRet;

	/* 長度為0時不繼續操作,直接認為成功 */
	if (_usWrLen == 0)
	{
		return SFLASH_PASSED;
	}

	/* 如果偏移地址超過Flash容量則退出 */
	if (_uiWrAddr >= SPIFLASHDrv->tSF->TotalSize)
	{
		return SFLASH_FAILED;
	}

	/* 如果資料長度大于扇區容量，則退出 */
	if (_usWrLen > SPIFLASHDrv->tSF->SectorSize)
	{
		return SFLASH_FAILED;
	}

	/* 如果FLASH中的資料没有變化,則不寫FLASH */
	sf_BufferRead(s_spiBuf, _uiWrAddr, _usWrLen);
	if (memcmp(s_spiBuf, _ucpSrc, _usWrLen) == 0)
	{
		return SFLASH_PASSED;
	}

	/* 判断是否需要先擦除扇區 */
	/* 如果舊資料修改為新資料，所有位均是 1->0 或者 0->0, 則無需擦除,提高Flash壽命 */
	ucNeedErase = 0;
	if (sf_NeedErase(s_spiBuf, _ucpSrc, _usWrLen))
	{
		ucNeedErase = 1;
	}

	uiFirstAddr = _uiWrAddr & (~(SPIFLASHDrv->tSF->SectorSize - 1));

	if (_usWrLen == SPIFLASHDrv->tSF->SectorSize) /* 整個扇區都改寫 */
	{
		for (i = 0; i < SPIFLASHDrv->tSF->SectorSize; i++)
		{
			s_spiBuf[i] = _ucpSrc[i];
		}
	}
	else /* 改寫部分資料 */
	{
		/* 先将整個扇區的資料讀出 */
		sf_BufferRead(s_spiBuf, uiFirstAddr, SPIFLASHDrv->tSF->SectorSize);
		/* 再用新資料覆盖 */
		i = _uiWrAddr & (SPIFLASHDrv->tSF->SectorSize - 1);
		memcpy(&s_spiBuf[i], _ucpSrc, _usWrLen);
	}

	/* 寫完之后進行校驗，如果不正確則重寫，最多3次 */
	cRet = 0;
	for (i = 0; i < 3; i++)
	{

		/* 如果舊資料修改為新資料，所有位均是 1->0 或者 0->0, 則無需擦除,提高Flash壽命 */
		if (ucNeedErase == 1)
		{
			sf_EraseSector(uiFirstAddr); /* 擦除1個扇區 */
		}

		/* 编程一個扇區 */
		sf_PageWrite(s_spiBuf, uiFirstAddr, SPIFLASHDrv->tSF->SectorSize);

		if (sf_CmpData(_uiWrAddr, _ucpSrc, _usWrLen) == SFLASH_SAME)
		{
			cRet = 1;
			break;
		}
		else
		{
			if (sf_CmpData(_uiWrAddr, _ucpSrc, _usWrLen) == SFLASH_SAME)
			{
				cRet = 1;
				break;
			}

			/* 失敗后延遲一段時間再重試 */
			for (j = 0; j < 10000; j++)
				;
		}
	}

	return cRet;
}

/**
 * @brief 寫1個扇區并校驗,如果不正確則再重寫兩次，本函數自動完成擦除操作
 * @param _pBuf 資料源緩衝區
 * @param _uiWriteAddr ：目標區域首地址
 * @param _usWriteSize 資料個數，任意大小，但不能超過Flash容量。
 * @return SFLASH_Status 1 : 成功， 0 ： 失敗
 */
SFLASH_Status sf_BufferWrite(uint8_t *_pBuf, uint32_t _uiWriteAddr, uint32_t _usWriteSize)
{
	uint32_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

	Addr = _uiWriteAddr % SPIFLASHDrv->tSF->SectorSize;
	count = SPIFLASHDrv->tSF->SectorSize - Addr;
	NumOfPage = _usWriteSize / SPIFLASHDrv->tSF->SectorSize;
	NumOfSingle = _usWriteSize % SPIFLASHDrv->tSF->SectorSize;

	if (Addr == 0) /* 起始地址是扇區首地址  */
	{
		if (NumOfPage == 0) /* 資料長度小于扇區大小 */
		{
			if (sf_AutoWriteSector(_pBuf, _uiWriteAddr, _usWriteSize) == 0)
			{
				return SFLASH_FAILED;
			}
		}
		else /* 資料長度大于等于扇區大小 */
		{
			while (NumOfPage--)
			{
				if (sf_AutoWriteSector(_pBuf, _uiWriteAddr, SPIFLASHDrv->tSF->SectorSize) == 0)
				{
					return SFLASH_FAILED;
				}
				_uiWriteAddr += SPIFLASHDrv->tSF->SectorSize;
				_pBuf += SPIFLASHDrv->tSF->SectorSize;
			}
			if (sf_AutoWriteSector(_pBuf, _uiWriteAddr, NumOfSingle) == 0)
			{
				return SFLASH_FAILED;
			}
		}
	}
	else /* 起始地址不是扇區首地址  */
	{
		if (NumOfPage == 0) /* 資料長度小于扇區大小 */
		{
			if (NumOfSingle > count) /* (_usWriteSize + _uiWriteAddr) > SPI_FLASH_PAGESIZE */
			{
				temp = NumOfSingle - count;

				if (sf_AutoWriteSector(_pBuf, _uiWriteAddr, count) == 0)
				{
					return SFLASH_FAILED;
				}

				_uiWriteAddr += count;
				_pBuf += count;

				if (sf_AutoWriteSector(_pBuf, _uiWriteAddr, temp) == 0)
				{
					return SFLASH_FAILED;
				}
			}
			else
			{
				if (sf_AutoWriteSector(_pBuf, _uiWriteAddr, _usWriteSize) == 0)
				{
					return SFLASH_FAILED;
				}
			}
		}
		else /* 資料長度大于等于扇區大小 */
		{
			_usWriteSize -= count;
			NumOfPage = _usWriteSize / SPIFLASHDrv->tSF->SectorSize;
			NumOfSingle = _usWriteSize % SPIFLASHDrv->tSF->SectorSize;
			if (sf_AutoWriteSector(_pBuf, _uiWriteAddr, count) == 0)
			{
				return SFLASH_FAILED;
			}

			_uiWriteAddr += count;
			_pBuf += count;

			while (NumOfPage--)
			{
				if (sf_AutoWriteSector(_pBuf, _uiWriteAddr, SPIFLASHDrv->tSF->SectorSize) == 0)
				{
					return SFLASH_FAILED;
				}
				_uiWriteAddr += SPIFLASHDrv->tSF->SectorSize;
				_pBuf += SPIFLASHDrv->tSF->SectorSize;
			}

			if (NumOfSingle != 0)
			{
				if (sf_AutoWriteSector(_pBuf, _uiWriteAddr, NumOfSingle) == 0)
				{
					return SFLASH_FAILED;
				}
			}
		}
	}
	return SFLASH_PASSED; /* 成功 */
}

/**
 * @brief 讀取器件ID
 * @return uint32_t 32bit的器件ID (最高8bit填0，有效ID位數為24bit）
 */
static uint32_t sf_ReadID(void)
{
	uint32_t uiID;
	uint8_t id1, id2, id3;
	sf_WaitForWriteEnd();
	sf_SetCS(0); /* 使能片選 */
	g_spiLen = 0;
	g_spiBuf[g_spiLen++] = (CMD_RDID); /* 發送讀ID命令 */
	bsp_spiTransfer(&hspix);

	g_spiLen = 3;
	bsp_spiTransfer(&hspix);
	id1 = g_spiBuf[0]; /* 讀ID的第1個Byte */
	id2 = g_spiBuf[1]; /* 讀ID的第2個Byte */
	id3 = g_spiBuf[2]; /* 讀ID的第3個Byte */
	sf_SetCS(1);	   /* 禁能片選 */

	uiID = ((uint32_t)id1 << 16) | ((uint32_t)id2 << 8) | id3;

	return uiID;
}

/**
 * @brief 進入省電模式
 * @return uint8_t  DeviceID
 */
static uint8_t sf_EnterPowerdown(void)
{
	uint8_t _deviceid;
	sf_WaitForWriteEnd();
	sf_SetCS(0); /* 使能片選 */
	g_spiLen = 0;
	g_spiBuf[g_spiLen++] = (CMD_EnterPowerdown); /* 發送讀ID命令 */
	bsp_spiTransfer(&hspix);
	sf_SetCS(1); /* 禁能片選 */
	return _deviceid;
}

/**
 * @brief 離開省電模式
 * @return uint8_t  DeviceID
 */
static uint8_t sf_ExitPowerdown(void)
{
	uint8_t _deviceid;
	sf_SetCS(0); /* 使能片選 */
	g_spiLen = 0;
	g_spiBuf[g_spiLen++] = (CMD_ExitPowerdown); /* 發送讀ID命令 */
	g_spiBuf[g_spiLen++] = (DUMMY_BYTE);
	g_spiBuf[g_spiLen++] = (DUMMY_BYTE);
	g_spiBuf[g_spiLen++] = (DUMMY_BYTE);
	bsp_spiTransfer(&hspix);

	g_spiLen = 1;
	bsp_spiTransfer(&hspix);
	_deviceid = g_spiBuf[0]; /* 讀ID的第1個Byte */
	sf_SetCS(1);			 /* 禁能片選 */
	sf_WaitForWriteEnd();
	return _deviceid;
}

/**
 * @brief 讀取器件ID,并填充器件参數
 */
void sf_ReadInfo(void)
{
	/* 自動識別Flash型號 */
	{
		SPIFLASHDrv->tSF->DeviceID = sf_ExitPowerdown();
		SPIFLASHDrv->tSF->ChipID = sf_ReadID();

		switch (SPIFLASHDrv->tSF->ChipID)
		{
		case SST25VF016B_ID:
			strcpy(SPIFLASHDrv->tSF->ChipName, "SST25VF016B");
			SPIFLASHDrv->tSF->TotalSize = 2 * 1024 * 1024; /* 總容量 = 2M */
			SPIFLASHDrv->tSF->SectorSize = 4 * 1024;	   /* 扇區大小 = 4K */
			break;

		case MX25L1606E_ID:
			strcpy(SPIFLASHDrv->tSF->ChipName, "MX25L1606E");
			SPIFLASHDrv->tSF->TotalSize = 2 * 1024 * 1024; /* 總容量 = 2M */
			SPIFLASHDrv->tSF->SectorSize = 4 * 1024;	   /* 扇區大小 = 4K */
			break;

		case W25Q64_ID:
			strcpy(SPIFLASHDrv->tSF->ChipName, "W25Q64");
			SPIFLASHDrv->tSF->TotalSize = 8 * 1024 * 1024; /* 總容量 = 8M */
			SPIFLASHDrv->tSF->SectorSize = 4 * 1024;	   /* 扇區大小 = 4K */
			break;

		case W25Q128_ID:
			strcpy(SPIFLASHDrv->tSF->ChipName, "W25Q128");
			SPIFLASHDrv->tSF->TotalSize = 16 * 1024 * 1024; /* 總容量 = 16M */
			SPIFLASHDrv->tSF->SectorSize = 4 * 1024;		/* 扇區大小 = 4K */
			break;

		default:
			strcpy(SPIFLASHDrv->tSF->ChipName, "Unknow Flash");
			SPIFLASHDrv->tSF->TotalSize = 0;
			SPIFLASHDrv->tSF->SectorSize = 0;
			break;
		}
	}
}

/**
 * @brief 向器件發送寫使能命令
 */
static void sf_WriteEnable(void)
{
	sf_SetCS(0); /* 使能片選 */
	g_spiLen = 0;
	g_spiBuf[g_spiLen++] = (CMD_WREN); /* 發送命令 */
	bsp_spiTransfer(&hspix);
	sf_SetCS(1); /* 禁能片選 */
}

#if 0
/**
 * @brief 寫狀態暫存器
 * @param _ucValue 狀態暫存器的值
 */
static void sf_WriteStatus(uint8_t _ucValue)
{
	if (SPIFLASHDrv->tSF->ChipID == SST25VF016B_ID)
	{
		/* 第1步：先使能寫状态寄存器 */
		sf_SetCS(0);									/* 使能片選 */
		g_spiLen = 0;
		g_spiBuf[g_spiLen++] = (CMD_EWRSR);							/* 發送命令， 允许寫状态寄存器 */
		bsp_spiTransfer(&hspix);
		sf_SetCS(1);									/* 禁能片選 */

		/* 第2步：再寫状态寄存器 */
		sf_SetCS(0);									/* 使能片選 */
		g_spiLen = 0;
		g_spiBuf[g_spiLen++] = (CMD_WRSR);							/* 發送命令， 寫状态寄存器 */
		g_spiBuf[g_spiLen++] = (_ucValue);							/* 發送資料：状态寄存器的值 */
		bsp_spiTransfer(&hspix);
		sf_SetCS(1);									/* 禁能片選 */
	}
	else
	{
		sf_SetCS(0);									/* 使能片選 */
		g_spiLen = 0;
		g_spiBuf[g_spiLen++] = (CMD_WRSR);							/* 發送命令， 寫状态寄存器 */
		g_spiBuf[g_spiLen++] = (_ucValue);							/* 發送資料：状态寄存器的值 */
		bsp_spiTransfer(&hspix);
		sf_SetCS(1);									/* 禁能片選 */
	}
}
#endif

/**
 * @brief 採用輪巡方式等待Flash寫完成
 */
static void sf_WaitForWriteEnd(void)
{
	while (1)
	{
		sf_SetCS(0);				/* 使能片選 */
		g_spiBuf[0] = (CMD_RDSR);	/* 發送命令， 讀状态寄存器 */
		g_spiBuf[1] = (DUMMY_BYTE); /* 無用資料 */
		g_spiLen = 2;
		bsp_spiTransfer(&hspix);
		sf_SetCS(1); /* 禁能片選 */

		/* 判斷狀態暫存器的忙標志位 */
		if ((g_spiBuf[1] & WIP_FLAG) != SET)
		{
			break;
		}
	}
}

/**
 * @brief 讀測試
 */
void sfReadTest(void)
{
	uint16_t i;
	memset(SPIFlash_buf, '\0', SPIFlash_BUF_SIZE);
	//SPIFLASHDrv->BufferRead(SPIFlash_buf, 0x4000 + 4275*16, SPIFlash_BUF_SIZE);
	SPIFLASHDrv->BufferRead(SPIFlash_buf, TEST_ADDR, SPIFlash_BUF_SIZE);
	printf("\r\n");
	for (i = 0; i < SPIFlash_BUF_SIZE; i++)
	{
		printf("%02X ", SPIFlash_buf[i]);

		if ((i & 15) == 15)
		{
			printf("\r\n");
		}
		// else if (i % 8 == 7)
		// {
		// 	printf(" ");
		// }
	}
}

/**
 * @brief 寫測試
 */
void sfWriteTest(void)
{
	uint16_t i;
	static uint8_t mod = 0;
	if (mod == 0)
	{
		for (i = 0; i < SPIFlash_BUF_SIZE; i++)
		{
			SPIFlash_buf[i] = i;
		}
		mod = 1;
	}
	else if (mod == 1)
	{
		for (i = 0; i < SPIFlash_BUF_SIZE; i++)
		{
			SPIFlash_buf[i] = (SPIFlash_BUF_SIZE - 1) - i;
		}
		mod = 0;
	}

	if (SPIFLASHDrv->BufferWrite(SPIFlash_buf, TEST_ADDR, SPIFlash_BUF_SIZE) == SFLASH_FAILED)
	{
		printf("Write Flash Error !\r\n");
		return;
	}
	else
	{
		printf("Write Flash OK !\r\n");
	}
}

/*****************************  (END OF FILE) *********************************/
