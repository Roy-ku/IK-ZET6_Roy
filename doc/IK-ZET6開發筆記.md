## SPI FASH 地址分配

```c
/* W25Q128FVSIG 分配 size = 16,777,216(0x1000000) */
// 0xc0000000 ~ 0xc00005EF 1520Byte
#define ABSLOCATION_ASCII8x16 __attribute__((section(".ARM.__at_0xc0000000")))
// 0xc00005f0 ~ 0xC00017BF 4560Byte
#define ABSLOCATION_ASCII16x24 __attribute__((section(".ARM.__at_0xc00005f0"))) 
// 0xc00017C0 ~ 0xC0003B5F 9120Byte
#define ABSLOCATION_ASCII24x32 __attribute__((section(".ARM.__at_0xc00017C0")))
// 0xc0004000 ~ 0xC00297FF 153600Byte
#define ABSLOCATION_IKZET6_LOGO __attribute__((section(".ARM.__at_0xc0004000")))

/* 目前結束地址 0x4000 + 0x25800 = 0x29800 -1 */

/* EasyFlash */
// 0xc0E00000 ~ 0xc0F7FFFF 1.5M 
/* 升級APP暫存 */
// 0xc0F80000 ~ 0xc0FFFFFF 512K
```

在 Linker >> Misc controls >>  添加 --keep=*_ExSPIFLASH
常數須以 _ExSPIFLASH 結尾 ，如: ASCII8x16_ExSPIFLASH[]

## SPI Flash 讀取圖片

16bit顯示時，需要把讀出來的資料高位元與低位元順序互換，才能正常顯示

```c
color = (((uint16_t)(SPIFlash_buf[_count + 1] << 8) & 0xFF00) | (SPIFlash_buf[_count]) & 0xFF);
```

## PIN Conflict

- RS-485(J1)  與 藍芽(J15)
- 原 CON_W5500(J16)  W_INT_PG9(INT) 更改為  PCF8574T的INT 使用

 



## 軟體定時器使用

```c
  bsp_SoftTimer_AutoStart(0, 100); /* 重複循環 */
  bsp_SoftTimer_Start(1, 1000);	   /* 只循環一次 */

    if bsp_SoftTimer_Check(0))
    {
      /* 每隔100ms 进来一次 */
      HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    }
    if bsp_SoftTimer_Check(1))
    {
      /* 每隔100ms 进来一次 */
      HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
    }
```

## 硬體定時器使用

```c
/* 1.先初始化 */
bsp_HardTimer_init();
/* 2.開啟定時器，並設定CallBack */
bsp_HardTimer_Start(1 ,100, (void *)TIM_CallBack1);

void TIM_CallBack1(void)
{
  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
}

```

## MODBUS 移植注意

如MODH_WriteParam_06H發送信息成功時，但還是返回0(超時)時

1. 請加大 TIMEOUT 時間

```c
/* modbus_host.c */
#define TIMEOUT 200 /* 原為100 接收命令超時時間, 單位ms */
```

2. 發送 function

```C
void MODH_SendPacket(uint8_t *_buf, uint16_t _len)
{
    /* 自行添加對應的發送 function */
	bsp_rs485_SendStr_length(_buf, _len);
}
```

3. 中斷接收，把 bsp_rs485_IRQHandler() 放在對應的中斷裡

```c
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */

  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */
  bsp_rs485_IRQHandler(&huart2);
  /* USER CODE END USART2_IRQn 1 */
}		
```

```c
 void bsp_rs485_IRQHandler(UART_HandleTypeDef *huart)
 {
     uint8_t rs485_rxbuff;
     if (huart == &RS485_HUARTx)
     {
         if (__HAL_UART_GET_IT_SOURCE(&RS485_HUARTx, UART_IT_RXNE) != RESET)
         {
             HAL_UART_Receive(&RS485_HUARTx, (uint8_t *)(&rs485_rxbuff), 1, 1000);
             MODH_ReciveNew(rs485_rxbuff);
         }
     }
 }
```



