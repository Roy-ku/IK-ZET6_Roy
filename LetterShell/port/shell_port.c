/**
 * @file shell_port.c
 * @author Letter (NevermindZZT@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2019-02-22
 * 
 * @copyright (c) 2019 Letter
 * 
 */

// #include "FreeRTOS.h"
// #include "task.h"
// #include "shell.h"
#include "shell_port.h"
// #include "serial.h"
// #include "stm32f4xx_hal.h"
// #include "usart.h"
// #include "cevent.h"
// #include "log.h"

Shell shell;
char shellBuffer[512];

uint8_t LetterShell_Recv = 0;
__IO uint8_t LetterShell_Recv_flag = 0;
//static SemaphoreHandle_t shellMutex;

/**
 * @brief 用户shell写
 * 
 * @param data 数据
 * @param len 数据长度
 * 
 * @return short 实际写入的数据长度
 */
short userShellWrite(char *data, unsigned short len)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)data, len, 1000);
    return len;
}

/**
 * @brief 用户shell读
 * 
 * @param data 数据
 * @param len 数据长度
 * 
 * @return short 实际读取到
 */
short userShellRead(char *data, unsigned short len)
{
    /* 由於使用中斷接收不需要在這實現 */
    //return serialReceive(&debugSerial, (uint8_t *)data, len, 0);
    return 0;
}

/**
 * @brief 用户shell上锁
 * 
 * @param shell shell
 * 
 * @return int 0
 */
int userShellLock(Shell *shell)
{
    //xSemaphoreTakeRecursive(shellMutex, portMAX_DELAY);
    return 0;
}

/**
 * @brief 用户shell解锁
 * 
 * @param shell shell
 * 
 * @return int 0
 */
int userShellUnlock(Shell *shell)
{
    //xSemaphoreGiveRecursive(shellMutex);
    return 0;
}

/**
 * @brief 用户shell初始化
 * 
 */
void userShellInit(void)
{
    shell.write = userShellWrite;
    shell.read = userShellRead;

    shellInit(&shell, shellBuffer, 512);
}
//CEVENT_EXPORT(EVENT_INIT_STAGE2, userShellInit);
