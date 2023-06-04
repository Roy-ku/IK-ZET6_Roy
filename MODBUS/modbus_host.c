#include "bsp.h"
#include "modbus_crc_table.h"

#define TIMEOUT 200 /* 接收命令超時時間, 單位ms */
#define NUM 1		/* 循環發送次數 */

/*
Baud rate	Bit rate	 Bit time	 Character time	  3.5 character times
  2400	    2400 bits/s	  417 us	      4.6 ms	      16 ms
  4800	    4800 bits/s	  208 us	      2.3 ms	      8.0 ms
  9600	    9600 bits/s	  104 us	      1.2 ms	      4.0 ms
 19200	   19200 bits/s    52 us	      573 us	      2.0 ms
 38400	   38400 bits/s	   26 us	      286 us	      1.75 ms(1.0 ms)
 115200	   115200 bit/s	  8.7 us	       95 us	      1.75 ms(0.33 ms) 后面固定都為1750us
*/

typedef struct
{
	uint32_t Bps;
	uint32_t usTimeOut;
} MODBUSBPS_T;

const MODBUSBPS_T ModbusBaudRate[] =
	{
		{2400, 16000}, /* Baudrate 2400bps, 3.5字符延遲時間 16000us */
		{4800, 8000},
		{9600, 4000},
		{19200, 2000},
		{38400, 1750},
		{115200, 1750},
		{128000, 1750},
		{230400, 1750},
};

MODH_T g_tModH = {0};
__IO uint8_t g_modh_timeout = 0;
VAR_T g_tVar;

static void MODH_RxTimeOut(void);
static void MODH_Analyze(void);

static void MODH_Read_01H(void);
static void MODH_Read_02H(void);
static void MODH_Read_03H(void);
static void MODH_Read_04H(void);
static void MODH_Read_05H(void);
static void MODH_Read_06H(void);
static void MODH_Read_10H(void);

/*
*********************************************************************************************************
*	函 数 名: MODH_SendPacket
*	功能说明: 发送数据包 COM1口
*	形    参: _buf : 数据缓冲区
*			  _len : 数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_SendPacket(uint8_t *_buf, uint16_t _len)
{
	bsp_rs485_SendStr_length(_buf, _len);
}

/*
*********************************************************************************************************
*	函 数 名: MODH_SendAckWithCRC
*	功能说明: 发送应答,自动加CRC.
*	形    参: 无。发送数据在 g_tModH.TxBuf[], [g_tModH.TxCount
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODH_SendAckWithCRC(void)
{
	uint16_t crc;

	crc = CRC16_Modbus(g_tModH.TxBuf, g_tModH.TxCount);
	g_tModH.TxBuf[g_tModH.TxCount++] = crc >> 8;
	g_tModH.TxBuf[g_tModH.TxCount++] = crc;
	MODH_SendPacket(g_tModH.TxBuf, g_tModH.TxCount);

#if 0 /* 此部分为了串口打印结果,实际运用中可不要 */
	g_tPrint.Txlen = g_tModH.TxCount;
	memcpy(g_tPrint.TxBuf, g_tModH.TxBuf, g_tModH.TxCount);
