#include "bsp_spi_bus.h"
/*
*********************************************************************************************************
*	                             選擇DMA/INT/POLL方式
*********************************************************************************************************
*/
//#define USE_SPI_DMA    /* DMA方式  */
//#define USE_SPI_INT    /* 中斷方式 */
#define USE_SPI_POLL /* 查詢方式 */

enum
{
	TRANSFER_WAIT,
	TRANSFER_COMPLETE,
	TRANSFER_ERROR
};

/* 變量 ------------------------------------------------------------------------------------------------ */
uint32_t g_spiLen;
uint8_t g_spi_busy; /* SPI忙狀態，0表示不忙，1表示忙 */
__IO uint8_t wTransferState = TRANSFER_WAIT;
/* ----------------------------------------------------------------------------------------------------- */

/* 查詢方式 */
#if defined(USE_SPI_POLL)

uint8_t g_spiBuf[SPI_BUFFER_SIZE];

/* 中斷方式 */
#elif defined(USE_SPI_INT)

uint8_t g_spiBuf[SPI_BUFFER_SIZE];

/* DMA方式使用的SRAM4 */
#elif defined(USE_SPI_DMA)
#if defined(__CC_ARM)	  /* IAR *******/
__attribute__((section(".RAM_D3"))) uint8_t g_spiBuf[SPI_BUFFER_SIZE];
#elif defined(__ICCARM__) /* MDK ********/
#pragma location = ".RAM_D3"
uint8_t g_spiBuf[SPI_BUFFER_SIZE];
#endif // __CC_ARM
#endif //USE_SPI_POLL

/**
 * @brief SPI 傳輸&接收
 * @param hspi SPIHandle
 */
void bsp_spiTransfer(SPI_HandleTypeDef *hspi)
{
	if (g_spiLen > SPI_BUFFER_SIZE)
	{
		bsp_Log_Error("SPI_BUFFER_SIZE is overload.");
		return;
	}

	/* DMA方式 */
#ifdef USE_SPI_DMA
	wTransferState = TRANSFER_WAIT;

	if (HAL_SPI_TransmitReceive_DMA(&hspi, (uint8_t *)g_spiBuf, (uint8_t *)g_spiBuf, g_spiLen) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}

	while (wTransferState == TRANSFER_WAIT)
	{
		;
	}
#endif

	/* 中斷方式 */
#ifdef USE_SPI_INT
	wTransferState = TRANSFER_WAIT;

	if (HAL_SPI_TransmitReceive_IT(&hspi, (uint8_t *)g_spiBuf, (uint8_t *)g_spiBuf, g_spiLen) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}

	while (wTransferState == TRANSFER_WAIT)
	{
		;
	}
#endif

	/* 查詢方式 */
#ifdef USE_SPI_POLL
	if (HAL_SPI_TransmitReceive(hspi, (uint8_t *)g_spiBuf, (uint8_t *)g_spiBuf, g_spiLen, 1000) != HAL_OK)
	{
		bsp_Log_Error("SPI_TransmitReceive happen error.");
	}
#endif
}

/**
 * @brief SPI資料傳輸完成回調
 * @param hspi SPIHandle
 */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	wTransferState = TRANSFER_COMPLETE;
}

/**
 * @brief SPI資料傳輸錯誤回調
 * @param hspi SPIHandle
 */
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
	wTransferState = TRANSFER_ERROR;
}

/**
 * @brief 占用SPIBus
 */
void bsp_SpiBusEnter(void)
{
	g_spi_busy = 1;
}

/**
 * @brief 釋放SPIBus
 */
void bsp_SpiBusExit(void)
{
	g_spi_busy = 0;
}

/**
 * @brief 檢測SPIBus 是否為忙
 * @return uint8_t 0 表示不忙  1表示忙
 */
uint8_t bsp_SpiBusBusy(void)
{
	return g_spi_busy;
}

/**
 * @brief 中斷服務程式
 */
#ifdef USE_SPI_INT
void SPIx_IRQHandler(void)
{
	HAL_SPI_IRQHandler(&hspi);
}
#endif

#ifdef USE_SPI_DMA
void SPIx_DMA_RX_IRQHandler(void)
{
	HAL_DMA_IRQHandler(hspi.hdmarx);
}

void SPIx_DMA_TX_IRQHandler(void)
{
	HAL_DMA_IRQHandler(hspi.hdmatx);
}

void SPIx_IRQHandler(void)
{
	HAL_SPI_IRQHandler(&hspi);
}
#endif

/***************************** (END OF FILE) *********************************/
