/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BTN_UP_Pin GPIO_PIN_6
#define BTN_UP_GPIO_Port GPIOA
#define BTN_DOWN_Pin GPIO_PIN_7
#define BTN_DOWN_GPIO_Port GPIOA
#define BTN_SELECT_Pin GPIO_PIN_0
#define BTN_SELECT_GPIO_Port GPIOB
#define EFUSE_OFF_Pin GPIO_PIN_13
#define EFUSE_OFF_GPIO_Port GPIOB
#define EFUSE_READY_Pin GPIO_PIN_14
#define EFUSE_READY_GPIO_Port GPIOB
#define EFUSE_nFLT_Pin GPIO_PIN_15
#define EFUSE_nFLT_GPIO_Port GPIOB
#define CFG3_Pin GPIO_PIN_8
#define CFG3_GPIO_Port GPIOA
#define CFG1_Pin GPIO_PIN_6
#define CFG1_GPIO_Port GPIOC
#define CFG2_Pin GPIO_PIN_7
#define CFG2_GPIO_Port GPIOC
#define PD_PG_Pin GPIO_PIN_15
#define PD_PG_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
