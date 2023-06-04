#ifndef __BSP_Button_FIFO_H__
#define __BSP_Button_FIFO_H__
#include "bsp.h"

/* 根據應用的功能重命名按键宏 */
#define KEY_DOWN_K1 KEY_1_DOWN
#define KEY_UP_K1 KEY_1_UP
#define KEY_LONG_K1 KEY_1_LONG

#define KEY_DOWN_K2 KEY_2_DOWN
#define KEY_UP_K2 KEY_2_UP
#define KEY_LONG_K2 KEY_2_LONG

#define KEY_DOWN_K3 KEY_3_DOWN
#define KEY_UP_K3 KEY_3_UP
#define KEY_LONG_K3 KEY_3_LONG

#define KEY_DOWN_K4 KEY_4_DOWN
#define KEY_UP_K4 KEY_4_UP
#define KEY_LONG_K4 KEY_4_LONG

#define KEY_DOWN_TOUCH KEY_TOUCH_DOWN
#define KEY_UP_TOUCH KEY_TOUCH_UP
#define KEY_LONG_TOUCH KEY_TOUCH_LONG

/*
	定義鍵值代碼, 必須按如下次序定時每個鍵的按下、彈起和長按事件

	推薦使用enum, 不用#define，原因：
	(1) 便於新增鍵值,方便調整順序。
	(2) 編譯器可幫我們避免鍵值重複。
*/
typedef enum
{
	KEY_NONE = 0, /* 0 表示按鍵事件 */

	KEY_1_DOWN, /* 按下 */
	KEY_1_UP,	/* 彈起 */
	KEY_1_LONG, /* 長按 */

	KEY_2_DOWN, /* 按下 */
	KEY_2_UP,	/* 彈起 */
	KEY_2_LONG, /* 長按 */

	KEY_3_DOWN, /* 按下 */
	KEY_3_UP,	/* 彈起 */
	KEY_3_LONG, /* 長按 */

	KEY_4_DOWN, /* 按下 */
	KEY_4_UP,	/* 彈起 */
	KEY_4_LONG, /* 長按 */

	KEY_TOUCH_DOWN, /* 按下 */
	KEY_TOUCH_UP,	/* 彈起 */
	KEY_TOUCH_LONG, /* 長按 */
} KEY_ENUM;

/* 按鍵ID, 主要用於bsp_GetKeyState()函數的入口參數 */
typedef enum
{
	KID_K1 = 0,
	KID_K2,
	KID_K3,
	KID_K4,
	KID_KEY_TOUCH,
} KEY_ID_E;

/*
	按鍵濾波時間50ms, 單位10ms。
	只有連續檢測到50ms狀態不變才認為有效，包括彈起和按下兩種事件
	即使按鍵電路不做硬件濾波，該濾波機制也可以保證可靠地檢測到按鍵事件
*/
#define KEY_FILTER_TIME 5

#define KEY_LONG_TIME 100 /* 單位10ms， 持續1秒，認為長按事件 */

/*
	每个按键对应1个全局的结构体变量。
*/
typedef struct
{
	/* 下面是一個函數指針，指向判斷按鍵手否按下的函數 */
	uint8_t (*IsKeyDownFunc)(void); /* 按鍵按下的判斷函數,1表示按下 */

	uint8_t Count;		 /* 濾波器計數器 */
	uint16_t LongCount;	 /* 長按計數器 */
	uint16_t LongTime;	 /* 按鍵按下持續時間, 0表示不檢測長按 */
	uint8_t State;		 /* 按鍵當前狀態（按下還是彈起） */
	uint8_t RepeatSpeed; /* 連續按鍵週期 */
	uint8_t RepeatCount; /* 連續按鍵計數器 */
} KEY_T;

/* 按鍵FIFO的容量 */
#define KEY_FIFO_SIZE 10
typedef struct
{
	uint8_t Buf[KEY_FIFO_SIZE]; /* 鍵值緩衝區 */
	uint8_t Read;				/* 緩衝區讀指針1 */
	uint8_t Write;				/* 緩衝區寫指針 */
	uint8_t Read2;				/* 緩衝區讀指針2 */
} KEY_FIFO_T;

/* 供外部調用的函數聲明 */
void Initial_Key_Input_FIFO(void);
void bsp_KeyScan10ms(void);
// void bsp_KeyScan1ms(void);
void bsp_PutKey(uint8_t _KeyCode);
uint8_t bsp_GetKeyState(KEY_ID_E _ucKeyID);
void bsp_SetKeyParam(uint8_t _ucKeyID, uint16_t _LongTime, uint8_t _RepeatSpeed);
void bsp_ClearKey(void);
uint8_t bsp_keyEvent(void);

#endif // __BSP_Button_FIFO_H__
/*****************************  (END OF FILE) *********************************/
