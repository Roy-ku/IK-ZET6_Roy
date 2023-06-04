#include "bsp_rs485.h"
#include <stdarg.h>

/*控制收發*/
#define RS485_RX_EN() HAL_GPIO_WritePin(RS485_TXEN_GPIO_Port, RS485_TXEN_Pin, GPIO_PIN_RESET)
#define RS485_TX_EN() HAL_GPIO_WritePin(RS485_TXEN_GPIO_Port, RS485_TXEN_Pin, GPIO_PIN_SET)

static void rs485_Delay(__IO uint32_t nCount);
static void bsp_rs485_SendByte(uint8_t ch);

void bsp_rs485_Config(void)
{
    //進入接收模式
    RS485_RX_EN();
    __HAL_UART_ENABLE_IT(&RS485_HUARTx, UART_IT_RXNE);
}

/**
 * @brief 單byte資料，發送前要使能發送，發送後要使能接收。
 * @param ch byte
 */
static void bsp_rs485_SendByte(uint8_t ch)
{
    HAL_UART_Transmit(&RS485_HUARTx, (uint8_t *)&ch, 1, 0xFFFF);
    while (__HAL_UART_GET_FLAG(&RS485_HUARTx, UART_FLAG_TC) == 0)
        ;
}

/**
 * @brief 發送多筆資料，發送前要使能發送，發送後要使能接收
 * @param str 資料緩衝區
 * @param strlen 資料長度
 */
void bsp_rs485_SendStr_length(uint8_t *str, uint32_t strlen)
{
    unsigned int _count = 0;

    RS485_TX_EN();
    do
    {
        bsp_rs485_SendByte(*(str + _count));
        _count++;
    } while (_count < strlen);

    // rs485_Delay(0xFFF);

    RS485_RX_EN();
}

/**
 * @brief 發送多筆資料，發送前要使能發送，發送後要使能接收
 * @param str 資料緩衝區
 */
void bsp_rs485_SendString(uint8_t *str)
{
    unsigned int k = 0;

    RS485_TX_EN();

    do
    {
        bsp_rs485_SendByte(*(str + k));
        k++;
    } while (*(str + k) != '\0');

    // rs485_Delay(0xFFF);

    RS485_RX_EN();
}

#define UART_BUFF_SIZE 256
volatile uint16_t uart_p = 0;
uint8_t uart_buff[UART_BUFF_SIZE];

/**
 * @brief 中斷服務程式
 * @param huart UART句柄
 */
void bsp_rs485_IRQHandler(UART_HandleTypeDef *huart)
{
    uint8_t rs485_rxbuff;
    if (huart == &RS485_HUARTx)
    {
        // if (uart_p < UART_BUFF_SIZE)
        // {
        //     if (__HAL_UART_GET_IT_SOURCE(&RS485_HUARTx, UART_IT_RXNE) != RESET)
        //     {
        //         HAL_UART_Receive(&RS485_HUARTx, (uint8_t *)(&uart_buff[uart_p]), 1, 1000);
        //         uart_p++;
        //     }
        // }
        // else
        // {
        //     bsp_rs485_Clean_Rxbuff();
        // }

        if (__HAL_UART_GET_IT_SOURCE(&RS485_HUARTx, UART_IT_RXNE) != RESET)
        {
            HAL_UART_Receive(&RS485_HUARTx, (uint8_t *)(&rs485_rxbuff), 1, 1000);
            MODH_ReciveNew(rs485_rxbuff);
        }
    }
}

/**
 * @brief 取得訊息
 * @param len 接受到的資料長度
 * @return char* 緩衝區首地址
 */
char *bsp_rs485_Get_Message(uint16_t *len)
{
    *len = uart_p;
    return (char *)&uart_buff;
}

/**
 * @brief 清空緩衝區
 */
void bsp_rs485_Clean_Rxbuff(void)
{
    uint16_t i = UART_BUFF_SIZE + 1;
    uart_p = 0;
    while (i)
        uart_buff[--i] = 0;
}

/**
 * @brief 功能說明
 * @param nCount 參數說明
 */
static void rs485_Delay(__IO uint32_t nCount)
{
    for (; nCount != 0; nCount--)
        ;
}
