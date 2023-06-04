#include "bsp_lcd.h"

extern SPIFLASH_DrvTypeDef *SPIFLASHDrv;
LCD_DrvTypeDef *LCDDrv;

//液晶屏掃描模式，本變量主要用於方便選擇觸摸屏的計算參數
//參數可選值為0-7
//調用ILI9341_GramScan函數設置方向時會自動更改
// LCD剛初始化完成時會使用本默認值
uint8_t LCD_SCAN_MODE = 6;

static sFONT *LCD_Currentfonts = &Font8x16; //英文字體
static uint16_t CurrentTextColor = WHITE;   //前景色
static uint16_t CurrentBackColor = BLACK;   //背景色

static __inline void bsp_LCD_FillColor(uint32_t ulAmout_Point, uint16_t usColor);
static uint16_t bsp_LCD_Read_PixelData(void);

/**
 * @brief  向ILI9341寫入命令
 * @param  usCmd :要寫入的命令（表寄存器地址）
 * @retval 無
 */
__inline void bsp_LCD_Write_Cmd(uint16_t usCmd)
{
  *(__IO uint16_t *)(FSMC_Addr_ILI9341_CMD) = usCmd;
}

/**
 * @brief  向ILI9341寫入數據
 * @param  usData :要寫入的數據
 * @retval 無
 */
__inline void bsp_LCD_Write_Data(uint16_t usData)
{
  *(__IO uint16_t *)(FSMC_Addr_ILI9341_DATA) = usData;
}

/**
 * @brief  從ILI9341讀取數據
 * @param  無
 * @retval 讀取到的數據
 */
__inline uint16_t bsp_LCD_Read_Data(void)
{
  return (*(__IO uint16_t *)(FSMC_Addr_ILI9341_DATA));
}

/**
 * @brief  ILI9341初始化函數，如果要用到lcd，一定要調用這個函數
 * @param  無
 * @retval 無
 */
void bsp_LCD_Init(void)
{
  LCDDrv = &ili9341_drv;
  LCDDrv->RESET();
  LCDDrv->LCDID = LCDDrv->ReadID();
  bsp_Log_Info("\r\nLCD info\r\nChipID : 0x%x\r\n", LCDDrv->LCDID);
  LCDDrv->Init();
  LCDDrv->GramScan(LCD_SCAN_MODE);
  HAL_Delay(20);
  bsp_LCD_Show_Logo();
  HAL_Delay(60);
  LCDDrv->DisplayOn();
}

/**
 * @brief LCD背光控制
 * @param enumState ENABLE :背光ON,DISABLE :背光OFF
 */
void bsp_LCD_Backlight_Control(FunctionalState enumState)
{
  if (enumState)
  {
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_RESET);
  }
  else
  {
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);
  }
}

/**
 * @brief  在ILI9341顯示器上以某一顏色填充像素點
 * @param  ulAmout_Point ：要填充顏色的像素點的總數目
 * @param  usColor ：顏色
 * @retval 無
 */
static __inline void bsp_LCD_FillColor(uint32_t ulAmout_Point, uint16_t usColor)
{
  uint32_t i = 0;

  /* memory write */
  bsp_LCD_Write_Cmd(CMD_SetPixel);

  for (i = 0; i < ulAmout_Point; i++)
    bsp_LCD_Write_Data(usColor);
}

void bsp_LCD_FlushColor(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color)
{
  uint16_t height, width;
  uint16_t i, j;
  width = ex - sx + 1;  //得到填充的宽度
  height = ey - sy + 1; //高度
  LCDDrv->SetDisplayWindow(sx, sy, width, height);
  bsp_LCD_Write_Cmd(CMD_SetPixel);
  for (i = 0; i < height; i++)
  {
    for (j = 0; j < width; j++)
    {
      bsp_LCD_Write_Data(*color);
    }
  }
}

