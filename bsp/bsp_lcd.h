#ifndef __BSP_LCD_H
#define __BSP_LCD_H

#include "bsp.h"
#include "fonts.h"

typedef struct
{
  uint16_t LCDID;
  void (*Init)(void);
  void (*RESET)(void);
  uint16_t (*ReadID)(void);
  void (*GramScan)(uint8_t);
  void (*DisplayOn)(void);
  void (*DisplayOff)(void);
  void (*SetCursor)(uint16_t, uint16_t);
  /* Optimized operation */
  void (*SetDisplayWindow)(uint16_t, uint16_t, uint16_t, uint16_t);
  uint16_t (*GetLcdPixelWidth)(void);
  uint16_t (*GetLcdPixelHeight)(void);
} LCD_DrvTypeDef;
extern LCD_DrvTypeDef ili9341_drv;

/***************************************************************************************
2^26 =0X0400 0000 = 64MB,每個 BANK 有4*64MB = 256MB
64MB:FSMC_Bank1_NORSRAM1:0X6000 0000 ~ 0X63FF FFFF
64MB:FSMC_Bank1_NORSRAM2:0X6400 0000 ~ 0X67FF FFFF
64MB:FSMC_Bank1_NORSRAM3:0X6800 0000 ~ 0X6BFF FFFF
64MB:FSMC_Bank1_NORSRAM4:0X6C00 0000 ~ 0X6FFF FFFF

選擇BANK1-BORSRAM4 連接 TFT，地址範圍為0X6C00 0000 ~ 0X6FFF FFFF
FSMC_A10 接LCD的DC(寄存器/數據選擇)腳
寄存器基地址 = 0X6C00 0000
RAM基地址 = 0X6C000800 = 0X6C000000+2^10*2 = 0X6C000000 + 0X800 = 0x6C000800
當選擇不同的地址線時，地址要重新計算
****************************************************************************************/

/******************************* ILI9341 顯示屏的 FSMC 參數定義 ***************************/
// FSMC_Bank1_NORSRAM用於LCD命令操作的地址
#define FSMC_Addr_ILI9341_CMD ((uint32_t)0x6C000000) // Disp Reg ADDR

// FSMC_Bank1_NORSRAM用於LCD數據操作的地址
#define FSMC_Addr_ILI9341_DATA ((uint32_t)0x6C000800) // Disp Data ADDR      // A10 PG0


/*************************************** LCD_RESET_Pin ******************************************/
#define LCD_RESETPIN_H HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET)
#define LCD_RESETPIN_L HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET)

/***************************** ILI934 顯示區域的起始坐標和總行列數 ***************************/
#define ILI9341_DispWindow_X_Star 0 //起始點的X坐標
#define ILI9341_DispWindow_Y_Star 0 //起始點的Y坐標

//根據液晶掃描方向而變化的XY像素寬度
//調用ILI9341_GramScan函數設置方向時會自動更改
extern uint16_t LCD_X_LENGTH, LCD_Y_LENGTH;

//液晶屏掃描模式
//參數可選值為0-7
extern uint8_t LCD_SCAN_MODE;

/******************************* 定義 ILI934 顯示屏常用顏色 ********************************/
#define BACKGROUND BLACK //默認背景顏色

#define WHITE 0xFFFF   //白色
#define BLACK 0x0000   //黑色
#define GREY 0xF7DE    //灰色
#define BLUE 0x001F    //藍色
#define BLUE2 0x051F   //淺藍色
#define RED 0xF800     //紅色
#define MAGENTA 0xF81F //紅紫色，洋紅色
#define GREEN 0x07E0   //綠色
#define CYAN 0x7FFF    //藍綠色，青色
#define YELLOW 0xFFE0  //黃色
#define BRED 0xF81F
#define GRED 0xFFE0
#define GBLUE 0x07FF

/******************************* 定義 ILI934 常用命令 ********************************/
#define CMD_SetCoordinateX 0x2A //設置X坐標
#define CMD_SetCoordinateY 0x2B //設置Y坐標
#define CMD_SetPixel 0x2C       //填充像素

/********************************** 聲明 ILI934 函數 ***************************************/

void bsp_LCD_Write_Cmd(uint16_t usCmd);
void bsp_LCD_Write_Data(uint16_t usData);
uint16_t bsp_LCD_Read_Data(void);

void bsp_LCD_Init(void);
void bsp_LCD_Backlight_Control(FunctionalState enumState);
void bsp_LCD_Clear(uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight);
void bsp_LCD_FlushColor(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color);
void bsp_LCD_show_Picture_at_SPIFlash(uint32_t addr, uint32_t size);
void bsp_LCD_SetPointPixel(uint16_t usX, uint16_t usY);
uint16_t bsp_LCD_GetPointPixel(uint16_t usX, uint16_t usY);
void bsp_LCD_DrawLine(uint16_t usX1, uint16_t usY1, uint16_t usX2, uint16_t usY2);
void bsp_LCD_DrawRectangle(uint16_t usX_Start, uint16_t usY_Start, uint16_t usWidth, uint16_t usHeight, uint8_t ucFilled);
void bsp_LCD_DrawCircle(uint16_t usX_Center, uint16_t usY_Center, uint16_t usRadius, uint8_t ucFilled);
void bsp_LCD_DispChar_EN(uint16_t usX, uint16_t usY, const char cChar);
void bsp_LCD_DispStringLine_EN(uint16_t line, char *pStr);
void bsp_LCD_DispString_EN(uint16_t usX, uint16_t usY, char *pStr);
void bsp_LCD_DispString_EN_YDir(uint16_t usX, uint16_t usY, char *pStr);
void bsp_LCD_Show_Logo(void);

void bsp_LCD_SetFont(sFONT *fonts);
sFONT *bsp_LCD_GetFont(void);
void bsp_LCD_ClearLine(uint16_t Line);
void bsp_LCD_SetBackColor(uint16_t Color);
void bsp_LCD_SetTextColor(uint16_t Color);
void bsp_LCD_SetColors(uint16_t TextColor, uint16_t BackColor);
void bsp_LCD_GetColors(uint16_t *TextColor, uint16_t *BackColor);
void bsp_LCD_Test(void);
void bsp_LCD_Test2(void);
void bsp_LCD_Test3(void);
#endif /* __BSP_ILI9341_ILI9341_H */