#endif
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Analyze
*	功能说明: 分析应用层协议。处理应答。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODH_Analyze(void)
{
	switch (g_tModH.RxBuf[1]) /* 第2个字节 功能码 */
	{
	case 0x01: /* 读取线圈状态 */
		MODH_Read_01H();
		break;

	case 0x02: /* 读取输入状态 */
		MODH_Read_02H();
		break;

	case 0x03: /* 读取保持寄存器 在一个或多个保持寄存器中取得当前的二进制值 */
		MODH_Read_03H();
		break;

	case 0x04: /* 读取输入寄存器 */
		MODH_Read_04H();
		break;

	case 0x05: /* 强制单线圈 */
		MODH_Read_05H();
		break;

	case 0x06: /* 写单个寄存器 */
		MODH_Read_06H();
		break;

	case 0x10: /* 写多个寄存器 */
		MODH_Read_10H();
		break;

	default:
		break;
	}
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Send01H
*	功能说明: 发送01H指令，查询1个或多个保持寄存器
*	形    参: _addr : 从站地址
*			  _reg : 寄存器编号
*			  _num : 寄存器个数
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Send01H(uint8_t _addr, uint16_t _reg, uint16_t _num)
{
	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;	  /* 从站地址 */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x01;	  /* 功能码 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8; /* 寄存器编号 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;	  /* 寄存器编号 低字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num >> 8; /* 寄存器个数 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num;	  /* 寄存器个数 低字节 */

	MODH_SendAckWithCRC(); /* 发送数据，自动加CRC */
	g_tModH.fAck01H = 0;   /* 清接收标志 */
	g_tModH.RegNum = _num; /* 寄存器个数 */
	g_tModH.Reg01H = _reg; /* 保存03H指令中的寄存器地址，方便对应答数据进行分类 */
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Send02H
*	功能说明: 发送02H指令，读离散输入寄存器
*	形    参: _addr : 从站地址
*			  _reg : 寄存器编号
*			  _num : 寄存器个数
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Send02H(uint8_t _addr, uint16_t _reg, uint16_t _num)
{
	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;	  /* 从站地址 */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x02;	  /* 功能码 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8; /* 寄存器编号 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;	  /* 寄存器编号 低字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num >> 8; /* 寄存器个数 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num;	  /* 寄存器个数 低字节 */

	MODH_SendAckWithCRC(); /* 发送数据，自动加CRC */
	g_tModH.fAck02H = 0;   /* 清接收标志 */
	g_tModH.RegNum = _num; /* 寄存器个数 */
	g_tModH.Reg02H = _reg; /* 保存03H指令中的寄存器地址，方便对应答数据进行分类 */
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Send03H
*	功能说明: 发送03H指令，查询1个或多个保持寄存器
*	形    参: _addr : 从站地址
*			  _reg : 寄存器编号
*			  _num : 寄存器个数
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Send03H(uint8_t _addr, uint16_t _reg, uint16_t _num)
{
	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;	  /* 从站地址 */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x03;	  /* 功能码 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8; /* 寄存器编号 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;	  /* 寄存器编号 低字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num >> 8; /* 寄存器个数 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num;	  /* 寄存器个数 低字节 */

	MODH_SendAckWithCRC(); /* 发送数据，自动加CRC */
	g_tModH.fAck03H = 0;   /* 清接收标志 */
	g_tModH.RegNum = _num; /* 寄存器个数 */
	g_tModH.Reg03H = _reg; /* 保存03H指令中的寄存器地址，方便对应答数据进行分类 */
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Send04H
*	功能说明: 发送04H指令，读输入寄存器
*	形    参: _addr : 从站地址
*			  _reg : 寄存器编号
*			  _num : 寄存器个数
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Send04H(uint8_t _addr, uint16_t _reg, uint16_t _num)
{
	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;	  /* 从站地址 */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x04;	  /* 功能码 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8; /* 寄存器编号 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;	  /* 寄存器编号 低字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num >> 8; /* 寄存器个数 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num;	  /* 寄存器个数 低字节 */

	MODH_SendAckWithCRC(); /* 发送数据，自动加CRC */
	g_tModH.fAck04H = 0;   /* 清接收标志 */
	g_tModH.RegNum = _num; /* 寄存器个数 */
	g_tModH.Reg04H = _reg; /* 保存03H指令中的寄存器地址，方便对应答数据进行分类 */
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Send05H
*	功能说明: 发送05H指令，写强置单线圈
*	形    参: _addr : 从站地址
*			  _reg : 寄存器编号
*			  _value : 寄存器值,2字节
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Send05H(uint8_t _addr, uint16_t _reg, uint16_t _value)
{
	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;		/* 从站地址 */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x05;		/* 功能码 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8;	/* 寄存器编号 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;		/* 寄存器编号 低字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _value >> 8; /* 寄存器值 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _value;		/* 寄存器值 低字节 */

	MODH_SendAckWithCRC(); /* 发送数据，自动加CRC */

	g_tModH.fAck05H = 0; /* 如果收到从机的应答，则这个标志会设为1 */
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Send06H
*	功能说明: 发送06H指令，写1个保持寄存器
*	形    参: _addr : 从站地址
*			  _reg : 寄存器编号
*			  _value : 寄存器值,2字节
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Send06H(uint8_t _addr, uint16_t _reg, uint16_t _value)
{
	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;		/* 从站地址 */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x06;		/* 功能码 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8;	/* 寄存器编号 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;		/* 寄存器编号 低字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _value >> 8; /* 寄存器值 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _value;		/* 寄存器值 低字节 */

	MODH_SendAckWithCRC(); /* 发送数据，自动加CRC */

	g_tModH.fAck06H = 0; /* 如果收到从机的应答，则这个标志会设为1 */
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Send10H
*	功能说明: 发送10H指令，连续写多个保持寄存器. 最多一次支持23个寄存器。
*	形    参: _addr : 从站地址
*			  _reg : 寄存器编号
*			  _num : 寄存器个数n (每个寄存器2个字节) 值域
*			  _buf : n个寄存器的数据。长度 = 2 * n
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Send10H(uint8_t _addr, uint16_t _reg, uint8_t _num, uint8_t *_buf)
{
	uint16_t i;

	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;	  /* 从站地址 */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x10;	  /* 从站地址 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8; /* 寄存器编号 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;	  /* 寄存器编号 低字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num >> 8; /* 寄存器个数 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num;	  /* 寄存器个数 低字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = 2 * _num;  /* 数据字节数 */

	for (i = 0; i < 2 * _num; i++)
	{
		if (g_tModH.TxCount > H_RX_BUF_SIZE - 3)
		{
			return; /* 数据超过缓冲区超度，直接丢弃不发送 */
		}
		g_tModH.TxBuf[g_tModH.TxCount++] = _buf[i]; /* 后面的数据长度 */
	}

	MODH_SendAckWithCRC(); /* 发送数据，自动加CRC */
}

/*
*********************************************************************************************************
*	函 数 名: MODH_ReciveNew
*	功能说明: 串口接收中断服务程序会调用本函数。当收到一个字节时，执行一次本函数。
*	形    参:
*	返 回 值: 1 表示有数据
*********************************************************************************************************
*/
void MODH_ReciveNew(uint8_t _data)
{
	uint8_t i;
	g_modh_timeout = 0;

	/* 根据波特率，获取需要延迟的时间 */
	for (i = 0; i < (sizeof(ModbusBaudRate) / sizeof(ModbusBaudRate[0])); i++)
	{
		if (HBAUD485 == ModbusBaudRate[i].Bps)
		{
			break;
		}
	}

	/* 硬件定时中断，硬件定时器1用于MODBUS从机, 定时器2用于MODBUS主机*/
	bsp_HardTimer_Start(2, ModbusBaudRate[i].usTimeOut, (void *)MODH_RxTimeOut);

	if (g_tModH.RxCount < H_RX_BUF_SIZE)
	{
		g_tModH.RxBuf[g_tModH.RxCount++] = _data;
	}
}

/*
*********************************************************************************************************
*	函 数 名: MODH_RxTimeOut
*	功能说明: 超过3.5个字符时间后执行本函数。 设置全局变量 g_rtu_timeout = 1; 通知主程序开始解码。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODH_RxTimeOut(void)
{
	g_modh_timeout = 1;
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Poll
*	功能说明: 接收控制器指令. 1ms 响应时间。
*	形    参: 无
*	返 回 值: 0 表示无数据 1表示收到正确命令
*********************************************************************************************************
*/
void MODH_Poll(void)
{
	uint16_t crc1;

	if (g_modh_timeout == 0) /* 超过3.5个字符时间后执行MODH_RxTimeOut()函数。全局变量 g_rtu_timeout = 1 */
	{
		/* 没有超时，继续接收。不要清零 g_tModH.RxCount */
		return;
	}

	/* 收到命令
		05 06 00 88 04 57 3B70 (8 字节)
			05    :  数码管屏的号站，
			06    :  指令
			00 88 :  数码管屏的显示寄存器
			04 57 :  数据,,,转换成 10 进制是 1111.高位在前,
			3B70  :  二个字节 CRC 码	从05到 57的校验
	*/
	g_modh_timeout = 0;

	if (g_tModH.RxCount < 4)
	{
		goto err_ret;
	}

	/* 计算CRC校验和 */
	crc1 = CRC16_Modbus(g_tModH.RxBuf, g_tModH.RxCount);
	if (crc1 != 0)
	{
		goto err_ret;
	}

	/* 分析应用层协议 */
	MODH_Analyze();

err_ret:
#if 0 /* 此部分为了串口打印结果,实际运用中可不要 */
	g_tPrint.Rxlen = g_tModH.RxCount;
	memcpy(g_tPrint.RxBuf, g_tModH.RxBuf, g_tModH.RxCount);
#endif

	g_tModH.RxCount = 0; /* 必须清零计数器，方便下次帧同步 */
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Read_01H
*	功能说明: 分析01H指令的应答数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODH_Read_01H(void)
{
	uint8_t bytes;
	uint8_t *p;

	if (g_tModH.RxCount > 0)
	{
		bytes = g_tModH.RxBuf[2]; /* 資料長度 字節數 */
		switch (g_tModH.Reg01H)
		{
		case REG_D01:
			switch (bytes)
			{
			case 1:
				p = &g_tModH.RxBuf[3];
				g_tVar.D01 = *p;

				bsp_Log_Info("D01 : %d", g_tVar.D01);
				g_tModH.fAck01H = 1;
				break;

			case 2:
				p = &g_tModH.RxBuf[3];
				g_tVar.D01 = BEBufToUint16(p);
				p += 2;
				g_tVar.D02 = BEBufToUint16(p);

				bsp_Log_Info("D02 : %d,D01 : %d", g_tVar.D02, g_tVar.D01);
				g_tModH.fAck01H = 1;
				break;

			case 3:
				p = &g_tModH.RxBuf[3];
				g_tVar.D01 = BEBufToUint16(p);
				p += 2;
				g_tVar.D02 = BEBufToUint16(p);
				p += 2;
				g_tVar.D03 = *p;

				bsp_Log_Info("D03 : %d,D02 : %d,D01 : %d", g_tVar.D02, g_tVar.D02, g_tVar.D01);
				g_tModH.fAck01H = 1;
				break;

			case 8:
				p = &g_tModH.RxBuf[3];
				g_tVar.D01 = BEBufToUint16(p);
				p += 2;
				g_tVar.D02 = BEBufToUint16(p);
				p += 2;
				g_tVar.D03 = BEBufToUint16(p);
				p += 2;
				g_tVar.D04 = BEBufToUint16(p);
				p += 2;

				g_tModH.fAck01H = 1;
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Read_02H
*	功能说明: 分析02H指令的应答数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODH_Read_02H(void)
{
	uint8_t bytes;
	uint8_t *p;

	if (g_tModH.RxCount > 0)
	{
		bytes = g_tModH.RxBuf[2]; /* 資料長度 字節數 */
		switch (g_tModH.Reg02H)
		{
		case REG_T01:
			if (bytes == 6)
			{
				p = &g_tModH.RxBuf[3];

				g_tVar.T01 = BEBufToUint16(p);
				p += 2;
				g_tVar.T02 = BEBufToUint16(p);
				p += 2;
				g_tVar.T03 = BEBufToUint16(p);
				p += 2;

				g_tModH.fAck02H = 1;
			}
			break;
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Read_04H
*	功能说明: 分析04H指令的应答数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODH_Read_04H(void)
{
	uint8_t bytes;
	uint8_t *p;

	if (g_tModH.RxCount > 0)
	{
		bytes = g_tModH.RxBuf[2]; /* 資料長度 字節數 */
		switch (g_tModH.Reg04H)
		{
		case REG_T01:
			if (bytes == 2)
			{
				p = &g_tModH.RxBuf[3];

				g_tVar.A01 = BEBufToUint16(p);
				p += 2;

				g_tModH.fAck04H = 1;
			}
			break;
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Read_05H
*	功能说明: 分析05H指令的应答数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODH_Read_05H(void)
{
	if (g_tModH.RxCount > 0)
	{
		if (g_tModH.RxBuf[0] == SlaveAddr)
		{
			g_tModH.fAck05H = 1; /* 接收到应答 */
		}
	};
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Read_06H
*	功能说明: 分析06H指令的应答数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODH_Read_06H(void)
{
	if (g_tModH.RxCount > 0)
	{
		if (g_tModH.RxBuf[0] == SlaveAddr)
		{
			g_tModH.fAck06H = 1; /* 接收到应答 */
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Read_03H
*	功能说明: 分析03H指令的应答数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Read_03H(void)
{
	uint8_t bytes;
	uint8_t *p;

	if (g_tModH.RxCount > 0)
	{
		bytes = g_tModH.RxBuf[2]; /* 資料長度 字節數 */
		switch (g_tModH.Reg03H)
		{
		case REG_P01:
			if (bytes == 32)
			{
				p = &g_tModH.RxBuf[3];

				g_tVar.P01 = BEBufToUint16(p);
				p += 2;
				g_tVar.P02 = BEBufToUint16(p);
				p += 2;

				g_tModH.fAck03H = 1;
			}
			break;
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Read_10H
*	功能说明: 分析10H指令的应答数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Read_10H(void)
{
	/*
		10H指令的应答:
			从机地址                11
			功能码                  10
			寄存器起始地址高字节	00
			寄存器起始地址低字节    01
			寄存器数量高字节        00
			寄存器数量低字节        02
			CRC校验高字节           12
			CRC校验低字节           98
	*/
	if (g_tModH.RxCount > 0)
	{
		if (g_tModH.RxBuf[0] == SlaveAddr)
		{
			g_tModH.fAck10H = 1; /* 接收到应答 */
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: MODH_ReadParam_01H
*	功能说明: 单个参数. 通过发送01H指令实现，发送之后，等待从机应答。
*	形    参: 无
*	返 回 值: 1 表示成功。0 表示失败（通信超时或被拒绝）
*********************************************************************************************************
*/
uint8_t MODH_ReadParam_01H(uint16_t _reg, uint16_t _num)
{
	int32_t time1;
	uint8_t i;

	for (i = 0; i < NUM; i++)
	{
		MODH_Send01H(SlaveAddr, _reg, _num); /* 发送命令 */
		time1 = bsp_GetRunTime();			 /* 记录命令发送的时刻 */

		while (1) /* 等待应答,超时或接收到应答则break  */
		{

			if (bsp_CheckRunTime(time1) > TIMEOUT)
			{
				break; /* 通信超时了 */
			}

			if (g_tModH.fAck01H > 0)
			{
				break; /* 接收到应答 */
			}
		}

		if (g_tModH.fAck01H > 0)
		{
			break; /* 循环NUM次，如果接收到命令则break循环 */
		}
	}

	if (g_tModH.fAck01H == 0)
	{
		return 0;
	}
	else
	{
		return 1; /* 01H 读成功 */
	}
}

/*
*********************************************************************************************************
*	函 数 名: MODH_ReadParam_02H
*	功能说明: 单个参数. 通过发送02H指令实现，发送之后，等待从机应答。
*	形    参: 无
*	返 回 值: 1 表示成功。0 表示失败（通信超时或被拒绝）
*********************************************************************************************************
*/
uint8_t MODH_ReadParam_02H(uint16_t _reg, uint16_t _num)
{
	int32_t time1;
	uint8_t i;

	for (i = 0; i < NUM; i++)
	{
		MODH_Send02H(SlaveAddr, _reg, _num);
		time1 = bsp_GetRunTime(); /* 记录命令发送的时刻 */

		while (1)
		{

			if (bsp_CheckRunTime(time1) > TIMEOUT)
			{
				break; /* 通信超时了 */
			}

			if (g_tModH.fAck02H > 0)
			{
				break;
			}
		}

		if (g_tModH.fAck02H > 0)
		{
			break;
		}
	}

	if (g_tModH.fAck02H == 0)
	{
		return 0;
	}
	else
	{
		return 1; /* 02H 读成功 */
	}
}
/*
*********************************************************************************************************
*	函 数 名: MODH_ReadParam_03H
*	功能说明: 单个参数. 通过发送03H指令实现，发送之后，等待从机应答。
*	形    参: 无
*	返 回 值: 1 表示成功。0 表示失败（通信超时或被拒绝）
*********************************************************************************************************
*/
uint8_t MODH_ReadParam_03H(uint16_t _reg, uint16_t _num)
{
	int32_t time1;
	uint8_t i;

	for (i = 0; i < NUM; i++)
	{
		MODH_Send03H(SlaveAddr, _reg, _num);
		time1 = bsp_GetRunTime(); /* 记录命令发送的时刻 */

		while (1)
		{

			if (bsp_CheckRunTime(time1) > TIMEOUT)
			{
				break; /* 通信超时了 */
			}

			if (g_tModH.fAck03H > 0)
			{
				break;
			}
		}

		if (g_tModH.fAck03H > 0)
		{
			break;
		}
	}

	if (g_tModH.fAck03H == 0)
	{
		return 0; /* 通信超时了 */
	}
	else
	{
		return 1; /* 写入03H参数成功 */
	}
}

/*
*********************************************************************************************************
*	函 数 名: MODH_ReadParam_04H
*	功能说明: 单个参数. 通过发送04H指令实现，发送之后，等待从机应答。
*	形    参: 无
*	返 回 值: 1 表示成功。0 表示失败（通信超时或被拒绝）
*********************************************************************************************************
*/
uint8_t MODH_ReadParam_04H(uint16_t _reg, uint16_t _num)
{
	int32_t time1;
	uint8_t i;

	for (i = 0; i < NUM; i++)
	{
		MODH_Send04H(SlaveAddr, _reg, _num);
		time1 = bsp_GetRunTime(); /* 记录命令发送的时刻 */

		while (1)
		{

			if (bsp_CheckRunTime(time1) > TIMEOUT)
			{
				break; /* 通信超时了 */
			}

			if (g_tModH.fAck04H > 0)
			{
				break;
			}
		}

		if (g_tModH.fAck04H > 0)
		{
			break;
		}
	}

	if (g_tModH.fAck04H == 0)
	{
		return 0; /* 通信超时了 */
	}
	else
	{
		return 1; /* 04H 读成功 */
	}
}
/*
*********************************************************************************************************
*	函 数 名: MODH_WriteParam_05H
*	功能说明: 单个参数. 通过发送05H指令实现，发送之后，等待从机应答。
*	形    参: 无
*	返 回 值: 1 表示成功。0 表示失败（通信超时或被拒绝）
*********************************************************************************************************
*/
uint8_t MODH_WriteParam_05H(uint16_t _reg, uint16_t _value)
{
	int32_t time1;
	uint8_t i;

	for (i = 0; i < NUM; i++)
	{
		MODH_Send05H(SlaveAddr, _reg, _value);
		time1 = bsp_GetRunTime(); /* 记录命令发送的时刻 */

		while (1)
		{

			/* 超时大于 TIMEOUT，则认为异常 */
			if (bsp_CheckRunTime(time1) > TIMEOUT)
			{
				break; /* 通信超时了 */
			}

			if (g_tModH.fAck05H > 0)
			{
				break;
			}
		}

		if (g_tModH.fAck05H > 0)
		{
			break;
		}
	}

	if (g_tModH.fAck05H == 0)
	{
		return 0; /* 通信超时了 */
	}
	else
	{
		return 1; /* 05H 写成功 */
	}
}

/*
*********************************************************************************************************
*	函 数 名: MODH_WriteParam_06H
*	功能说明: 单个参数. 通过发送06H指令实现，发送之后，等待从机应答。循环NUM次写命令
*	形    参: 无
*	返 回 值: 1 表示成功。0 表示失败（通信超时或被拒绝）
*********************************************************************************************************
*/
uint8_t MODH_WriteParam_06H(uint16_t _reg, uint16_t _value)
{
	__IO int32_t time1;
	uint8_t i;

	for (i = 0; i < NUM; i++)
	{
		MODH_Send06H(SlaveAddr, _reg, _value);
		time1 = bsp_GetRunTime(); /* 记录命令发送的时刻 */

		while (1)
		{

			if (bsp_CheckRunTime(time1) > TIMEOUT)
			{
				break;
			}

			if (g_tModH.fAck06H > 0)
			{
				break;
			}
		}

		if (g_tModH.fAck06H > 0)
		{
			break;
		}
	}

	if (g_tModH.fAck06H == 0)
	{
		return 0; /* 通信超时了 */
	}
	else
	{
		return 1; /* 写入06H参数成功 */
	}
}

/*
*********************************************************************************************************
*	函 数 名: MODH_WriteParam_10H
*	功能说明: 单个参数. 通过发送10H指令实现，发送之后，等待从机应答。循环NUM次写命令
*	形    参: 无
*	返 回 值: 1 表示成功。0 表示失败（通信超时或被拒绝）
*********************************************************************************************************
*/
uint8_t MODH_WriteParam_10H(uint16_t _reg, uint8_t _num, uint8_t *_buf)
{
	int32_t time1;
	uint8_t i;

	for (i = 0; i < NUM; i++)
	{
		MODH_Send10H(SlaveAddr, _reg, _num, _buf);
		time1 = bsp_GetRunTime(); /* 记录命令发送的时刻 */

		while (1)
		{

			if (bsp_CheckRunTime(time1) > TIMEOUT)
			{
				break;
			}

			if (g_tModH.fAck10H > 0)
			{
				break;
			}
		}

		if (g_tModH.fAck10H > 0)
		{
			break;
		}
	}

	if (g_tModH.fAck10H == 0)
	{
		return 0; /* 通信超时了 */
	}
	else
	{
		return 1; /* 写入10H参数成功 */
	}
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/