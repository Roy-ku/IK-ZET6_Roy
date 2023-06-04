#ifndef __BSP_WIFI_H
#define __BSP_WIFI_H

#include "bsp.h"

#define WIFI_HUARTx huart3
#define WIFI_RX_BUF_SIZE 64
#define WIFI_TX_BUF_SIZE 128
#define WIFI_SoftTimer_ID 0

void bsp_wifi_Config(void);
void bsp_wifi_SendPacket_length(uint8_t *str, uint16_t strlen);
void bsp_wifi_SendPacket(char *str);
bsp_Status bsp_wifi_WaitResponse(char *_pAckStr, uint16_t _usTimeOut);
void bsp_wifi_IRQHandler(UART_HandleTypeDef *huart);
#endif /* __BSP_WIFI_H */
