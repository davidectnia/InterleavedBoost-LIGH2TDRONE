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
#include "stm32f3xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define DLL_CALIBRATIONTIMEOUT  ((uint32_t) 10)
#define REPETITON_RATE          ((uint32_t) 7)
/* 4.608 GHz / 400 kHz = 11520 ticks */
#define MULTIPHASE_PERIOD       ((uint32_t) 11520)


/* Tempo di conversione ADC per i dummy triggers (~250 ns a 4.608 GHz = 1152 ticks) */
#define ADC_CONVERSION_TIME     ((uint32_t) 1152)

/* Duty cycle iniziale (es. 25% = 2880 ticks) e sfasamento iniziale (4 fasi = 90 gradi) */
#define DUTY_CYCLE_INIT         ((uint32_t) (MULTIPHASE_PERIOD / 4))
#define PHASE_SHIFT_INIT        ((uint32_t) (MULTIPHASE_PERIOD / 4))

#define BURST_PERIOD            ((uint32_t) 1)
#define BURST_IDLE              ((uint32_t) 0)
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
/* Macro to place ISR and control functions in Core Coupled Memory (0 Wait States) */
#define CCMRAM __attribute__((section(".ccmram")))

/* Fixed-Point Math Macros */
#define FLOAT_TO_Q16(x)  ((int32_t)((x) * 65536.0f))
#define Q16_TO_FLOAT(x)  ((float)(x) / 65536.0f)
/* USER CODE END EM */

void HAL_HRTIM_MspPostInit(HRTIM_HandleTypeDef *hhrtim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MPER 0x27FF
#define DUTY_RATIO 0x1BFF
#define CHC1_PWM3_Pin GPIO_PIN_12
#define CHC1_PWM3_GPIO_Port GPIOB
#define CHD1_PWM4_Pin GPIO_PIN_14
#define CHD1_PWM4_GPIO_Port GPIOB
#define CHA1_PWM1_Pin GPIO_PIN_8
#define CHA1_PWM1_GPIO_Port GPIOA
#define CHB1_PWM2_Pin GPIO_PIN_10
#define CHB1_PWM2_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
