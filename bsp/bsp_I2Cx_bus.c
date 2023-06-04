#include "bsp_I2Cx_bus.h"

#ifdef HAL_I2C_MODULE_ENABLED

/* 僅供內部使用 */
#define bsp_USE_I2CxDMA 0
static void bsp_I2Cx_bus_Error(I2C_HandleTypeDef *hi2c, uint16_t devaddress);

/****************************************************************************************************/

/**
 * @brief  Writes a single data.
 * @param  devaddress: I2C address
 * @param  reg: Register address
 * @param  value: Data to be written
 * @retval None
 */
HAL_StatusTypeDef bsp_I2Cx_bus_Write_8bit(I2C_HandleTypeDef *hi2c, uint16_t devaddress, uint8_t reg, uint8_t value)
{
  HAL_StatusTypeDef status = HAL_OK;

#if defined(BSP_USE_OS)
  /* Get semaphore to prevent multiple I2C access */
  tx_mutex_get(&AppI2CSemp, TX_WAIT_FOREVER);
#endif

  status = HAL_I2C_Mem_Write(hi2c, devaddress, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 100);

#if defined(BSP_USE_OS)
  /* Release semaphore to prevent multiple I2C access */
  tx_mutex_put(&AppI2CSemp);
#endif

  /* Check the communication status */
  if (status != HAL_OK)
  {
    /* Execute user timeout callback */
    bsp_I2Cx_bus_Error(hi2c, devaddress);
  }
  return status;
}

/**
 * @brief  Writes a single data.
 * @param  devaddress: I2C address
 * @param  reg: Register address
 * @param  value: Data to be written
 * @retval None
 */
HAL_StatusTypeDef bsp_I2Cx_bus_Write_16bit(I2C_HandleTypeDef *hi2c, uint16_t devaddress, uint16_t reg, uint8_t value)
{
  HAL_StatusTypeDef status = HAL_OK;

#if defined(BSP_USE_OS)
  /* Get semaphore to prevent multiple I2C access */
  tx_mutex_get(&AppI2CSemp, TX_WAIT_FOREVER);
#endif

  status = HAL_I2C_Mem_Write(hi2c, devaddress, reg, I2C_MEMADD_SIZE_16BIT, &value, 1, 100);

#if defined(BSP_USE_OS)
  /* Release semaphore to prevent multiple I2C access */
  tx_mutex_put(&AppI2CSemp);
#endif

  /* Check the communication status */
  if (status != HAL_OK)
  {
    /* Execute user timeout callback */
    bsp_I2Cx_bus_Error(hi2c, devaddress);
  }
  return status;
}

/**
 * @brief  Writes a value in a register of the device through BUS in using DMA mode.
 * @param  devaddress: Device address on BUS Bus.
 * @param  reg: The target register address to write
 * @param  MemAddress: memory address to be written
 * @param  buffer: The target register value to be written
 * @param  length: buffer size to be written
 * @retval HAL status
 */
HAL_StatusTypeDef bsp_I2Cx_bus_WriteMultiple_8bit(I2C_HandleTypeDef *hi2c, uint16_t devaddress, uint8_t reg, uint8_t *buffer, uint16_t length)
{
  HAL_StatusTypeDef status = HAL_OK;

#if defined(BSP_USE_OS)
  /* Get semaphore to prevent multiple I2C access */
  tx_mutex_get(&AppI2CSemp, TX_WAIT_FOREVER);
#endif

#if (bsp_USE_I2CxDMA == 0)
  status = HAL_I2C_Mem_Write(hi2c, devaddress, reg, I2C_MEMADD_SIZE_8BIT, buffer, length, 1000);
#elif (bsp_USE_I2CxDMA == 1)
  status = HAL_I2C_Mem_Write_DMA(hi2c, devaddress, reg, I2C_MEMADD_SIZE_8BIT, buffer, length);
  while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY)
  {
    bsp_Delay(1);
  }
#endif // bsp_USE_I2CxDMA

#if defined(BSP_USE_OS)
  /* Release semaphore to prevent multiple I2C access */
  tx_mutex_put(&AppI2CSemp);
#endif

  /* Check the communication status */
  if (status != HAL_OK)
  {
    /* Re-Initiaize the I2C Bus */
    bsp_I2Cx_bus_Error(hi2c, devaddress);
  }
  return status;
}

