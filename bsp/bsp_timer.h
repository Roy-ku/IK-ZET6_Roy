#ifndef __BSP_TIMER_H__
#define __BSP_TIMER_H__
#include "bsp.h"

/*********************************************************************************************************/
/* 定時器 */
#define SOFTTIME 1
#define HARDTIME 1
/*********************************************************************************************************/


/* 提供給其他C文件調用的函數 */
extern __IO uint8_t g_ucEnableSystickISR;

void bsp_TimerStart(void);
int32_t bsp_GetRunTime(void);
int32_t bsp_CheckRunTime(int32_t _LastTime);
void bsp_DelayMS(uint32_t ms);
void bsp_DelayUS(uint32_t us);

#if (SOFTTIME == 1)
void bsp_SoftTimer_Start(uint8_t _id, uint32_t _period);
void bsp_SoftTimer_AutoStart(uint8_t _id, uint32_t _period);
void bsp_SoftTimer_Stop(uint8_t _id);
uint8_t bsp_SoftTimer_Check(uint8_t _id);
#endif // SOFTTIME

#if (HARDTIME == 1)
void bsp_HardTimer_Start(uint8_t _CC, uint32_t _uiTimeOut, void *_pCallBack);
void bsp_TIM_HARD_IRQHandler(void);
#endif // HARDTIME

void bsp_SysTick_ISR(void);
#endif //__BSP_TIMER_H__

/***************************** (END OF FILE) *********************************/
