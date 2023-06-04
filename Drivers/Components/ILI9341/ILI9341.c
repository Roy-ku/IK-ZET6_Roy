#include "ILI9341.h"


/*************************************** Debug  **********************************/
#define DEBUG_DELAY()
/*********************************************************************************/
//根据液晶扫描方向而变化的XY像素宽度
//调用ILI9341_GramScan函数设置方向时会自动更改
#define ILI9341_LCD_PIXEL_WIDTH ((uint16_t)240)
#define ILI9341_LCD_PIXEL_HEIGHT ((uint16_t)320)
uint16_t LCD_X_LENGTH = ILI9341_LCD_PIXEL_WIDTH;
uint16_t LCD_Y_LENGTH = ILI9341_LCD_PIXEL_HEIGHT;
/*********************************************************************************/
LCD_DrvTypeDef   ili9341_drv =
{
  .LCDID = 0,
  ILI9341_REG_Config,
  ILI9341_RESET,
  ILI9341_ReadID,
  ILI9341_GramScan,
  ILI9341_DisplayOn,
  ILI9341_DisplayOff,
  ILI9341_SetCursor,
  ILI9341_SetDisplayWindow,
  ILI9341_GetLcdPixelWidth,
  ILI9341_GetLcdPixelHeight,
};
/*********************************************************************************/
static void ILI9341_Delay(__IO uint32_t nCount);
/*********************************************************************************/
void ILI9341_RESET(void)
{
  LCD_RESETPIN_L;
  ILI9341_Delay(0xAFF);

  LCD_RESETPIN_H;
  ILI9341_Delay(0xAFF);
}

uint16_t ILI9341_ReadID(void)
{
  uint16_t id = 0;

  bsp_LCD_Write_Cmd(0xD3);
  bsp_LCD_Read_Data();
  bsp_LCD_Read_Data();
  id = bsp_LCD_Read_Data();
  id <<= 8;
  id |= bsp_LCD_Read_Data();
  if (id == LCDID_ILI9341)
  {
    return id;
  }

  return LCDID_UNKNOWN;
}

/**
 * @brief  设置ILI9341的GRAM的扫描方向
 * @param  ucOption ：选择GRAM的扫描方向
 *     @arg 0-7 :参数可选值为0-7这八个方向
 *
 *	！！！其中0、3、5、6 模式适合从左至右显示文字，
 *				不推荐使用其它模式显示文字	其它模式显示文字会有镜像效果
 *
 *	其中0、2、4、6 模式的X方向像素为240，Y方向像素为320
 *	其中1、3、5、7 模式下X方向像素为320，Y方向像素为240
 *
 *	其中 6 模式为大部分液晶例程的默认显示方向
 *	其中 3 模式为摄像头例程使用的方向
 *	其中 0 模式为BMP图片显示例程使用的方向
 *
 * @retval 无
 * @note  坐标图例：A表示向上，V表示向下，<表示向左，>表示向右
          X表示X轴，Y表示Y轴

------------------------------------------------------------
模式0：				.		模式1：		.	模式2：			.	模式3：
          A		.					A		.		A					.		A
          |		.					|		.		|					.		|
          Y		.					X		.		Y					.		X
          0		.					1		.		2					.		3
  <--- X0 o		.	<----Y1	o		.		o 2X--->  .		o 3Y--->
------------------------------------------------------------
模式4：				.	模式5：			.	模式6：			.	模式7：
  <--- X4 o		.	<--- Y5 o		.		o 6X--->  .		o 7Y--->
          4		.					5		.		6					.		7
          Y		.					X		.		Y					.		X
          |		.					|		.		|					.		|
          V		.					V		.		V					.		V
---------------------------------------------------------
                       LCD屏示例
                |-----------------|
                |			野火Logo		|
                |									|
                |									|
                |									|
                |									|
                |									|
                |									|
                |									|
                |									|
                |-----------------|
                屏幕正面（宽240，高320）

 *******************************************************/