/**
 * @brief  Writes a value in a register of the device through BUS in using DMA mode.
 * @param  devaddress: Device address on BUS Bus.
 * @param  reg: The target register address to write
 * @param  MemAddress: memory address to be written
 * @param  buffer: The target register value to be written
 * @param  length: buffer size to be written
 * @retval HAL status
 */
HAL_StatusTypeDef bsp_I2Cx_bus_WriteMultiple_16bit(I2C_HandleTypeDef *hi2c, uint16_t devaddress, uint16_t reg, uint8_t *buffer, uint16_t length)
{
  HAL_StatusTypeDef status = HAL_OK;

#if defined(BSP_USE_OS)
  /* Get semaphore to prevent multiple I2C access */
  tx_mutex_get(&AppI2CSemp, TX_WAIT_FOREVER);
#endif

#if (bsp_USE_I2CxDMA == 0)
  status = HAL_I2C_Mem_Write(hi2c, devaddress, reg, I2C_MEMADD_SIZE_16BIT, buffer, length, 1000);
#elif (bsp_USE_I2CxDMA == 1)
  status = HAL_I2C_Mem_Write_DMA(hi2c, devaddress, reg, I2C_MEMADD_SIZE_16BIT, buffer, length);
  while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY)
  {
    bsp_Delay(1);
  }
#endif // bsp_USE_I2CxDMA

#if defined(BSP_USE_OS)
  /* Release semaphore to prevent multiple I2C access */
  tx_mutex_put(&AppI2CSemp);
#endif

  /* Check the communication status */
  if (status != HAL_OK)
  {
    /* Re-Initiaize the I2C Bus */
    bsp_I2Cx_bus_Error(hi2c, devaddress);
  }
  return status;
}

/**
 * @brief  Reads a single data.
 * @param  devaddress: I2C address
 * @param  reg: Register address
 * @retval Read data
 */
uint8_t bsp_I2Cx_bus_Read_8bit(I2C_HandleTypeDef *hi2c, uint16_t devaddress, uint8_t reg)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint8_t value = 0;

#if defined(BSP_USE_OS)
  /* Get semaphore to prevent multiple I2C access */
  tx_mutex_get(&AppI2CSemp, TX_WAIT_FOREVER);
#endif

  status = HAL_I2C_Mem_Read(hi2c, devaddress, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 1000);

#if defined(BSP_USE_OS)
  /* Release semaphore to prevent multiple I2C access */
  tx_mutex_put(&AppI2CSemp);
#endif

  /* Check the communication status */
  if (status != HAL_OK)
  {
    /* Execute user timeout callback */
    bsp_I2Cx_bus_Error(hi2c, devaddress);
  }
  return value;
}

/**
 * @brief  Reads a single data.
 * @param  devaddress: I2C address
 * @param  reg: Register address
 * @retval Read data
 */
uint8_t bsp_I2Cx_bus_Read_16bit(I2C_HandleTypeDef *hi2c, uint16_t devaddress, uint16_t reg)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint8_t value = 0;

#if defined(BSP_USE_OS)
  /* Get semaphore to prevent multiple I2C access */
  tx_mutex_get(&AppI2CSemp, TX_WAIT_FOREVER);
#endif

  status = HAL_I2C_Mem_Read(hi2c, devaddress, reg, I2C_MEMADD_SIZE_16BIT, &value, 1, 1000);

#if defined(BSP_USE_OS)
  /* Release semaphore to prevent multiple I2C access */
  tx_mutex_put(&AppI2CSemp);
#endif

  /* Check the communication status */
  if (status != HAL_OK)
  {
    /* Execute user timeout callback */
    bsp_I2Cx_bus_Error(hi2c, devaddress);
  }
  return value;
}

/**
 * @brief  Reads multiple data.
 * @param  devaddress: I2C address
 * @param  reg: reg address
 * @param  MemAddress: memory address to be read
 * @param  buffer: Pointer to data buffer
 * @param  length: length of the data
 * @retval Number of read data
 */
