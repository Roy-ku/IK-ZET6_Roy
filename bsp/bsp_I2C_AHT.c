#include "bsp_I2C_AHT.h"

#if (AHT_SOFTIIC == 0)
#define hI2Cx hi2c2
#define AHT_TIMEOUT_MAX 100
#define AHT_MAX_TRIALS 30
#endif

AHT_DrvTypeDef *AHTDrv;
float AHT10_Humidity, AHT10_Temperature;
bsp_Status AHTisReady = bsp_FAILED;

/**
 * @brief 初始化AHT
 */
void bsp_AHT_Init(void)
{
  AHTDrv = &aht10_drv;
  bsp_Log_Info("\r\nAHT info\r\nChipID : %s\r\n", AHTDrv->ChipName);
  AHTDrv->Correction();
  HAL_Delay(40);
  AHTDrv->ReadTEMPHUM(&AHT10_Humidity, &AHT10_Temperature);
  AHTisReady = bsp_PASSED;
}

#if (AHT_SOFTIIC == 0)

/**
 * @brief Reset
 * @param drvaddr 設備地址
 * @param _cmddata 命令
 * @return bsp_Status status
 */
bsp_Status bsp_I2C_AHT_Reset(uint8_t drvaddr, uint8_t *_cmddata)
{
  bsp_Status status = bsp_PASSED;

  status = (bsp_Status)bsp_I2Cx_bus_Master_Write(&hI2Cx, drvaddr, _cmddata, 1, 1000);
  HAL_Delay(20);

  /* Check if the ATH is ready for a new operation */
  status = (bsp_Status)bsp_I2Cx_bus_IsDeviceReady(&hI2Cx, drvaddr, AHT_MAX_TRIALS, AHT_TIMEOUT_MAX);
  if (status != bsp_PASSED)
  {
    bsp_Log_Error("DeviceReady happen error (%d)", status);
    return status;
  }
  return status;
}

/**
 * @brief 校正AHT
 * @param drvaddr 設備地址
 * @param _cmddata 命令
 * @return bsp_Status status
 */
bsp_Status bsp_I2C_AHT_Correction(uint8_t drvaddr, uint8_t *_cmddata)
{
  bsp_Status status = bsp_PASSED;

  HAL_Delay(40);
  status = (bsp_Status)bsp_I2Cx_bus_Master_Write(&hI2Cx, drvaddr, _cmddata, 3, 1000);
  HAL_Delay(20);

  /* Check if the ATH is ready for a new operation */
  status = (bsp_Status)bsp_I2Cx_bus_IsDeviceReady(&hI2Cx, drvaddr, AHT_MAX_TRIALS, AHT_TIMEOUT_MAX);
  if (status != bsp_PASSED)
  {
    bsp_Log_Error("DeviceReady happen error (%d)", status);
    return status;
  }
  return status;
}

/**
 * @brief 讀取溫濕度
 * @param drvaddr 設備地址
 * @param _cmddata 命令
 * @param _humidity 濕度
 * @param _temperature 溫度
 * @return bsp_Status status
 */
bsp_Status bsp_I2C_AHT_ReadTEMPHUM(uint8_t drvaddr, uint8_t *_cmddata, float *_humidity, float *_temperature)
{
  bsp_Status status = bsp_PASSED;
  uint32_t u32_RH = 0, u32_T = 0;
  uint8_t databuff[6] = {0};

  status = (bsp_Status)bsp_I2Cx_bus_Master_Write(&hI2Cx, drvaddr, _cmddata, 3, 1000);
  HAL_Delay(85);
  status = (bsp_Status)bsp_I2Cx_bus_Master_Read(&hI2Cx, drvaddr, databuff, 6, 1000);

  if ((databuff[0] & 0x40) == 0 && status == bsp_PASSED)
  {
    u32_RH = (databuff[1] << 12) | (databuff[2] << 4) | ((databuff[3] >> 4) & 0X0F);
    u32_T = ((databuff[3] & 0X0f) << 16) | ((databuff[4] << 8) | databuff[5]);

    /*放大10倍計算*/
    *_humidity = (float)((u32_RH * 100 * 10) >> 20) / 10;
    *_temperature = (float)(((u32_T * 200 * 10) >> 20) - 500) / 10;
  }
  else
  {
    status = bsp_FAILED;
    bsp_Log_Error("AHT_Read data happen error");
    return status;
  }

  /* Check if the ATH is ready for a new operation */
  status = (bsp_Status)bsp_I2Cx_bus_IsDeviceReady(&hI2Cx, drvaddr, AHT_MAX_TRIALS, AHT_TIMEOUT_MAX);
  if (status != bsp_PASSED)
  {
    bsp_Log_Error("DeviceReady happen error (%d)", status);
    return status;
  }

  return status;
}

