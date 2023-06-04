#ifndef __BSP_MSG_H
#define __BSP_MSG_H

#include "stdint.h"

#define MSG_FIFO_TOTAL 40 /* FIFO數量 */
#define MSG_FIFO_SIZE (MSG_FIFO_TOTAL + 1)

typedef enum
{
	MSG_NONE = 0,

	MSG_LED2_Sparkle,
	MSG_Show_TEMPHUM,
	MSG_Show_TimeAndDate,
	MSG_CANTX,
	MSG_CANRX,
	MSG_WIFIRX,
	MSG_SPICANTEST,

} MSG_INFO;

/* 消息FIFO用到變量 */
typedef struct
{
	uint16_t MsgCode;  /* 消息code */
	uint32_t MsgParam; /* 消息的資料體, 也可以是指針（强制轉換）*/
} MSG_T;

typedef struct
{
	MSG_T Buf[MSG_FIFO_SIZE]; /* 消息緩存區 */
	uint8_t Read;			  /* 緩存區讀指針1 */
	uint8_t Write;			  /* 緩存區寫指針 */
	uint8_t Read2;			  /* 緩存區讀指針2 */
} MSG_FIFO_T;

/* 供外部調用的函數聲明 */
void bsp_InitMsg(void);
void bsp_PrintMsgBuffer(void);
void bsp_PutMsg(uint16_t _MsgCode, uint32_t _MsgParam);
void bsp_PutMsgUrgent(uint16_t _MsgCode, uint32_t _MsgParam);
uint8_t bsp_GetMsg(MSG_T *_pMsg);
uint8_t bsp_GetMsg2(MSG_T *_pMsg);
void bsp_ClearMsg(void);

#endif

/***************************** (END OF FILE) *********************************/
