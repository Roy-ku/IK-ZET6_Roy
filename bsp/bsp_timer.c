#include "bsp_timer.h"

/* 
	硬體定時器的延遲補参數
	不同CPU需做出不同調整
 */
#define HARDTIMER_COMPENSATE 25

/*
	全局運行時間，單位1ms
	最長可以表示 49天，如果你的產品連續運行時間超過這個數，則必須考慮溢出問題
*/
static __IO uint32_t g_iRunTime = 0;
__IO uint8_t g_ucEnableSystickISR = 0; /* 等待變量初始化 */

/* 這2個全局變量用於 bsp_DelayMS() 函數 */
static __IO uint32_t s_uiDelayCount = 0;
static __IO uint8_t s_ucTimeOutFlag = 0;

/*********************************************************************************************************/
#if (SOFTTIME == 1)
/*
	在此定義若干個軟體定時器全局變量
	注意，必須增加__IO 即 volatile，因为这個變量在中断和主程序中同时被訪問，有可能造成編譯器錯誤優化。
*/
#define TMR_COUNT 4 /* 軟體定時器的個数 （定時器ID範圍 0 - 3) */

typedef enum
{
	TMR_ONCE_MODE = 0, /* 一次工作模式 */
	TMR_AUTO_MODE = 1  /* 自動定時工作模式 */
} TMR_MODE_E;

typedef struct
{
	volatile uint8_t Mode;	   /* 計數器模式，1次性 */
	volatile uint8_t Flag;	   /* 定時到達標誌  */
	volatile uint32_t Count;   /* 計數器 */
	volatile uint32_t PreLoad; /* 計數器預装值 */
} SOFT_TMR;

static SOFT_TMR s_tTmr[TMR_COUNT] = {0}; /* 軟體定時器的結構體變量 */

static void bsp_SoftTimer_init(void);
static void bsp_SoftTimerDec(SOFT_TMR *_tmr);

#endif // SOFTTIME
/*********************************************************************************************************/
#if (HARDTIME == 1)

/*
	定義用於硬體定時器的TIM， 可以使 TIM2 - TIM5
*/
//#define USE_TIM2
//#define USE_TIM3
//#define USE_TIM4
#define USE_TIM5

#ifdef USE_TIM2
#define TIM_HARD_Handle htim2
#define TIM_HARD TIM2
#define RCC_TIM_HARD_CLK_ENABLE() __HAL_RCC_TIM2_CLK_ENABLE()
#endif

#ifdef USE_TIM3
#define TIM_HARD_Handle htim3
#define TIM_HARD TIM3
#define RCC_TIM_HARD_CLK_ENABLE() __HAL_RCC_TIM3_CLK_ENABLE()
#endif

#ifdef USE_TIM4
#define TIM_HARD_Handle htim4
#define TIM_HARD TIM4
#define RCC_TIM_HARD_CLK_ENABLE() __HAL_RCC_TIM4_CLK_ENABLE()
#endif

#ifdef USE_TIM5
#define TIM_HARD_Handle htim5
#define TIM_HARD TIM5
#define RCC_TIM_HARD_CLK_ENABLE() __HAL_RCC_TIM5_CLK_ENABLE()
#endif

/* 保存 TIM定時器中斷到后執行的回調函數指针 */
static void (*s_TIM_CallBack1)(void);
static void (*s_TIM_CallBack2)(void);
static void (*s_TIM_CallBack3)(void);
static void (*s_TIM_CallBack4)(void);

static void bsp_HardTimer_init(void);

#endif // HARDTIME