#elif (AHT_SOFTIIC == 1)

/**
 * @brief Reset
 * @param drvaddr 設備地址
 * @param _cmddata 命令
 * @return bsp_Status status
 */
bsp_Status bsp_I2C_AHT_Reset(uint8_t drvaddr, uint8_t *_cmddata)
{
  uint16_t m;

  /* 判斷內部寫操作是否完成 */
  for (m = 0; m < 1000; m++)
  {
    /* 第1步：發送起動信號 */
    i2c_Start();

    /* 第2步：發送控制字節，高7bit是地址，bit0是讀寫控制位，0表示寫，1表示讀 */
    i2c_SendByte(drvaddr | I2C_WR);

    /* 第3步：發送一個時鐘，判斷器件是否正確應答 */
    if (i2c_WaitAck() == 0)
    {
      break;
    }
  }
  if (m == 1000)
  {
    goto cmd_fail; /* 超時 */
  }

  /* 第4步：寫入命令 */
  i2c_SendByte((uint8_t)_cmddata[0]);
  if (i2c_WaitAck() != 0)
  {
    goto cmd_fail; /* 無應答 */
  }

  i2c_Stop(); /* 發送停止信號 */

  /* 判斷內部寫操作是否完成 */
  for (m = 0; m < 1000; m++)
  {
    if (i2c_CheckDevice(drvaddr) == 0)
    {
      break;
    }
  }
  if (m == 1000)
  {
    goto cmd_fail; /* 超時 */
  }

  i2c_Stop(); /* 發送停止信號 */
  HAL_Delay(20);
  return bsp_PASSED; /* 成功 */

cmd_fail: /* 命令執行失敗後，切記發送停止信號，避免影響I2C總線上其他設備 */

  i2c_Stop(); /* 發送停止信號 */
  bsp_Log_Error("I2C device add(0x%x) happen error", drvaddr);
  return bsp_FAILED; /* 失敗 */
}

/**
 * @brief 校正AHT
 * @param drvaddr 設備地址
 * @param _cmddata 命令
 * @return bsp_Status status
 */
bsp_Status bsp_I2C_AHT_Correction(uint8_t drvaddr, uint8_t *_cmddata)
{
  uint16_t m;
  HAL_Delay(40);
  /* 判斷內部寫操作是否完成 */
  for (m = 0; m < 1000; m++)
  {
    /* 第1步：發送起動信號 */
    i2c_Start();

    /* 第2步：發送控制字節，高7bit是地址，bit0是讀寫控制位，0表示寫，1表示讀 */
    i2c_SendByte(drvaddr | I2C_WR);

    /* 第3步：發送一個時鐘，判斷器件是否正確應答 */
    if (i2c_WaitAck() == 0)
    {
      break;
    }
  }
  if (m == 1000)
  {
    goto cmd_fail; /* 超時 */
  }

  /* 第4步：寫入命令 */
  i2c_SendByte((uint8_t)_cmddata[0]);
  if (i2c_WaitAck() != 0)
  {
    goto cmd_fail; /* 無應答 */
  }
  i2c_SendByte((uint8_t)_cmddata[1]);
  if (i2c_WaitAck() != 0)
  {
    goto cmd_fail; /* 無應答 */
  }
  i2c_SendByte((uint8_t)_cmddata[2]);
  if (i2c_WaitAck() != 0)
  {
    goto cmd_fail; /* 無應答 */
  }

  i2c_Stop(); /* 發送停止信號 */
  return bsp_PASSED; /* 成功 */

cmd_fail: /* 命令執行失敗後，切記發送停止信號，避免影響I2C總線上其他設備 */

  i2c_Stop(); /* 發送停止信號 */
  bsp_Log_Error("I2C device add(0x%x) happen error", drvaddr);
  return bsp_FAILED; /* 失敗 */
}

/**
 * @brief 讀取溫濕度
 * @param drvaddr 設備地址
 * @param _cmddata 命令
 * @param _humidity 濕度
 * @param _temperature 溫度
 * @return bsp_Status status
 */