void bsp_LCD_show_Picture_at_SPIFlash(uint32_t addr, uint32_t size)
{
  __IO uint16_t package;
  __IO uint16_t color;
  package = size / SPIFlash_BUF_SIZE;
  LCDDrv->SetDisplayWindow(0, 0, LCDDrv->GetLcdPixelWidth(), LCDDrv->GetLcdPixelHeight());
  /* memory write */
  bsp_LCD_Write_Cmd(CMD_SetPixel);
  for (uint16_t count = 0; count < package; count++)
  {
    SPIFLASHDrv->BufferRead(SPIFlash_buf, addr, SPIFlash_BUF_SIZE);
    for (uint16_t _count = 0; _count + 1 < (SPIFlash_BUF_SIZE); _count += 2)
    {
      color = (((uint16_t)(SPIFlash_buf[_count + 1] << 8) & 0xFF00) | (SPIFlash_buf[_count]) & 0xFF);
      bsp_LCD_Write_Data(color);
    }
    addr += SPIFlash_BUF_SIZE;
  }
}

/**
 * @brief  對ILI9341顯示器的某一窗口以某種顏色進行清屏
 * @param  usX ：在特定掃描方向下窗口的起點X坐標
 * @param  usY ：在特定掃描方向下窗口的起點Y坐標
 * @param  usWidth ：窗口的寬度
 * @param  usHeight ：窗口的高度
 * @note 可使用bsp_LCD_SetBackColor、bsp_LCD_SetTextColor、bsp_LCD_SetColors函數設置顏色
 * @retval 無
 */
void bsp_LCD_Clear(uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight)
{
  LCDDrv->SetDisplayWindow(usX, usY, usWidth, usHeight);

  bsp_LCD_FillColor(usWidth * usHeight, CurrentBackColor);
}

/**
 * @brief  對ILI9341顯示器的某一點以某種顏色進行填充
 * @param  usX ：在特定掃描方向下該點的X坐標
 * @param  usY ：在特定掃描方向下該點的Y坐標
 * @note 可使用bsp_LCD_SetBackColor、bsp_LCD_SetTextColor、bsp_LCD_SetColors函數設置顏色
 * @retval 無
 */
void bsp_LCD_SetPointPixel(uint16_t usX, uint16_t usY)
{
  if ((usX < LCDDrv->GetLcdPixelWidth()) && (usY < LCDDrv->GetLcdPixelHeight()))
  {
    LCDDrv->SetCursor(usX, usY);

    bsp_LCD_FillColor(1, CurrentTextColor);
  }
}

/**
 * @brief  讀取 GRAM 的一個像素數據
 * @param  無
 * @retval 像素數據
 */
static uint16_t bsp_LCD_Read_PixelData(void)
{
  uint16_t usRG = 0, usB = 0;

  bsp_LCD_Write_Cmd(0x2E); /* 讀數據 */
  //去掉前一次讀取結果
  bsp_LCD_Read_Data(); /*FIRST READ OUT DUMMY DATA*/

  //獲取紅色通道與綠色通道的值
  usRG = bsp_LCD_Read_Data(); /*READ OUT RED AND GREEN DATA  */
  usB = bsp_LCD_Read_Data();  /*READ OUT BLUE DATA*/

  return ((usRG & 0xF800) | ((usRG << 3) & 0x7E0) | (usB >> 11));
}

/**
 * @brief  獲取 ILI9341 顯示器上某一個坐標點的像素數據
 * @param  usX ：在特定掃描方向下該點的X坐標
 * @param  usY ：在特定掃描方向下該點的Y坐標
 * @retval 像素數據
 */
uint16_t bsp_LCD_GetPointPixel(uint16_t usX, uint16_t usY)
{
  uint16_t usPixelData;

  LCDDrv->SetCursor(usX, usY);

  usPixelData = bsp_LCD_Read_PixelData();

  return usPixelData;
}

/**
 * @brief  在 ILI9341 顯示器上使用 Bresenham 算法畫線段
 * @param  usX1 ：在特定掃描方向下線段的一個端點X坐標
 * @param  usY1 ：在特定掃描方向下線段的一個端點Y坐標
 * @param  usX2 ：在特定掃描方向下線段的另一個端點X坐標
 * @param  usY2 ：在特定掃描方向下線段的另一個端點Y坐標
 * @note 可使用bsp_LCD_SetBackColor、bsp_LCD_SetTextColor、bsp_LCD_SetColors函數設置顏色
 * @retval 無
 */
