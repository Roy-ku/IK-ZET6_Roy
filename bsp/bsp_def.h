#ifndef __BSP_DEF_H
#define __BSP_DEF_H

/*********************************************************************************************************/
/* APP 訊息 */
#define USER_APP_VER "0.0.1"            /* 版本號 */
#define LAST_UPDATE_DATE __DATE__       /* 日期 */
#define LAST_UPDATE_TIME __TIME__       /* 時間 */
#define AUTHOR "RoyLee"                 /* 作者 */
#define EVALUATION_BOARD_TYPE "IK-ZET6" /* 開發板型號 */
/*********************************************************************************************************/

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define ENABLE_INT() NVIC_RESETPRIMASK()
#define DISABLE_INT() NVIC_SETPRIMASK()
#define NVIC_SETPRIMASK() __set_PRIMASK(1)   //禁止全局中斷
#define NVIC_RESETPRIMASK() __set_PRIMASK(0) //使能全局中斷

/*********************************************************************************************************/
/* 內部Ram分配 */
#define Enable_D2SRAM 0                                            // 使能D2域 SRAM1,SRAM2,SRAM3
#define RAM_DTCM __attribute__((section(".RAM_DTCM"), aligned(4))) // 用於RTOS
#define RAM_D1 __attribute__((section(".RAM_D1"), aligned(32)))    // 用於DMA Buffer SDMMC1,DMA
#define RAM_D2 __attribute__((section(".RAM_D2")))                 // 用於DMA Buffer
#define RAM_D3 __attribute__((section(".RAM_D3")))
#define BackupSRAM __attribute__((section(".BackupSRAM"), aligned(4))) // 用於系統進入低功耗模式後，繼續保存資料（Vbat引腳外接電池）
#define RAM_SDRAM __attribute__((section(".bss.RAM_SDRAM"), aligned(32)))
/*********************************************************************************************************/

/*********************************************************************************************************/
/* bsp 全局變數 */
extern int Touch_X, Touch_Y, Touch_W; // 保存觸摸座標訊息
extern uint8_t Touch_event;           // 觸摸事件
/*********************************************************************************************************/

typedef enum
{
    bsp_false,
    bsp_true
} bsp_bool;

typedef enum
{
    bsp_PASSED = 0x00U,
    bsp_FAILED = 0x01U,
    bsp_BUSY = 0x02U,
    bsp_TIMEOUT = 0x03U
} bsp_Status;

#endif /* __BSP_DEF_H */