void ILI9341_GramScan(uint8_t ucOption)
{
  //参数检查，只可输入0-7
  if (ucOption > 7)
    return;

  //根据模式更新LCD_SCAN_MODE的值，主要用于触摸屏选择计算参数
  //LCD_SCAN_MODE = ucOption;

  //根据模式更新XY方向的像素宽度
  if (ucOption % 2 == 0)
  {
    // 0 2 4 6模式下X方向像素宽度为240，Y方向为320
    LCD_X_LENGTH = ILI9341_LCD_PIXEL_WIDTH;
    LCD_Y_LENGTH = ILI9341_LCD_PIXEL_HEIGHT;
  }
  else
  {
    // 1 3 5 7模式下X方向像素宽度为320，Y方向为240
    LCD_X_LENGTH = ILI9341_LCD_PIXEL_HEIGHT;
    LCD_Y_LENGTH = ILI9341_LCD_PIXEL_WIDTH;
  }

  // 0x36命令参数的高3位可用于设置GRAM扫描方向
  bsp_LCD_Write_Cmd(0x36);

  bsp_LCD_Write_Data(0x08 | (ucOption << 5)); //根据ucOption的值设置LCD参数，共0-7种模式
  bsp_LCD_Write_Cmd(CMD_SetCoordinateX);
  bsp_LCD_Write_Data(0x00);                             /* x 起始坐标高8位 */
  bsp_LCD_Write_Data(0x00);                             /* x 起始坐标低8位 */
  bsp_LCD_Write_Data(((LCD_X_LENGTH - 1) >> 8) & 0xFF); /* x 结束坐标高8位 */
  bsp_LCD_Write_Data((LCD_X_LENGTH - 1) & 0xFF);        /* x 结束坐标低8位 */

  bsp_LCD_Write_Cmd(CMD_SetCoordinateY);
  bsp_LCD_Write_Data(0x00);                             /* y 起始坐标高8位 */
  bsp_LCD_Write_Data(0x00);                             /* y 起始坐标低8位 */
  bsp_LCD_Write_Data(((LCD_Y_LENGTH - 1) >> 8) & 0xFF); /* y 结束坐标高8位 */
  bsp_LCD_Write_Data((LCD_Y_LENGTH - 1) & 0xFF);        /* y 结束坐标低8位 */

  /* write gram start */
  bsp_LCD_Write_Cmd(CMD_SetPixel);
}

void ILI9341_DisplayOn(void)
{
  bsp_LCD_Backlight_Control(ENABLE);
}

void ILI9341_DisplayOff(void)
{
  bsp_LCD_Backlight_Control(DISABLE);
}

/**
 * @brief  在ILI9341显示器上开辟一个窗口
 * @param  usX ：在特定扫描方向下窗口的起点X坐标
 * @param  usY ：在特定扫描方向下窗口的起点Y坐标
 * @param  usWidth ：窗口的宽度
 * @param  usHeight ：窗口的高度
 * @retval 无
 */
void ILI9341_SetDisplayWindow(uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight)
{
  bsp_LCD_Write_Cmd(CMD_SetCoordinateX); /* 设置X坐标 */
  bsp_LCD_Write_Data(usX >> 8);          /* 先高8位，然后低8位 */
  bsp_LCD_Write_Data(usX & 0xff);        /* 设置起始点和结束点*/
  bsp_LCD_Write_Data((usX + usWidth - 1) >> 8);
  bsp_LCD_Write_Data((usX + usWidth - 1) & 0xff);

  bsp_LCD_Write_Cmd(CMD_SetCoordinateY); /* 设置Y坐标*/
  bsp_LCD_Write_Data(usY >> 8);
  bsp_LCD_Write_Data(usY & 0xff);
  bsp_LCD_Write_Data((usY + usHeight - 1) >> 8);
  bsp_LCD_Write_Data((usY + usHeight - 1) & 0xff);
}

/**
 * @brief 設定ILI9341的光標坐標
 * @param usX 在特定掃描方向下光標的X坐標
 * @param usY 在特定掃描方向下光標的Y坐標
 */
void ILI9341_SetCursor(uint16_t usX, uint16_t usY)
{
  ILI9341_SetDisplayWindow(usX, usY, 1, 1);
}

/**
 * @brief  初始化ILI9341寄存器
 * @param  无
 * @retval 无
 */