void bsp_LCD_DrawLine(uint16_t usX1, uint16_t usY1, uint16_t usX2, uint16_t usY2)
{
  uint16_t us;
  uint16_t usX_Current, usY_Current;

  int32_t lError_X = 0, lError_Y = 0, lDelta_X, lDelta_Y, lDistance;
  int32_t lIncrease_X, lIncrease_Y;

  lDelta_X = usX2 - usX1; //計算坐標增量
  lDelta_Y = usY2 - usY1;

  usX_Current = usX1;
  usY_Current = usY1;

  if (lDelta_X > 0)
    lIncrease_X = 1; //設置單步方向

  else if (lDelta_X == 0)
    lIncrease_X = 0; //垂直線

  else
  {
    lIncrease_X = -1;
    lDelta_X = -lDelta_X;
  }

  if (lDelta_Y > 0)
    lIncrease_Y = 1;

  else if (lDelta_Y == 0)
    lIncrease_Y = 0; //水平線

  else
  {
    lIncrease_Y = -1;
    lDelta_Y = -lDelta_Y;
  }

  if (lDelta_X > lDelta_Y)
    lDistance = lDelta_X; //選取基本增量坐標軸

  else
    lDistance = lDelta_Y;

  for (us = 0; us <= lDistance + 1; us++) //畫線輸出
  {
    bsp_LCD_SetPointPixel(usX_Current, usY_Current); //畫點

    lError_X += lDelta_X;
    lError_Y += lDelta_Y;

    if (lError_X > lDistance)
    {
      lError_X -= lDistance;
      usX_Current += lIncrease_X;
    }

    if (lError_Y > lDistance)
    {
      lError_Y -= lDistance;
      usY_Current += lIncrease_Y;
    }
  }
}

/**
 * @brief  在 ILI9341 顯示器上畫一個矩形
 * @param  usX_Start ：在特定掃描方向下矩形的起始點X坐標
 * @param  usY_Start ：在特定掃描方向下矩形的起始點Y坐標
 * @param  usWidth：矩形的寬度（單位：像素）
 * @param  usHeight：矩形的高度（單位：像素）
 * @param  ucFilled ：選擇是否填充該矩形
 *   該參數為以下值之一：
 *     @arg 0 :空心矩形
 *     @arg 1 :實心矩形
 * @note 可使用bsp_LCD_SetBackColor、bsp_LCD_SetTextColor、bsp_LCD_SetColors函數設置顏色
 * @retval 無
 */
void bsp_LCD_DrawRectangle(uint16_t usX_Start, uint16_t usY_Start, uint16_t usWidth, uint16_t usHeight, uint8_t ucFilled)
{
  if (ucFilled)
  {
    LCDDrv->SetDisplayWindow(usX_Start, usY_Start, usWidth, usHeight);
    bsp_LCD_FillColor(usWidth * usHeight, CurrentTextColor);
  }
  else
  {
    bsp_LCD_DrawLine(usX_Start, usY_Start, usX_Start + usWidth - 1, usY_Start);
    bsp_LCD_DrawLine(usX_Start, usY_Start + usHeight - 1, usX_Start + usWidth - 1, usY_Start + usHeight - 1);
    bsp_LCD_DrawLine(usX_Start, usY_Start, usX_Start, usY_Start + usHeight - 1);
    bsp_LCD_DrawLine(usX_Start + usWidth - 1, usY_Start, usX_Start + usWidth - 1, usY_Start + usHeight - 1);
  }
}

/**
 * @brief  在 ILI9341 顯示器上使用 Bresenham 算法畫圓
 * @param  usX_Center ：在特定掃描方向下圓心的X坐標
 * @param  usY_Center ：在特定掃描方向下圓心的Y坐標
 * @param  usRadius：圓的半徑（單位：像素）
 * @param  ucFilled ：選擇是否填充該圓
 *   該參數為以下值之一：
 *     @arg 0 :空心圓
 *     @arg 1 :實心圓
 * @note 可使用bsp_LCD_SetBackColor、bsp_LCD_SetTextColor、bsp_LCD_SetColors函數設置顏色
 * @retval 無
 */
