#include "bsp_Button_FIFO.h"

#define HARD_KEY_NUM 5				 /* 實體按鍵個數 */
#define KEY_COUNT (HARD_KEY_NUM + 0) /* 獨立按鍵 + 組合按鍵 */

/* 依次定義GPIO */
typedef struct
{
	GPIO_TypeDef *gpio;
	uint16_t pin;
	uint8_t ActiveLevel; /* 有效電平 */
} X_GPIO_T;

/* GPIO和PIN定義 */
static const X_GPIO_T s_gpio_list[HARD_KEY_NUM] = {
	{GPIOE, GPIO_PIN_2, 0}, /* K1 */
	{GPIOE, GPIO_PIN_3, 0}, /* K2 */
	{GPIOE, GPIO_PIN_4, 0}, /* K3 */
	{GPIOE, GPIO_PIN_5, 0}, /* K4 */
	{GPIOE, GPIO_PIN_1, 1}, /* K_TOUCH */
};

static KEY_T s_tBtn[KEY_COUNT] = {0}; //定義一個宏函數簡化後續代碼判斷GPIO引腳是否有效按下
static KEY_FIFO_T s_tKey;			  //按鍵FIFO變量,結構體

static void bsp_InitKeyVar(void);
static void bsp_InitKeyHard(void);
static void bsp_DetectKey(uint8_t i);
static uint8_t bsp_GetKey(void);
static uint8_t bsp_GetKey2(void);
#define KEY_PIN_ACTIVE(id)

/**
 * @brief 判斷案鍵是否按下
 * @param _id 按鍵ID
 * @return uint8_t 1:表示按下(導通),0表示未按下(釋放)
 */
