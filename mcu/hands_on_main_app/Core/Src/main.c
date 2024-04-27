/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "aes.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "arm_math.h"
#include "adc_dblbuf.h"
#include "retarget.h"
#include "s2lp.h"
#include "spectrogram.h"
#include "eval_radio.h"
#include "eval_specgram.h"
#include "packet.h"
#include "config.h"
#include "utils.h"
#include "usart.h"

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

volatile uint8_t btn_press;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == B1_Pin) {
		btn_press = 1;
	}
	else if (GPIO_Pin == RADIO_INT_Pin)
		S2LP_IRQ_Handler();
}

static void acquire_and_send_packet() {
	if (StartADCAcq(N_MELVECS) != HAL_OK) {
		DEBUG_PRINT("Error while enabling the DMA\r\n");
	}
	while (!IsADCFinished()) {
		FLASH->ACR |= (1 << 14);
		__WFI();
		FLASH->ACR &= ~(1 << 14);
	}
}

void run(void)
{

	while (1)
	{
#if (CONTINUOUS_ACQ == 1)
		acquire_and_send_packet();
#elif (CONTINUOUS_ACQ == 0)
	if (btn_press){
	  acquire_and_send_packet();
	  btn_press = 0;
	}
#else
#error "Wrong value for CONTINUOUS_ACQ."
#endif
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  RCC->AHB1SMENR &= ~(1 << 9); // deactivate SRAM1 (has to be after the RCC is initialized) (SystemClock_Config() initialized it)
  PWR->CR1 |= (1 << 14); // set the low power run bit (LPR)
  HAL_DBGMCU_DisableDBGStopMode();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_TIM3_Init();
  MX_ADC1_Init();
  MX_AES_Init();
  /* USER CODE BEGIN 2 */
  if (ENABLE_UART) {
	  MX_LPUART1_UART_Init();
  }

  RetargetInit(&hlpuart1);
  DEBUG_PRINT("Hello world\r\n");

#if ENABLE_RADIO
  // Enable S2LP Radio
  HAL_StatusTypeDef err = S2LP_Init(&hspi1);
  if (err)  {
	  DEBUG_PRINT("[S2LP] Error while initializing: %u\r\n", err);
	  Error_Handler();
  } else {
	  DEBUG_PRINT("[S2LP] Init OK\r\n");
  }
#endif

  if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK) {
	  DEBUG_PRINT("Error while calibrating the ADC\r\n");
	  Error_Handler();
  }
  if (HAL_TIM_Base_Start(&htim3) != HAL_OK) {
	  DEBUG_PRINT("Error while enabling timer TIM3\r\n");
	  Error_Handler();
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
#if (RUN_CONFIG == MAIN_APP)
  run();
#elif (RUN_CONFIG == EVAL_RADIO)
  eval_radio();
#elif (RUN_CONFIG == EVAL_SPECGRAM)
#if CONTINUOUS_ACQ
  eval_spectrogram();
#else
  while(1){
	  if (btn_press){
		  eval_spectrogram();
		  btn_press = 0;
	  }
  }
#endif
#else
#error "Wrong value for RUN_CONFIG."
#endif

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_5;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();
  DEBUG_PRINT("Entering error Handler\r\n");
  while (1)
  {
	  // Blink LED3 (red)
	  HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_SET);
	  for (volatile int i=0; i < SystemCoreClock/200; i++);
	  HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_RESET);
	  for (volatile int i=0; i < SystemCoreClock/200; i++);
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
     ex: DEBUG_PRINT("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