void bsp_LCD_DrawCircle(uint16_t usX_Center, uint16_t usY_Center, uint16_t usRadius, uint8_t ucFilled)
{
  int16_t sCurrentX, sCurrentY;
  int16_t sError;

  sCurrentX = 0;
  sCurrentY = usRadius;

  sError = 3 - (usRadius << 1); // 判斷下個點位置的標誌

  while (sCurrentX <= sCurrentY)
  {
    int16_t sCountY;

    if (ucFilled)
      for (sCountY = sCurrentX; sCountY <= sCurrentY; sCountY++)
      {
        bsp_LCD_SetPointPixel(usX_Center + sCurrentX, usY_Center + sCountY); // 1，研究對象
        bsp_LCD_SetPointPixel(usX_Center - sCurrentX, usY_Center + sCountY); // 2
        bsp_LCD_SetPointPixel(usX_Center - sCountY, usY_Center + sCurrentX); // 3
        bsp_LCD_SetPointPixel(usX_Center - sCountY, usY_Center - sCurrentX); // 4
        bsp_LCD_SetPointPixel(usX_Center - sCurrentX, usY_Center - sCountY); // 5
        bsp_LCD_SetPointPixel(usX_Center + sCurrentX, usY_Center - sCountY); // 6
        bsp_LCD_SetPointPixel(usX_Center + sCountY, usY_Center - sCurrentX); // 7
        bsp_LCD_SetPointPixel(usX_Center + sCountY, usY_Center + sCurrentX); // 0
      }

    else
    {
      bsp_LCD_SetPointPixel(usX_Center + sCurrentX, usY_Center + sCurrentY); // 1，研究對象
      bsp_LCD_SetPointPixel(usX_Center - sCurrentX, usY_Center + sCurrentY); // 2
      bsp_LCD_SetPointPixel(usX_Center - sCurrentY, usY_Center + sCurrentX); // 3
      bsp_LCD_SetPointPixel(usX_Center - sCurrentY, usY_Center - sCurrentX); // 4
      bsp_LCD_SetPointPixel(usX_Center - sCurrentX, usY_Center - sCurrentY); // 5
      bsp_LCD_SetPointPixel(usX_Center + sCurrentX, usY_Center - sCurrentY); // 6
      bsp_LCD_SetPointPixel(usX_Center + sCurrentY, usY_Center - sCurrentX); // 7
      bsp_LCD_SetPointPixel(usX_Center + sCurrentY, usY_Center + sCurrentX); // 0
    }

    sCurrentX++;

    if (sError < 0)
      sError += 4 * sCurrentX + 6;

    else
    {
      sError += 10 + 4 * (sCurrentX - sCurrentY);
      sCurrentY--;
    }
  }
}

/**
 * @brief  在 ILI9341 顯示器上顯示一個英文字符
 * @param  usX ：在特定掃描方向下字符的起始X坐標
 * @param  usY ：在特定掃描方向下該點的起始Y坐標
 * @param  cChar ：要顯示的英文字符
 * @note 可使用bsp_LCD_SetBackColor、bsp_LCD_SetTextColor、bsp_LCD_SetColors函數設置顏色
 * @retval 無
 */
void bsp_LCD_DispChar_EN(uint16_t usX, uint16_t usY, const char cChar)
{
  uint8_t byteCount, bitCount, fontLength = 0;
  uint16_t ucRelativePositon;
  uint32_t Pfont, Poffset;

  //對ascii碼表偏移（字模表不包含ASCII表的前32個非圖形符號）
  ucRelativePositon = cChar - ' ';

  //每個字模的字節數
  fontLength = (LCD_Currentfonts->Width * LCD_Currentfonts->Height) / 8;

  //字模首地址
  /*ascii碼表偏移值乘以每個字模的字節數，求出字模的偏移位置*/
  Poffset = LCD_Currentfonts->Offset;
  Pfont = (uint32_t)(ucRelativePositon * fontLength) + Poffset;
  //設置顯示窗口
  LCDDrv->SetDisplayWindow(usX, usY, LCD_Currentfonts->Width, LCD_Currentfonts->Height);

  bsp_LCD_Write_Cmd(CMD_SetPixel);

  uint8_t fontbuf[fontLength];

  SPIFLASHDrv->BufferRead(fontbuf, Pfont, fontLength);

  //按字節讀取字模數據
  //由於前面直接設置了顯示窗口，顯示數據會自動換行
  for (byteCount = 0; byteCount < fontLength; byteCount++)
  {
    //一位一位處理要顯示的顏色
    for (bitCount = 0; bitCount < 8; bitCount++)
    {
      if (fontbuf[byteCount] & (0x80 >> bitCount))
        bsp_LCD_Write_Data(CurrentTextColor);
      else
        bsp_LCD_Write_Data(CurrentBackColor);
    }
  }
}

