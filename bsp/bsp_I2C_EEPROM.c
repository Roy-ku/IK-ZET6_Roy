#include "bsp_I2C_EEPROM.h"

#if (EEPROM_SOFTIIC == 0)
// extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
#define hI2Cx hi2c2
#endif

#define EEPROM_TIMEOUT_MAX 100
#define EEPROM_MAX_TRIALS 30
#define EEPROM_PAGESIZE EEROMDrv->PageSize()
#define EEPROM_MEMADDRSIZE EEROMDrv->MemaddrSize
#define DATA_Size 32

uint8_t I2c_Buf_Write[DATA_Size];
uint8_t I2c_Buf_Read[DATA_Size];

EEPROM_DrvTypeDef *EEROMDrv;

/* 僅供內部使用 */
static bsp_Status bsp_I2C_EEPROM_PageWrite(uint16_t drvaddr, uint8_t *pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite);

/**********************************************************************************************************/

void bsp_EEPROM_init(void)
{
  EEROMDrv = &at24c02_drv;
  bsp_Log_Info("\r\nEEPROM info\r\nChipID : %s\r\n", EEROMDrv->ChipName);
}

/**
 * @brief 將緩衝區中的數據寫到I2C EEPROM中
 * @param pBuffer 緩衝區指針
 * @param WriteAddr 寫地址
 * @param NumByteToWrite 寫的字節數
 * @return bsp_Status 返回說明
 */
