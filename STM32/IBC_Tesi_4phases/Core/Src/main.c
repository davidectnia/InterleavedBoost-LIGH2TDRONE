/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
DMA_HandleTypeDef hdma_adc1;
DMA_HandleTypeDef hdma_adc2;

HRTIM_HandleTypeDef hhrtim1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */


/*vairabili per UART*/
uint8_t uart_rx_byte;         /* Singolo carattere ricevuto */
char    uart_rx_buffer[16];   /* Stringa accumulata fino alla pressione di INVIO */
uint8_t uart_rx_index = 0;

/* Variabili modificabili in runtime (Open Loop) */
volatile uint8_t  NumActivePhases = 4;              /* Valori ammessi: 1, 2, 3, 4 */
volatile uint8_t  PrevActivePhases = 0;             /* Usata per rilevare il cambio fase */
volatile float    TargetDutyPercent = 25.0f;        /* Duty cycle impostabile (es. 0.0 - 80.0 %) */
volatile uint32_t CurrentDutyTicks = DUTY_CYCLE_INIT;

volatile uint16_t PhaseCurrents_ADC2[4]; 		/*Accessed by DMA, PH1->PH4, PA4-PA6-PA7-PC4*/
volatile uint16_t PhaseVoltages_ADC1[4];		/*Accessed by DMA, PH1->PH4, PC0-PC1-PC2-PC3*/

/* Injected Channels No DMA, triggered by software */