/**
 * @brief  在 ILI9341 顯示器上顯示英文字符串
 * @param  line ：在特定掃描方向下字符串的起始Y坐標
 *   本參數可使用宏LINE(0)、LINE(1)等方式指定文字坐標，
 *   宏LINE(x)會根據當前選擇的字體來計算Y坐標值。
 *		顯示中文且使用LINE宏時，需要把英文字體設置成 Font8x16
 * @param  pStr ：要顯示的英文字符串的首地址
 * @note 可使用bsp_LCD_SetBackColor、bsp_LCD_SetTextColor、bsp_LCD_SetColors函數設置顏色
 * @retval 無
 */
void bsp_LCD_DispStringLine_EN(uint16_t line, char *pStr)
{
  uint16_t usX = 0;

  while (*pStr != '\0')
  {
    if ((usX - ILI9341_DispWindow_X_Star + LCD_Currentfonts->Width) > LCDDrv->GetLcdPixelWidth())
    {
      usX = ILI9341_DispWindow_X_Star;
      line += LCD_Currentfonts->Height;
    }

    if ((line - ILI9341_DispWindow_Y_Star + LCD_Currentfonts->Height) > LCDDrv->GetLcdPixelHeight())
    {
      usX = ILI9341_DispWindow_X_Star;
      line = ILI9341_DispWindow_Y_Star;
    }

    bsp_LCD_DispChar_EN(usX, line, *pStr);

    pStr++;

    usX += LCD_Currentfonts->Width;
  }
}

/**
 * @brief  在 ILI9341 顯示器上顯示英文字符串
 * @param  usX ：在特定掃描方向下字符的起始X坐標
 * @param  usY ：在特定掃描方向下字符的起始Y坐標
 * @param  pStr ：要顯示的英文字符串的首地址
 * @note 可使用bsp_LCD_SetBackColor、bsp_LCD_SetTextColor、bsp_LCD_SetColors函數設置顏色
 * @retval 無
 */
void bsp_LCD_DispString_EN(uint16_t usX, uint16_t usY, char *pStr)
{
  while (*pStr != '\0')
  {
    if ((usX - ILI9341_DispWindow_X_Star + LCD_Currentfonts->Width) > LCDDrv->GetLcdPixelWidth())
    {
      usX = ILI9341_DispWindow_X_Star;
      usY += LCD_Currentfonts->Height;
    }

    if ((usY - ILI9341_DispWindow_Y_Star + LCD_Currentfonts->Height) > LCDDrv->GetLcdPixelHeight())
    {
      usX = ILI9341_DispWindow_X_Star;
      usY = ILI9341_DispWindow_Y_Star;
    }

    bsp_LCD_DispChar_EN(usX, usY, *pStr);

    pStr++;

    usX += LCD_Currentfonts->Width;
  }
}

/**
 * @brief  在 ILI9341 顯示器上顯示英文字符串(沿Y軸方向)
 * @param  usX ：在特定掃描方向下字符的起始X坐標
 * @param  usY ：在特定掃描方向下字符的起始Y坐標
 * @param  pStr ：要顯示的英文字符串的首地址
 * @note 可使用bsp_LCD_SetBackColor、bsp_LCD_SetTextColor、bsp_LCD_SetColors函數設置顏色
 * @retval 無
 */
void bsp_LCD_DispString_EN_YDir(uint16_t usX, uint16_t usY, char *pStr)
{
  while (*pStr != '\0')
  {
    if ((usY - ILI9341_DispWindow_Y_Star + LCD_Currentfonts->Height) > LCDDrv->GetLcdPixelHeight())
    {
      usY = ILI9341_DispWindow_Y_Star;
      usX += LCD_Currentfonts->Width;
    }

    if ((usX - ILI9341_DispWindow_X_Star + LCD_Currentfonts->Width) > LCDDrv->GetLcdPixelWidth())
    {
      usX = ILI9341_DispWindow_X_Star;
      usY = ILI9341_DispWindow_Y_Star;
    }

    bsp_LCD_DispChar_EN(usX, usY, *pStr);

    pStr++;

    usY += LCD_Currentfonts->Height;
  }
}

