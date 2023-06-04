#ifndef __BSP_SPI_BUS_H
#define __BSP_SPI_BUS_H

#include "bsp.h"

void bsp_InitSPIBus(void);
void bsp_spiTransfer(SPI_HandleTypeDef *hspi);
void bsp_InitSPIParam(uint32_t _BaudRatePrescaler, uint32_t _CLKPhase, uint32_t _CLKPolarity);

void bsp_SpiBusEnter(void);
void bsp_SpiBusExit(void);
uint8_t bsp_SpiBusBusy(void);

#define	SPI_BUFFER_SIZE		(4 * 1024)  /* 緩衝大小 */

extern uint8_t g_spiBuf[SPI_BUFFER_SIZE];
//extern uint8_t g_spiRxBuf[SPI_BUFFER_SIZE];
extern uint32_t g_spiLen;
extern uint8_t g_spi_busy;

#endif

/*****************************(END OF FILE) *********************************/
