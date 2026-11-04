/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body - 4-Phase Interleaved Boost Converter
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "control_loop.h"
#include "analog_sensing.h"
#include "power_stage.h"
#include "telemetry.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* Limiti operativi del controllo */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
DMA_HandleTypeDef hdma_adc1;
DMA_HandleTypeDef hdma_adc2;

HRTIM_HandleTypeDef hhrtim1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

typedef enum {
    SYS_STATE_CALIBRATION = 0,
    SYS_STATE_OPEN_LOOP,
    SYS_STATE_BUMPLESS_TRANSITION,
    SYS_STATE_CLOSED_LOOP,
    SYS_STATE_FAULT
} SystemState_t;

volatile SystemState_t SystemState = SYS_STATE_CALIBRATION;
/* ========================================================================== */
/* 1. VARIABILI DI SENSING E CALIBRAZIONE (Modificabili da Live Expressions)  */
/* ========================================================================== */

/* Letture ADC grezze (Raw LSB) */
volatile uint16_t PhaseCurrents_ADC2[4];     /* DMA: PH1->PH4 (PA4, PA6, PA7, PC4) */
volatile uint16_t PhaseVoltages_ADC1[4];     /* DMA: PH1->PH4 (PC0, PC1, PC2, PC3) */
volatile uint16_t OutputCurrent_ADC1 = 0;    /* Injected: PB1  (ADC1_IN12) */
volatile uint16_t OutputVoltage_ADC2 = 0;    /* Injected: PC5  (ADC2_IN11) */

/* Grandezze fisiche convertite (Osservabili in Live Expressions per taratura) */


/* ========================================================================== */
/* 2. VARIABILI DI CONTROLLO OPEN / CLOSED LOOP                               */
/* ========================================================================== */
/* NOTA: Parte a 0 (Open Loop) per permettere la taratura sicura dei guadagni.
 * Imposta ClosedLoopRequest = 1 in Live Expressions per passare in Closed Loop
 * con aggancio bumpless e rampa di Soft-Start! */
volatile uint8_t  ClosedLoopEnable  = 0;
volatile uint8_t  ClosedLoopRequest = 0;

volatile uint8_t  NumActivePhases   = 4;     /* Fasi attive: 1, 2, 3 o 4 */
volatile uint8_t  PrevActivePhases  = 0;
volatile float    TargetDutyPercent = 25.0f; /* Duty cycle in Open Loop [%] */


volatile uint32_t debug_cmp_ph1 = 0, debug_cmp_ph2 = 0, debug_cmp_ph3 = 0, debug_cmp_ph4 = 0;
volatile uint32_t IrqCounter    = 0;

volatile uint32_t isr_start_cycles = 0;
volatile uint32_t isr_duration_cycles = 0;