/**
 * @brief  設置英文字體類型
 * @param  fonts: 指定要選擇的字體
 *		參數為以下值之一
 * 	@arg：Font24x32;
 * 	@arg：Font16x24;
 * 	@arg：Font8x16;
 * @retval None
 */
void bsp_LCD_SetFont(sFONT *fonts)
{
  LCD_Currentfonts = fonts;
}

/**
 * @brief  獲取當前字體類型
 * @param  None.
 * @retval 返回當前字體類型
 */
sFONT *bsp_LCD_GetFont(void)
{
  return LCD_Currentfonts;
}

/**
 * @brief  設置LCD的前景(字體)及背景顏色,RGB565
 * @param  TextColor: 指定前景(字體)顏色
 * @param  BackColor: 指定背景顏色
 * @retval None
 */
void bsp_LCD_SetColors(uint16_t TextColor, uint16_t BackColor)
{
  CurrentTextColor = TextColor;
  CurrentBackColor = BackColor;
}

/**
 * @brief  獲取LCD的前景(字體)及背景顏色,RGB565
 * @param  TextColor: 用來存儲前景(字體)顏色的指針變量
 * @param  BackColor: 用來存儲背景顏色的指針變量
 * @retval None
 */
void bsp_LCD_GetColors(uint16_t *TextColor, uint16_t *BackColor)
{
  *TextColor = CurrentTextColor;
  *BackColor = CurrentBackColor;
}

/**
 * @brief  設置LCD的前景(字體)顏色,RGB565
 * @param  Color: 指定前景(字體)顏色
 * @retval None
 */
void bsp_LCD_SetTextColor(uint16_t Color)
{
  CurrentTextColor = Color;
}

/**
 * @brief  設置LCD的背景顏色,RGB565
 * @param  Color: 指定背景顏色
 * @retval None
 */
void bsp_LCD_SetBackColor(uint16_t Color)
{
  CurrentBackColor = Color;
}

/**
 * @brief  清除某行文字
 * @param  Line: 指定要刪除的行
 *   本參數可使用宏LINE(0)、LINE(1)等方式指定要刪除的行，
 *   宏LINE(x)會根據當前選擇的字體來計算Y坐標值，並刪除當前字體高度的第x行。
 * @retval None
 */
void bsp_LCD_ClearLine(uint16_t Line)
{
  bsp_LCD_Clear(0, Line, LCDDrv->GetLcdPixelWidth(), ((sFONT *)bsp_LCD_GetFont())->Height); /* 清屏，显示全黑 */
}

void bsp_LCD_Show_Logo(void)
{
  bsp_LCD_show_Picture_at_SPIFlash(0x4000, 153600);
}