volatile uint16_t OutputCurrent_ADC1 = 0; /* PB1 (ADC1_IN12) */
volatile uint16_t OutputVoltage_ADC2 = 0; /* PC5 (ADC2_IN11) */
volatile uint16_t Temperature_ADC2   = 0; /* PB15 (ADC2_IN15) */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_HRTIM1_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
static void Update_Boost_Parameters(void);
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

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* Initialize User_Button on STM32F3348-DISCO */

  /* Initialize ADC to be triggered by the HRTIMER */
  //ADC_Config();


  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  //MX_HRTIM1_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  /* 1. Calibrazione hardware di ADC1 e ADC2 in Single-Ended (come in main.c) */
  if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
  {
	  Error_Handler();
  }
  if (HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED) != HAL_OK)
  {
	  Error_Handler();
  }
  /* 2. Avvia il DMA in modalità Circular per i 4 canali Regular di ADC1 e ADC2 */
    /* Da questo momento gli ADC aspettano in automatico i trigger HRTIM_ADCTRIGGER_1 */
  if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)PhaseVoltages_ADC1, 4) != HAL_OK)
  {
	  Error_Handler();
  }
  if (HAL_ADC_Start_DMA(&hadc2, (uint32_t *)PhaseCurrents_ADC2, 4) != HAL_OK)
  {
	  Error_Handler();
  }
  /* DISABILITA GLI INTERRUPT DEL DMA: il DMA continua a copiare i dati in RAM in
       modalità Circular a 400 kHz, ma lascia libera al 100% la CPU e la UART! */


  __HAL_DMA_DISABLE_IT(&hdma_adc1, DMA_IT_HT | DMA_IT_TC | DMA_IT_TE);
  __HAL_DMA_DISABLE_IT(&hdma_adc2, DMA_IT_HT | DMA_IT_TC | DMA_IT_TE);
  HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
  HAL_NVIC_DisableIRQ(DMA1_Channel2_IRQn);

  /* 3. Configura l'HRTIM1 e collega i pin fisici PA8, PA10, PB12, PB14 */
  HRTIM_Config();
  HAL_HRTIM_MspPostInit(&hhrtim1);
  /* Enable HRTIM's outputs TA2, TB1, TC2, TD1, TD2 */
  HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TD1);
  /* Start HRTIM's TIMER A, B, C, D */
  HAL_HRTIM_WaveformCountStart(&hhrtim1, HRTIM_TIMERID_MASTER | HRTIM_TIMERID_TIMER_A | HRTIM_TIMERID_TIMER_B | HRTIM_TIMERID_TIMER_C | HRTIM_TIMERID_TIMER_D);

  char welcome_msg[] = "\r\n--- BOOST 4 FASI PRONTO ---\r\nDigita Duty (1-100) + INVIO (oppure P1..P4 per le fasi):\r\n";
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  /* Aggiorna numero fasi e duty cycle da Live Expressions */
	  Update_Boost_Parameters();

	  HAL_ADCEx_InjectedStart(&hadc1);
	  HAL_ADCEx_InjectedStart(&hadc2);

	      /* 3. Leggi i risultati dai registri dedicati JDR1 e JDR2 */
	  OutputCurrent_ADC1 = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1);
	  OutputVoltage_ADC2 = HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_1);
	  Temperature_ADC2   = HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_2);

	  /* Avvia l'ascolto della tastiera su USART2 (1 carattere alla volta in Interrupt) */
	  HAL_UART_Receive_IT(&huart2, &uart_rx_byte, 1);

	  HAL_Delay(10); /* Controlla modifiche ogni 10 ms */
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

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};
  ADC_InjectionConfTypeDef sConfigInjected = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
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

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Injected Channel
  */
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
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};
  ADC_InjectionConfTypeDef sConfigInjected = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */

  /** Common config
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.DiscontinuousConvMode = ENABLE;
  hadc2.Init.NbrOfDiscConversion = 1;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
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

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Injected Channel
  */
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

  /** Configure Injected Channel
  */
  sConfigInjected.InjectedChannel = ADC_CHANNEL_15;
  sConfigInjected.InjectedRank = ADC_INJECTED_RANK_2;
  if (HAL_ADCEx_InjectedConfigChannel(&hadc2, &sConfigInjected) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

/**
  * @brief HRTIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_HRTIM1_Init(void)
{

  /* USER CODE BEGIN HRTIM1_Init 0 */

  /* USER CODE END HRTIM1_Init 0 */

  HRTIM_ADCTriggerCfgTypeDef pADCTriggerCfg = {0};
  HRTIM_TimeBaseCfgTypeDef pTimeBaseCfg = {0};
  HRTIM_TimerCfgTypeDef pTimerCfg = {0};
  HRTIM_CompareCfgTypeDef pCompareCfg = {0};
  HRTIM_OutputCfgTypeDef pOutputCfg = {0};

  /* USER CODE BEGIN HRTIM1_Init 1 */

  /* USER CODE END HRTIM1_Init 1 */
  hhrtim1.Instance = HRTIM1;
  hhrtim1.Init.HRTIMInterruptResquests = HRTIM_IT_NONE;
  hhrtim1.Init.SyncOptions = HRTIM_SYNCOPTION_NONE;
  if (HAL_HRTIM_Init(&hhrtim1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_HRTIM_DLLCalibrationStart(&hhrtim1, HRTIM_CALIBRATIONRATE_14) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_HRTIM_PollForDLLCalibration(&hhrtim1, 10) != HAL_OK)
  {
    Error_Handler();
  }
  pADCTriggerCfg.UpdateSource = HRTIM_ADCTRIGGERUPDATE_MASTER;
  pADCTriggerCfg.Trigger = HRTIM_ADCTRIGGEREVENT13_NONE;
  if (HAL_HRTIM_ADCTriggerConfig(&hhrtim1, HRTIM_ADCTRIGGER_1, &pADCTriggerCfg) != HAL_OK)
  {
    Error_Handler();
  }
  pTimeBaseCfg.Period = MPER;
  pTimeBaseCfg.RepetitionCounter = 0x20;
  pTimeBaseCfg.PrescalerRatio = HRTIM_PRESCALERRATIO_MUL32;
  pTimeBaseCfg.Mode = HRTIM_MODE_CONTINUOUS;
  if (HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_MASTER, &pTimeBaseCfg) != HAL_OK)
  {
    Error_Handler();
  }
  pTimerCfg.InterruptRequests = HRTIM_MASTER_IT_NONE;
  pTimerCfg.DMARequests = HRTIM_MASTER_DMA_NONE;
  pTimerCfg.DMASrcAddress = 0x0000;
  pTimerCfg.DMADstAddress = 0x0000;
  pTimerCfg.DMASize = 0x1;
  pTimerCfg.HalfModeEnable = HRTIM_HALFMODE_DISABLED;
  pTimerCfg.StartOnSync = HRTIM_SYNCSTART_DISABLED;
  pTimerCfg.ResetOnSync = HRTIM_SYNCRESET_DISABLED;
  pTimerCfg.DACSynchro = HRTIM_DACSYNC_NONE;
  pTimerCfg.PreloadEnable = HRTIM_PRELOAD_DISABLED;
  pTimerCfg.UpdateGating = HRTIM_UPDATEGATING_INDEPENDENT;
  pTimerCfg.BurstMode = HRTIM_TIMERBURSTMODE_MAINTAINCLOCK;
  pTimerCfg.RepetitionUpdate = HRTIM_UPDATEONREPETITION_DISABLED;
  if (HAL_HRTIM_WaveformTimerConfig(&hhrtim1, HRTIM_TIMERINDEX_MASTER, &pTimerCfg) != HAL_OK)
  {
    Error_Handler();
  }
  pCompareCfg.CompareValue = 0xFFDF;
  if (HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_1, &pCompareCfg) != HAL_OK)
  {
    Error_Handler();
  }
  pTimeBaseCfg.Mode = HRTIM_MODE_SINGLESHOT_RETRIGGERABLE;
  if (HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, &pTimeBaseCfg) != HAL_OK)
  {
    Error_Handler();
  }
  pTimerCfg.InterruptRequests = HRTIM_TIM_IT_NONE;
  pTimerCfg.DMARequests = HRTIM_TIM_DMA_NONE;
  pTimerCfg.PushPull = HRTIM_TIMPUSHPULLMODE_DISABLED;
  pTimerCfg.FaultEnable = HRTIM_TIMFAULTENABLE_NONE;
  pTimerCfg.FaultLock = HRTIM_TIMFAULTLOCK_READWRITE;
  pTimerCfg.DeadTimeInsertion = HRTIM_TIMDEADTIMEINSERTION_DISABLED;
  pTimerCfg.DelayedProtectionMode = HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DISABLED;
  pTimerCfg.UpdateTrigger = HRTIM_TIMUPDATETRIGGER_NONE;
  pTimerCfg.ResetTrigger = HRTIM_TIMRESETTRIGGER_NONE;
  pTimerCfg.ResetUpdate = HRTIM_TIMUPDATEONRESET_DISABLED;
  if (HAL_HRTIM_WaveformTimerConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, &pTimerCfg) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_HRTIM_WaveformTimerConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, &pTimerCfg) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_HRTIM_WaveformTimerConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, &pTimerCfg) != HAL_OK)
  {
    Error_Handler();
  }
  pTimerCfg.DelayedProtectionMode = HRTIM_TIMER_D_E_DELAYEDPROTECTION_DISABLED;
  if (HAL_HRTIM_WaveformTimerConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, &pTimerCfg) != HAL_OK)
  {
    Error_Handler();
  }
  pOutputCfg.Polarity = HRTIM_OUTPUTPOLARITY_HIGH;
  pOutputCfg.SetSource = HRTIM_OUTPUTSET_NONE;
  pOutputCfg.ResetSource = HRTIM_OUTPUTRESET_NONE;
  pOutputCfg.IdleMode = HRTIM_OUTPUTIDLEMODE_NONE;
  pOutputCfg.IdleLevel = HRTIM_OUTPUTIDLELEVEL_INACTIVE;
  pOutputCfg.FaultLevel = HRTIM_OUTPUTFAULTLEVEL_NONE;
  pOutputCfg.ChopperModeEnable = HRTIM_OUTPUTCHOPPERMODE_DISABLED;
  pOutputCfg.BurstModeEntryDelayed = HRTIM_OUTPUTBURSTMODEENTRY_REGULAR;
  if (HAL_HRTIM_WaveformOutputConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA1, &pOutputCfg) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_HRTIM_WaveformOutputConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_OUTPUT_TB1, &pOutputCfg) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_HRTIM_WaveformOutputConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, HRTIM_OUTPUT_TC1, &pOutputCfg) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_HRTIM_WaveformOutputConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_OUTPUT_TD1, &pOutputCfg) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, &pTimeBaseCfg) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, &pTimeBaseCfg) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, &pTimeBaseCfg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN HRTIM1_Init 2 */

  /* USER CODE END HRTIM1_Init 2 */
  HAL_HRTIM_MspPostInit(&hhrtim1);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
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
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel2_IRQn interrupt configuration */
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
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
static void HRTIM_Config(void)
{
  HRTIM_TimeBaseCfgTypeDef timebase_config = {0};
  HRTIM_TimerCfgTypeDef timer_config = {0};
  HRTIM_OutputCfgTypeDef output_config = {0};
  HRTIM_CompareCfgTypeDef compare_config = {0};
  HRTIM_ADCTriggerCfgTypeDef adc_trigger_config = {0};
  HRTIM_FaultCfgTypeDef fault_config = {0};
  HRTIM_BurstModeCfgTypeDef burst_config = {0};

  /* ----------------------------*/
  /* HRTIM Global initialization */
  /* ----------------------------*/
  hhrtim1.Instance = HRTIM1;
  hhrtim1.Init.HRTIMInterruptResquests = HRTIM_IT_NONE;
  hhrtim1.Init.SyncOptions = HRTIM_SYNCOPTION_NONE;

  if (HAL_HRTIM_Init(&hhrtim1) != HAL_OK)
  {
    Error_Handler();
  }

  /* HRTIM DLL calibration: periodic calibration, set period to 14 us */
  HAL_HRTIM_DLLCalibrationStart(&hhrtim1, HRTIM_CALIBRATIONRATE_14);
  if (HAL_HRTIM_PollForDLLCalibration(&hhrtim1, DLL_CALIBRATIONTIMEOUT) != HAL_OK)
  {
    Error_Handler();
  }

  /* ---------------------------------------------------------------- */
  /* 1. MASTER TIMER TIMEBASE (Quello che era stato sovrascritto!)    */
  /* ---------------------------------------------------------------- */
  timebase_config.Period = MULTIPHASE_PERIOD;
  timebase_config.RepetitionCounter = REPETITON_RATE;
  timebase_config.PrescalerRatio = HRTIM_PRESCALERRATIO_MUL32;
  timebase_config.Mode = HRTIM_MODE_CONTINUOUS;
  if (HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_MASTER, &timebase_config) != HAL_OK)
  {
    Error_Handler();
  }

  /* ---------------------------------------------------------- */
  /* 2. MASTER TIMER WAVEFORM CONFIG                            */
  /* ---------------------------------------------------------- */
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
  timer_config.InterruptRequests = HRTIM_MASTER_IT_NONE; /* NONE per Open Loop */
  HAL_HRTIM_WaveformTimerConfig(&hhrtim1, HRTIM_TIMERINDEX_MASTER, &timer_config);

  /* -------------------------------------------------------------- */
  /* 3. TIMERA, TIMERB, TIMERC, TIMERD TIMEBASE                     */
  /* -------------------------------------------------------------- */
  timebase_config.Period = MULTIPHASE_PERIOD - 1 - ADC_CONVERSION_TIME;
  timebase_config.RepetitionCounter = REPETITON_RATE;
  timebase_config.PrescalerRatio = HRTIM_PRESCALERRATIO_MUL32;
  timebase_config.Mode = HRTIM_MODE_SINGLESHOT_RETRIGGERABLE;
  HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, &timebase_config);
  HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, &timebase_config);
  HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, &timebase_config);
  HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, &timebase_config);

  /* --------------------------------------------------------------------- */
  /* 4. TIMERA, TIMERB, TIMERC, TIMERD WAVEFORM CONFIG                     */
  /* --------------------------------------------------------------------- */
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

  /* -------------------------------------------- */
  /* 5. CONFIGURAZIONE USCITE: TA1, TB1, TC1, TD1 */
  /* -------------------------------------------- */
  output_config.Polarity = HRTIM_OUTPUTPOLARITY_HIGH;
  output_config.SetSource = HRTIM_OUTPUTSET_MASTERPER;
  output_config.ResetSource = HRTIM_OUTPUTRESET_TIMCMP1;
  output_config.IdleMode = HRTIM_OUTPUTIDLEMODE_IDLE;
  output_config.IdleLevel = HRTIM_OUTPUTIDLELEVEL_INACTIVE;
  output_config.FaultLevel = HRTIM_OUTPUTFAULTLEVEL_INACTIVE;
  output_config.ChopperModeEnable = HRTIM_OUTPUTCHOPPERMODE_DISABLED;
  output_config.BurstModeEntryDelayed = HRTIM_OUTPUTBURSTMODEENTRY_REGULAR;

  /* Fase 1: TA1 (PA8) */
  HAL_HRTIM_WaveformOutputConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA1, &output_config);

  /* Fase 2: TB1 (PA10) */
  output_config.SetSource = HRTIM_OUTPUTSET_MASTERCMP1;
  output_config.ResetSource = HRTIM_OUTPUTRESET_TIMCMP1;
  HAL_HRTIM_WaveformOutputConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_OUTPUT_TB1, &output_config);

  /* Fase 3: TC1 (PB12) */
  output_config.SetSource = HRTIM_OUTPUTSET_MASTERCMP2;
  output_config.ResetSource = HRTIM_OUTPUTRESET_TIMCMP1;
  HAL_HRTIM_WaveformOutputConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, HRTIM_OUTPUT_TC1, &output_config);

  /* Fase 4: TD1 (PB14) */
  output_config.SetSource = HRTIM_OUTPUTSET_MASTERCMP3;
  output_config.ResetSource = HRTIM_OUTPUTRESET_TIMCMP1;
  HAL_HRTIM_WaveformOutputConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_OUTPUT_TD1, &output_config);

  /* ------------------------------------------------------------------- */
  /* 6. DUTY CYCLE (COMPAREUNIT_1) - Quello che era stato sovrascritto!  */
  /* ------------------------------------------------------------------- */
  compare_config.AutoDelayedMode = HRTIM_AUTODELAYEDMODE_REGULAR;
  compare_config.AutoDelayedTimeout = 0;
  compare_config.CompareValue = DUTY_CYCLE_INIT;

  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, &compare_config);
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_1, &compare_config);
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, HRTIM_COMPAREUNIT_1, &compare_config);
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_1, &compare_config);

  /* ------------------------------------------------------------------- */
  /* 7. SFASAMENTI SUL MASTER TIMER (MCMP1, MCMP2, MCMP3)                */
  /* ------------------------------------------------------------------- */
  compare_config.CompareValue = PHASE_SHIFT_INIT;
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_1, &compare_config);

  compare_config.CompareValue = 2 * PHASE_SHIFT_INIT;
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_2, &compare_config);

  compare_config.CompareValue = 3 * PHASE_SHIFT_INIT;
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_3, &compare_config);

  /* -------------------------------------------------- */
  /* 8. BURST MODE CONFIG                               */
  /* -------------------------------------------------- */
  burst_config.ClockSource = HRTIM_BURSTMODECLOCKSOURCE_MASTER;
  burst_config.IdleDuration = BURST_IDLE;
  burst_config.Mode = HRTIM_BURSTMODE_CONTINOUS;
  burst_config.Period = BURST_PERIOD;
  burst_config.PreloadEnable = HRIM_BURSTMODEPRELOAD_ENABLED;
  burst_config.Prescaler = HRTIM_BURSTMODEPRESCALER_DIV1;
  burst_config.Trigger = HRTIM_BURSTMODETRIGGER_MASTER_RESET;
  HAL_HRTIM_BurstModeConfig(&hhrtim1, &burst_config);

  /* -------------------------------------------------------------- */
  /* 9. TRIGGER ADC A META' TON + DUMMY TRIGGERS                    */
  /* -------------------------------------------------------------- */
  compare_config.AutoDelayedMode = HRTIM_AUTODELAYEDMODE_REGULAR;
  compare_config.AutoDelayedTimeout = 0;
  compare_config.CompareValue = DUTY_CYCLE_INIT / 2;

  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_2, &compare_config);
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_2, &compare_config);
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, HRTIM_COMPAREUNIT_2, &compare_config);
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_3, &compare_config);

  /* Dummy trigger per 3 fasi */
  compare_config.CompareValue = (MULTIPHASE_PERIOD / 3) - 1 - ADC_CONVERSION_TIME;
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, HRTIM_COMPAREUNIT_3, &compare_config);

  /* Dummy triggers per 2 fasi */
  compare_config.CompareValue = (MULTIPHASE_PERIOD / 2) - 1 - ADC_CONVERSION_TIME;
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_3, &compare_config);

  compare_config.CompareValue = (MULTIPHASE_PERIOD / 2) - 1 - (ADC_CONVERSION_TIME * 2);
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_4, &compare_config);

  /* Dummy triggers per 1 fase */
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

  /* ---------------------*/
  /* 10. FAULT CONFIG     */
  /* ---------------------*/
  fault_config.Filter = HRTIM_FAULTFILTER_NONE;
  fault_config.Lock = HRTIM_FAULTLOCK_READWRITE;
  fault_config.Polarity = HRTIM_FAULTPOLARITY_LOW;
  fault_config.Source = HRTIM_FAULTSOURCE_DIGITALINPUT;
  HAL_HRTIM_FaultConfig(&hhrtim1, HRTIM_FAULT_1, &fault_config);
}

