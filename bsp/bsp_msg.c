#include "bsp.h"

MSG_FIFO_T g_tMsg;

/**
 * @brief 初始化消息緩衝區
 */
void bsp_InitMsg(void)
{
	bsp_ClearMsg();
}

/**
 * @brief 輸出目前Msg緩衝區的值
 */
void bsp_PrintMsgBuffer(void)
{
	uint8_t i;
	if (g_tMsg.Read != g_tMsg.Write)
	{
		for (i = g_tMsg.Read; i != g_tMsg.Write; i = (i + 1) % MSG_FIFO_SIZE)
		{
			MSG_T *p = &g_tMsg.Buf[i];
			bsp_Log_Info("MsgCode: %d, MsgParam: %d", p->MsgCode, p->MsgParam);
		}
	}
	else
	{
		bsp_Log_Info("The container is empty.");
	}
	bsp_Log_Info("-----------");
}

/**
 * @brief 將一個消息押入消息FIFO緩衝區
 * @param _MsgCode 消息Code
 * @param _MsgParam 消息參數，一般向某個特定的結構體，或是0
 */
void bsp_PutMsg(uint16_t _MsgCode, uint32_t _MsgParam)
{
	if (g_tMsg.Read == (g_tMsg.Write + 1) % MSG_FIFO_SIZE)
	{
		// 緩衝區已滿，無法插入新的消息
		bsp_Log_Info("The container is full and cannot insert new elements.\n");
		return;
	}

	g_tMsg.Buf[g_tMsg.Write].MsgCode = _MsgCode;
	g_tMsg.Buf[g_tMsg.Write].MsgParam = _MsgParam;

	if (++g_tMsg.Write >= MSG_FIFO_SIZE)
	{
		g_tMsg.Write = 0;
	}
}

/**
 * @brief 將一個消息押入消息FIFO緩衝區首位
 * @param _MsgCode 消息Code
 * @param _MsgParam 消息參數，一般向某個特定的結構體，或是0
 */
void bsp_PutMsgUrgent(uint16_t _MsgCode, uint32_t _MsgParam)
{
	if (g_tMsg.Read == (g_tMsg.Write + 1) % MSG_FIFO_SIZE)
	{
		// 緩衝區已滿，無法插入新的消息
		bsp_Log_Info("The container is full and cannot insert new elements.\n");
		return;
	}

	if (g_tMsg.Read == 0)
	{
		g_tMsg.Read = MSG_FIFO_SIZE - 1;
	}
	else
	{
		g_tMsg.Read--;
	}

	g_tMsg.Buf[g_tMsg.Read].MsgCode = _MsgCode;
	g_tMsg.Buf[g_tMsg.Read].MsgParam = _MsgParam;
}

/**
 * @brief 從消息FIFO緩衝區讀取一個鍵值
 * @param _pMsg 消息Code
 * @return uint8_t 0 表示無消息； 1表示有消息
 */
uint8_t bsp_GetMsg(MSG_T *_pMsg)
{
	MSG_T *p;

	if (g_tMsg.Read == g_tMsg.Write)
	{
		return 0;
	}
	else
	{
		p = &g_tMsg.Buf[g_tMsg.Read];

		if (++g_tMsg.Read >= MSG_FIFO_SIZE)
		{
			g_tMsg.Read = 0;
		}

		_pMsg->MsgCode = p->MsgCode;
		_pMsg->MsgParam = p->MsgParam;
		return 1;
	}
}

/**
 * @brief 從消息FIFO緩衝區讀取一個鍵值。使用第二個讀取指針。可以2個進程同時訪問消息區
 * @param _pMsg 消息Code
 * @return uint8_t 0 表示無消息； 1表示有消息
 */
uint8_t bsp_GetMsg2(MSG_T *_pMsg)
{
	MSG_T *p;

	if (g_tMsg.Read2 == g_tMsg.Write)
	{
		return 0;
	}
	else
	{
		p = &g_tMsg.Buf[g_tMsg.Read2];

		if (++g_tMsg.Read2 >= MSG_FIFO_SIZE)
		{
			g_tMsg.Read2 = 0;
		}

		_pMsg->MsgCode = p->MsgCode;
		_pMsg->MsgParam = p->MsgParam;
		return 1;
	}
}

/**
 * @brief 清空消息FIFO緩衝區
 */
void bsp_ClearMsg(void)
{
	g_tMsg.Read = g_tMsg.Write;
}

/***************************** (END OF FILE) *********************************/
