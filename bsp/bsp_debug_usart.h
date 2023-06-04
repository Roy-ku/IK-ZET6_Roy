#ifndef __DEBUG_USART_H
#define __DEBUG_USART_H

#include "bsp.h"
#define RXBUFF_SIZE 32
extern uint8_t UART_RXBUFF[RXBUFF_SIZE];
void bsp_USART1_Interrupt_Set(void);
void bsp_Log_print(const char *fmt, ...);
void bsp_USART1_SendString(char *str);
uint8_t bsp_USART1_IRQHandler(UART_HandleTypeDef *huart);
int fputc(int ch, FILE *f);
int fgetc(FILE *f);
#endif /* __USART1_H */