volatile uint16_t Offset_OutputVoltage = 0;
volatile uint16_t Offset_OutputCurrent = 0;
volatile uint16_t Offset_PhaseCurrents[4] = {0, 0, 0, 0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
static void HRTIM_Config(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */
  /* 1. Calibrazione hardware interna di ADC1 e ADC2 in modalità Single-Ended */
  if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED) != HAL_OK)
  {
    Error_Handler();
  }

  char msg_init[] = "\r\nMisurazione automatica offset ADC in corso (64 campioni)...\r\n";
  HAL_UART_Transmit(&huart2, (uint8_t*)msg_init, strlen(msg_init), 100);

  /* 2. Configura temporaneamente gli ADC con Software Trigger per misurare gli offset a PWM spento */
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  HAL_ADC_Init(&hadc1);
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  HAL_ADC_Init(&hadc2);



  uint32_t sum_vout = 0;
  uint32_t sum_iout = 0;
  uint32_t sum_iph[4] = {0, 0, 0, 0};

  for (int s = 0; s < 64; s++)
  {
    /* Misura offset Corrente di Uscita (ADC1 Injected Rank 1 - PB1) */
    HAL_ADCEx_InjectedStart(&hadc1);
    HAL_ADCEx_InjectedPollForConversion(&hadc1, 10);
    sum_iout += HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1);

    /* Misura offset Tensione di Uscita (ADC2 Injected Rank 1 - PC5) */
    HAL_ADCEx_InjectedStart(&hadc2);
    HAL_ADCEx_InjectedPollForConversion(&hadc2, 10);
    sum_vout += HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_1);

    /* Misura offset Correnti di Fase (ADC2 Regular Rank 1..4 in Discontinuous Mode) */
    for (int i = 0; i < 4; i++)
    {
      HAL_ADC_Start(&hadc2);
      HAL_ADC_PollForConversion(&hadc2, 10);
      sum_iph[i] += HAL_ADC_GetValue(&hadc2);
    }
  }

  Offset_OutputVoltage = (uint16_t)(sum_vout / 64);
  Offset_OutputCurrent = (uint16_t)(sum_iout / 64);
  for (int i = 0; i < 4; i++)
  {
    Offset_PhaseCurrents[i] = (uint16_t)(sum_iph[i] / 64);
  }
  AnalogSensing_InitOffsets(Offset_OutputVoltage, Offset_OutputCurrent, (uint16_t*)Offset_PhaseCurrents);
  char msg_ok[160];

  snprintf(msg_ok, sizeof(msg_ok),
           "Offset OK -> Vout:%u | Iout:%u | Iph:[%u,%u,%u,%u]\r\n"
           "OPEN LOOP IS SET (Duty=%.1f%%). Imposta ClosedLoopRequest=1 from Live Expressions to enter Closed Loop.\r\n",
           Offset_OutputVoltage, Offset_OutputCurrent,
           Offset_PhaseCurrents[0], Offset_PhaseCurrents[1],
           Offset_PhaseCurrents[2], Offset_PhaseCurrents[3],
           TargetDutyPercent);
  HAL_UART_Transmit(&huart2, (uint8_t*)msg_ok, strlen(msg_ok), 200);
  SystemState=SYS_STATE_OPEN_LOOP;
  /* 3. Ripristina la configurazione ADC con trigger hardware da HRTIM_TRG1 */
  MX_ADC1_Init();
  MX_ADC2_Init();

  /* 4. Avvia il DMA in modalità Circolare per i canali Regolari */
  if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)PhaseVoltages_ADC1, 4) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_ADC_Start_DMA(&hadc2, (uint32_t *)PhaseCurrents_ADC2, 4) != HAL_OK)
  {
    Error_Handler();
  }

  /* Disabilita gli interrupt DMA non necessari (il campionamento è sincrono col PWM) */
  __HAL_DMA_DISABLE_IT(&hdma_adc1, DMA_IT_HT | DMA_IT_TC | DMA_IT_TE);
  __HAL_DMA_DISABLE_IT(&hdma_adc2, DMA_IT_HT | DMA_IT_TC | DMA_IT_TE);
  HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
  HAL_NVIC_DisableIRQ(DMA1_Channel2_IRQn);

  /* Avvia la prima conversione Injected prima che parta l'HRTIM */
  HAL_ADCEx_InjectedStart(&hadc1);
  HAL_ADCEx_InjectedStart(&hadc2);

  /* 5. Configura e avvia HRTIM1 */
  HRTIM_Config();
  HAL_HRTIM_MspPostInit(&hhrtim1);
  HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TD1);
  HAL_HRTIM_WaveformCountStart_IT(&hhrtim1, HRTIM_TIMERID_MASTER | HRTIM_TIMERID_TIMER_A | HRTIM_TIMERID_TIMER_B | HRTIM_TIMERID_TIMER_C | HRTIM_TIMERID_TIMER_D);

  /* Avvia ricezione UART in interrupt */
  Telemetry_Init(&huart2);

  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* Gestione dinamica del numero di fasi attive (Phase Shedding) */
	PowerStage_UpdatePhases(NumActivePhases);

    /* Gestione comandi da UART (invia il nuovo valore di Vout_Ref seguito da INVIO) */
    Telemetry_Process(&huart2);
    temperature_deg_celsius = check_Temperature(HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_2));

  }
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
}
/* USER CODE END 3 */

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_HRTIM1;
  PeriphClkInit.Hrtim1ClockSelection = RCC_HRTIM1CLK_PLLCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{
  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};
  ADC_InjectionConfTypeDef sConfigInjected = {0};

  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = ENABLE;
  hadc1.Init.NbrOfDiscConversion = 1;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONVHRTIM_TRG1;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 4;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /* Canali Regolari ADC1: Tensioni di fase (SamplingTime aumentato a 19.5 cicli per stabilità S&H) */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.SamplingTime = ADC_SAMPLETIME_19CYCLES_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /* Canale Injected ADC1: Corrente di uscita sul carico (PB1 - ADC1_IN12) */
  sConfigInjected.InjectedChannel = ADC_CHANNEL_12;
  sConfigInjected.InjectedRank = ADC_INJECTED_RANK_1;
  sConfigInjected.InjectedSingleDiff = ADC_SINGLE_ENDED;
  sConfigInjected.InjectedNbrOfConversion = 1;
  sConfigInjected.InjectedSamplingTime = ADC_SAMPLETIME_19CYCLES_5;
  sConfigInjected.ExternalTrigInjecConvEdge = ADC_EXTERNALTRIGINJECCONV_EDGE_RISING;
  sConfigInjected.ExternalTrigInjecConv = ADC_INJECTED_SOFTWARE_START;
  sConfigInjected.AutoInjectedConv = DISABLE;
  sConfigInjected.InjectedDiscontinuousConvMode = DISABLE;
  sConfigInjected.QueueInjectedContext = DISABLE;
  sConfigInjected.InjectedOffset = 0;
  sConfigInjected.InjectedOffsetNumber = ADC_OFFSET_NONE;
  if (HAL_ADCEx_InjectedConfigChannel(&hadc1, &sConfigInjected) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};
  ADC_InjectionConfTypeDef sConfigInjected = {0};

  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.DiscontinuousConvMode = ENABLE;
  hadc2.Init.NbrOfDiscConversion = 1;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc2.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONVHRTIM_TRG1;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 4;
  hadc2.Init.DMAContinuousRequests = ENABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc2.Init.LowPowerAutoWait = DISABLE;
  hadc2.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  /* Canali Regolari ADC2: Correnti di fase PH1..PH4 (SamplingTime aumentato a 19.5 cicli) */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.SamplingTime = ADC_SAMPLETIME_19CYCLES_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /* Canali Injected ADC2: Tensione di uscita (PC5 - IN11) e Temperatura (PB15 - IN15) */
  sConfigInjected.InjectedChannel = ADC_CHANNEL_11;
  sConfigInjected.InjectedRank = ADC_INJECTED_RANK_1;
  sConfigInjected.InjectedSingleDiff = ADC_SINGLE_ENDED;
  sConfigInjected.InjectedNbrOfConversion = 2;
  sConfigInjected.InjectedSamplingTime = ADC_SAMPLETIME_19CYCLES_5;
  sConfigInjected.ExternalTrigInjecConvEdge = ADC_EXTERNALTRIGINJECCONV_EDGE_RISING;
  sConfigInjected.ExternalTrigInjecConv = ADC_INJECTED_SOFTWARE_START;
  sConfigInjected.AutoInjectedConv = DISABLE;
  sConfigInjected.InjectedDiscontinuousConvMode = DISABLE;
  sConfigInjected.QueueInjectedContext = DISABLE;
  sConfigInjected.InjectedOffset = 0;
  sConfigInjected.InjectedOffsetNumber = ADC_OFFSET_NONE;
  if (HAL_ADCEx_InjectedConfigChannel(&hadc2, &sConfigInjected) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigInjected.InjectedChannel = ADC_CHANNEL_15;
  sConfigInjected.InjectedRank = ADC_INJECTED_RANK_2;
  if (HAL_ADCEx_InjectedConfigChannel(&hadc2, &sConfigInjected) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{
  __HAL_RCC_DMA1_CLK_ENABLE();

  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */

/* ========================================================================== */
/* ISR DI CONTROLLO (Eseguita a f_sw / (REPETITON_RATE + 1))                  */
/* ========================================================================== */
void HRTIM1_Master_IRQHandler(void)
{

	isr_start_cycles = DWT->CYCCNT;
    /* 1. Hardware Clear & Trigger Next Conversion */
    __HAL_HRTIM_MASTER_CLEAR_IT(&hhrtim1, HRTIM_MASTER_IT_MREP);

    IrqCounter++;

    OutputCurrent_ADC1 = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1);
    OutputVoltage_ADC2 = HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_1);


    HAL_ADCEx_InjectedStart(&hadc1);
    HAL_ADCEx_InjectedStart(&hadc2);

    /* 2. Sensor Updates (LUT Temperature evaluation happens here) */
    AnalogSensing_Update(OutputVoltage_ADC2, OutputCurrent_ADC1, (uint16_t*)PhaseCurrents_ADC2, NumActivePhases);



    /* 4. State Machine Execution */
    switch (SystemState) {
        case SYS_STATE_OPEN_LOOP:
            if (ClosedLoopRequest) {
                SystemState = SYS_STATE_BUMPLESS_TRANSITION;
            } else {
                ControlLoop_Execute(0, NumActivePhases, Vout_Meas_V, (float*)Iphase_Meas_A, TargetDutyPercent);
                PowerStage_ApplyDutyCycles(Duty_Phase);
            }
            break;

        case SYS_STATE_BUMPLESS_TRANSITION:
            {
                float i_avg = (NumActivePhases > 0) ? (Iphase_Total_A / (float)NumActivePhases) : 0.0f;
                ControlLoop_ResetAll(Duty_Phase[0], i_avg);
                ClosedLoopRequest = 0;
                SystemState = SYS_STATE_CLOSED_LOOP;
            }
            break;

        case SYS_STATE_CLOSED_LOOP:
            ControlLoop_Execute(1, NumActivePhases, Vout_Meas_V, (float*)Iphase_Meas_A, TargetDutyPercent);
            PowerStage_ApplyDutyCycles(Duty_Phase);
            break;

        case SYS_STATE_CALIBRATION:
            /* Await end of software calibration before entering Open Loop */
            break;

        case SYS_STATE_FAULT:
        default:
            PowerStage_EmergencyStop();
            break;
    }

    isr_duration_cycles = DWT->CYCCNT - isr_start_cycles;
}

static void HRTIM_Config(void)
{
  HRTIM_TimeBaseCfgTypeDef timebase_config = {0};
  HRTIM_TimerCfgTypeDef timer_config = {0};
  HRTIM_OutputCfgTypeDef output_config = {0};
  HRTIM_CompareCfgTypeDef compare_config = {0};
  HRTIM_ADCTriggerCfgTypeDef adc_trigger_config = {0};
  HRTIM_FaultCfgTypeDef fault_config = {0};
  HRTIM_BurstModeCfgTypeDef burst_config = {0};

  /* Inizializzazione globale HRTIM1 e calibrazione DLL */
  hhrtim1.Instance = HRTIM1;
  hhrtim1.Init.HRTIMInterruptResquests = HRTIM_IT_NONE;
  hhrtim1.Init.SyncOptions = HRTIM_SYNCOPTION_NONE;
  HAL_HRTIM_Init(&hhrtim1);

  HAL_HRTIM_DLLCalibrationStart(&hhrtim1, HRTIM_CALIBRATIONRATE_14);
  if (HAL_HRTIM_PollForDLLCalibration(&hhrtim1, DLL_CALIBRATIONTIMEOUT) != HAL_OK)
  {
    Error_Handler();
  }

  /* 1. Configurazione Master Timer */
  timebase_config.Period = MULTIPHASE_PERIOD;
  timebase_config.RepetitionCounter = REPETITON_RATE;
  timebase_config.PrescalerRatio = HRTIM_PRESCALERRATIO_MUL32;
  timebase_config.Mode = HRTIM_MODE_CONTINUOUS;
  if (HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_MASTER, &timebase_config) != HAL_OK)
  {
    Error_Handler();
  }

  timer_config.DMARequests = HRTIM_TIM_DMA_NONE;
  timer_config.DMASrcAddress = 0x0;
  timer_config.DMADstAddress = 0x0;
  timer_config.DMASize = 0x0;
  timer_config.HalfModeEnable = HRTIM_HALFMODE_DISABLED;
  timer_config.StartOnSync = HRTIM_SYNCSTART_DISABLED;
  timer_config.ResetOnSync = HRTIM_SYNCRESET_DISABLED;
  timer_config.DACSynchro = HRTIM_DACSYNC_NONE;
  timer_config.PreloadEnable = HRTIM_PRELOAD_ENABLED;
  timer_config.UpdateGating = HRTIM_UPDATEGATING_INDEPENDENT;
  timer_config.BurstMode = HRTIM_TIMERBURSTMODE_MAINTAINCLOCK;
  timer_config.RepetitionUpdate = HRTIM_UPDATEONREPETITION_ENABLED;
  timer_config.InterruptRequests = HRTIM_MASTER_IT_MREP;
  HAL_HRTIM_WaveformTimerConfig(&hhrtim1, HRTIM_TIMERINDEX_MASTER, &timer_config);

  /* 2. Configurazione TimeBase Timer A, B, C, D (unica e senza sovrascritture) */
  timebase_config.Period = MULTIPHASE_PERIOD - 1 - ADC_CONVERSION_TIME;
  timebase_config.RepetitionCounter = REPETITON_RATE;
  timebase_config.PrescalerRatio = HRTIM_PRESCALERRATIO_MUL32;
  timebase_config.Mode = HRTIM_MODE_SINGLESHOT_RETRIGGERABLE;
  HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, &timebase_config);
  HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, &timebase_config);
  HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, &timebase_config);
  HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, &timebase_config);

  /* 3. Configurazione Waveform Timer A, B, C, D */
  timer_config.RepetitionUpdate = HRTIM_UPDATEONREPETITION_DISABLED;
  timer_config.ResetUpdate = HRTIM_TIMUPDATEONRESET_DISABLED;
  timer_config.InterruptRequests = HRTIM_TIM_IT_NONE;
  timer_config.PushPull = HRTIM_TIMPUSHPULLMODE_DISABLED;
  timer_config.FaultEnable = HRTIM_TIMFAULTENABLE_NONE;
  timer_config.FaultLock = HRTIM_TIMFAULTLOCK_READWRITE;
  timer_config.DeadTimeInsertion = HRTIM_TIMDEADTIMEINSERTION_DISABLED;
  timer_config.DelayedProtectionMode = HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DISABLED;
  timer_config.UpdateTrigger = HRTIM_TIMUPDATETRIGGER_MASTER;

  timer_config.ResetTrigger = HRTIM_TIMRESETTRIGGER_MASTER_PER;
  HAL_HRTIM_WaveformTimerConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, &timer_config);

  timer_config.ResetTrigger = HRTIM_TIMRESETTRIGGER_MASTER_CMP1;
  HAL_HRTIM_WaveformTimerConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, &timer_config);

  timer_config.ResetTrigger = HRTIM_TIMRESETTRIGGER_MASTER_CMP2;
  HAL_HRTIM_WaveformTimerConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, &timer_config);

  timer_config.DelayedProtectionMode = HRTIM_TIMER_D_E_DELAYEDPROTECTION_DISABLED;
  timer_config.ResetTrigger = HRTIM_TIMRESETTRIGGER_MASTER_CMP3;
  HAL_HRTIM_WaveformTimerConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, &timer_config);

  /* 4. Configurazione Uscite TA1, TB1, TC1, TD1 */
  output_config.Polarity = HRTIM_OUTPUTPOLARITY_HIGH;
  output_config.SetSource = HRTIM_OUTPUTSET_MASTERPER;
  output_config.ResetSource = HRTIM_OUTPUTRESET_TIMCMP1;
  output_config.IdleMode = HRTIM_OUTPUTIDLEMODE_IDLE;
  output_config.IdleLevel = HRTIM_OUTPUTIDLELEVEL_INACTIVE;
  output_config.FaultLevel = HRTIM_OUTPUTFAULTLEVEL_INACTIVE;
  output_config.ChopperModeEnable = HRTIM_OUTPUTCHOPPERMODE_DISABLED;
  output_config.BurstModeEntryDelayed = HRTIM_OUTPUTBURSTMODEENTRY_REGULAR;
  HAL_HRTIM_WaveformOutputConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA1, &output_config);

  output_config.SetSource = HRTIM_OUTPUTSET_MASTERCMP1;
  HAL_HRTIM_WaveformOutputConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_OUTPUT_TB1, &output_config);

  output_config.SetSource = HRTIM_OUTPUTSET_MASTERCMP2;
  HAL_HRTIM_WaveformOutputConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, HRTIM_OUTPUT_TC1, &output_config);

  output_config.SetSource = HRTIM_OUTPUTSET_MASTERCMP3;
  HAL_HRTIM_WaveformOutputConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_OUTPUT_TD1, &output_config);

  /* 5. Inizializzazione registri Compare per Duty Cycle e Phase Shift */
  compare_config.AutoDelayedMode = HRTIM_AUTODELAYEDMODE_REGULAR;
  compare_config.AutoDelayedTimeout = 0;
  compare_config.CompareValue = DUTY_CYCLE_INIT;
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, &compare_config);
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_1, &compare_config);
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, HRTIM_COMPAREUNIT_1, &compare_config);
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_1, &compare_config);

  compare_config.CompareValue = PHASE_SHIFT_INIT;
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_1, &compare_config);

  compare_config.CompareValue = 2 * PHASE_SHIFT_INIT;
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_2, &compare_config);

  compare_config.CompareValue = 3 * PHASE_SHIFT_INIT;
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_3, &compare_config);

  /* 6. Burst mode configuration */
  burst_config.ClockSource = HRTIM_BURSTMODECLOCKSOURCE_MASTER;
  burst_config.IdleDuration = BURST_IDLE;
  burst_config.Mode = HRTIM_BURSTMODE_CONTINOUS;
  burst_config.Period = BURST_PERIOD;
  burst_config.PreloadEnable = HRIM_BURSTMODEPRELOAD_ENABLED;
  burst_config.Prescaler = HRTIM_BURSTMODEPRESCALER_DIV1;
  burst_config.Trigger = HRTIM_BURSTMODETRIGGER_MASTER_RESET;
  HAL_HRTIM_BurstModeConfig(&hhrtim1, &burst_config);

  /* 7. Configurazione Trigger ADC (campionamento al 50% del Ton + dummy triggers per Phase Shedding) */
  compare_config.AutoDelayedMode = HRTIM_AUTODELAYEDMODE_REGULAR;
  compare_config.CompareValue = DUTY_CYCLE_INIT / 2;
  compare_config.AutoDelayedTimeout = 0;
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_2, &compare_config);
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_2, &compare_config);
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, HRTIM_COMPAREUNIT_2, &compare_config);
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_3, &compare_config);

  /* Dummy trigger 3 fasi */
  compare_config.CompareValue = (MULTIPHASE_PERIOD / 3) - 1 - ADC_CONVERSION_TIME;
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, HRTIM_COMPAREUNIT_3, &compare_config);

  /* Dummy triggers 2 fasi */
  compare_config.CompareValue = (MULTIPHASE_PERIOD / 2) - 1 - ADC_CONVERSION_TIME;
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_3, &compare_config);
  compare_config.CompareValue = (MULTIPHASE_PERIOD / 2) - 1 - (ADC_CONVERSION_TIME * 2);
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_4, &compare_config);

  /* Dummy triggers 1 fase */
  compare_config.CompareValue = MULTIPHASE_PERIOD - 1 - (ADC_CONVERSION_TIME * 3);
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_3, &compare_config);
  compare_config.CompareValue = MULTIPHASE_PERIOD - 1 - (ADC_CONVERSION_TIME * 2);
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_4, &compare_config);

  adc_trigger_config.Trigger = HRTIM_ADCTRIGGEREVENT13_TIMERA_CMP2
                             | HRTIM_ADCTRIGGEREVENT13_TIMERB_CMP2
                             | HRTIM_ADCTRIGGEREVENT13_TIMERC_CMP2
                             | HRTIM_ADCTRIGGEREVENT13_TIMERD_CMP3;
  adc_trigger_config.UpdateSource = HRTIM_ADCTRIGGERUPDATE_MASTER;
  HAL_HRTIM_ADCTriggerConfig(&hhrtim1, HRTIM_ADCTRIGGER_1, &adc_trigger_config);

  /* 8. Configurazione Fault e Interrupt Master */
  fault_config.Filter = HRTIM_FAULTFILTER_NONE;
  fault_config.Lock = HRTIM_FAULTLOCK_READWRITE;
  fault_config.Polarity = HRTIM_FAULTPOLARITY_LOW;
  fault_config.Source = HRTIM_FAULTSOURCE_DIGITALINPUT;
  HAL_HRTIM_FaultConfig(&hhrtim1, HRTIM_FAULT_1, &fault_config);

  HAL_NVIC_SetPriority(HRTIM1_Master_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(HRTIM1_Master_IRQn);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TD1);
  while (1)
  {
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    for (volatile uint32_t i = 0; i < 1500000; i++);
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