bsp_Status bsp_I2C_EEPROM_BufferWrite(uint16_t drvaddr, uint8_t *pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite)
{
  bsp_Status status = bsp_PASSED;
  uint16_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0;
  Addr = WriteAddr % EEPROM_PAGESIZE;
  count = EEPROM_PAGESIZE - Addr;
  NumOfPage = NumByteToWrite / EEPROM_PAGESIZE;
  NumOfSingle = NumByteToWrite % EEPROM_PAGESIZE;

  /* If WriteAddr is I2C_PageSize aligned  */
  if (Addr == 0)
  {
    /* If NumByteToWrite < I2C_PageSize */
    if (NumOfPage == 0)
    {
      status = (bsp_Status)bsp_I2C_EEPROM_PageWrite(drvaddr, pBuffer, WriteAddr, NumOfSingle);
    }
    /* If NumByteToWrite > I2C_PageSize */
    else
    {
      while (NumOfPage--)
      {
        status = (bsp_Status)bsp_I2C_EEPROM_PageWrite(drvaddr, pBuffer, WriteAddr, EEPROM_PAGESIZE);
        WriteAddr += EEPROM_PAGESIZE;
        pBuffer += EEPROM_PAGESIZE;
      }

      if (NumOfSingle != 0)
      {
        status = (bsp_Status)bsp_I2C_EEPROM_PageWrite(drvaddr, pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }
  /* If WriteAddr is not I2C_PageSize aligned  */
  else
  {
    /* If NumByteToWrite < I2C_PageSize */
    if (NumOfPage == 0)
    {
      status = (bsp_Status)bsp_I2C_EEPROM_PageWrite(drvaddr, pBuffer, WriteAddr, NumOfSingle);
    }
    /* If NumByteToWrite > I2C_PageSize */
    else
    {
      NumByteToWrite -= count;
      NumOfPage = NumByteToWrite / EEPROM_PAGESIZE;
      NumOfSingle = NumByteToWrite % EEPROM_PAGESIZE;

      if (count != 0)
      {
        status = (bsp_Status)bsp_I2C_EEPROM_PageWrite(drvaddr, pBuffer, WriteAddr, count);
        WriteAddr += count;
        pBuffer += count;
      }

      while (NumOfPage--)
      {
        status = (bsp_Status)bsp_I2C_EEPROM_PageWrite(drvaddr, pBuffer, WriteAddr, EEPROM_PAGESIZE);
        WriteAddr += EEPROM_PAGESIZE;
        pBuffer += EEPROM_PAGESIZE;
      }
      if (NumOfSingle != 0)
      {
        status = (bsp_Status)bsp_I2C_EEPROM_PageWrite(drvaddr, pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }

  return status;
}

/**********************************************************************************************************/

#if (EEPROM_SOFTIIC == 0)

/**
 * @brief 寫一個字節到I2C EEPROM中
 * @param pBuffer 緩衝區指針
 * @param WriteAddr 寫地址
 * @return uint8_t 返回說明
 */
bsp_Status bsp_I2C_EEPROM_ByteWrite(uint16_t drvaddr, uint8_t value, uint16_t WriteAddr)
{
  bsp_Status status = bsp_PASSED;

  /* Write Byte */
  status = (bsp_Status)bsp_I2Cx_bus_Write_8bit(&hI2Cx, drvaddr, WriteAddr, value);

  /* Check if the EEPROM is ready for a new operation */
  status = (bsp_Status)bsp_I2Cx_bus_IsDeviceReady(&hI2Cx, drvaddr, EEPROM_MAX_TRIALS, EEPROM_TIMEOUT_MAX);
  if (status != bsp_PASSED)
  {
    bsp_Log_Error("DeviceReady happen error (%d)", status);
    return status;
  }
  return status;
}

/**
 * @brief 在EEPROM的一個寫循環中可以寫多個字節，但一次寫入的字節數
 *        不能超過EEPROM頁的大小，AT24C02每頁有8個字節
 * @param pBuffer 緩衝區指針
 * @param WriteAddr 寫地址
 * @param NumByteToWrite 寫的字節數
 * @return uint8_t  0:OK 1:ERROR
 */
static bsp_Status bsp_I2C_EEPROM_PageWrite(uint16_t drvaddr, uint8_t *pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite)
{
  bsp_Status status = bsp_PASSED;

  /* Write EEPROM_PAGESIZE */
  status = (bsp_Status)bsp_I2Cx_bus_WriteMultiple_8bit(&hI2Cx, drvaddr, WriteAddr, pBuffer, NumByteToWrite);

  /* Check if the EEPROM is ready for a new operation */
  status = (bsp_Status)bsp_I2Cx_bus_IsDeviceReady(&hI2Cx, drvaddr, EEPROM_MAX_TRIALS, EEPROM_TIMEOUT_MAX);
  if (status != bsp_PASSED)
  {
    bsp_Log_Error("DeviceReady happen error (%d)", status);
    return status;
  }

  return status;
}

/**
 * @brief 從EEPROM裡面讀取一塊數據
 * @param pBuffer 存放從EEPROM讀取的數據的緩衝區指針
 * @param ReadAddr 接收數據的EEPROM的地址
 * @param NumByteToRead 要從EEPROM讀取的字節數
 * @return uint8_t 0:OK 1:ERROR
 */
bsp_Status bsp_I2C_EEPROM_BufferRead(uint16_t drvaddr, uint8_t *pBuffer, uint16_t ReadAddr, uint16_t NumByteToRead)
{
  bsp_Status status = bsp_PASSED;
  status = (bsp_Status)bsp_I2Cx_bus_ReadMultiple_8bit(&hI2Cx, drvaddr, ReadAddr, pBuffer, NumByteToRead);
  return status;
}

#elif (EEPROM_SOFTIIC == 1)

#define EE_ADDR_BYTES 1

/**
 * @brief 寫一個字節到I2C EEPROM中
 * @param pBuffer 緩衝區指針
 * @param WriteAddr 寫地址
 * @return uint8_t 返回Status
 */
bsp_Status bsp_I2C_EEPROM_ByteWrite(uint16_t drvaddr, uint8_t value, uint16_t WriteAddr)
{
  uint16_t i, m;
  uint16_t usAddr;
  usAddr = WriteAddr;

  /* 當發送第1個字節或是頁面首地址時，需要重新發起起動信號和地址 */
  if ((i == 0) || (usAddr & (EEPROM_PAGESIZE - 1)) == 0)
  {
    /*　發送停止信號，啟動內部寫操作　*/
    i2c_Stop();

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

    /* 第4步：發送字節地址，24C02只有256字節，因此1個字節就夠了，如果是24C04以上，那麼此處需要連發多個地址 */
    if (EE_ADDR_BYTES == 1)
    {
      i2c_SendByte((uint8_t)usAddr);
      if (i2c_WaitAck() != 0)
      {
        goto cmd_fail; /* 無應答 */
      }
    }
    else
    {
      i2c_SendByte(usAddr >> 8);
      if (i2c_WaitAck() != 0)
      {
        goto cmd_fail; /* 無應答 */
      }

      i2c_SendByte(usAddr);
      if (i2c_WaitAck() != 0)
      {
        goto cmd_fail; /* 無應答 */
      }
    }
  }

  /* 第6步：寫入資料 */
  i2c_SendByte(value);

  /* 第7步：发送ACK */
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

  i2c_Stop();        /* 發送停止信號 */
  return bsp_PASSED; /* 成功 */

cmd_fail: /* 命令執行失敗後，切記發送停止信號，避免影響I2C總線上其他設備 */

  i2c_Stop();        /* 發送停止信號 */
  bsp_Log_Error("I2C device add(0x%x) happen error", drvaddr);
  return bsp_FAILED; /* 失敗 */
}

/**
 * @brief 在EEPROM的一個寫循環中可以寫多個字節，但一次寫入的字節數
 *        不能超過EEPROM頁的大小，AT24C02每頁有8個字節
 * @param pBuffer 緩衝區指針
 * @param WriteAddr 寫地址
 * @param NumByteToWrite 寫的字節數
 * @return uint8_t  0:OK 1:ERROR
 * @note 寫串行EEPROM不像讀操作可以連續讀取很多字節，每次寫操作只能在同一個page。
 *       對於24xx02，page size = 8
 *       簡單的處理方法為：按字節寫操作模式，每寫1個字節，都發送地址
 *       為了提高連續寫的效率: 本函數採用page wirte操作。
 */
static bsp_Status bsp_I2C_EEPROM_PageWrite(uint16_t drvaddr, uint8_t *pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite)
{
  uint16_t i, m;
  uint16_t usAddr;

  usAddr = WriteAddr;
  for (i = 0; i < NumByteToWrite; i++)
  {
    /* 當發送第1個字節或是頁面首地址時，需要重新發起起動信號和地址 */
    if ((i == 0) || (usAddr & (EEPROM_PAGESIZE - 1)) == 0)
    {
      /*　第0步：發停止信號，啟動內部寫操作　*/
      i2c_Stop();

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

      /* 第4步：發送字節地址，24C02只有256字節，因此1個字節就夠了，如果是24C04以上，那麼此處需要連發多個地址 */
      if (EE_ADDR_BYTES == 1)
      {
        i2c_SendByte((uint8_t)usAddr);
        if (i2c_WaitAck() != 0)
        {
          goto cmd_fail; /* 無應答 */
        }
      }
      else
      {
        i2c_SendByte(usAddr >> 8);
        if (i2c_WaitAck() != 0)
        {
          goto cmd_fail; /* 無應答 */
        }

        i2c_SendByte(usAddr);
        if (i2c_WaitAck() != 0)
        {
          goto cmd_fail; /* 無應答 */
        }
      }
    }

    /* 第6步：寫入資料 */
    i2c_SendByte(pBuffer[i]);

    /* 第7步：發送ACK */
    if (i2c_WaitAck() != 0)
    {
      goto cmd_fail; /* 無應答 */
    }

    usAddr++; /* 地址增1 */
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

  i2c_Stop();        /*發送停止信號 */
  return bsp_PASSED; /* 成功 */

cmd_fail: /* 命令執行失敗後，切記發送停止信號，避免影響I2C總線上其他設備 */

  i2c_Stop();        /* 發送停止信號 */
  bsp_Log_Error("I2C device add(0x%x) happen error", drvaddr);
  return bsp_FAILED; /* 失敗 */
}

/**********************************************************************************************************/

/**
 * @brief 從EEPROM裡面讀取一塊數據
 * @param pBuffer 存放從EEPROM讀取的數據的緩衝區指針
 * @param ReadAddr 接收數據的EEPROM的地址
 * @param NumByteToRead 要從EEPROM讀取的字節數
 * @return uint8_t 0:OK 1:ERROR
 */
bsp_Status bsp_I2C_EEPROM_BufferRead(uint16_t drvaddr, uint8_t *pBuffer, uint16_t ReadAddr, uint16_t NumByteToRead)
{
  uint16_t i;

  /* 第1步：發送起動信號 */
  i2c_Start();

  /* 第2步：發送控制字節，高7bit是地址，bit0是讀寫控制位，0表示寫，1表示讀 */
  i2c_SendByte(drvaddr | I2C_WR);

  /* 第3步：發送ACK */
  if (i2c_WaitAck() != 0)
  {
    goto cmd_fail; /* 無應答 */
  }

  /* 第4步：發送字節地址，24C02只有256字節，因此1個字節就夠了，如果是24C04以上，那麼此處需要連發多個地址 */
  if (EEPROM_MEMADDRSIZE == 1)
  {
    i2c_SendByte((uint8_t)ReadAddr);
    if (i2c_WaitAck() != 0)
    {
      goto cmd_fail; /* 無應答 */
    }
  }
  else
  {
    i2c_SendByte(ReadAddr >> 8);
    if (i2c_WaitAck() != 0)
    {
      goto cmd_fail; /* 無應答 */
    }

    i2c_SendByte(ReadAddr);
    if (i2c_WaitAck() != 0)
    {
      goto cmd_fail; /* 無應答 */
    }
  }

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
  for (i = 0; i < NumByteToRead; i++)
  {
    pBuffer[i] = i2c_ReadByte(); /* 讀1個字節 */

    /* 每讀完1個字節後，需要發送Ack， 最後一個字節不需要Ack，發Nack */
    if (i != NumByteToRead - 1)
    {
      i2c_Ack(); /* 中間字節讀完後，CPU產生ACK信號(驅動SDA = 0) */
    }
    else
    {
      i2c_NAck(); /* 最後1個字節讀完後，CPU產生NACK信號(驅動SDA = 1) */
    }
  }

  i2c_Stop();        /* 發送停止信號 */
  return bsp_PASSED; /* 成功 */

cmd_fail: /* 命令執行失敗後，切記發送停止信號，避免影響I2C總線上其他設備 */

  i2c_Stop();        /* 發送停止信號 */
  bsp_Log_Error("I2C device add(0x%x) happen error", drvaddr);
  return bsp_FAILED; /* 失敗 */
}

#endif

/**
 * @brief I2C(AT24C02)讀寫測試
 * @return uint8_t 0:OK 1:ERROR
 */
bsp_Status bsp_I2C_EEPROM_Test(void)
{
  uint16_t i;

#if (EEPROM_SOFTIIC == 0)
  bsp_Log_Info("ID (0x%x) : %d", EEROMDrv->DrvAddr(), bsp_I2Cx_bus_IsDeviceReady(&hI2Cx, 0xA0, 100, 1000));
#elif (EEPROM_SOFTIIC == 1)
  bsp_Log_Info("i2c_CheckDevice(0x%x) : %d", EEROMDrv->DrvAddr(), i2c_CheckDevice(EEROMDrv->DrvAddr()));
#endif

  // /* 填充資料 */
  // for (i = 0; i < DATA_Size; i++)
  // {
  //   I2c_Buf_Write[i] = DATA_Size - i;
  // }

  // EEROMDrv->BufferWrite(I2c_Buf_Write, 0x00, DATA_Size);
  EEROMDrv->BufferRead(I2c_Buf_Read, 0x00, DATA_Size);

  /* 輸出&校驗資料 */
  for (i = 0; i < DATA_Size; i++)
  {
    // if (I2c_Buf_Read[i] != I2c_Buf_Write[i])
    // {
    //   // printf("0x%02X ", I2c_Buf_Read[i]);
    //   bsp_Log_Error("I2C(AT24C02)test error");
    //   return bsp_FAILED;
    // }
    printf("0x%02X ", I2c_Buf_Read[i]);
    if (i % 8 == 7)
    {
      printf("\r\n");
    }
  }

  // printf("\r\n");
  // EEROMDrv->ByteWrite(0xA5, 0x00);
  // EEROMDrv->BufferRead(I2c_Buf_Read, 0x00, DATA_Size);

  // /* 輸出資料 */
  // for (i = 0; i < DATA_Size; i++)
  // {
  //   printf("0x%02X ", I2c_Buf_Read[i]);
  //   if (i % 8 == 7)
  //   {
  //     printf("\r\n");
  //   }
  // }

  bsp_Log_Info("I2C(AT24C02)test ok~");
  return bsp_PASSED;
}

/*********************************************END OF FILE**********************/
