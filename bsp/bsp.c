#include "bsp.h"

extern LCD_DrvTypeDef *LCDDrv;
extern IOExpand_DrvTypeDef *IODrv;
static void bsp_UniqueDeviceID(void);

void bsp_Init(void)
{

#if (Enable_EventRecorder == 1)
	EventRecorderInitialize(EventRecordAll, 1U);
#endif

/* AXI SRAM的時鐘是上電自動使能的，而D2域的SRAM1，SRAM2和SRAM3要單獨使能 */
#if (Enable_D2SRAM == 1)
	__HAL_RCC_D2SRAM1_CLK_ENABLE();
	__HAL_RCC_D2SRAM2_CLK_ENABLE();
	__HAL_RCC_D2SRAM3_CLK_ENABLE();
#endif
	// bsp_UniqueDeviceID();
	bsp_RTC_AbnormalCheck();
	bsp_SFlash_Init();
	bsp_EEPROM_init();
	bsp_IOEXPAND_Init();
	bsp_InitMsg();
	bsp_LCD_Init();
	bsp_USART1_Interrupt_Set();
	bsp_rs485_Config();
	bsp_wifi_Config();
	// bsp_CAN_Init();
	CANSPI_Initialize();
	bsp_SoftI2C_init();
	Initial_Key_Input_FIFO();
	bsp_AHT_Init();
	// bsp_LED_GPIO_Init();
	// bsp_LED_PWM_Start();

	/*初始化完成*/
	HAL_Delay(200);
	bsp_LCD_Clear(0, 0, LCDDrv->GetLcdPixelWidth(), LCDDrv->GetLcdPixelHeight());

	bsp_RTC_TimeAndDate_Show();
	bsp_ATH_Show();

	bsp_TimerStart();
	userShellInit();
	uint8_t ret;
	ret = easyflash_init();
	if (ret != EF_NO_ERR)
	{
		bsp_Log_Error("EasyFlash init fall, EfErrCode = %d", ret);
	}
	// char Username[20] = {0}, pwdbuff[20] = {0};
	// char Waveform1[10] = {10,20,30,40,50,40,30,20,20,40};
	// size_t len = 0;
	// ef_env_set_default();
	// ef_set_env_blob("username", "Roy456", 6);
	// ef_set_env_blob("pwd", "1234567", 7);
	// ef_set_env_blob("waveform1", Waveform1, 10);

	// ef_get_env_blob("username", NULL, 0, &len);
	// ef_get_env_blob("username", Username, len, NULL);
	// bsp_Log_Info("username env is %s", Username);
	// len = 0;
	// ef_get_env_blob("pwd", NULL, 0, &len);
	// ef_get_env_blob("pwd", pwdbuff, len, NULL);
	// bsp_Log_Info("pwd env is %s", pwdbuff);

	// len = 0;
	// memset(pwdbuff,'\0',20);
	// ef_get_env_blob("waveform1", NULL, 0, &len);
	// ef_get_env_blob("waveform1", pwdbuff, len, NULL);
	// for(int i = 0;i<len;i++)
	// {
	// 	bsp_Log_Info("%d ", pwdbuff[i]);
	// }
	// ef_print_env();
}
/*******************************************************************************************************/

/**
 * @brief 取得App版本
 * @param version 版本號
 * @return bsp_Status 狀態
 */
bsp_Status bsp_Get_App_Version(char *version)
{
	strcpy(version, USER_APP_VER);
	return bsp_PASSED;
}

/**
 * @brief 比較Buffer
 * @param pBuffer1 Buffer1
 * @param pBuffer2 Buffer
 * @param BufferLength Buffer Size
 * @return bsp_Status PASSED : 結果一致 , FAILED : 結果不一致
 */
bsp_Status bsp_Buffercmp(uint8_t *pBuffer1, uint8_t *pBuffer2, uint32_t BufferLength)
{
	while (BufferLength--)
	{
		if (*pBuffer1 != *pBuffer2)
		{
			return bsp_FAILED;
		}

		pBuffer1++;
		pBuffer2++;
	}
	return bsp_PASSED;
}

/**
 * @brief 讀取唯一ID
 */
static void bsp_UniqueDeviceID(void)
{
	uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;
	CPU_Sn0 = *(__IO uint32_t *)(0x1FFFF7E8);
	CPU_Sn1 = *(__IO uint32_t *)(0x1FFFF7E8 + 4);
	CPU_Sn2 = *(__IO uint32_t *)(0x1FFFF7E8 + 8);
	bsp_Log_Info("CPU : STM32F103ZET6, SystemCoreClock: %dMHz", SystemCoreClock / 1000000);
	bsp_Log_Info("UID = %08X %08X %08X\r\n", CPU_Sn2, CPU_Sn1, CPU_Sn0);
}
/*******************************************************************************************************/

/**
 * @brief 該函數每隔1ms被Systick中斷調用1次。一些需要周期性處理的事務
 *			 可以放在此函數。比如：觸摸坐標掃描。
 */
void bsp_RunPer1ms(void)
{
	MODH_Poll();
}

