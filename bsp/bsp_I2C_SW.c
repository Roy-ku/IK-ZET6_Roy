/*
	應用說明：
	在訪問I2C設備前，請先調用 i2c_CheckDevice() 檢測I2C設備是否正常，該函數會配置GPIO
*/

#include "bsp_I2C_SW.h"
/*
		i2c總線GPIO:
		PB6/I2C1_SCL
		PB7/I2C1_SDA
*/

#define I2C_SCL_1() HAL_GPIO_WritePin(I2C_SW_SCL_GPIO_Port, I2C_SW_SCL_Pin, GPIO_PIN_SET)	/* SCL = 1 */
#define I2C_SCL_0() HAL_GPIO_WritePin(I2C_SW_SCL_GPIO_Port, I2C_SW_SCL_Pin, GPIO_PIN_RESET) /* SCL = 0 */

#define I2C_SDA_1() HAL_GPIO_WritePin(I2C_SW_SDA_GPIO_Port, I2C_SW_SDA_Pin, GPIO_PIN_SET)	/* SDA = 1 */
#define I2C_SDA_0() HAL_GPIO_WritePin(I2C_SW_SDA_GPIO_Port, I2C_SW_SDA_Pin, GPIO_PIN_RESET) /* SDA = 0 */

#define I2C_SCL_READ() HAL_GPIO_ReadPin(I2C_SW_SCL_GPIO_Port, I2C_SW_SCL_Pin) /* 讀SCL狀態 */
#define I2C_SDA_READ() HAL_GPIO_ReadPin(I2C_SW_SDA_GPIO_Port, I2C_SW_SDA_Pin) /* 讀SDA狀態 */

/*
*********************************************************************************************************
*	函 數 名: bsp_SoftI2C_init
*	功能說明: 配置I2C總線的GPIO，採用模擬IO的方式實現
*	形    參:  無
*	返 回 值: 無
*********************************************************************************************************
*/
void bsp_SoftI2C_init(void)
{
/* 已在CubeMX配置好 */
#if 0 
	GPIO_InitTypeDef gpio_init;

	__HAL_RCC_GPIOB_CLK_ENABLE();

	gpio_init.Mode = GPIO_MODE_OUTPUT_OD;  /* 开漏输出 */
	gpio_init.Pull = GPIO_NOPULL;
	gpio_init.Speed = GPIO_SPEED_FREQ_LOW; // GPIO_SPEED_FREQ_HIGH;

	gpio_init.Pin = I2C_SW_SCL_Pin;
	HAL_GPIO_Init(I2C_SW_SCL_GPIO_Port, &gpio_init);

	gpio_init.Pin = I2C_SW_SDA_Pin;
	HAL_GPIO_Init(I2C_SW_SDA_GPIO_Port, &gpio_init);
#endif

	/* 給一個停止信號, 復位I2C總線上的所有設備到待機模式 */
	i2c_Stop();
}

/*
*********************************************************************************************************
*	函 數 名: i2c_Delay
*	功能說明: I2C總線位延遲，最快400KHz
*	形    參:  無
*	返 回 值: 無
*********************************************************************************************************
*/
static void i2c_Delay(void)
{
	/*　
			CPU主頻168MHz時，在內部Flash運行, MDK工程不優化。用台式示波器觀測波形。
			循環次數為5時，SCL頻率 = 1.78MHz (讀耗時: 92ms, 讀寫正常，但是用示波器探頭碰上就讀寫失敗。時序接近臨界)
			循環次數為10時，SCL頻率 = 1.1MHz (讀耗時: 138ms, 讀速度: 118724B/s)
			循環次數為30時，SCL頻率 = 440KHz， SCL高電平時間1.0us，SCL低電平時間1.2us

			上拉電阻選擇2.2K歐時，SCL上升沿時間約0.5us，如果選4.7K歐，則上升沿約1us

			實際應用選擇400KHz左右的速率即可
		*/
	// for (i = 0; i < 30; i++);
	// for (i = 0; i < 60; i++);
	// bsp_DelayUS(2); 229.57KHz时钟
	bsp_DelayUS(10);
	// HAL_Delay(1);
}

/*
*********************************************************************************************************
*	函 數 名: i2c_Start
*	功能說明: CPU發起I2C總線啟動信號
*	形    參:  無
*	返 回 值: 無
*********************************************************************************************************
*/
void i2c_Start(void)
{
	/* 當SCL高電平時，SDA出現一個下跳沿表示I2C總線啟動信號 */
	I2C_SDA_1();
	I2C_SCL_1();
	i2c_Delay();
	I2C_SDA_0();
	i2c_Delay();
	I2C_SCL_0();
	i2c_Delay();
}

