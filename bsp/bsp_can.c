#include "bsp_can.h"

#ifdef HAL_I2C_MODULE_ENABLED

#define CAN_MAX_DATA_SIZE 8
#define CAN_RX_STDID 0x100
/*
*********************************************************************************************************
*                                             CAN配置
*********************************************************************************************************
*/
CAN_RxHeaderTypeDef g_CanRxHeader;
uint8_t g_CanRxData[8];

/**
 * @brief 初始CAN
 */
void bsp_CAN_Init(void)
{
	bsp_CAN_Filter_Set();											  /* 設定過濾器 */
	HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING); /* 啟動 RX FIFO0的Notification */
	HAL_CAN_Start(&hcan);											  /* 啟動 CAN */
}

/**
 * @brief CAN 過濾器設定
 */
void bsp_CAN_Filter_Set(void)
{
	CAN_FilterTypeDef sFilterConfig = {0};
	uint8_t status = HAL_OK;

	sFilterConfig.FilterActivation = ENABLE;
	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;

	/* 只接收 標準ID & DATA */
	sFilterConfig.FilterIdHigh = (((uint32_t)CAN_RX_STDID << 21) & 0xFFFF0000) >> 16;
	sFilterConfig.FilterIdLow = (((uint32_t)CAN_RX_STDID << 21) | CAN_ID_STD | CAN_RTR_DATA) & 0xFFFF;
	sFilterConfig.FilterMaskIdHigh = 0xFFFF;
	sFilterConfig.FilterMaskIdLow = 0xFFFF;

	// /* 只接收 標準ID & REMOTE */
	// sFilterConfig.FilterIdHigh = (((uint32_t)CAN_RX_STDID << 21) & 0xFFFF0000) >> 16;
	// sFilterConfig.FilterIdLow = (((uint32_t)CAN_RX_STDID << 21) | CAN_ID_STD | CAN_RTR_REMOTE) & 0xFFFF;
	// sFilterConfig.FilterMaskIdHigh = 0xFFFF;
	// sFilterConfig.FilterMaskIdLow = 0xFFFF;

	/* 只接收 擴展ID & DATA */
	// sFilterConfig.FilterIdHigh = (((uint32_t)CAN_RX_EXTID << 3) & 0xFFFF0000) >> 16;
	// sFilterConfig.FilterIdLow = (((uint32_t)CAN_RX_EXTID << 3) | CAN_ID_EXT | CAN_RTR_DATA)&0xFFFF;
	// sFilterConfig.FilterMaskIdHigh = 0xFFFF; //高16bit每位必須匹配
	// sFilterConfig.FilterMaskIdLow = 0xFFFF;  //低16bit每位必須匹配

	/* 只接收 擴展ID & REMOTE */
	// sFilterConfig.FilterIdHigh = (((uint32_t)CAN_RX_EXTID << 3) & 0xFFFF0000) >> 16;
	// sFilterConfig.FilterIdLow = (((uint32_t)CAN_RX_EXTID << 3) | CAN_ID_EXT | CAN_RTR_REMOTE) & 0xFFFF;
	// sFilterConfig.FilterMaskIdHigh = 0xFFFF; //高16bit每位必須匹配
	// sFilterConfig.FilterMaskIdLow = 0xFFFF;	 //低16bit每位必須匹配

	status = HAL_CAN_ConfigFilter(&hcan, &sFilterConfig);
	if (status != HAL_OK)
	{
		bsp_Log_Error("HAL_CAN_ConfigFilter happen error (%d).", status);
	}
}

/**
 * @brief 發送資料
 * @param _DataBuf 資料
 * @param _Len 資料長度 最大支持 8Byte
 */
bsp_Status bsp_CAN_SendPacket(uint8_t *_DataBuf, uint8_t _Len)
{
	CAN_TxHeaderTypeDef sTxHeader = {0};
	uint32_t _TxMailbox = 0;
	uint8_t status = HAL_OK;

	if (_Len > CAN_MAX_DATA_SIZE)
	{
		_Len = CAN_MAX_DATA_SIZE;
	}

	/* 配置發送參數 */
	sTxHeader.StdId = 0x111;
	sTxHeader.ExtId = 0;
	sTxHeader.IDE = CAN_ID_STD;
	sTxHeader.RTR = CAN_RTR_DATA;
	sTxHeader.DLC = _Len;
	sTxHeader.TransmitGlobalTime = DISABLE;
	status = (bsp_Status)HAL_CAN_AddTxMessage(&hcan, &sTxHeader, _DataBuf, &_TxMailbox);
	if (status != bsp_PASSED)
	{
		bsp_Log_Error("HAL_CAN_AddTxMessage happen error (%d).", status);
		return bsp_FAILED;
	}
	while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) != 3)
	{
		/* wait. */
	}
	return bsp_PASSED;
}

/**
 * @brief 處理CAN接收到的信息
 */
void bsp_CAN_Analyze(void)
{
	char canmsg[128] = {0};
	char cantemp[32] = {0};
	sprintf(cantemp, "CAN StdId : 0x%3x | ", g_CanRxHeader.StdId);
	strcat(canmsg, cantemp);
	sprintf(cantemp, "DLC : %02d | Msg : ", g_CanRxHeader.DLC);
	strcat(canmsg, cantemp);
	for (uint8_t i = 0; i < g_CanRxHeader.DLC; i++)
	{
		sprintf(cantemp, "0x%02x ", g_CanRxData[i]);
		strcat(canmsg, cantemp);
	}

	bsp_Log_Info("%s", canmsg);
}

/**
 * @brief CAN中斷(回調)
 * @param hcan CAN_HandleTypeDef
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	if (hcan->Instance == CAN1)
	{
		HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &g_CanRxHeader, g_CanRxData); /* 讀取資料 */

		// HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING); /* 啟動 RX FIFO0的Notification */

		if ((g_CanRxHeader.StdId == CAN_RX_STDID) && (g_CanRxHeader.IDE == CAN_ID_STD))
		{
			bsp_PutMsg(MSG_CANRX, 0); /* 發消息到MSG中，資料在g_CanRxHeader， g_Can1RxData */
		}
	}
}

#endif /* HAL_I2C_MODULE_ENABLED */

/***************************** (END OF FILE) *********************************/
