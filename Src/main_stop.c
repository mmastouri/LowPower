/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "main.h"
#include "stm32_seq.h"
#include "stm32_lpm_if.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define ACTIVE_TASK_ID   0x00

/* Private variables ---------------------------------------------------------*/
static TIM_HandleTypeDef    htim;
static RTC_HandleTypeDef    hrtc;
static UART_HandleTypeDef   huart;
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
static void UTIL_AtiveTask( void );

/* Private function prototypes -----------------------------------------------*/
#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

uint32_t count = 0;
/* Private user code ---------------------------------------------------------*/

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  GPIO_InitTypeDef  gpio_init;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;  
  
  uint32_t freq_stop = HAL_RCC_GetSysClockFreq();
    
  /* Init HAL Library */
  HAL_Init();   
  
  /* Configure the system clock to 48 MHz */
  SystemClock_Config();

  /* Enable Power Clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* Ensure that MSI is wake-up system clock */ 
  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);
  
  HAL_RCC_GetOscConfig(&RCC_OscInitStruct);
  RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
  
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
  HAL_NVIC_SetPriority(RTC_WKUP_IRQn, 0x0, 0);
  
  /* Enable the RTC global Interrupt */
  HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);
  __HAL_RCC_RTC_ENABLE();
  
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 0x7F;
  hrtc.Init.SynchPrediv = 0xFF;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  
  HAL_RTC_Init(&hrtc);
  
  HAL_RTCEx_DeactivateWakeUpTimer(&hrtc) ;
  
  /*Init UART */
  __HAL_RCC_USART2_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  
  gpio_init.Pin       = GPIO_PIN_2 | GPIO_PIN_3;
  gpio_init.Mode      = GPIO_MODE_AF_PP;
  gpio_init.Pull      = GPIO_PULLUP;
  gpio_init.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init.Alternate = GPIO_AF7_USART2;
  
  HAL_GPIO_Init(GPIOA, &gpio_init);
  
  
  huart.Instance        = USART2;
  huart.Init.BaudRate   = 9600;
  huart.Init.WordLength = UART_WORDLENGTH_8B;
  huart.Init.StopBits   = UART_STOPBITS_1;
  huart.Init.Parity     = UART_PARITY_NONE;
  huart.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  huart.Init.Mode       = UART_MODE_TX_RX;
  
  HAL_UART_Init(&huart);
  
  /* Init the LED*/  
  BSP_LED_Init(LED_GREEN);
  
  /* Init the BUTTON*/  
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);  
    
  printf ("> Starting.\n\r");
  printf ("      - SysFreq @RESET = %d MHz.\n\r", freq_stop/1000000);   
  printf ("      - SysFreq @RUN = %d MHz.\n\r", HAL_RCC_GetSysClockFreq()/1000000); 

  /* Init the sequencer*/
  UTIL_SEQ_Init();
  
  /* Add tasks*/
  UTIL_SEQ_RegTask(1 << ACTIVE_TASK_ID, 0, UTIL_AtiveTask);
  
  HAL_DBGMCU_EnableDBGStopMode();
  while (1)
  { 
    UTIL_SEQ_Run(UTIL_SEQ_DEFAULT);
  }
}

/**
  * @brief  Idle task.
  * @retval none
  */
void UTIL_SEQ_PreIdle( void )
{  
  HAL_Delay(5000);
  printf ("> Entered in STOP mode [%d].\n\r", count++);
  HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 5 * 0x800, RTC_WAKEUPCLOCK_RTCCLK_DIV16);  
  BSP_LED_Off(LED_GREEN);
  PWR_EnterStopMode(); 
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
   BSP_LED_On(LED_GREEN);
}



/**
  * @brief  Function.
  * @param  None
  * @retval None
  */
void  HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
   
  uint32_t freq_stop = HAL_RCC_GetSysClockFreq();
  SystemClock_Config();
  HAL_RTCEx_DeactivateWakeUpTimer(&hrtc) ;
  UTIL_SEQ_SetTask(1 << ACTIVE_TASK_ID, 0);
 
  PWR_ExitStopMode();

  printf ("> Exited from STOP mode [EXTI].\n\r");
  printf ("       - SysFreq @WU = %d MHz.\n\r", freq_stop/1000000);  
  printf ("       - SysFreq @RUN= %d MHz.\n\r",HAL_RCC_GetSysClockFreq()/1000000);   
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
  * @brief  RTC wakeup timer callback 
  * @param  htim : TIM IC handle
  * @retval None
*/
void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{    
  uint32_t freq_stop = HAL_RCC_GetSysClockFreq();
  SystemClock_Config();
  HAL_RTCEx_DeactivateWakeUpTimer(hrtc) ;
  UTIL_SEQ_SetTask(1 << ACTIVE_TASK_ID, 0);
 
  PWR_ExitStopMode();

  printf ("> Exited from STOP mode [Wakeup].\n\r");
  printf ("      - SysFreq @RESET = %d MHz.\n\r", freq_stop/1000000);   
  printf ("      - SysFreq @RUN = %d MHz.\n\r", HAL_RCC_GetSysClockFreq()/1000000);   
}

/**
  * @brief  This function handles RTC wakeup interrupt request.
  * @param  None
  * @retval None
  */
void RTC_WKUP_IRQHandler(void)
{
  HAL_RTCEx_WakeUpTimerIRQHandler(&hrtc);
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  
  /* MSI is enabled after System reset, activate PLL with MSI as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_4;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 80;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLP = 7;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
  clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }
}


/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART1 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
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