/**
 * @brief 該函數每隔5ms被Systick中斷調用1次。一些處理時間要求不嚴格的
 *			任務可以放在此函數。比如：按鍵掃描、蜂鳴器鳴叫控制等。
 */
void bsp_RunPer5ms(void)
{
}

/**
 * @brief 該函數每隔10ms被Systick中斷調用1次。一些處理時間要求不嚴格的
 *			任務可以放在此函數。比如：按鍵掃描、蜂鳴器鳴叫控制等。
 */
void bsp_RunPer10ms(void)
{
	bsp_KeyScan10ms();
	//  bsp_LED_R_PWM();
	//  bsp_LED_G_PWM();
	//  bsp_LED_B_PWM();
	static uint16_t _ms = 0;
	if (_ms % 100 == 0)
	{
		bsp_PutMsg(MSG_Show_TimeAndDate, 0);
	}
	if (_ms % 200 == 0)
	{
		bsp_PutMsg(MSG_Show_TEMPHUM, 0);
		bsp_ADC1_Start();
		_ms = 0;
	}
	_ms++;
}
/**
 * @brief 該函數每隔20ms被Systick中斷調用1次。一些處理時間要求不嚴格的
 *			任務可以放在此函數。比如：按鍵掃描、蜂鳴器鳴叫控制等。
 */
void bsp_RunPer20ms(void)
{
}
/**
 * @brief 該函數每隔100ms被Systick中斷調用1次。一些處理時間要求不嚴格的
 *			任務可以放在此函數。
 */
void bsp_RunPer100ms(void)
{
	bsp_RTC_Date_update();
}

/**
 * @brief bsp的空閒任務，在while中調用
 */
void bsp_Idle(void)
{
	bsp_keyEvent();
}
/*******************************************************************************************************/
/**
 * @brief Key1按下
 */
void bsp_Key1_DOWN(void)
{
	bsp_Log_Info("K1 DOWN");
	bsp_wifi_SendPacket("AT\r\n");
	bsp_PutMsg(MSG_WIFIRX, 0);
	//  if (bsp_LCD_Test > 1 && bsp_LCD_Test != 1)
	//  {
	//  	bsp_LCD_Test--;
	//  }
	//__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, 0);
}

/**
 * @brief Key1彈起
 */
void bsp_Key1_UP(void)
{
	bsp_Log_Info("K1 UP");
}

/**
 * @brief Key1長按
 */
void bsp_Key1_LONG(void)
{
	bsp_Log_Info("K1 LONG");
}

/**
 * @brief Key2按下
 */
void bsp_Key2_DOWN(void)
{
	bsp_Log_Info("K2 DOWN");
	bsp_wifi_SendPacket("AA\r\n");
	bsp_PutMsg(MSG_WIFIRX, 0);
	//__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, 10000 - 1);
}

/**
 * @brief Key2彈起
 */
void bsp_Key2_UP(void)
{
	bsp_Log_Info("K2 UP");
}

/**
 * @brief Key2長按
 */
void bsp_Key2_LONG(void)
{
	bsp_Log_Info("K2 LONG");
}
/**
 * @brief Key3按下
 */
void bsp_Key3_DOWN(void)
{
	bsp_Log_Info("K3 DOWN");
	// bsp_PutMsg(MSG_CANTX, 0);
	bsp_PutMsg(MSG_SPICANTEST, 0);
}

/**
 * @brief Key3彈起
 */
void bsp_Key3_UP(void)
{
	bsp_Log_Info("K3 UP");
}

/**
 * @brief Key3長按
 */
void bsp_Key3_LONG(void)
{
	bsp_Log_Info("K3 LONG");
}
/**
 * @brief Key4按下
 */
void bsp_Key4_DOWN(void)
{
	bsp_Log_Info("K4 DOWN");
}

/**
 * @brief Key4彈起
 */
void bsp_Key4_UP(void)
{
	bsp_Log_Info("K4 UP");
}

/**
 * @brief Key4長按
 */
void bsp_Key4_LONG(void)
{
	bsp_Log_Info("K4 LONG");
}

/**
 * @brief KeyTouch按下
 */
void bsp_KeyTouch_DOWN(void)
{
	bsp_Log_Info("KeyTouch DOWN");
	bsp_PutMsg(MSG_LED2_Sparkle, 0);
}

/**
 * @brief KeyTouch彈起
 */
void bsp_KeyTouch_UP(void)
{
	bsp_Log_Info("KeyTouch UP");
	bsp_PutMsg(MSG_LED2_Sparkle, 1);
}

/**
 * @brief KeyTouch長按
 */
void bsp_KeyTouch_LONG(void)
{
	bsp_Log_Info("KeyTouch LONG");
}

/*******************************************************************************************************/
/* Callback 函數  */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	uint8_t _value = 0;
	if(GPIO_Pin == PCF8574T_INT_Pin)
	{
		IODrv->ReadByte(&_value);
		bsp_Log_Info("PCF8574T happen INT.");
	}
}
/*********************************************END OF FILE**********************/
