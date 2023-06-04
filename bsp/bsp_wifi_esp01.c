#include "bsp_wifi_esp01.h"

typedef struct
{
    __IO uint8_t RxBuf[WIFI_RX_BUF_SIZE]; // 接收緩存
    __IO uint8_t RxCount;                 // 接收數量
    __IO uint8_t IsComplete;              // 接收完成 0:未完成,1:完成
    // uint8_t TxBuf[WIFI_TX_BUF_SIZE];
    // uint8_t TxCount;
} WIFI_T;

WIFI_T g_tWIFIH = {0};
__IO uint8_t g_rxbuff;

static void bsp_wifi_SendByte(uint8_t ch);
static void bsp_wifi_ReciveNew(uint8_t _data);

/**
 * @brief 設定UART中斷
 * @note  不能放在HAL_UART_MspInit中,第一次DMA接收時資料會丟失
 */
void bsp_wifi_Config(void)
{
    __HAL_UART_ENABLE_IT(&WIFI_HUARTx, UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&WIFI_HUARTx, UART_IT_IDLE);
    // HAL_UART_Receive_IT(&WIFI_HUARTx, (uint8_t *)g_tWIFIH.RxBuf, WIFI_RX_BUF_SIZE);
}

/**
 * @brief 單byte資料，發送前要使能發送，發送後要使能接收。
 * @param ch byte
 */
static void bsp_wifi_SendByte(uint8_t ch)
{
    HAL_UART_Transmit(&WIFI_HUARTx, (uint8_t *)&ch, 1, 0xFFFF);
    while (__HAL_UART_GET_FLAG(&WIFI_HUARTx, UART_FLAG_TC) == 0)
        ;
}

void bsp_wifi_SendPacket_length(uint8_t *str, uint16_t strlen)
{
    HAL_UART_Transmit(&WIFI_HUARTx, (uint8_t *)str, strlen, 1000);
}

void bsp_wifi_SendPacket(char *str)
{
    uint16_t len = 0;
    len = strlen(str);
    bsp_wifi_SendPacket_length((uint8_t *)str, len);
}

/**
 * @brief 等待ESP01s返回指定的應答字符串, 可以包含任意字符。只要接收齊全即可返回。
 * @param _pAckStr 應答的字符串， 長度不得超過64
 * @param _usTimeOut 命令執行超時，0表示一直等待. >０表示超時時間，單位1ms
 * @return bsp_Status 0 表示成功  1 表示失敗
 */
bsp_Status bsp_wifi_WaitResponse(char *_pAckStr, uint16_t _usTimeOut)
{
    bsp_Status status = bsp_PASSED;
    uint8_t ucData;
    uint16_t pos = 0;
    uint32_t len;

    len = strlen(_pAckStr);
    if (len > 64)
    {
        return 0;
    }

    /* _usTimeOut == 0 表示无限等待 */
    if (_usTimeOut > 0)
    {
        bsp_SoftTimer_Start(WIFI_SoftTimer_ID, _usTimeOut);
    }
    while (1)
    {
        bsp_Idle();
        if (_usTimeOut > 0)
        {
            if (bsp_SoftTimer_Check(WIFI_SoftTimer_ID))
            {
                status = bsp_FAILED; /* 超時 */
                bsp_Log_Error("Time out...");
                break;
            }
        }
        if (g_tWIFIH.IsComplete == 1)
        {
            if (g_tWIFIH.RxBuf[pos + 7] == _pAckStr[pos])
            {
                pos++;

                if (pos == len)
                {
                    g_tWIFIH.IsComplete = 0;
                    status = bsp_PASSED; /* 收到指定的应答数据，返回成功 */
                    break;
                }
            }
            else
            {
                pos = 0;
            }
        }
    }
    memset((void *)g_tWIFIH.RxBuf, 0, WIFI_RX_BUF_SIZE);
    return status;
}

static void bsp_wifi_ReciveNew(uint8_t _data)
{
    if (g_tWIFIH.RxCount < H_RX_BUF_SIZE)
    {
        g_tWIFIH.RxBuf[g_tWIFIH.RxCount++] = _data;
    }
    else
    {
        bsp_Log_Error("WIFI_Recive is overflow");
        g_tWIFIH.RxCount = 0;
    }
}

/**
 * @brief WIFI中斷處理
 * @param huart 句柄
 */
void bsp_wifi_IRQHandler(UART_HandleTypeDef *huart)
{
    if (huart == &WIFI_HUARTx)
    {

        /* 收到一個Byte */
        if (__HAL_UART_GET_FLAG(&WIFI_HUARTx, UART_FLAG_RXNE) != RESET)
        {
            HAL_UART_Receive(&WIFI_HUARTx, (uint8_t *)(&g_rxbuff), 1, 1000);
            bsp_wifi_ReciveNew(g_rxbuff);
            // HAL_UART_Receive_IT(&WIFI_HUARTx, (uint8_t *)g_tWIFIH.RxBuf, WIFI_RX_BUF_SIZE);
        }

        /* 接收完成 */
        if (__HAL_UART_GET_FLAG(&WIFI_HUARTx, UART_FLAG_IDLE) != RESET)
        {
            uint8_t clr;

            /* 清除暫存器資料 */
            clr = USART3->SR;
            clr = USART3->DR;

            g_tWIFIH.IsComplete = 1;
            g_tWIFIH.RxCount = 0;
        }
    }
}
