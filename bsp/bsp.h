#ifndef __YH_BSP_H
#define __YH_BSP_H

#include "main.h"
#include "dma.h"
//#include "i2c.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"
// #include "can.h"
#include "adc.h"
#include "sdio.h"

/* C 標準庫 */
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

/* BSP */
#include "bsp_def.h" // 順序須為第一個
#include "bsp_log.h"
#include "bsp_msg.h"
#include "bsp_Button_FIFO.h"
#include "bsp_timer.h"
#include "bsp_debug_usart.h"
#include "bsp_cpu_flash.h"
#include "bsp_spi_flash.h"
#include "bsp_spi_bus.h"
#include "bsp_I2C_EEPROM.h"
#include "bsp_I2C_AHT.h"
#include "bsp_I2C_IOexpand.h"
#include "bsp_I2Cx_bus.h"
#include "bsp_I2C_SW.h"
#include "bsp_rtc.h"
#include "bsp_lcd.h"
#include "bsp_rs485.h"
#include "bsp_can.h"
#include "bsp_sdio_sd.h"
#include "bsp_wifi_esp01.h"
#include "bsp_spi_can.h"

#include "easyflash.h"
#include "shell_port.h"
#include "modbus_host.h"
/* Components */
#include "W25Q64.h"
#include "W25Q128.h"
#include "AT24C02.h"
#include "AHT10.h"
#include "ILI9341.h"
#include "MCP2515.h"
#include "PCF8574T.h"
/* RTOS */
//#define BSP_USE_OS
#define bsp_OS_NOS

//#define bsp_OS_RTX
//#define bsp_OS_THREADX
//#include "app_threadx.h"
//#include "rtos_thread.h"

#if !defined(BSP_USE_OS) && defined(bsp_OS_NOS)
#define bsp_Delay(millisecond) HAL_Delay(millisecond)
#endif // bsp_OS_NOS
#if defined(BSP_USE_OS) && defined(bsp_OS_RTX)
#define bsp_Delay(millisecond) osDelay(millisecond)
#endif // bsp_OS_NOS
#if defined(BSP_USE_OS) && defined(bsp_OS_THREADX)
#define bsp_Delay(millisecond) tx_thread_sleep(millisecond)
#endif // BSP_USE_OS

#define Enable_EventRecorder 0
#if Enable_EventRecorder == 1
#include "EventRecorder.h"
#endif

void bsp_Init(void);
bsp_Status bsp_Get_App_Version(char *version);
bsp_Status bsp_Buffercmp(uint8_t *pBuffer1, uint8_t *pBuffer2, uint32_t BufferLength);

void bsp_RunPer1ms(void);
void bsp_RunPer5ms(void);
void bsp_RunPer10ms(void);
void bsp_RunPer20ms(void);
void bsp_RunPer100ms(void);
void bsp_Idle(void);
/* KEK1 */
void bsp_Key1_DOWN(void);
void bsp_Key1_UP(void);
void bsp_Key1_LONG(void);
/* KEK2 */
void bsp_Key2_DOWN(void);
void bsp_Key2_UP(void);
void bsp_Key2_LONG(void);
/* KEK3 */
void bsp_Key3_DOWN(void);
void bsp_Key3_UP(void);
void bsp_Key3_LONG(void);
/* KEK4 */
void bsp_Key4_DOWN(void);
void bsp_Key4_UP(void);
void bsp_Key4_LONG(void);

/* KEKTOUCH */
void bsp_KeyTouch_DOWN(void);
void bsp_KeyTouch_UP(void);
void bsp_KeyTouch_LONG(void);
#endif /* __YH_BSP_H */