bsp_Status bsp_I2C_AHT_ReadTEMPHUM(uint8_t drvaddr, uint8_t *_cmddata, float *_humidity, float *_temperature)
{
  bsp_Status status = bsp_PASSED;
  uint32_t u32_RH = 0, u32_T = 0;
  uint8_t databuff[6] = {0}, _NumByteToRead = 6;
  uint16_t i = 0, m = 0;

  /* 判斷內部寫操作是否完成 */
  for (m = 0; m < 1000; m++)
  {
    /* 第1步：發送起動信號 */
    i2c_Start();

    /* 第2步：發送控制字節，高7bit是地址，bit0是讀寫控制位，0表示寫，1表示讀 */
    i2c_SendByte(drvaddr | I2C_WR);

    /* 第3步：發送一個時鐘，判斷器件是否正確應答 */
    if (i2c_WaitAck() == 0)
    {
      break;
    }
  }
  if (m == 1000)
  {
    goto cmd_fail; /* 超時 */
  }

  /* 第4步：寫入命令 */
  i2c_SendByte((uint8_t)_cmddata[0]);
  if (i2c_WaitAck() != 0)
  {
    goto cmd_fail; /* 無應答 */
  }
  i2c_SendByte((uint8_t)_cmddata[1]);
  if (i2c_WaitAck() != 0)
  {
    goto cmd_fail; /* 無應答 */
  }
  i2c_SendByte((uint8_t)_cmddata[2]);
  if (i2c_WaitAck() != 0)
  {
    goto cmd_fail; /* 無應答 */
  }
  i2c_Stop(); /* 發送停止信號 */

  HAL_Delay(85);

  /*---------讀取溫度---------*/

  /* 第5步：重新啟動I2C總線。下面開始讀取數據 */
  i2c_Start();

  /* 第6步：發起控製字節，高7bit是地址，bit0是讀寫控制位，0表示寫，1表示讀 */
  i2c_SendByte(drvaddr | I2C_RD);

  /* 第7步：發送ACK */
  if (i2c_WaitAck() != 0)
  {
    goto cmd_fail; /* 無應答 */
  }

  /* 第8步：讀取資料 */
  for (i = 0; i < _NumByteToRead; i++)
  {
    databuff[i] = i2c_ReadByte(); /* 讀1個字節 */

    /* 每讀完1個字節後，需要發送Ack， 最後一個字節不需要Ack，發Nack */
    if (i != _NumByteToRead - 1)
    {
      i2c_Ack(); /* 中間字節讀完後，CPU產生ACK信號(驅動SDA = 0) */
    }
    else
    {
      i2c_NAck(); /* 最後1個字節讀完後，CPU產生NACK信號(驅動SDA = 1) */
    }
  }

  i2c_Stop(); /* 發送停止信號 */

  /* 分析資料 */
  if ((databuff[0] & 0x40) == 0)
  {
    u32_RH = (databuff[1] << 12) | (databuff[2] << 4) | ((databuff[3] >> 4) & 0X0F);
    u32_T = ((databuff[3] & 0X0f) << 16) | ((databuff[4] << 8) | databuff[5]);

    /*放大10倍計算*/
    *_humidity = (float)((u32_RH * 100 * 10) >> 20) / 10;
    *_temperature = (float)(((u32_T * 200 * 10) >> 20) - 500) / 10;
  }
  else
  {
    goto cmd_fail; /* 資料有誤 */
  }

  return bsp_PASSED; /* 成功 */

cmd_fail: /* 命令執行失敗後，切記發送停止信號，避免影響I2C總線上其他設備 */

  i2c_Stop(); /* 發送停止信號 */
  bsp_Log_Error("I2C device add(0x%x) happen error", drvaddr);
  return bsp_FAILED; /* 失敗 */
}

#endif

/**
 * @brief ATH_Test
 * @return uint8_t 0:OK 1:ERROR
 */
bsp_Status bsp_ATH_Test(void)
{
#if (AHT_SOFTIIC == 0)
  bsp_Log_Info("ID (0x%x) : %d", AHTDrv->DrvAddr(), bsp_I2Cx_bus_IsDeviceReady(&hI2Cx, 0xA0, 100, 1000));
#elif (AHT_SOFTIIC == 1)
  bsp_Log_Info("i2c_CheckDevice(0x%x) : %d", AHTDrv->DrvAddr(), i2c_CheckDevice(AHTDrv->DrvAddr()));
#endif
  AHTDrv->Correction();
  HAL_Delay(40);
  AHTDrv->ReadTEMPHUM(&AHT10_Humidity, &AHT10_Temperature);
  bsp_Log_Info("HUM:%.1f,TEMP:%.1f\n", AHT10_Humidity, AHT10_Temperature);
  return bsp_PASSED;
}

/**
 * @brief 顯示在LCD上
 */
void bsp_ATH_Show(void)
{
  if (AHTisReady == bsp_PASSED)
  {
    char dispBuff[24];
    sprintf(dispBuff, "HUM:%.1f%%,TEMP:%.1fC", AHT10_Humidity, AHT10_Temperature);
    bsp_LCD_SetFont(&Font8x16);
    bsp_LCD_SetTextColor(YELLOW);
    bsp_LCD_DispStringLine_EN(LINE(1), dispBuff);
  }
}
/*********************************************END OF FILE**********************/
