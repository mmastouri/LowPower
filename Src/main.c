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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32_seq.h"
#include "stm32_lpm_if.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
//#define LP_STOP
#define LP_SLEEP
/* Private macro -------------------------------------------------------------*/
#define ACTIVE_TASK_ID   0x00

/* Private variables ---------------------------------------------------------*/
static TIM_HandleTypeDef    htim;
void TIM3_IRQHandler(void);

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
static void UTIL_AtiveTask( void );

/* Private user code ---------------------------------------------------------*/

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* Init HAL Library */
  HAL_Init();
   
  /* Configure the system clock to 48 MHz */
  SystemClock_Config();
  
  /* Configure timer*/
  __HAL_RCC_TIM3_CLK_ENABLE();
  HAL_NVIC_SetPriority(TIM3_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(TIM3_IRQn);  
  
  // 2 HZ = N/Presc
  htim.Instance = TIM3;
  htim.Init.Period = SystemCoreClock/2000 - 1;
  htim.Init.Prescaler = 1000 - 1;
  htim.Init.ClockDivision = 0;
  htim.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  HAL_TIM_Base_Init(&htim); 
  
  HAL_TIM_Base_Start_IT(&htim);
  
  /* Init the LED*/  
  BSP_LED_Init(LED_GREEN);
  
  /* Init the BUTTON*/  
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);  
  
  /* Init the LPM*/  
  UTIL_LPM_Init();
  
  /* Init the sequencer*/
  UTIL_SEQ_Init();

  /* Add tasks*/
  UTIL_SEQ_RegTask(1 << ACTIVE_TASK_ID, 0, UTIL_AtiveTask);
  
  while (1)
  { 
    UTIL_SEQ_Run(UTIL_SEQ_DEFAULT);
  }
}

/**
  * @brief  Idle task.
  * @retval none
  */
void UTIL_SEQ_Idle( void )
{
#ifdef LP_SLEEP  
  PWR_EnterSleepMode();
#endif

#ifdef LP_STOP  
  PWR_EnterStopMode();
#endif
  
}

/**
  * @brief  The working task.
  * @retval none
  */
/**
  * @brief  Active task.
  * @retval none
  */
static void UTIL_AtiveTask( void )
{
   BSP_LED_Toggle(LED_GREEN);
}

/**
  * @brief  Function.
  * @param  None
  * @retval None
  */
void TIM3_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim);
}

/**
  * @brief  Function.
  * @param  None
  * @retval None
  */
void  HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
   UTIL_SEQ_SetTask(1 << ACTIVE_TASK_ID, 0);
#ifdef LP_SLEEP  
  PWR_ExitSleepMode();
#endif

#ifdef LP_STOP  
  PWR_ExitStopMode();
#endif
}

/**
  * @brief  Function.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler (USER_BUTTON_PIN); 
}

/**
  * @brief  Function.
  * @param  None
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
   UTIL_SEQ_SetTask(1 << ACTIVE_TASK_ID, 0);
#ifdef LP_SLEEP  
  PWR_ExitSleepMode();
#endif

#ifdef LP_STOP  
  PWR_ExitStopMode();
#endif
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_HIGH);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
