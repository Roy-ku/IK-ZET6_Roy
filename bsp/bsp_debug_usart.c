#include "bsp_debug_usart.h"
#include "stdarg.h"

#define SUPPORT_LETTERSHELL 1
#if (SUPPORT_LETTERSHELL == 0)
uint8_t UART_RXBUFF[RXBUFF_SIZE] = {0};
__IO uint8_t UART1_ORE = 0;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == &huart1)
	{
		HAL_UART_DMAStop(&huart1);
		__HAL_UART_CLEAR_IDLEFLAG(&huart1);
		//  SCB_CleanInvalidateDCache();
		uint8_t mgs1[] = "RXBUFF OVER!\r\n";
		HAL_UART_Transmit(&huart1, mgs1, sizeof(mgs1) - 1, 0x50);
		// bsp_Log_Info("RXBUFF OVER!");
		memset(UART_RXBUFF, '\0', RXBUFF_SIZE);
		HAL_UART_Receive_DMA(&huart1, UART_RXBUFF, RXBUFF_SIZE);
		UART1_ORE = 1;
	}
}

uint8_t bsp_USART1_IRQHandler(UART_HandleTypeDef *huart)
{
	if (huart == &huart1)
	{
		if (RESET != __HAL_UART_GET_FLAG(&huart1, UART_FLAG_ORE))
		{
			__HAL_UART_CLEAR_OREFLAG(&huart1);
			__HAL_UART_CLEAR_IDLEFLAG(&huart1);
		}
		if (RESET != __HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE))
		{
			__HAL_UART_CLEAR_IDLEFLAG(&huart1);
			HAL_UART_DMAStop(&huart1);
			// SCB_CleanInvalidateDCache();
			// SCB_InvalidateDCache_by_Addr((uint32_t *)UART_RXBUFF, 1);
			uint8_t data_length = RXBUFF_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);
			if (data_length != 0 && UART1_ORE != 1)
			{
				HAL_UART_Transmit(&huart1, UART_RXBUFF, data_length, 0x50);
			}
			else
			{
				memset(UART_RXBUFF, '\0', RXBUFF_SIZE);
				HAL_UART_Receive_DMA(&huart1, UART_RXBUFF, RXBUFF_SIZE);
				UART1_ORE = 0;
				return 1;
			}
			memset(UART_RXBUFF, '\0', RXBUFF_SIZE);
			HAL_UART_Receive_DMA(&huart1, UART_RXBUFF, RXBUFF_SIZE);
		}
	}
	return 0;
}

/**
 * @brief 設定UART中斷
 * @note  不能放在HAL_UART_MspInit中,第一次DMA接收時資料會丟失
 */
void bsp_USART1_Interrupt_Set(void)
{
	//__HAL_UART_ENABLE_IT(&huart1,UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
	HAL_UART_Receive_DMA(&huart1, UART_RXBUFF, RXBUFF_SIZE);
}

/**
 * @brief  提供格式化輸出(用法與pringf相同)
 * @param fmt 訊息
 * @param ... 可以省略
 */
void bsp_Log_print(const char *fmt, ...)
{
	char buf_str[256]; /* 特別注意，如果printf的變量較多，注意此局部變量的大小是否夠用 */
	va_list v_args;
	uint16_t len;

	va_start(v_args, fmt);
	len = vsnprintf((char *)&buf_str[0],
					(size_t)sizeof(buf_str),
					(char const *)fmt,
					v_args);
	va_end(v_args);

	HAL_UART_Transmit(&huart1, buf_str, len, 0x50);
}

#elif (SUPPORT_LETTERSHELL == 1)

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == &huart1)
	{
		HAL_UART_DMAStop(&huart1);
		__HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_RXNE);
		//  SCB_CleanInvalidateDCache();
		LetterShell_Recv_flag = 1;
		HAL_UART_Receive_DMA(&huart1, &LetterShell_Recv, 1);
	}
}

uint8_t bsp_USART1_IRQHandler(UART_HandleTypeDef *huart)
{
	if (huart == &huart1)
	{
	}
	return 0;
}

/**
 * @brief 設定UART中斷
 * @note  不能放在HAL_UART_MspInit中,第一次DMA接收時資料會丟失
 */
void bsp_USART1_Interrupt_Set(void)
{
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
	//__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
	HAL_UART_Receive_DMA(&huart1, &LetterShell_Recv, 1);
}

/**
 * @brief  提供格式化輸出(用法與pringf相同)
 * @param fmt 訊息
 * @param ... 可以省略
 */
void bsp_Log_print(const char *fmt, ...)
{
	char buf_str[256]; /* 特別注意，如果printf的變量較多，注意此局部變量的大小是否夠用 */
	va_list v_args;
	uint16_t len;

	va_start(v_args, fmt);
	len = vsnprintf((char *)&buf_str[0],
					(size_t)sizeof(buf_str),
					(char const *)fmt,
					v_args);
	va_end(v_args);

	if (g_ucEnableSystickISR == 1)
	{
		shellWriteEndLine(&shell, buf_str, len);
	}
	else
	{
		HAL_UART_Transmit(&huart1, (uint8_t *)buf_str, len, 0x50);
	}
}
#endif

/**
 * @brief 傳送字串
 * @param str 字串
 */
void bsp_USART1_SendString(char *str)
{
	unsigned int _count = 0;
	do
	{
		HAL_UART_Transmit(&huart1, (uint8_t *)(str + _count), 1, 1000);
		while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == 0)
			;
		_count++;
	} while (*(str + _count) != '\0');
}

/**
 * @brief 重定向c庫函數printf到串口DEBUG_USART，重定向後可使用printf函數
 * @param ch 參數說明
 * @param f 參數說明
 * @return int 返回說明
 */
int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1000);
	return (ch);
}

/**
 * @brief 重定向c庫函數scanf到串口DEBUG_USART，重寫向後可使用scanf、getchar等函數
 * @param f 參數說明
 * @return int 參數說明
 */
int fgetc(FILE *f)
{
	int ch;
	HAL_UART_Receive(&huart1, (uint8_t *)&ch, 1, 1000);
	return (ch);
}

/*********************************************END OF FILE**********************/