/*
*********************************************************************************************************
*	函 數 名: i2c_Start
*	功能說明: CPU發起I2C總線停止信號
*	形    參:  無
*	返 回 值: 無
*********************************************************************************************************
*/
void i2c_Stop(void)
{
	/* 當SCL高電平時，SDA出現一個上跳沿表示I2C總線停止信號 */
	I2C_SDA_0();
	i2c_Delay();
	I2C_SCL_1();
	i2c_Delay();
	I2C_SDA_1();
	i2c_Delay();
}

/*
*********************************************************************************************************
*	函 數 名: i2c_SendByte
*	功能說明: CPU向I2C總線設備發送8bit數據
*	形    參:  _ucByte ： 等待發送的字節
*	返 回 值: 無
*********************************************************************************************************
*/
void i2c_SendByte(uint8_t _ucByte)
{
	uint8_t i;

	/* 先發送字節的高位bit7 */
	for (i = 0; i < 8; i++)
	{
		if (_ucByte & 0x80)
		{
			I2C_SDA_1();
		}
		else
		{
			I2C_SDA_0();
		}
		i2c_Delay();
		I2C_SCL_1();
		i2c_Delay();
		I2C_SCL_0();
		if (i == 7)
		{
			I2C_SDA_1(); /* 釋放總線 */
		}
		_ucByte <<= 1; /* 左移一個bit */
	}
}

/*
*********************************************************************************************************
*	函 數 名: i2c_ReadByte
*	功能說明: CPU從I2C總線設備讀取8bit數據
*	形    參:  無
*	返 回 值: 讀到的數據
*********************************************************************************************************
*/
uint8_t i2c_ReadByte(void)
{
	uint8_t i;
	uint8_t value;

	/* 讀到第1個bit為數據的bit7 */
	value = 0;
	for (i = 0; i < 8; i++)
	{
		value <<= 1;
		I2C_SCL_1();
		i2c_Delay();
		if (I2C_SDA_READ())
		{
			value++;
		}
		I2C_SCL_0();
		i2c_Delay();
	}
	return value;
}

/*
*********************************************************************************************************
*	函 數 名: i2c_WaitAck
*	功能說明: CPU產生一個時鐘，並讀取器件的ACK應答信號
*	形    參:  無
*	返 回 值: 返回0表示正確應答，1表示無器件響應
*********************************************************************************************************
*/
uint8_t i2c_WaitAck(void)
{
	uint8_t re;

	I2C_SDA_1(); /* CPU釋放SDA總線 */
	i2c_Delay();
	I2C_SCL_1(); /* CPU驅動SCL = 1, 此時器件會返回ACK應答 */
	i2c_Delay();
	if (I2C_SDA_READ()) /* CPU讀取SDA口線狀態 */
	{
		re = 1;
	}
	else
	{
		re = 0;
	}
	I2C_SCL_0();
	i2c_Delay();
	return re;
}

/*
*********************************************************************************************************
*	函 數 名: i2c_Ack
*	功能說明: CPU產生一個ACK信號
*	形    參:  無
*	返 回 值: 無
*********************************************************************************************************
*/
void i2c_Ack(void)
{

	I2C_SDA_0(); /* CPU驅動SDA = 0 */
	i2c_Delay();
	I2C_SCL_1(); /* CPU產生1個時鐘 */
	i2c_Delay();
	I2C_SCL_0();
	i2c_Delay();
	I2C_SDA_1(); /* CPU釋放SDA總線 */
	i2c_Delay();
}

/*
*********************************************************************************************************
*	函 數 名: i2c_NAck
*	功能說明: CPU產生1個NACK信號
*	形    參:  無
*	返 回 值: 無
*********************************************************************************************************
*/
void i2c_NAck(void)
{
	I2C_SDA_1(); /* CPU驅動SDA = 1 */
	i2c_Delay();
	I2C_SCL_1(); /* CPU產生1個時鐘 */
	i2c_Delay();
	I2C_SCL_0();
	i2c_Delay();
}

/*
*********************************************************************************************************
*	函 數 名: i2c_CheckDevice
*	功能說明: 檢測I2C總線設備，CPU向發送設備地址，然後讀取設備應答來判斷該設備是否存在
*	形    參:  _Address：設備的I2C總線地址
*	返 回 值: 返回值 0 表示正確， 返回1表示未探測到
*********************************************************************************************************
*/
uint8_t i2c_CheckDevice(uint8_t _Address)
{
	uint8_t ucAck;

	if (I2C_SDA_READ() && I2C_SCL_READ())
	{
		i2c_Start(); /* 發送啟動信號 */

		/* 發送設備地址+讀寫控制bit（0 = w， 1 = r) bit7 先傳 */
		i2c_SendByte(_Address | I2C_WR);
		ucAck = i2c_WaitAck(); /* 檢測設備的ACK應答 */

		i2c_Stop(); /* 發送停止信號 */

		return ucAck;
	}
	return 1; /* I2C總線異常 */
}

/***************************** (END OF FILE) *********************************/