static void Update_Boost_Parameters(void)
{
  HRTIM_ADCTriggerCfgTypeDef adc_trigger_config;
  float clamped_duty = TargetDutyPercent;

  /* 1. Sicurezza sul Duty Cycle (limita tra 2.0% e 80.0%) */
  if (clamped_duty < 2.0f)  clamped_duty = 2.0f;
  if (clamped_duty > 80.0f) clamped_duty = 80.0f;

  /* Converti da percentuale a ticks dell'HRTIM (su base 11520) */
  uint32_t new_duty_ticks = (uint32_t)((clamped_duty / 100.0f) * ((float)MULTIPHASE_PERIOD));

  /* Se il duty cycle è stato cambiato in Live Expressions, aggiorna i registri */
  if (new_duty_ticks != CurrentDutyTicks)
  {
    CurrentDutyTicks = new_duty_ticks;
    uint32_t half_ton_ticks = CurrentDutyTicks / 2; /* Nuovo punto a 50% di Ton per l'ADC */

    /* Aggiorna Duty Cycle (CMP1) su tutte e 4 le fasi */
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, CurrentDutyTicks);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_1, CurrentDutyTicks);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, HRTIM_COMPAREUNIT_1, CurrentDutyTicks);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_1, CurrentDutyTicks);

    /* Aggiorna l'istante di campionamento ADC per restare sempre a metà Ton */
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_2, half_ton_ticks);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_2, half_ton_ticks);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, HRTIM_COMPAREUNIT_2, half_ton_ticks);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_3, half_ton_ticks);
  }

  /* 2. Gestione cambio numero di fasi (1, 2, 3 o 4) */
  if (NumActivePhases != PrevActivePhases)
  {
    adc_trigger_config.UpdateSource = HRTIM_ADCTRIGGERUPDATE_MASTER;

    switch (NumActivePhases)
    {
      case 1: /* 1 FASE ATTIVA: Solo PH1 (TA1) */
        /* Spegni PH2, PH3, PH4 e mantieni accesa PH1 */
        HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TD1);
        HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1);

        /* Trigger ADC: 1 reale (TA_CMP2) + 3 dummy (TA_CMP3, TA_CMP4, TA_PER) */
        adc_trigger_config.Trigger = HRTIM_ADCTRIGGEREVENT13_TIMERA_CMP2
                                   | HRTIM_ADCTRIGGEREVENT13_TIMERA_CMP3
                                   | HRTIM_ADCTRIGGEREVENT13_TIMERA_CMP4
                                   | HRTIM_ADCTRIGGEREVENT13_TIMERA_PERIOD;
        HAL_HRTIM_ADCTriggerConfig(&hhrtim1, HRTIM_ADCTRIGGER_1, &adc_trigger_config);
        PrevActivePhases = 1;
        break;

      case 2: /* 2 FASI ATTIVE: PH1 e PH2 sfasate di 180° (MULTIPHASE_PERIOD / 2) */
        /* Sfasamento PH2 a 180 gradi */
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_1, (MULTIPHASE_PERIOD / 2));

        /* Spegni PH3, PH4 e abilita PH1, PH2 */
        HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TD1);
        HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TB1);

        /* Trigger ADC: 2 reali (TA_CMP2, TB_CMP2) + 2 dummy (TB_CMP3, TB_CMP4) */
        adc_trigger_config.Trigger = HRTIM_ADCTRIGGEREVENT13_TIMERA_CMP2
                                   | HRTIM_ADCTRIGGEREVENT13_TIMERB_CMP2
                                   | HRTIM_ADCTRIGGEREVENT13_TIMERB_CMP3
                                   | HRTIM_ADCTRIGGEREVENT13_TIMERB_CMP4;
        HAL_HRTIM_ADCTriggerConfig(&hhrtim1, HRTIM_ADCTRIGGER_1, &adc_trigger_config);
        PrevActivePhases = 2;
        break;

      case 3: /* 3 FASI ATTIVE: PH1, PH2, PH3 sfasate di 120° (MULTIPHASE_PERIOD / 3) */
        /* Sfasamento PH2 a 120° e PH3 a 240° */
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_1, (MULTIPHASE_PERIOD / 3));
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_2, 2 * (MULTIPHASE_PERIOD / 3));

        /* Spegni PH4 e abilita PH1, PH2, PH3 */
        HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TD1);
        HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TC1);

        /* Trigger ADC: 3 reali (TA_CMP2, TB_CMP2, TC_CMP2) + 1 dummy (TC_CMP3) */
        adc_trigger_config.Trigger = HRTIM_ADCTRIGGEREVENT13_TIMERA_CMP2
                                   | HRTIM_ADCTRIGGEREVENT13_TIMERB_CMP2
                                   | HRTIM_ADCTRIGGEREVENT13_TIMERC_CMP2
                                   | HRTIM_ADCTRIGGEREVENT13_TIMERC_CMP3;
        HAL_HRTIM_ADCTriggerConfig(&hhrtim1, HRTIM_ADCTRIGGER_1, &adc_trigger_config);
        PrevActivePhases = 3;
        break;

      case 4: /* 4 FASI ATTIVE: PH1, PH2, PH3, PH4 sfasate di 90° (MULTIPHASE_PERIOD / 4) */
      default:
        NumActivePhases = 4; /* Se scrivi per sbaglio un numero fuori range, forza 4 fasi */
        /* Sfasamento PH2 a 90°, PH3 a 180°, PH4 a 270° */
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_1, (MULTIPHASE_PERIOD / 4));
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_2, 2 * (MULTIPHASE_PERIOD / 4));
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_3, 3 * (MULTIPHASE_PERIOD / 4));

        /* Abilita tutte e 4 le uscite */
        HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TD1);

        /* Trigger ADC: 4 reali (TA_CMP2, TB_CMP2, TC_CMP2, TD_CMP3) */
        adc_trigger_config.Trigger = HRTIM_ADCTRIGGEREVENT13_TIMERA_CMP2
                                   | HRTIM_ADCTRIGGEREVENT13_TIMERB_CMP2
                                   | HRTIM_ADCTRIGGEREVENT13_TIMERC_CMP2
                                   | HRTIM_ADCTRIGGEREVENT13_TIMERD_CMP3;
        HAL_HRTIM_ADCTriggerConfig(&hhrtim1, HRTIM_ADCTRIGGER_1, &adc_trigger_config);
        PrevActivePhases = 4;
        break;
    }
  }
}