void bsp_LCD_Test(void)
{
  static uint8_t testCNT = 0;
  char dispBuff[100];

  testCNT++;

  bsp_LCD_SetFont(&Font8x16);
  bsp_LCD_SetColors(RED, BLACK);

  bsp_LCD_Clear(0, 0, LCDDrv->GetLcdPixelWidth(), LCDDrv->GetLcdPixelHeight()); /* 清屏，显示全黑 */
                                                                                /********显示字符串示例*******/
  bsp_LCD_DispStringLine_EN(LINE(0), "IK-ZET6 2.8 inch LCD para:");
  bsp_LCD_DispStringLine_EN(LINE(1), "Image resolution:240x320 px");
  bsp_LCD_DispStringLine_EN(LINE(2), "ILI9341 LCD driver");
  bsp_LCD_DispStringLine_EN(LINE(3), "XPT2046 Touch Pad driver");

  /********顯示變量示例*******/
  bsp_LCD_SetFont(&Font16x24);
  bsp_LCD_SetTextColor(GREEN);

  /*使用c標準庫把變量轉化成字符串*/
  sprintf(dispBuff, "Count : %d ", testCNT);
  bsp_LCD_ClearLine(LINE(4)); /* 清除單行文字 */

  /*然後顯示該字符串即可，其它變量也是這樣處理*/
  bsp_LCD_DispStringLine_EN(LINE(4), dispBuff);

  /*******顯示圖形示例******/
  bsp_LCD_SetFont(&Font24x32);
  /* 畫直線 */

  bsp_LCD_ClearLine(LINE(4)); /* 清除單行文字 */
  bsp_LCD_SetTextColor(BLUE);

  bsp_LCD_DispStringLine_EN(LINE(4), "Draw line:");

  bsp_LCD_SetTextColor(RED);
  bsp_LCD_DrawLine(50, 170, 210, 230);
  bsp_LCD_DrawLine(50, 200, 210, 240);

  bsp_LCD_SetTextColor(GREEN);
  bsp_LCD_DrawLine(100, 170, 200, 230);
  bsp_LCD_DrawLine(200, 200, 220, 240);

  bsp_LCD_SetTextColor(BLUE);
  bsp_LCD_DrawLine(110, 170, 110, 230);
  bsp_LCD_DrawLine(130, 200, 220, 240);

  bsp_Delay(1000);

  bsp_LCD_Clear(0, 16 * 8, LCDDrv->GetLcdPixelWidth(), LCDDrv->GetLcdPixelHeight() - 16 * 8); /* 清屏，显示全黑 */

  /*畫矩形*/

  bsp_LCD_ClearLine(LINE(4)); /* 清除單行文字 */
  bsp_LCD_SetTextColor(BLUE);

  bsp_LCD_DispStringLine_EN(LINE(4), "Draw Rect:");

  bsp_LCD_SetTextColor(RED);
  bsp_LCD_DrawRectangle(50, 200, 100, 30, 1);

  bsp_LCD_SetTextColor(GREEN);
  bsp_LCD_DrawRectangle(160, 200, 20, 40, 0);

  bsp_LCD_SetTextColor(BLUE);
  bsp_LCD_DrawRectangle(170, 200, 50, 20, 1);

  bsp_Delay(1000);

  bsp_LCD_Clear(0, 16 * 8, LCDDrv->GetLcdPixelWidth(), LCDDrv->GetLcdPixelHeight() - 16 * 8); /* 清屏，顯示全黑 */

  /* 畫圓 */
  bsp_LCD_ClearLine(LINE(4)); /* 清除單行文字 */
  bsp_LCD_SetTextColor(BLUE);

  bsp_LCD_DispStringLine_EN(LINE(4), "Draw Cir:");

  bsp_LCD_SetTextColor(RED);
  bsp_LCD_DrawCircle(100, 200, 20, 0);

  bsp_LCD_SetTextColor(GREEN);
  bsp_LCD_DrawCircle(100, 200, 10, 1);

  bsp_LCD_SetTextColor(BLUE);
  bsp_LCD_DrawCircle(140, 200, 20, 0);

  bsp_Delay(1000);

  bsp_LCD_Clear(0, 16 * 8, LCDDrv->GetLcdPixelWidth(), LCDDrv->GetLcdPixelHeight() - 16 * 8); /* 清屏，显示全黑 */
}

void bsp_LCD_Test2(void)
{
  uint16_t colors[1] = {0X82C5};
  bsp_LCD_FlushColor(100, 120, 200, 240, colors);
}

void bsp_LCD_Test3(void)
{
  LCDDrv->SetDisplayWindow(0, 0, LCDDrv->GetLcdPixelWidth(), LCDDrv->GetLcdPixelHeight());
  bsp_LCD_FillColor(LCDDrv->GetLcdPixelHeight() * LCDDrv->GetLcdPixelWidth(), RED);
  bsp_Delay(1000);
  bsp_LCD_FillColor(LCDDrv->GetLcdPixelHeight() * LCDDrv->GetLcdPixelWidth(), GREEN);
  bsp_Delay(1000);
  bsp_LCD_FillColor(LCDDrv->GetLcdPixelHeight() * LCDDrv->GetLcdPixelWidth(), BLUE);
  bsp_Delay(1000);
}
/*********************end of file*************************/
