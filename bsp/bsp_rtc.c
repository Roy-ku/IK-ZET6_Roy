#include "bsp_rtc.h"

extern RTC_HandleTypeDef hrtc;
RTC_TimeTypeDef Get_Time = {0};
RTC_DateTypeDef Get_Date = {0};

static void bsp_RTC_Time_init(void);
static void bsp_RTC_Date_init(void);
/**
 * @brief 檢查RTC是否電源異常
 */
void bsp_RTC_AbnormalCheck(void)
{
	/* 檢查是否電源復位 */
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) != RESET)
	{
		bsp_Log_Info("Power reset...");
	}
	/* 檢查是否外部復位 */
	else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET)
	{
		bsp_Log_Info("External reset...");
	}
	/* 檢查是否軟體復位 */
	else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST) != RESET)
	{
		bsp_Log_Info("Software reset...");
	}

	if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DRX) != RTC_BKP_DATA)
	{
		bsp_Log_Info("RTC TimeAndDate initialization...");
		bsp_RTC_Time_init();
	}
	bsp_RTC_Date_init();
}

/**
 * @brief 設定時間(初始化)
 */
static void bsp_RTC_Time_init(void)
{
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};

	sDate.WeekDay = RTC_WEEKDAY_SATURDAY;
	sDate.Month = RTC_MONTH_SEPTEMBER;
	sDate.Date = 3;
	sDate.Year = 22;
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, (uint16_t)sDate.Year);
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, (uint16_t)sDate.Month);
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4, (uint16_t)sDate.Date);
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5, (uint16_t)sDate.WeekDay);

	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
	{
		bsp_Log_Error("RTC_Time_init Fail.");
		Error_Handler();
	}

	sTime.Hours = 21;
	sTime.Minutes = 9;
	sTime.Seconds = 10;

	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
		bsp_Log_Error("RTC_Time_init Fail.");
		Error_Handler();
	}

	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DRX, RTC_BKP_DATA);
}

/**
 * @brief 設定日期(初始化)
 */
static void bsp_RTC_Date_init(void)
{
	RTC_DateTypeDef sDate = {0};

	sDate.WeekDay = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR5);
	sDate.Month = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR3);
	sDate.Date = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR4);
	sDate.Year = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR2);

	// sDate.WeekDay = RTC_WEEKDAY_SATURDAY;
	// sDate.Month = RTC_MONTH_SEPTEMBER;
	// sDate.Date = 3;
	// sDate.Year = 22;

	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
	{
		bsp_Log_Error("RTC_Date_init Fail.");
		Error_Handler();
	}
}

/**
 * @brief 更新日期，保存到備分暫存器中
 */
void bsp_RTC_Date_update(void)
{
	if ((Get_Time.Minutes / 10) == 0)
	{
		HAL_RTC_GetDate(&hrtc, &Get_Date, RTC_FORMAT_BIN);
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, (uint16_t)Get_Date.Year);
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, (uint16_t)Get_Date.Month);
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4, (uint16_t)Get_Date.Date);
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5, (uint16_t)Get_Date.WeekDay);
	}
}

/**
 * @brief 設定時間
 */
void bsp_RTC_TimeAndDate_set(void)
{
}

/**
 * @brief RTC_TimeAndDate_Show
 */
void bsp_RTC_TimeAndDate_Show(void)
{
	char dispBuff[32];
	HAL_RTC_GetTime(&hrtc, &Get_Time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &Get_Date, RTC_FORMAT_BIN);
	// printf("%02d/%02d/%02d ", 2000 + Get_Date.Year, Get_Date.Month, Get_Date.Date);
	// printf("%02d:%02d:%02d\r\n", Get_Time.Hours, Get_Time.Minutes, Get_Time.Seconds);
	sprintf(dispBuff, "%02d/%02d/%02d %02d:%02d:%02d", 2000 + Get_Date.Year, Get_Date.Month, Get_Date.Date, Get_Time.Hours, Get_Time.Minutes, Get_Time.Seconds);
	bsp_LCD_SetFont(&Font8x16);
	bsp_LCD_SetTextColor(GREEN);
	bsp_LCD_DispStringLine_EN(LINE(0), dispBuff);
}

/**
 * @brief bsp_RTC_Alarm1Set
 */
void bsp_RTC_Alarm1Set(void)
{
}

/**********************************END OF FILE*************************************/