/* Callback chiamata in automatico ogni volta che premi un tasto sulla tastiera */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2)
  {
    /* Se l'utente ha premuto INVIO (\r o \n) */
    if (uart_rx_byte == '\r' || uart_rx_byte == '\n')
    {
      if (uart_rx_index > 0) /* Se c'è almeno un carattere scritto */
      {
        uart_rx_buffer[uart_rx_index] = '\0'; /* Chiudi la stringa */
        char msg_out[64];

        /* Controlla se l'utente vuole cambiare il numero di fasi (es. "P2" o "p2") */
        if (uart_rx_buffer[0] == 'P' || uart_rx_buffer[0] == 'p')
        {
          int phases = atoi(&uart_rx_buffer[1]);
          if (phases >= 1 && phases <= 4)
          {
            NumActivePhases = (uint8_t)phases;
            snprintf(msg_out, sizeof(msg_out), "\r\n-> Fasi attive impostate a: %d\r\n", NumActivePhases);
            HAL_UART_Transmit(&huart2, (uint8_t*)msg_out, strlen(msg_out), 50);
          }
        }
        else
        {
          /* Altrimenti interpreta l'input come Duty Cycle (da 1 a 100%) */
          float val = atof(uart_rx_buffer);
          if (val >= 1.0f && val <= 100.0f)
          {
            TargetDutyPercent = val;
            snprintf(msg_out, sizeof(msg_out), "\r\n-> Nuovo Duty Cycle: %d%%\r\n", (int)TargetDutyPercent);
            HAL_UART_Transmit(&huart2, (uint8_t*)msg_out, strlen(msg_out), 50);
          }
          else
          {
            char err_msg[] = "\r\nErrore: inserisci un valore tra 1 e 100!\r\n";
            HAL_UART_Transmit(&huart2, (uint8_t*)err_msg, strlen(err_msg), 50);
          }
        }
        uart_rx_index = 0; /* Azzera il buffer per il prossimo comando */
      }
    }
    else
    {
      /* Rimanda il carattere a schermo (Echo) così vedi cosa stai digitando */
      HAL_UART_Transmit(&huart2, &uart_rx_byte, 1, 10);

      /* Salva il carattere nel buffer se c'è spazio */
      if (uart_rx_index < sizeof(uart_rx_buffer) - 1)
      {
        uart_rx_buffer[uart_rx_index++] = (char)uart_rx_byte;
      }
    }

    /* Ri-attiva subito l'ascolto per il prossimo tasto premuto */
    HAL_UART_Receive_IT(&huart2, &uart_rx_byte, 1);
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
/* Spegne subito tutte le uscite PWM per sicurezza in caso di errore */
  HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TD1);
  /* User can add his own implementation to report the HAL error return state */
  //__disable_irq();
  while (1)
  {
	  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	  for (volatile uint32_t i = 0; i < 1500000; i++);
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