void ILI9341_REG_Config(void)
{

    /*  Power control B (CFh)  */
    DEBUG_DELAY();
    bsp_LCD_Write_Cmd(0xCF);
    bsp_LCD_Write_Data(0x00);
    bsp_LCD_Write_Data(0x81);
    bsp_LCD_Write_Data(0x30);

    /*  Power on sequence control (EDh) */
    DEBUG_DELAY();
    bsp_LCD_Write_Cmd(0xED);
    bsp_LCD_Write_Data(0x64);
    bsp_LCD_Write_Data(0x03);
    bsp_LCD_Write_Data(0x12);
    bsp_LCD_Write_Data(0x81);

    /*  Driver timing control A (E8h) */
    DEBUG_DELAY();
    bsp_LCD_Write_Cmd(0xE8);
    bsp_LCD_Write_Data(0x85);
    bsp_LCD_Write_Data(0x10);
    bsp_LCD_Write_Data(0x78);

    /*  Power control A (CBh) */
    DEBUG_DELAY();
    bsp_LCD_Write_Cmd(0xCB);
    bsp_LCD_Write_Data(0x39);
    bsp_LCD_Write_Data(0x2C);
    bsp_LCD_Write_Data(0x00);
    bsp_LCD_Write_Data(0x34);
    bsp_LCD_Write_Data(0x02);

    /* Pump ratio control (F7h) */
    DEBUG_DELAY();
    bsp_LCD_Write_Cmd(0xF7);
    bsp_LCD_Write_Data(0x20);

    /* Driver timing control B */
    DEBUG_DELAY();
    bsp_LCD_Write_Cmd(0xEA);
    bsp_LCD_Write_Data(0x00);
    bsp_LCD_Write_Data(0x00);

    /* Frame Rate Control (In Normal Mode/Full Colors) (B1h) */
    DEBUG_DELAY();
    bsp_LCD_Write_Cmd(0xB1);
    bsp_LCD_Write_Data(0x00);
    bsp_LCD_Write_Data(0x1B);

    /*  Display Function Control (B6h) */
    DEBUG_DELAY();
    bsp_LCD_Write_Cmd(0xB6);
    bsp_LCD_Write_Data(0x0A);
    bsp_LCD_Write_Data(0xA2);

    /* Power Control 1 (C0h) */
    DEBUG_DELAY();
    bsp_LCD_Write_Cmd(0xC0);
    bsp_LCD_Write_Data(0x35);

    /* Power Control 2 (C1h) */
    DEBUG_DELAY();
    bsp_LCD_Write_Cmd(0xC1);
    bsp_LCD_Write_Data(0x11);

    /* VCOM Control 1(C5h) */
    bsp_LCD_Write_Cmd(0xC5);
    bsp_LCD_Write_Data(0x45);
    bsp_LCD_Write_Data(0x45);

    /*  VCOM Control 2(C7h)  */
    bsp_LCD_Write_Cmd(0xC7);
    bsp_LCD_Write_Data(0xA2);

    /* Enable 3G (F2h) */
    bsp_LCD_Write_Cmd(0xF2);
    bsp_LCD_Write_Data(0x00);

    /* Gamma Set (26h) */
    bsp_LCD_Write_Cmd(0x26);
    bsp_LCD_Write_Data(0x01);
    DEBUG_DELAY();

    /* Positive Gamma Correction */
    bsp_LCD_Write_Cmd(0xE0); // Set Gamma
    bsp_LCD_Write_Data(0x0F);
    bsp_LCD_Write_Data(0x26);
    bsp_LCD_Write_Data(0x24);
    bsp_LCD_Write_Data(0x0B);
    bsp_LCD_Write_Data(0x0E);
    bsp_LCD_Write_Data(0x09);
    bsp_LCD_Write_Data(0x54);
    bsp_LCD_Write_Data(0xA8);
    bsp_LCD_Write_Data(0x46);
    bsp_LCD_Write_Data(0x0C);
    bsp_LCD_Write_Data(0x17);
    bsp_LCD_Write_Data(0x09);
    bsp_LCD_Write_Data(0x0F);
    bsp_LCD_Write_Data(0x07);
    bsp_LCD_Write_Data(0x00);

    /* Negative Gamma Correction (E1h) */
    bsp_LCD_Write_Cmd(0XE1); // Set Gamma
    bsp_LCD_Write_Data(0x00);
    bsp_LCD_Write_Data(0x19);
    bsp_LCD_Write_Data(0x1B);
    bsp_LCD_Write_Data(0x04);
    bsp_LCD_Write_Data(0x10);
    bsp_LCD_Write_Data(0x07);
    bsp_LCD_Write_Data(0x2A);
    bsp_LCD_Write_Data(0x47);
    bsp_LCD_Write_Data(0x39);
    bsp_LCD_Write_Data(0x03);
    bsp_LCD_Write_Data(0x06);
    bsp_LCD_Write_Data(0x06);
    bsp_LCD_Write_Data(0x30);
    bsp_LCD_Write_Data(0x38);
    bsp_LCD_Write_Data(0x0F);

    /* memory access control set */
    DEBUG_DELAY();
    bsp_LCD_Write_Cmd(0x36);
    bsp_LCD_Write_Data(0xC8); /*竖屏  左上角到(起点)到右下角(终点)扫描方式*/
    DEBUG_DELAY();

    /* column address control set */
    bsp_LCD_Write_Cmd(0X2A);
    bsp_LCD_Write_Data(0x00);
    bsp_LCD_Write_Data(0x00);
    bsp_LCD_Write_Data(0x00);
    bsp_LCD_Write_Data(0xEF);

    /* page address control set */
    DEBUG_DELAY();
    bsp_LCD_Write_Cmd(0X2B);
    bsp_LCD_Write_Data(0x00);
    bsp_LCD_Write_Data(0x00);
    bsp_LCD_Write_Data(0x01);
    bsp_LCD_Write_Data(0x3F);

    /*  Pixel Format Set (3Ah)  */
    DEBUG_DELAY();
    bsp_LCD_Write_Cmd(0x3a);
    bsp_LCD_Write_Data(0x55);

    /* Sleep Out (11h)  */
    bsp_LCD_Write_Cmd(0x11);

    ILI9341_Delay(0xFFFF);
    
    /* Display ON (29h) */
    bsp_LCD_Write_Cmd(0x29);

}

uint16_t ILI9341_GetLcdPixelWidth(void)
{
    /* Return LCD PIXEL WIDTH */
    return LCD_X_LENGTH;
}


uint16_t ILI9341_GetLcdPixelHeight(void)
{
    /* Return LCD PIXEL HEIGHT */
    return LCD_Y_LENGTH;
}


static void ILI9341_Delay(__IO uint32_t nCount)
{
  for (; nCount != 0; nCount--)
    ;
}
