/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "rtc.h"
#include "sdio.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
MSG_T msg;
uint8_t CAN_TX[3] = {0x33, 0x32, 0x31};
uCAN_MSG txMessage;
uCAN_MSG rxMessage;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void TIM_CallBack1(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern AHT_DrvTypeDef *AHTDrv;
extern IOExpand_DrvTypeDef *IODrv;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  SCB->VTOR = FLASH_BASE | 0x8000;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  MX_RTC_Init();
  MX_FSMC_Init();
  MX_USART2_UART_Init();
  MX_TIM5_Init();
  MX_USART3_UART_Init();
  MX_ADC1_Init();
  MX_SDIO_SD_Init();
  MX_SPI1_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  bsp_Init();

  bsp_LCD_SetTextColor(WHITE);
  bsp_LCD_DispStringLine_EN(LINE(2), "IK-ZET6 2.8 inch LCD para:");
  bsp_LCD_SetFont(&Font16x24);
  bsp_LCD_DispStringLine_EN(LINE(3), "Image resolution:240x320 px");
  bsp_LCD_SetFont(&Font24x32);
  bsp_LCD_DispStringLine_EN(LINE(4), "ILI9341 LCD driver...");
  bsp_LCD_SetFont(&Font8x16);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // bsp_LCD_Test3();
    if (bsp_GetMsg(&msg))
    {
      switch (msg.MsgCode)
      {
      case MSG_LED2_Sparkle:
        /* LED2_ON */
        if (msg.MsgParam == 1)
        {
          HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
        }
        /* LED2_OFF */
        if (msg.MsgParam == 0)
        {
          HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
        }
        break;

      case MSG_Show_TEMPHUM: /* Show_TEMPHUM */
        AHTDrv->ReadTEMPHUM(&AHT10_Humidity, &AHT10_Temperature);
        bsp_ATH_Show();
        HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
        bsp_HardTimer_Start(1, 100, (void *)TIM_CallBack1);
        break;

      case MSG_Show_TimeAndDate:
        bsp_RTC_TimeAndDate_Show();
        break;

      case MSG_CANTX:
        // bsp_CAN_SendPacket(CAN_TX, 3);
        break;

      case MSG_CANRX:
        // bsp_CAN_Analyze();
        break;

      case MSG_WIFIRX:
        bsp_Log_Info("Response:%d", bsp_wifi_WaitResponse("OK", 100));
        break;

      case MSG_SPICANTEST:
        txMessage.frame.idType = dSTANDARD_CAN_MSG_ID_2_0B;
        txMessage.frame.id = 0x100;
        txMessage.frame.dlc = 8;
        txMessage.frame.data0 = 0;
        txMessage.frame.data1 = 1;
        txMessage.frame.data2 = 2;
        txMessage.frame.data3 = 3;
        txMessage.frame.data4 = 4;
        txMessage.frame.data5 = 5;
        txMessage.frame.data6 = 6;
        txMessage.frame.data7 = 32;
        CANSPI_Transmit(&txMessage);
        break;

      default:
        break;
      }
    }

    /* LetterShell Tick */
    if (LetterShell_Recv_flag == 1)
    {
      LetterShell_Recv_flag = 0;
      shellHandler(&shell, LetterShell_Recv);
      HAL_UART_Receive_DMA(&huart1, &LetterShell_Recv, 1);
    }

    if (CANSPI_Receive(&rxMessage) == 1)
    {
      bsp_Log_Info("CANSPI_Receive ok");
      char canmsg[128] = {0};
      char cantemp[32] = {0};
      char len = 0;
      sprintf(cantemp, "CAN StdId : 0x%3x | ", rxMessage.frame.id);
      strcat(canmsg, cantemp);
      sprintf(cantemp, "DLC : %02d | Msg : ", rxMessage.frame.dlc);
      strcat(canmsg, cantemp);
      {
        if (len < rxMessage.frame.dlc)
        {
          sprintf(cantemp, "0x%02x ", rxMessage.frame.data0);
          strcat(canmsg, cantemp);
          len++;
        }

        if (len < rxMessage.frame.dlc)
        {
          sprintf(cantemp, "0x%02x ", rxMessage.frame.data1);
          strcat(canmsg, cantemp);
          len++;
        }

        if (len < rxMessage.frame.dlc)
        {
          sprintf(cantemp, "0x%02x ", rxMessage.frame.data2);
          strcat(canmsg, cantemp);
          len++;
        }

        if (len < rxMessage.frame.dlc)
        {
          sprintf(cantemp, "0x%02x ", rxMessage.frame.data3);
          strcat(canmsg, cantemp);
          len++;
        }

        if (len < rxMessage.frame.dlc)
        {
          sprintf(cantemp, "0x%02x ", rxMessage.frame.data4);
          strcat(canmsg, cantemp);
          len++;
        }

        if (len < rxMessage.frame.dlc)
        {
          sprintf(cantemp, "0x%02x ", rxMessage.frame.data5);
          strcat(canmsg, cantemp);
          len++;
        }

        if (len < rxMessage.frame.dlc)
        {
          sprintf(cantemp, "0x%02x ", rxMessage.frame.data6);
          strcat(canmsg, cantemp);
          len++;
        }

        if (len < rxMessage.frame.dlc)
        {
          sprintf(cantemp, "0x%02x ", rxMessage.frame.data7);
          strcat(canmsg, cantemp);
          len++;
        }
      }
      bsp_Log_Info("%s", canmsg);
    }
    bsp_Idle();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC
                              |RCC_PERIPHCLK_USB;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
static void TIM_CallBack1(void)
{
  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