## LetterShell移植注意

1. 移植時只需提供 port.c 進行對接即可，Source Code 裡有提供模板可參考

   ```c
   /* shell_port.c 模板*/
   
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
   ```

   ```c
   /* shell_port.h 模板*/
   
   #ifndef __SHELL_PORT_H__
   #define __SHELL_PORT_H__
   
   //#include "serial.h"
   #include "shell.h"
   #include "bsp.h"
   extern Shell shell;
   extern uint8_t LetterShell_Recv;
   extern __IO uint8_t LetterShell_Recv_flag;
   void userShellInit(void);
   #endif
   ```

2.  修改 shell_cfg.h 

   ```c
   /* shell_cfg.h */
   #ifndef __SHELL_CFG_H__
   #define __SHELL_CFG_H__
   /****************************************************************************/
   /**
    * @brief 命令描述的最大字符數量
    */
   #define     COMMAND_DESC_MAX 60
   /****************************************************************************/
   
   /**
    * @brief 是否使用默认shell任务while循环，使能宏`SHELL_USING_TASK`后此宏有意义
    *        使能此宏，则`shellTask()`函数会一直循环读取输入，一般使用操作系统建立shell
    *        任务时使能此宏，关闭此宏的情况下，一般适用于无操作系统，在主循环中调用`shellTask()`
    */
   #define     SHELL_TASK_WHILE            0
   
   /**
    * @brief 是否使用命令导出方式
    *        使能此宏后，可以使用`SHELL_EXPORT_CMD()`等导出命令
    *        定义shell命令，关闭此宏的情况下，需要使用命令表的方式
    */
   #define     SHELL_USING_CMD_EXPORT      1
   
   /**
    * @brief 是否使用shell伴生对象
    *        一些扩展的组件(文件系统支持，日志工具等)需要使用伴生对象
    */
   #define     SHELL_USING_COMPANION       0
   
   /**
    * @brief 支持shell尾行模式
    */
   #define     SHELL_SUPPORT_END_LINE      1
   
   /**
    * @brief 是否在输出命令列表中列出用户
    */
   #define     SHELL_HELP_LIST_USER        0
   
   /**
    * @brief 是否在输出命令列表中列出变量
    */
   #define     SHELL_HELP_LIST_VAR         0
   
   /**
    * @brief 是否在输出命令列表中列出按键
    */
   #define     SHELL_HELP_LIST_KEY         0
   
   /**
    * @brief 是否在输出命令列表中展示命令权限
    */
   #define     SHELL_HELP_SHOW_PERMISSION  1
   
   /**
    * @brief 使用LF作为命令行回车触发
    *        可以和SHELL_ENTER_CR同时开启
    */
   #define     SHELL_ENTER_LF              1
   
   /**
    * @brief 使用CR作为命令行回车触发
    *        可以和SHELL_ENTER_LF同时开启
    */
   #define     SHELL_ENTER_CR              1
   
   /**
    * @brief 使用CRLF作为命令行回车触发
    *        不可以和SHELL_ENTER_LF或SHELL_ENTER_CR同时开启
    */
   #define     SHELL_ENTER_CRLF            0
   
   /**
    * @brief 使用执行未导出函数的功能
    *        启用后，可以通过`exec [addr] [args]`直接执行对应地址的函数
    * @attention 如果地址错误，可能会直接引起程序崩溃
    */
   #define     SHELL_EXEC_UNDEF_FUNC       0
   
   /**
    * @brief shell命令参数最大数量
    *        包含命令名在内，超过16个参数并且使用了参数自动转换的情况下，需要修改源码
    */
   #define     SHELL_PARAMETER_MAX_NUMBER  8
   
   /**
    * @brief 历史命令记录数量
    */
   #define     SHELL_HISTORY_MAX_NUMBER    5
   
   /**
    * @brief 双击间隔(ms)
    *        使能宏`SHELL_LONG_HELP`后此宏生效，定义双击tab补全help的时间间隔
    */
   #define     SHELL_DOUBLE_CLICK_TIME     200
   
   /**
    * @brief 快速帮助
    *        作用于双击tab的场景，当使能此宏时，双击tab不会对命令进行help补全，而是直接显示对应命令的帮助信息
    */
   #define     SHELL_QUICK_HELP            1
   
   /**
    * @brief 保存命令返回值
    *        开启后会默认定义一个`RETVAL`变量，会保存上一次命令执行的返回值，可以在随后的命令中进行调用
    *        如果命令的`SHELL_CMD_DISABLE_RETURN`标志被设置，则该命令不会更新`RETVAL`
    */
   #define     SHELL_KEEP_RETURN_VALUE     0
   
   /**
    * @brief 管理的最大shell数量
    */
   #define     SHELL_MAX_NUMBER            5
   
   /**
    * @brief shell格式化输出的缓冲大小
    *        为0时不使用shell格式化输出
    */
   #define     SHELL_PRINT_BUFFER          128
   
   /**
    * @brief shell格式化输入的缓冲大小
    *        为0时不使用shell格式化输入
    * @note shell格式化输入会阻塞shellTask, 仅适用于在有操作系统的情况下使用
    */
   #define     SHELL_SCAN_BUFFER          0
   
   /**
    * @brief 获取系统时间(ms)
    *        定义此宏为获取系统Tick，如`HAL_GetTick()`
    * @note 此宏不定义时无法使用双击tab补全命令help，无法使用shell超时锁定
    */
   #define     SHELL_GET_TICK()            0
   
   /**
    * @brief 使用锁
    * @note 使用shell锁时，需要对加锁和解锁进行实现
    */
   #define     SHELL_USING_LOCK            0
   
   /**
    * @brief shell内存分配
    *        shell本身不需要此接口，若使用shell伴生对象，需要进行定义
    */
   #define     SHELL_MALLOC(size)          0
   
   /**
    * @brief shell内存释放
    *        shell本身不需要此接口，若使用shell伴生对象，需要进行定义
    */
   #define     SHELL_FREE(obj)             0
   
   /**
    * @brief 是否显示shell信息
    */
   #define     SHELL_SHOW_INFO             1
   
   /**
    * @brief 是否在登录后清除命令行
    */
   #define     SHELL_CLS_WHEN_LOGIN        1
   
   /**
    * @brief shell默认用户
    */
   #define     SHELL_DEFAULT_USER          "IK-ZET6"
   
   /**
    * @brief shell默认用户密码
    *        若默认用户不需要密码，设为""
    */
   #define     SHELL_DEFAULT_USER_PASSWORD ""
   
   /**
    * @brief shell自动锁定超时
    *        shell当前用户密码有效的时候生效，超时后会自动重新锁定shell
    *        设置为0时关闭自动锁定功能，时间单位为`SHELL_GET_TICK()`单位
    * @note 使用超时锁定必须保证`SHELL_GET_TICK()`有效
    */
   #define     SHELL_LOCK_TIMEOUT          0 * 60 * 1000
   
   #endif
   ```