/*
*********************************************************************************************************
*	函 數 名: bsp_TimerStart
*	功能說明: 啟動bsp_SysTick_ISR中斷，並初始化軟體&硬體定時器
*	形    參:  無
*	返 回 值: 無
*********************************************************************************************************
*/
void bsp_TimerStart(void)
{
#if (SOFTTIME == 1)
	bsp_SoftTimer_init();
#endif // SOFTTIME

#if (HARDTIME == 1)
	bsp_HardTimer_init();
#endif // HARDTIME

	g_ucEnableSystickISR = 1;
	HAL_Delay(10);
}
/*
*********************************************************************************************************
*	函 數 名: bsp_SysTick_ISR
*	功能說明: 放在SysTick中斷服務程序，約每隔1ms進入1次
*	形    參:  無
*	返 回 值: 無
*********************************************************************************************************
*/
void bsp_SysTick_ISR(void)
{
	static uint8_t s_count100ms = 0;
	static uint8_t s_count20ms = 0;
	static uint8_t s_count10ms = 0;
	static uint8_t s_count5ms = 0;

	g_iRunTime++; // 每1ms增1

	/* 變量是 uint32_t 類型，最大數為 0xFFFFFFFF */
	if (g_iRunTime == UINT32_MAX)
	{
		g_iRunTime = 0;
	}
	bsp_RunPer1ms(); // 約每隔1ms調用一次此函數

	if (++s_count5ms >= 5)
	{
		s_count5ms = 0;
		bsp_RunPer5ms(); // 約每隔5ms調用一次此函數
	}

	if (++s_count10ms >= 10)
	{
		s_count10ms = 0;
		bsp_RunPer10ms(); // 約每隔10ms調用一次此函數
	}

	if (++s_count20ms >= 20)
	{
		s_count20ms = 0;
		bsp_RunPer20ms(); // 約每隔20ms調用一次此函數
	}

	if (++s_count100ms >= 100)
	{
		s_count100ms = 0;
		bsp_RunPer100ms(); // 約每隔100ms調用一次此函數
	}

	/* 每隔1ms進來1次 （僅用於 bsp_DelayMS） */
	if (s_uiDelayCount > 0)
	{
		if (--s_uiDelayCount == 0)
		{
			s_ucTimeOutFlag = 1;
		}
	}

#if (SOFTTIME == 1)
	/* 每隔1ms，對軟體計數器減一 */
	for (uint8_t i = 0; i < TMR_COUNT; i++)
	{
		bsp_SoftTimerDec(&s_tTmr[i]);
	}
#endif // SOFTTIME
}

/*
*********************************************************************************************************
*	函 數 名: bsp_DelayMS
*	功能說明: ms級延遲，延遲精度為正負1ms
*	形    參:  ms : 延遲長度，單位1 ms。 ms 應大於2
*	返 回 值: 無
*********************************************************************************************************
*/
void bsp_DelayMS(uint32_t ms)
{
	if (ms == 0)
	{
		return;
	}
	else if (ms == 1)
	{
		ms = 2;
	}

	DISABLE_INT();

	s_uiDelayCount = ms;
	s_ucTimeOutFlag = 0;

	ENABLE_INT();

	while (1)
	{
		// bsp_Idle();
		/*
			等待延遲時間到
			注意：編譯器認為 s_ucTimeOutFlag = 0，所以可能優化錯誤，因此 s_ucTimeOutFlag 變量必須申明為 volatile
		*/
		if (s_ucTimeOutFlag == 1)
		{
			break;
		}
	}
}

