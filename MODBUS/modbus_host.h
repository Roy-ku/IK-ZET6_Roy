#ifndef __MOSBUS_HOST_H
#define __MOSBUS_HOST_H

/* 面板作為時，主板作從機 */
#define SlaveAddr 0x01
#define HBAUD485 RS485_HUARTx.Init.BaudRate

/* 01H 讀強制單線圈 */
/* 05H 寫強制單線圈 */
#define REG_D01 0x0100
#define REG_D02 0x0101
#define REG_D03 0x0102
#define REG_D04 0x0103
#define REG_D05 0x0104
#define REG_DXX REG_D05

/* 02H 讀取輸入狀態 */
#define REG_T01 0x0201
#define REG_T02 0x0202
#define REG_T03 0x0203
#define REG_TXX REG_T03

/* 03H 讀保持寄存器 */
/* 06H 寫保持寄存器 */
/* 10H 寫多個保存寄存器 */
#define REG_P01 0x0301
#define REG_P02 0x0302

/* 04H 讀取輸入寄存器(模擬信號) */
#define REG_A01 0x0401
#define REG_AXX REG_A01

/* RTU 應答代碼 */
#define RSP_OK 0			  /* 成功 */
#define RSP_ERR_CMD 0x01	  /* 不支持的功能碼 */
#define RSP_ERR_REG_ADDR 0x02 /* 寄存器地址錯誤 */
#define RSP_ERR_VALUE 0x03	  /* 數據值域錯誤 */
#define RSP_ERR_WRITE 0x04	  /* 寫入失敗 */

#define H_RX_BUF_SIZE 64
#define H_TX_BUF_SIZE 128

typedef struct
{
	uint8_t RxBuf[H_RX_BUF_SIZE];
	uint8_t RxCount;
	uint8_t RxStatus;
	uint8_t RxNewFlag;

	uint8_t RspCode;

	uint8_t TxBuf[H_TX_BUF_SIZE];
	uint8_t TxCount;

	uint16_t Reg01H; /* 保存主機發送的寄存器首地址 */
	uint16_t Reg02H;
	uint16_t Reg03H;
	uint16_t Reg04H;

	uint8_t RegNum; /* 寄存器個數 */

	uint8_t fAck01H; /* 應答命令標誌 0 表示執行失敗 1表示執行成功 */
	uint8_t fAck02H;
	uint8_t fAck03H;
	uint8_t fAck04H;
	uint8_t fAck05H;
	uint8_t fAck06H;
	uint8_t fAck10H;

} MODH_T;

typedef struct
{
	/* 03H 06H 讀寫保持寄存器 */
	uint16_t P01;
	uint16_t P02;

	/* 02H 讀寫離散輸入寄存器 */
	uint16_t T01;
	uint16_t T02;
	uint16_t T03;

	/* 04H 讀取模擬量寄存器 */
	uint16_t A01;

	/* 01H 05H 讀寫單個強制線圈 */
	uint16_t D01;
	uint16_t D02;
	uint16_t D03;
	uint16_t D04;

} VAR_T;

void MODH_ReciveNew(uint8_t _data);
void MODH_Poll(void);
uint8_t MODH_ReadParam_01H(uint16_t _reg, uint16_t _num);
uint8_t MODH_ReadParam_02H(uint16_t _reg, uint16_t _num);
uint8_t MODH_ReadParam_03H(uint16_t _reg, uint16_t _num);
uint8_t MODH_ReadParam_04H(uint16_t _reg, uint16_t _num);
uint8_t MODH_WriteParam_05H(uint16_t _reg, uint16_t _value);
uint8_t MODH_WriteParam_06H(uint16_t _reg, uint16_t _value);
uint8_t MODH_WriteParam_10H(uint16_t _reg, uint8_t _num, uint8_t *_buf);

extern MODH_T g_tModH;

#endif

/***************************** (END OF FILE) *********************************/