3. 與自己的Printf對接

   ```c
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
   ```

   4. 修改static unsigned short shellWriteCommandDesc(Shell *shell, const char *string)

   ```c
   /****************************************************************************/
   /**
    * @brief 命令描述的最大字符數量
    */
   #define     COMMAND_DESC_MAX 60
   /****************************************************************************/
   
   /**
    * @brief shell 写命令描述字符串
    *
    * @param shell shell对象
    * @param string 字符串数据
    *
    * @return unsigned short 写入字符的数量
    */
   static unsigned short shellWriteCommandDesc(Shell *shell, const char *string)
   {
       unsigned short count = 0;
       const char *p = string;
       SHELL_ASSERT(shell->write, return 0);
       while (*p && *p != '\r' && *p != '\n')
       {
           p++;
           count++;
       }
   
       if (count > COMMAND_DESC_MAX)
       {
           shell->write((char *)string, COMMAND_DESC_MAX);
           shell->write("...", 3);
       }
       else
       {
           shell->write((char *)string, count);
       }
       return count > COMMAND_DESC_MAX ? COMMAND_DESC_MAX : COMMAND_DESC_MAX+3;
   }
   ```
## SPICAN MCP2515 移植注意

   1. 在MCP2515.c 中添加對應的驅動即可
   
      ```c
      /* SPI Tx Wrapper 함수 */
      static void SPI_Tx(uint8_t data)
      {
        /* 添加對應的Code 開始 */
        g_spiLen = 0;
        g_spiBuf[g_spiLen++] = data;
        bsp_spiTransfer(SPI_CAN);
        /* 添加對應的Code 結束 */
      }
      
      /* SPI Tx Wrapper 함수 */
      static void SPI_TxBuffer(uint8_t *buffer, uint8_t length)
      {
        /* 添加對應的Code 開始 */
        g_spiLen = 0;
        for (uint8_t len = 0; len < length; len++)
        {
          g_spiBuf[g_spiLen++] = buffer[len];
        }
        bsp_spiTransfer(SPI_CAN);
        /* 添加對應的Code 結束 */
      }
      
      /* SPI Rx Wrapper 함수 */
      static uint8_t SPI_Rx(void)
      {
        /* 添加對應的Code 開始 */
        g_spiLen = 1;
        bsp_spiTransfer(SPI_CAN);
        return g_spiBuf[0];
        /* 添加對應的Code 結束 */
      }
      
      /* SPI Rx Wrapper 함수 */
      static void SPI_RxBuffer(uint8_t *buffer, uint8_t length)
      {
        /* 添加對應的Code 開始 */
        g_spiLen = length;
        bsp_spiTransfer(SPI_CAN);
        memcpy(buffer, g_spiBuf, g_spiLen);
        /* 添加對應的Code 結束 */
      }
      
      /* MCP2515 초기화 */
      bsp_bool MCP2515_Initialize(void)
      {
        MCP2515_CS_HIGH();
        /* 添加對應的Code 開始 */
        uint8_t loop = 10;
        do
        {
          /* SPI Ready 확인 */
      
          if (HAL_SPI_GetState(SPI_CAN) == HAL_SPI_STATE_READY)
      
            return true;
      
          loop--;
        } while (loop > 0);
      
        return false;
        /* 添加對應的Code 結束 */
      }
      ```
   
   2. 在CANSPI_Initialize 中，設定速度及模式
   
      ```c
      /* 
        * tq = 2 * (brp(0) + 1) / 8000000 = 0.25us
        * tbit = (SYNC_SEG(1 fixed) + PROP_SEG + PS1 + PS2)
        * tbit = 1tq + 1tq + 1tq + 1tq = 4tq
        * 4tq = 1us = 1000kbps
        */
        /* 00(SJW 1tq) 000000 */
        MCP2515_WriteByte(MCP2515_CNF1, MCP2515_8MHz_1000kBPS_CFG1);
      
        /* 1 0 000(1tq) 000(1tq) */
        MCP2515_WriteByte(MCP2515_CNF2, MCP2515_8MHz_1000kBPS_CFG2);
      
        /* 1 0 000 000(1tq) */
        MCP2515_WriteByte(MCP2515_CNF3, MCP2515_8MHz_1000kBPS_CFG3);
      
        /* Normal 모드로 설정 */
        if (!MCP2515_SetNormalMode())
          return false;
      
        return true;
      ```
   
   3. 需自行實現bsp_bool  typedef
   
      ```c
      typedef enum
      {
          false,
          true
      } bsp_bool;
      ```

## CubxMX 配置

* CANbus 設定
  ![image-20221121135616054](IK-ZET6開發筆記.assets/image-20221121135616054.png)
  ![](IK-ZET6開發筆記.assets/image-20221121135653579.png)
* 