/*
*********************************************************************************************************
*    函 數 名: bsp_DelayUS
*    功能說明: us級延遲。必須在systick定時器啟動後才能調用此函數。
*    形    參:  us : 延遲長度，單位1 us
*    返 回 值: 無
*********************************************************************************************************
*/
void bsp_DelayUS(uint32_t us)
{
	uint32_t ticks;
	uint32_t told;
	uint32_t tnow;
	uint32_t tcnt = 0;
	uint32_t reload;

	reload = SysTick->LOAD;
	ticks = us * (SystemCoreClock / 1000000); /* 需要的節拍數 */

	tcnt = 0;
	told = SysTick->VAL; /* 剛進入時的計數器值 */

	while (1)
	{
		tnow = SysTick->VAL;
		if (tnow != told)
		{
			/* SYSTICK是一個遞減的計數器 */
			if (tnow < told)
			{
				tcnt += told - tnow;
			}
			/* 重新裝載遞減 */
			else
			{
				tcnt += reload - tnow + told;
			}
			told = tnow;

			/* 時間超過/等於要延遲的時間,則退出 */
			if (tcnt >= ticks)
			{
				break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 數 名: bsp_GetRunTime
*	功能說明: 獲取CPU運行時間，單位1ms。最長可以表示 49天，如果你的產品連續運行時間超過這個數，則必須考慮溢出問題
*	形    參:  無
*	返 回 值: CPU運行時間，單位1ms
*********************************************************************************************************
*/
int32_t bsp_GetRunTime(void)
{
	int32_t runtime;

	DISABLE_INT(); /* 關中斷 */

	runtime = g_iRunTime; /* 這個變量在Systick中斷中被改寫，因此需要關中斷進行保護 */

	ENABLE_INT(); /* 開中斷 */

	return runtime;
}

/*
*********************************************************************************************************
*	函 數 名: bsp_CheckRunTime
*	功能說明: 計算當前運行時間和給定時刻之間的差值。處理了計數器循環。
*	形    參:  _LastTime 上個時刻
*	返 回 值: 當前時間和過去時間的差值，單位1ms
*********************************************************************************************************
*/
int32_t bsp_CheckRunTime(int32_t _LastTime)
{
	int32_t now_time;
	int32_t time_diff;

	DISABLE_INT();

	now_time = g_iRunTime; /* 這個變量在Systick中斷中被改寫，因此需要關中斷進行保護 */

	ENABLE_INT();

	if (now_time >= _LastTime)
	{
		time_diff = now_time - _LastTime;
	}
	else
	{
		time_diff = UINT32_MAX - _LastTime + now_time;
	}

	return time_diff;
}

/********************************************************************************************************/
/*軟體定時器                                                                                              */
/********************************************************************************************************/
#if (SOFTTIME == 1)

/*
*********************************************************************************************************
*	函 數 名: bsp_SoftTimer_init
*	功能說明: 清零所有的軟體定時器(初始化)
*	形    參: 無
*	返 回 值: 無
*********************************************************************************************************
*/
static void bsp_SoftTimer_init(void)
{
	for (uint8_t i = 0; i < TMR_COUNT; i++)
	{
		s_tTmr[i].Count = 0;
		s_tTmr[i].PreLoad = 0;
		s_tTmr[i].Flag = 0;
		s_tTmr[i].Mode = TMR_ONCE_MODE; /* 默認單次模式 */
	}
}

/*
*********************************************************************************************************
*	函 數 名: bsp_SoftTimerDec
*	功能說明: 每隔1ms對所有定時器變量減1。必須被SysTick_ISR週期性調用。
*	形    參:  _tmr : 定時器變量指針
*	返 回 值: 無
*********************************************************************************************************
*/
static void bsp_SoftTimerDec(SOFT_TMR *_tmr)
{
	if (_tmr->Count > 0)
	{
		/* 如果定時器變量減到1則設置定時器到達標誌 */
		if (--_tmr->Count == 0)
		{
			_tmr->Flag = 1;

			/* 如果是自動模式，則自動重裝計數器 */
			if (_tmr->Mode == TMR_AUTO_MODE)
			{
				_tmr->Count = _tmr->PreLoad;
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 數 名: bsp_SoftTimer_Start
*	功能說明: 啟動一個定時器，並設置定時周期。
*	形    參:  	_id     : 定時器ID，值域【0,TMR_COUNT-1】。用戶必須自行維護定時器ID，以避免定時器ID衝突。
*				_period : 定時周期，單位1ms
*	返 回 值: 無
*********************************************************************************************************
*/
void bsp_SoftTimer_Start(uint8_t _id, uint32_t _period)
{
	if (_id >= TMR_COUNT)
	{
		/* 打印出錯的源代碼文件名、函數名稱 */
		bsp_Log_Info("Error: file %s, function %s()\r\n", __FILE__, __FUNCTION__);
	}

	DISABLE_INT();

	s_tTmr[_id].Count = _period;	  /* 實時計數器初值 */
	s_tTmr[_id].PreLoad = _period;	  /* 計數器自動重裝值，僅自動模式起作用 */
	s_tTmr[_id].Flag = 0;			  /* 定時時間到標誌 */
	s_tTmr[_id].Mode = TMR_ONCE_MODE; /* 1次性工作模式 */

	ENABLE_INT();
}

/*
*********************************************************************************************************
*	函 數 名: bsp_SoftTimer_AutoStart
*	功能說明: 啟動一個自動定時器，並設置定時周期。
*	形    參:  	_id     : 定時器ID，值域【0,TMR_COUNT-1】。用戶必須自行維護定時器ID，以避免定時器ID衝突。
*				_period : 定時周期，單位ms
*	返 回 值: 無
*********************************************************************************************************
*/
void bsp_SoftTimer_AutoStart(uint8_t _id, uint32_t _period)
{
	if (_id >= TMR_COUNT)
	{
		/* 打印出錯的源代碼文件名、函數名稱 */
		bsp_Log_Info("Error: file %s, function %s()\r\n", __FILE__, __FUNCTION__);
	}

	DISABLE_INT();

	s_tTmr[_id].Count = _period;	  /* 實時計數器初值 */
	s_tTmr[_id].PreLoad = _period;	  /* 計數器自動重裝值，僅自動模式起作用 */
	s_tTmr[_id].Flag = 0;			  /* 定時時間到標誌 */
	s_tTmr[_id].Mode = TMR_AUTO_MODE; /* 自動工作模式 */

	ENABLE_INT();
}

/*
*********************************************************************************************************
*	函 數 名: bsp_SoftTimer_Stop
*	功能說明: 停止一個定時器
*	形    參:  	_id     : 定時器ID，值域【0,TMR_COUNT-1】。用戶必須自行維護定時器ID，以避免定時器ID衝突。
*	返 回 值: 無
*********************************************************************************************************
*/
void bsp_SoftTimer_Stop(uint8_t _id)
{
	if (_id >= TMR_COUNT)
	{
		/* 打印出錯的源代碼文件名、函數名稱 */
		bsp_Log_Info("Error: file %s, function %s()\r\n", __FILE__, __FUNCTION__);
	}

	DISABLE_INT();

	s_tTmr[_id].Count = 0;			  /* 實時計數器初值 */
	s_tTmr[_id].Flag = 0;			  /* 定時時間到標誌 */
	s_tTmr[_id].Mode = TMR_ONCE_MODE; /* 自動工作模式 */

	ENABLE_INT();
}

/*
*********************************************************************************************************
*	函 數 名: bsp_SoftTimer_Check
*	功能說明: 檢測定時器是否超時
*	形    參:  	_id     : 定時器ID，值域【0,TMR_COUNT-1】。用戶必須自行維護定時器ID，以避免定時器ID衝突。
*				_period : 定時周期，單位1ms
*	返 回 值: 返回 0 表示定時未到， 1表示定時到
*********************************************************************************************************
*/
uint8_t bsp_SoftTimer_Check(uint8_t _id)
{
	if (_id >= TMR_COUNT)
	{
		return 0;
	}

	if (s_tTmr[_id].Flag == 1)
	{
		s_tTmr[_id].Flag = 0;
		return 1;
	}
	else
	{
		return 0;
	}
}
#endif // SOFTTIME

/********************************************************************************************************/
/*硬體定時器                                                                                              */
/********************************************************************************************************/
#if (HARDTIME == 1)

/*
*********************************************************************************************************
*	函 數 名: bsp_HardTimer_init
*	功能說明: 配置 TIMx，用於us級別硬件定時。 TIMx將自由運行，永不停止.
*			TIMx可以用TIM2 - TIM5 之間的TIM, 這些TIM有4個通道
*	形    參: 無
*	返 回 值: 無
*********************************************************************************************************
*/
static void bsp_HardTimer_init(void)
{
	uint32_t usPeriod;
	uint16_t usPrescaler;
	uint32_t uiTIMxCLK;

	RCC_TIM_HARD_CLK_ENABLE(); /* 使能TIM時鐘 */

	/*-----------------------------------------------------------------------

		System Clock source       = PLL (HSE)
		SYSCLK(Hz)                = 72000000 (CPU Clock)
		HCLK(Hz)                  = 72000000 (AXI and AHBs Clock)
		AHB Prescaler             = 1

		因為APB1 prescaler = 2, 所以 APB1上的TIMxCLK = APB1 x 2 = 72MHz;
		因為APB2 prescaler = 1, 所以 APB2上的TIMxCLK = APB2 x 1 = 72MHz;

		APB1 定時器有 TIM2, TIM3 ,TIM4, TIM5, TIM6, TIM7
		APB2 定時器有 TIM1, TIM8

	----------------------------------------------------------------------- */
	if ((TIM_HARD_Handle.Instance == TIM1) || (TIM_HARD_Handle.Instance == TIM8))
	{
		/* APB2 定時器時鐘 = 72M */
		uiTIMxCLK = SystemCoreClock;
	}
	else
	{
		/* APB1 定時器時鐘 = 72M */
		uiTIMxCLK = SystemCoreClock;
	}

	usPrescaler = uiTIMxCLK / 1000000 - 1; /* 分頻比 = 1 */
	usPeriod = 0xFFFF;

	/*
	   設置分頻為usPrescaler後，那麼定時器計數器計1次就是1us
	   而參數usPeriod的值是決定了最大計數：
	   usPeriod = 0xFFFF 表示最大0xFFFF(us)。
	*/
	TIM_HARD_Handle.Instance = TIM_HARD;
	TIM_HARD_Handle.Init.Prescaler = usPrescaler;
	TIM_HARD_Handle.Init.Period = usPeriod;
	TIM_HARD_Handle.Init.ClockDivision = 0;
	TIM_HARD_Handle.Init.CounterMode = TIM_COUNTERMODE_UP;
	TIM_HARD_Handle.Init.RepetitionCounter = 0;
	TIM_HARD_Handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

	if (HAL_TIM_Base_Init(&TIM_HARD_Handle) != HAL_OK)
	{
		bsp_Log_Info("Error: file %s, function %s()\r\n", __FILE__, __FUNCTION__);
	}

	/* 配置定時器中斷，給CC捕獲比較中斷使用 */
	{
		/* 會在 tim.c HAL_TIM_Base_MspInit 中進行配置 */
		// HAL_NVIC_SetPriority(TIM_HARD_IRQn, 4, 0);
		// HAL_NVIC_EnableIRQ(TIM_HARD_IRQn);
	}

	/* 啟動定時器 */
	HAL_TIM_Base_Start(&TIM_HARD_Handle);
}

/*
*********************************************************************************************************
*	函 數 名: bsp_HardTimer_Start
*	功能說明: 使用TIM2-5做單次定時器使用, 定時時間到後執行回調函數。可以同時啟動4個定時器通道，互不干擾。
*             定時精度正負1us （主要耗費在調用本函數的執行時間）
*	形    參: _CC : 捕獲比較通道幾，1，2，3，4
*             _uiTimeOut : 超時時間, 單位 1us. 對於16位定時器，最大 65.5ms; 對於32位定時器，最大 4294秒
*             _pCallBack : 定時時間到後，被執行的函數
*	返 回 值: 無
*********************************************************************************************************
*/
void bsp_HardTimer_Start(uint8_t _CC, uint32_t _uiTimeOut, void *_pCallBack)
{
	__IO uint32_t cnt_now;
	__IO uint32_t cnt_tar;

	/*
		補償延遲
	*/
	if (_uiTimeOut < HARDTIMER_COMPENSATE)
	{
		;
	}
	else
	{
		_uiTimeOut -= HARDTIMER_COMPENSATE;
	}

	cnt_now = __HAL_TIM_GET_COUNTER(&TIM_HARD_Handle); /* 取得目前計數器的值 */
	cnt_tar = cnt_now + _uiTimeOut;					   /* 計算捕獲的計數器值 */

	if (_CC == 1)
	{
		s_TIM_CallBack1 = (void (*)(void))_pCallBack;
		__HAL_TIM_SET_COMPARE(&TIM_HARD_Handle, TIM_CHANNEL_1, cnt_tar); /* 設定捕獲比較計數器CC1 */
		__HAL_TIM_CLEAR_IT(&TIM_HARD_Handle, TIM_IT_CC1);				 /* 清除CC1中斷標誌 */
		HAL_TIM_OC_Start_IT(&TIM_HARD_Handle, TIM_CHANNEL_1);			 /* 啟動CC1中斷 */
	}
	else if (_CC == 2)
	{
		s_TIM_CallBack2 = (void (*)(void))_pCallBack;
		__HAL_TIM_SET_COMPARE(&TIM_HARD_Handle, TIM_CHANNEL_2, cnt_tar); /* 設定捕獲比較計數器CC2 */
		__HAL_TIM_CLEAR_IT(&TIM_HARD_Handle, TIM_IT_CC2);				 /* 清除CC2中斷標誌 */
		HAL_TIM_OC_Start_IT(&TIM_HARD_Handle, TIM_CHANNEL_2);			 /* 啟動CC2中斷 */
	}
	else if (_CC == 3)
	{
		s_TIM_CallBack3 = (void (*)(void))_pCallBack;
		__HAL_TIM_SET_COMPARE(&TIM_HARD_Handle, TIM_CHANNEL_3, cnt_tar); /* 設定捕獲比較計數器CC3 */
		__HAL_TIM_CLEAR_IT(&TIM_HARD_Handle, TIM_IT_CC3);				 /* 清除CC3中斷標誌 */
		HAL_TIM_OC_Start_IT(&TIM_HARD_Handle, TIM_CHANNEL_3);			 /* 啟動CC3中斷 */
	}
	else if (_CC == 4)
	{
		s_TIM_CallBack4 = (void (*)(void))_pCallBack;
		__HAL_TIM_SET_COMPARE(&TIM_HARD_Handle, TIM_CHANNEL_4, cnt_tar); /* 設定捕獲比較計數器CC4 */
		__HAL_TIM_CLEAR_IT(&TIM_HARD_Handle, TIM_IT_CC4);				 /* 清除CC4中斷標誌 */
		HAL_TIM_OC_Start_IT(&TIM_HARD_Handle, TIM_CHANNEL_4);			 /* 啟動CC4中斷 */
	}
	else
	{
		return;
	}
}

/*
*********************************************************************************************************
*	函 數 名: bsp_TIM_HARD_IRQHandler
*	功能說明: TIM 用戶中斷服務程序
*	形    參：無
*	返 回 值: 無
*********************************************************************************************************
*/
void bsp_TIM_HARD_IRQHandler(void)
{
	if (__HAL_TIM_GET_FLAG(&TIM_HARD_Handle, TIM_FLAG_CC1) != RESET)
	{
		if (__HAL_TIM_GET_IT_SOURCE(&TIM_HARD_Handle, TIM_IT_CC1) != RESET)
		{
			__HAL_TIM_CLEAR_IT(&TIM_HARD_Handle, TIM_IT_CC1);
			HAL_TIM_OC_Stop_IT(&TIM_HARD_Handle, TIM_CHANNEL_1);

			/* 先關閉中斷，再執行回調函數。因為回調函數可能需要重啟定時器 */
			s_TIM_CallBack1();
		}
	}

	if (__HAL_TIM_GET_FLAG(&TIM_HARD_Handle, TIM_FLAG_CC2) != RESET)
	{
		if (__HAL_TIM_GET_IT_SOURCE(&TIM_HARD_Handle, TIM_IT_CC2) != RESET)
		{
			__HAL_TIM_CLEAR_IT(&TIM_HARD_Handle, TIM_IT_CC2);
			HAL_TIM_OC_Stop_IT(&TIM_HARD_Handle, TIM_CHANNEL_2);

			/* 先關閉中斷，再執行回調函數。因為回調函數可能需要重啟定時器 */
			s_TIM_CallBack2();
		}
	}

	if (__HAL_TIM_GET_FLAG(&TIM_HARD_Handle, TIM_FLAG_CC3) != RESET)
	{
		if (__HAL_TIM_GET_IT_SOURCE(&TIM_HARD_Handle, TIM_IT_CC3) != RESET)
		{
			__HAL_TIM_CLEAR_IT(&TIM_HARD_Handle, TIM_IT_CC3);
			HAL_TIM_OC_Stop_IT(&TIM_HARD_Handle, TIM_CHANNEL_3);

			/* 先關閉中斷，再執行回調函數。因為回調函數可能需要重啟定時器 */
			s_TIM_CallBack3();
		}
	}

	if (__HAL_TIM_GET_FLAG(&TIM_HARD_Handle, TIM_FLAG_CC4) != RESET)
	{
		if (__HAL_TIM_GET_IT_SOURCE(&TIM_HARD_Handle, TIM_IT_CC4) != RESET)
		{
			__HAL_TIM_CLEAR_IT(&TIM_HARD_Handle, TIM_IT_CC4);
			HAL_TIM_OC_Stop_IT(&TIM_HARD_Handle, TIM_CHANNEL_4);

			/* 先關閉中斷，再執行回調函數。因為回調函數可能需要重啟定時器 */
			s_TIM_CallBack4();
		}
	}
}

#endif // HARDTIME
/***************************** (END OF FILE) *********************************/