HAL_StatusTypeDef bsp_I2Cx_bus_ReadMultiple_8bit(I2C_HandleTypeDef *hi2c, uint16_t devaddress, uint8_t reg, uint8_t *buffer, uint16_t length)
{
  HAL_StatusTypeDef status = HAL_OK;

#if defined(BSP_USE_OS)
  /* Get semaphore to prevent multiple I2C access */
  tx_mutex_get(&AppI2CSemp, TX_WAIT_FOREVER);
#endif

#if (bsp_USE_I2CxDMA == 0)
  status = HAL_I2C_Mem_Read(hi2c, devaddress, reg, I2C_MEMADD_SIZE_8BIT, buffer, length, 1000);
#elif (bsp_USE_I2CxDMA == 1)
  // SCB_CleanInvalidateDCache();
  status = HAL_I2C_Mem_Read_DMA(hi2c, devaddress, reg, I2C_MEMADD_SIZE_8BIT, buffer, length);
  while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY)
  {
    bsp_Delay(1);
  }
#endif // bsp_USE_I2CxDMA

#if defined(BSP_USE_OS)
  /* Release semaphore to prevent multiple I2C access */
  tx_mutex_put(&AppI2CSemp);
#endif

  /* Check the communication status */
  if (status != HAL_OK)
  {
    /* I2C error occurred */
    bsp_I2Cx_bus_Error(hi2c, devaddress);
  }
  return status;
}

/**
 * @brief  Reads multiple data.
 * @param  devaddress: I2C address
 * @param  reg: reg address
 * @param  MemAddress: memory address to be read
 * @param  buffer: Pointer to data buffer
 * @param  length: length of the data
 * @retval Number of read data
 */
HAL_StatusTypeDef bsp_I2Cx_bus_ReadMultiple_16bit(I2C_HandleTypeDef *hi2c, uint16_t devaddress, uint16_t reg, uint8_t *buffer, uint16_t length)
{
  HAL_StatusTypeDef status = HAL_OK;

#if defined(BSP_USE_OS)
  /* Get semaphore to prevent multiple I2C access */
  tx_mutex_get(&AppI2CSemp, TX_WAIT_FOREVER);
#endif

#if (bsp_USE_I2CxDMA == 0)
  status = HAL_I2C_Mem_Read(hi2c, devaddress, reg, I2C_MEMADD_SIZE_16BIT, buffer, length, 1000);
#elif (bsp_USE_I2CxDMA == 1)
  // SCB_CleanInvalidateDCache();
  status = HAL_I2C_Mem_Read_DMA(hi2c, devaddress, reg, I2C_MEMADD_SIZE_16BIT, buffer, length);
  while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY)
  {
    bsp_Delay(1);
  }
#endif // bsp_USE_I2CxDMA

#if defined(BSP_USE_OS)
  /* Release semaphore to prevent multiple I2C access */
  tx_mutex_put(&AppI2CSemp);
#endif

  /* Check the communication status */
  if (status != HAL_OK)
  {
    /* I2C error occurred */
    bsp_I2Cx_bus_Error(hi2c, devaddress);
  }
  return status;
}

/**
 * @brief  Receives in master mode an amount of data in blocking mode.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *                the configuration information for the specified I2C.
 * @param  devaddress Target device address: The device 7 bits address value
 *         in datasheet must be shifted to the left before calling the interface
 * @param  buffer Pointer to data buffer
 * @param  length Amount of data to be sent
 * @retval HAL status
 */
HAL_StatusTypeDef bsp_I2Cx_bus_Master_Read(I2C_HandleTypeDef *hi2c, uint16_t devaddress, uint8_t *buffer, uint16_t length, uint32_t timeout)
{
  HAL_StatusTypeDef status = HAL_OK;

#if defined(BSP_USE_OS)
  /* Get semaphore to prevent multiple I2C access */
  tx_mutex_get(&AppI2CSemp, TX_WAIT_FOREVER);
#endif

#if (bsp_USE_I2CxDMA == 0)
  status = HAL_I2C_Master_Receive(hi2c, devaddress, buffer, length, timeout);
#elif (bsp_USE_I2CxDMA == 1)
  // SCB_CleanInvalidateDCache();
  status = HAL_I2C_Master_Receive_DMA(hi2c, devaddress, buffer, length);
  while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY)
  {
    bsp_Delay(1);
  }
#endif // bsp_USE_I2CxDMA
#if defined(BSP_USE_OS)
  /* Release semaphore to prevent multiple I2C access */
  tx_mutex_put(&AppI2CSemp);
