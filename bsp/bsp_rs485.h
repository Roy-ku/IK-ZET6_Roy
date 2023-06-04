#ifndef __BSP_RS485_H
#define __BSP_RS485_H

#include "bsp.h"

#define RS485_HUARTx huart2

void bsp_rs485_Config(void);
void bsp_rs485_SendStr_length(uint8_t *str, uint32_t strlen);
void bsp_rs485_SendString(uint8_t *str);

void bsp_rs485_IRQHandler(UART_HandleTypeDef *huart);
char *bsp_rs485_Get_Message(uint16_t *len);
void bsp_rs485_Clean_Rxbuff(void);
#endif /* __BSP_RS485_H */
