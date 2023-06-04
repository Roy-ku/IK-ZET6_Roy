#include "bsp_I2C_IOexpand.h"

#if (IOEXPAND_SOFTIIC == 0)
#define hI2Cx hi2c2
#define IOEXPAND_TIMEOUT_MAX 100
#define IOEXPAND_MAX_TRIALS 30
#endif

IOExpand_DrvTypeDef *IODrv;

/**
 * @brief 初始化AHT
 */
void bsp_IOEXPAND_Init(void)
{
  IODrv = &pcf8574Tt_drv;
  IODrv->WriteByte(0xFF);
  bsp_Log_Info("\r\nIO info\r\nChipID : %s\r\n", IODrv->ChipName);
}

#if (IOEXPAND_SOFTIIC == 0)

/**
 * @brief 寫入狀態
 * @param drvaddr 設備地址
 * @param value 狀態
 * @return bsp_Status 返回說明
 */
bsp_Status bsp_I2C_IOEXPAND_Write(uint8_t drvaddr, uint8_t value)
{
  bsp_Status status = bsp_PASSED;

  status = (bsp_Status)bsp_I2Cx_bus_Master_Write(&hI2Cx, drvaddr, value, 1, 1000);

  /* Check if the ATH is ready for a new operation */
  status = (bsp_Status)bsp_I2Cx_bus_IsDeviceReady(&hI2Cx, drvaddr, IOEXPAND_MAX_TRIALS, IOEXPAND_TIMEOUT_MAX);
  if (status != bsp_PASSED)
  {
    bsp_Log_Error("DeviceReady happen error (%d)", status);
    return status;
  }
  return status;
}

/**
 * @brief 讀取狀態
 * @param drvaddr 設備地址
 * @param value IO狀態
 * @return bsp_Status 狀態
 */
bsp_Status bsp_I2C_IOEXPAND_Read(uint8_t drvaddr, uint8_t *value)
{
  bsp_Status status = bsp_PASSED;

  status = (bsp_Status)bsp_I2Cx_bus_Master_Read(&hI2Cx, drvaddr, value, 1, 1000);

  /* Check if the ATH is ready for a new operation */
  status = (bsp_Status)bsp_I2Cx_bus_IsDeviceReady(&hI2Cx, drvaddr, IOEXPAND_MAX_TRIALS, IOEXPAND_TIMEOUT_MAX);
  if (status != bsp_PASSED)
  {
    bsp_Log_Error("DeviceReady happen error (%d)", status);
    return status;
  }

  return status;
}

#elif (IOEXPAND_SOFTIIC == 1)

/**
 * @brief 寫入狀態
 * @param drvaddr 設備地址
 * @param value IO狀態
 * @return bsp_Status 狀態
 */
bsp_Status bsp_I2C_IOEXPAND_Write(uint8_t drvaddr, uint8_t value)
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
  i2c_SendByte((uint8_t)value);
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
 * @brief 讀取狀態
 * @param drvaddr 設備地址
 * @param value IO狀態
 * @return bsp_Status 狀態
 */
bsp_Status bsp_I2C_IOEXPAND_Read(uint8_t drvaddr, uint8_t *value)
{
  bsp_Status status = bsp_PASSED;
  uint16_t m = 0;

  /* 判斷內部寫操作是否完成 */
  for (m = 0; m < 1000; m++)
  {
    /* 第1步：發送起動信號 */
    i2c_Start();

    /* 第2步：發送控制字節，高7bit是地址，bit0是讀寫控制位，0表示寫，1表示讀 */
    i2c_SendByte(drvaddr | I2C_RD);

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

  /* 第4步：讀取資料 */
  *value = i2c_ReadByte(); /* 讀1個字節 */

  i2c_NAck(); /* 最後1個字節讀完後，CPU產生NACK信號(驅動SDA = 1) */

  i2c_Stop(); /* 發送停止信號 */

  return bsp_PASSED; /* 成功 */

cmd_fail: /* 命令執行失敗後，切記發送停止信號，避免影響I2C總線上其他設備 */

  i2c_Stop(); /* 發送停止信號 */
  bsp_Log_Error("I2C device add(0x%x) happen error", drvaddr);
  return bsp_FAILED; /* 失敗 */
}
#endif

/*********************************************END OF FILE*************************************************/