static uint8_t KeyPinActive(uint8_t _id)
{
	uint8_t level;

	if ((s_gpio_list[_id].gpio->IDR & s_gpio_list[_id].pin) == 0)
	{
		level = 0;
	}
	else
	{
		level = 1;
	}

	if (level == s_gpio_list[_id].ActiveLevel)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/**
 * @brief 判斷按鍵是否按下。單鍵和組合鍵區分。單鍵事件不允許有其他鍵按下。
 * @param _id 按鍵ID
 * @return uint8_t 返回值 2表示按下(無效) ，1表示按下(導通），0表示未按下（釋放）
 */
static uint8_t IsKeyDownFunc(uint8_t _id)
{
	/* 實體單鍵 */
	if (_id < HARD_KEY_NUM)
	{
		uint8_t i;
		uint8_t count = 0;
		uint8_t save = 255;

		/* 判斷有幾個鍵按下 */
		for (i = 0; i < HARD_KEY_NUM; i++)
		{
			if (KeyPinActive(i))
			{
				count++;
				save = i;
			}
		}

		if (count == 1 && save == _id)
		{

			return 1; /* 只有1個鍵按下時才有效 */
		}

		if (count > 1 && (KEY_COUNT == HARD_KEY_NUM))
		{
			return 2; /* 單擊模式時，大於2時為無效 */
		}

		return 0;
	}

	/* 組合鍵 K1K2 */
	// if (_id == HARD_KEY_NUM + 0)
	// {
	// 	if (KeyPinActive(KID_K1) && KeyPinActive(KID_K2))
	// 	{
	// 		return 1;
	// 	}
	// 	else
	// 	{
	// 		return 0;
	// 	}
	// }

	// /* 組合鍵 K2K3 */
	// if (_id == HARD_KEY_NUM + 1)
	// {
	// 	if (KeyPinActive(KID_K2) && KeyPinActive(KID_K3))
	// 	{
	// 		return 1;
	// 	}
	// 	else
	// 	{
	// 		return 0;
	// 	}
	// }

	return 0;
}

/**
 * @brief 初始化按键(FIFO)
 */
void Initial_Key_Input_FIFO(void)
{
	bsp_InitKeyVar();  /* 初始化按键变量 */
	bsp_InitKeyHard(); /* 初始化按键硬件 */
}

/**
 * @brief 配置按鍵對應的GPIO
 */
static void bsp_InitKeyHard(void)
{
	/*已在MX_GPIO_Init()初始化完成*/
	// MX_GPIO_Init();
}

/**
 * @brief 初始化按鍵變量
 */
static void bsp_InitKeyVar(void)
{
	uint8_t i;

	/* 對按鍵FIFO讀寫指針清零 */
	s_tKey.Read = 0;
	s_tKey.Write = 0;
	s_tKey.Read2 = 0;

	/* 給每個按鍵結構體成員變量賦一組缺省值 */
	for (i = 0; i < KEY_COUNT; i++)
	{
		s_tBtn[i].LongTime = KEY_LONG_TIME; /* 長按時間 0 表示不檢測長按鍵事件 */
		s_tBtn[i].Count = 0;				/* 計數器設置為濾波時間的一半 KEY_FILTER_TIME / 2*/
		s_tBtn[i].State = 0;				/* 按鍵缺省狀態，0為未按下 */
		s_tBtn[i].RepeatSpeed = 0;			/* 按鍵連發的速度，0表示不支持連發 */
		s_tBtn[i].RepeatCount = 0;			/* 連發計數器 */
	}

	/* 如果需要單獨更改某個按键的参數，可以在此單獨重新赋值 */

	/* 長按1秒後，自動連發 */
	bsp_SetKeyParam(KID_K1, 100, 6);
	bsp_SetKeyParam(KID_K2, 100, 6);
	bsp_SetKeyParam(KID_K3, 100, 6);
	bsp_SetKeyParam(KID_K4, 100, 6);
}

/**
 * @brief 將1個鍵值壓入按鍵FIFO緩衝區。可用於模擬一個按鍵。
 * @param _KeyCode 按鍵代號
 */
void bsp_PutKey(uint8_t _KeyCode)
{
	s_tKey.Buf[s_tKey.Write] = _KeyCode;

	if (++s_tKey.Write >= KEY_FIFO_SIZE)
	{
		s_tKey.Write = 0;
	}
}

/**
 * @brief 從按鍵FIFO緩衝區讀取一個鍵值。
 * @return uint8_t 按键代號
 */
static uint8_t bsp_GetKey(void)
{
	uint8_t ret;

	if (s_tKey.Read == s_tKey.Write)
	{
		return KEY_NONE;
	}
	else
	{
		ret = s_tKey.Buf[s_tKey.Read];

		if (++s_tKey.Read >= KEY_FIFO_SIZE)
		{
			s_tKey.Read = 0;
		}
		return ret;
	}
}

/**
 * @brief 從按鍵FIFO緩衝區讀取一個鍵值。獨立的獨指針。
 * @return uint8_t 按键代號
 */
static uint8_t bsp_GetKey2(void)
{
	uint8_t ret;

	if (s_tKey.Read2 == s_tKey.Write)
	{
		return KEY_NONE;
	}
	else
	{
		ret = s_tKey.Buf[s_tKey.Read2];

		if (++s_tKey.Read2 >= KEY_FIFO_SIZE)
		{
			s_tKey.Read2 = 0;
		}
		return ret;
	}
}

/**
 * @brief 讀取按鍵的狀態
 * @param _ucKeyID 按鍵ID，從0開始
 * @return uint8_t 1 表示按下， 0 表示未按下
 */
uint8_t bsp_GetKeyState(KEY_ID_E _ucKeyID)
{
	return s_tBtn[_ucKeyID].State;
}

/**
 * @brief 設置按鍵參數
 * @param _ucKeyID 按鍵ID，從0開始
 * @param _LongTime 長按事件時間
 * @param _RepeatSpeed 連發速度
 */
void bsp_SetKeyParam(uint8_t _ucKeyID, uint16_t _LongTime, uint8_t _RepeatSpeed)
{
	s_tBtn[_ucKeyID].LongTime = _LongTime;		 /* 長按時間 0 表示不檢測長按鍵事件 */
	s_tBtn[_ucKeyID].RepeatSpeed = _RepeatSpeed; /* 按鍵連發的速度，0表示不支持連發 */
	s_tBtn[_ucKeyID].RepeatCount = 0;			 /* 連發計數器 */
}

/**
 * @brief 清空按鍵FIFO緩衝區
 */
void bsp_ClearKey(void)
{
	s_tKey.Read = s_tKey.Write;
}

/**
 * @brief 檢測一個按鍵。非阻塞狀態，必須被周期性的調用。
 * @param i IO的id， 從0開始編碼
 */
static void bsp_DetectKey(uint8_t i)
{
	KEY_T *pBtn;
	uint8_t _status;
	pBtn = &s_tBtn[i];
	_status = IsKeyDownFunc(i);
	if (_status == 1)
	{
		if (pBtn->Count < KEY_FILTER_TIME)
		{
			pBtn->Count = KEY_FILTER_TIME;
		}
		else if (pBtn->Count < 2 * KEY_FILTER_TIME)
		{
			pBtn->Count++;
		}
		else
		{
			if (pBtn->State == 0)
			{
				pBtn->State = 1;
				bsp_PutKey((uint8_t)(3 * i + 1)); //發送按鈕按下的消息
			}

			if (pBtn->LongTime > 0)
			{
				if (pBtn->LongCount < pBtn->LongTime)
				{
					/* 發送按鈕持續按下的消息 */
					if (++pBtn->LongCount == pBtn->LongTime)
					{
						bsp_PutKey((uint8_t)(3 * i + 3)); //鍵值放入按鍵FIFO
					}
				}
				else
				{
					if (pBtn->RepeatSpeed > 0)
					{
						if (++pBtn->RepeatCount >= pBtn->RepeatSpeed)
						{
							pBtn->RepeatCount = 0;
							bsp_PutKey((uint8_t)(3 * i + 1)); //長按鍵後，每隔10ms發送1個按鍵
						}
					}
				}
			}
		}
	}
	else if (_status == 0)
	{
		if (pBtn->Count > KEY_FILTER_TIME)
		{
			pBtn->Count = KEY_FILTER_TIME;
		}
		else if (pBtn->Count != 0)
		{
			pBtn->Count--;
		}
		else
		{
			if (pBtn->State == 1)
			{
				pBtn->State = 0;
				bsp_PutKey((uint8_t)(3 * i + 2)); //發送按鈕彈起的消息
			}
		}

		pBtn->LongCount = 0;
		pBtn->RepeatCount = 0;
	}
}

/**
 * @brief 檢測高速的輸入IO,1ms刷新一次
 * @param i IO的id， 從0開始編碼
 */
// static void bsp_DetectFastIO(uint8_t i)
// {
// 	KEY_T *pBtn;

// 	pBtn = &s_tBtn[i];
// 	if (IsKeyDownFunc(i))
// 	{
// 		if (pBtn->State == 0)
// 		{
// 			pBtn->State = 1;
// 			bsp_PutKey((uint8_t)(3 * i + 1)); //發送按鈕按下的消息
// 		}

// 		if (pBtn->LongTime > 0)
// 		{
// 			if (pBtn->LongCount < pBtn->LongTime)
// 			{
// 				/* 發送按鈕持續按下的消息 */
// 				if (++pBtn->LongCount == pBtn->LongTime)
// 				{
// 					bsp_PutKey((uint8_t)(3 * i + 3)); //鍵值放入按鍵FIFO
// 				}
// 			}
// 			else
// 			{
// 				if (pBtn->RepeatSpeed > 0)
// 				{
// 					if (++pBtn->RepeatCount >= pBtn->RepeatSpeed)
// 					{
// 						pBtn->RepeatCount = 0;
// 						bsp_PutKey((uint8_t)(3 * i + 1)); //長按鍵後，每隔10ms發送1個按鍵
// 					}
// 				}
// 			}
// 		}
// 	}
// 	else
// 	{
// 		if (pBtn->State == 1)
// 		{
// 			pBtn->State = 0;
// 			bsp_PutKey((uint8_t)(3 * i + 2)); //發送按鈕彈起的消息
// 		}

// 		pBtn->LongCount = 0;
// 		pBtn->RepeatCount = 0;
// 	}
// }

/**
 * @brief 掃描所有按鍵。非阻塞，被systick中斷週期性的調用，10ms一次
 */
void bsp_KeyScan10ms(void)
{
	uint8_t i;

	for (i = 0; i < KEY_COUNT; i++)
	{
		bsp_DetectKey(i);
	}
}

/**
 * @brief 掃描所有按鍵。非阻塞，被systick中斷週期性的調用，1ms一次.
 */

// void bsp_KeyScan1ms(void)
// {
// 	uint8_t i;

// 	for (i = 0; i < KEY_COUNT; i++)
// 	{
// 		bsp_DetectFastIO(i);
// 	}
// }

uint8_t bsp_keyEvent(void)
{
	static uint8_t ucKeyCode = 0;
	ucKeyCode = bsp_GetKey();
	if (ucKeyCode != KEY_NONE)
	{
		switch (ucKeyCode)
		{
		case KEY_DOWN_K1:
			bsp_Key1_DOWN();
			break;
		case KEY_UP_K1:
			bsp_Key1_UP();
			break;
		case KEY_LONG_K1:
			bsp_Key1_LONG();
			break;

		case KEY_DOWN_K2:
			bsp_Key2_DOWN();
			break;
		case KEY_UP_K2:
			bsp_Key2_UP();
			break;
		case KEY_LONG_K2:
			bsp_Key2_LONG();
			break;

		case KEY_DOWN_K3:
			bsp_Key3_DOWN();
			break;
		case KEY_UP_K3:
			bsp_Key3_UP();
			break;
		case KEY_LONG_K3:
			bsp_Key3_LONG();
			break;

		case KEY_DOWN_K4:
			bsp_Key4_DOWN();
			break;
		case KEY_UP_K4:
			bsp_Key4_UP();
			break;
		case KEY_LONG_K4:
			bsp_Key4_LONG();
			break;

		case KEY_DOWN_TOUCH:
			bsp_KeyTouch_DOWN();
			break;
		case KEY_UP_TOUCH:
			bsp_KeyTouch_UP();
			break;
		case KEY_LONG_TOUCH:
			bsp_KeyTouch_LONG();
			break;

		default:

			break;
		}
	}
	return ucKeyCode;
}
/*****************************  (END OF FILE) *********************************/