#endif

  /* Check the communication status */
  if (status != HAL_OK)
  {
    /* I2C error occurred */
    bsp_I2Cx_bus_Error(hi2c, devaddress);
  }

  return status;
}

/**
 * @brief  Transmits in master mode an amount of data in blocking mode.
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *                the configuration information for the specified I2C.
 * @param  devaddress Target device address: The device 7 bits address value
 *         in datasheet must be shifted to the left before calling the interface
 * @param  buffer Pointer to data buffer
 * @param  length Amount of data to be sent
 * @retval HAL status
 */
HAL_StatusTypeDef bsp_I2Cx_bus_Master_Write(I2C_HandleTypeDef *hi2c, uint16_t devaddress, uint8_t *buffer, uint16_t length, uint32_t timeout)
{
  HAL_StatusTypeDef status = HAL_OK;

#if defined(BSP_USE_OS)
  /* Get semaphore to prevent multiple I2C access */
  tx_mutex_get(&AppI2CSemp, TX_WAIT_FOREVER);
#endif

#if (bsp_USE_I2CxDMA == 0)
  status = HAL_I2C_Master_Transmit(hi2c, devaddress, buffer, length, timeout);
#elif (bsp_USE_I2CxDMA == 1)
  // SCB_CleanInvalidateDCache();
  status = HAL_I2C_Master_Transmit_DMA(hi2c, devaddress, buffer, length);
  while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY)
  {
    bsp_Delay(1);
  }
#endif // bsp_USE_I2CxDMA

#if defined(BSP_USE_OS)
  /* Release semaphore to prevent multiple I2C access */
  tx_mutex_put(&AppI2CSemp);
#endif

  /* Check the communication status */
  if (status != HAL_OK)
  {
    /* I2C error occurred */
    bsp_I2Cx_bus_Error(hi2c, devaddress);
  }

  return status;
}

/**
 * @brief  Checks if target device is ready for communication.
 * @note   This function is used with Memory devices
 * @param  DevAddress: Target device address
 * @param  Trials: Number of trials
 * @retval HAL status
 */
HAL_StatusTypeDef bsp_I2Cx_bus_IsDeviceReady(I2C_HandleTypeDef *hi2c, uint16_t devaddress, uint32_t trials, uint32_t timeout)
{
  HAL_StatusTypeDef status = HAL_OK;

#if defined(BSP_USE_OS)
  /* Get semaphore to prevent multiple I2C access */
  tx_mutex_get(&AppI2CSemp, TX_WAIT_FOREVER);
#endif

  status = HAL_I2C_IsDeviceReady(hi2c, devaddress, trials, timeout);

#if defined(BSP_USE_OS)
  /* Release semaphore to prevent multiple I2C access */
  tx_mutex_put(&AppI2CSemp);
#endif
  
  return status;
}

/**
 * @brief  Manages error callback by re-initializing I2C.
 * @param  devaddress: I2C Address
 * @retval None
 */
static void bsp_I2Cx_bus_Error(I2C_HandleTypeDef *hi2c, uint16_t devaddress)
{
  bsp_Log_Error("device add(0x%x) happen error", devaddress);
  /* De-initialize the I2C communication bus */

  /* Re-Initialize the I2C communication bus */
  if (hi2c->Instance == I2C1)
  {
    HAL_I2C_DeInit(hi2c);
    __I2C1_FORCE_RESET();
    __I2C1_RELEASE_RESET();
    //MX_I2C1_Init();
    bsp_Log_Info("I2C1 Reset");
  }
  else if (hi2c->Instance == I2C2)
  {
    HAL_I2C_DeInit(hi2c);
    __I2C2_FORCE_RESET();
    __I2C2_RELEASE_RESET();
    MX_I2C2_Init();
    bsp_Log_Info("I2C2 Reset");
  }
#if defined(BSP_USE_OS)
  /* Release semaphore to prevent multiple I2C access */
  tx_mutex_put(&AppI2CSemp);
#endif
}

#if (bsp_USE_I2CxDMA == 1)

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  if (hi2c->Instance == I2C1)
  {
  }
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  if (hi2c->Instance == I2C1)
  {
  }
}
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  if (hi2c->Instance == I2C2)
  {
  }
}
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  if (hi2c->Instance == I2C2)
  {
  }
}
#endif // bsp_USE_I2CxDMA
#endif // HAL_I2C_MODULE_ENABLED
