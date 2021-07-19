/**
  ******************************************************************************
  * @file    utilities_conf.h
  * @author  MCD Application Team
  * @brief   Header for configuration file to utilities
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UTILITIES_CONF_H__
#define __UTILITIES_CONF_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "cmsis_compiler.h"
#include "string.h"

/**
  * @brief macro used to initialize the critical section
  */
#define UTIL_SEQ_INIT_CRITICAL_SECTION( )    UTILS_INIT_CRITICAL_SECTION()

/**
  * @brief macro used to enter the critical section
  */
#define UTIL_SEQ_ENTER_CRITICAL_SECTION( )   UTILS_ENTER_CRITICAL_SECTION()

/**
  * @brief macro used to exit the critical section
  */
#define UTIL_SEQ_EXIT_CRITICAL_SECTION( )    UTILS_EXIT_CRITICAL_SECTION()

/**
  * @brief Memset utilities interface to application
  */
#define UTIL_SEQ_MEMSET8( dest, value, size )   memset( dest, value, size )

/**
  * @brief macro used to initialize the critical section
  */
#define UTILS_INIT_CRITICAL_SECTION()

/**
  * @brief macro used to enter the critical section
  */
#define UTILS_ENTER_CRITICAL_SECTION() uint32_t primask_bit= __get_PRIMASK();\
  __disable_irq()

/**
  * @brief macro used to exit the critical section
  */
#define UTILS_EXIT_CRITICAL_SECTION()  __set_PRIMASK(primask_bit)


/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /*__UTILITIES_CONF_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
