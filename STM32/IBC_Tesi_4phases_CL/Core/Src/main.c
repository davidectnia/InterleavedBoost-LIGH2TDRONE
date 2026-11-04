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
typedef struct {
  /* Coefficienti MATLAB: b = [b0, b1, b2], a = [1, a1, a2] */
  float b0;
  float b1;
  float b2;
  float a1; /* Nota: a0 = 1.0 (primo elemento del vettore 'a' di MATLAB) */
  float a2;

  /* Memorie degli stati precedenti: e[k-1], e[k-2], u[k-1], u[k-2] */
  float e1;
  float e2;
  float u1;
  float u2;

  /* Limiti di saturazione (Anti-Windup) */
  float u_min;
  float u_max;
} control_elem_discrete;
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

/* Variabili per UART */
uint8_t uart_rx_byte;         /* Singolo carattere ricevuto */
char    uart_rx_buffer[16];   /* Stringa accumulata fino alla pressione di INVIO */
uint8_t uart_rx_index = 0;
volatile uint8_t uart_rx_ready = 0;
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

/* Variabili per calibrazione Offset ADC (0 = Attesa Offset, 1 = Offset OK / Attesa Fonte, 2 = Controllo Attivo) */
volatile uint8_t  ClosedLoopEnable = 1;
volatile uint8_t  ClosedLoopRequest = 0;   /* settato dalla UART, consumato dall'ISR */
volatile uint8_t  uart_status = 0;         /* 0 = niente, 1 = ok, 2 = errore */
volatile float    uart_new_ref = 0.0f;


volatile uint8_t  CalibState = 0;
volatile uint16_t Offset_OutputVoltage = 0;
volatile uint16_t Offset_PhaseCurrents[4] = {0, 0, 0, 0};
/* Riferimento desiderato per l'anello chiuso (modificabile da Live Expressions) */

volatile float    Vout_Ref = 44.4f;       /* 44.4 V reference voltage */
volatile float    I_Ref = 0.0f; /* Uscita dell'anello di tensione (visibile in Live Expressions) */
volatile float    Duty_Phase[4] = {0.25f, 0.25f, 0.25f, 0.25f}; /* Duty calcolati per le 4 fasi */
volatile uint32_t IrqCounter = 0;         /* Contatore per verificare in Live Expressions che l'ISR giri */

#define ADC_TO_VOLTS  (0.0135385f) /* Guadagno Volt/LSB su PC5 per la tensione di uscita */
#define ADC_TO_AMPS   (0.021f) /* Sostituisci col tuo guadagno Ampere/LSB su PA4, PA6, PA7, PC4 */

/*Outer Voltage Loop PI digital filter coefficients*/
#define BV0  ( 5.6143e+04f) /* <-- b0_v */
#define BV1  ( 1.6273e+03f) /* <-- b1_v */
#define BV2  (-5.4516e+04f) /* <-- b2_v */
#define AV1  (-1.9099e+05f) /* <-- a1_v */
#define AV2  (-4.5070e+03f) /* <-- a2_v */
#define I_PHASE_MAX (8.0f) /* Corrente massima ammissibile per singola fase [A], massima corrente in uscita da PEMFC*/

/* --- COEFFICIENTI MATLAB ANELLO INTERNO DI CORRENTE: b_i = [b0, b1, b2], a_i = [1, a1, a2] --- */
#define BI0  ( 1.6062e+03f) /* <-- b0_i */
#define BI1  ( 168.3186f)   /* <-- b1_i */
#define BI2  (-1.4379e+03f) /* <-- b2_i */
#define AI1  (-2.2867e+05f) /* <-- a1_i */
#define AI2  ( 1.4335e+04f) /* <-- a2_i */
#define DUTY_MIN (0.02f) /* Duty cycle minimo = 2% */
#define DUTY_MAX (0.77f) /* Duty cycle massimo = 77% */


#define RAMP_STEP_V  (0.1f)               /* Incremento di 0.1 V ogni 10 ms (10 V/s) */
volatile float Vout_Ramp = 0.0f;          /* Tensione di riferimento istantanea applicata al PI */
uint32_t last_ramp_tick = 0;
/* Istanza del regolatore di Tensione (1 esterno) */
control_elem_discrete VoltageLoop = {
  .b0 = BV0, .b1 = BV1, .b2 = BV2,
  .a1 = AV1, .a2 = AV2,
  .e1 = 0.0f, .e2 = 0.0f, .u1 = 0.0f, .u2 = 0.0f,
  .u_min = 0.0f, .u_max = I_PHASE_MAX
};

/* Istanze dei 4 regolatori di Corrente indipendenti (PH1, PH2, PH3, PH4) */
control_elem_discrete CurrentLoop[4] = {
  { .b0 = BI0, .b1 = BI1, .b2 = BI2, .a1 = AI1, .a2 = AI2, .u_min = DUTY_MIN, .u_max = DUTY_MAX },
  { .b0 = BI0, .b1 = BI1, .b2 = BI2, .a1 = AI1, .a2 = AI2, .u_min = DUTY_MIN, .u_max = DUTY_MAX },
  { .b0 = BI0, .b1 = BI1, .b2 = BI2, .a1 = AI1, .a2 = AI2, .u_min = DUTY_MIN, .u_max = DUTY_MAX },
  { .b0 = BI0, .b1 = BI1, .b2 = BI2, .a1 = AI1, .a2 = AI2, .u_min = DUTY_MIN, .u_max = DUTY_MAX }
};

volatile uint32_t debug_cmp_ph1 = 0, debug_cmp_ph2 = 0, debug_cmp_ph3 = 0, debug_cmp_ph4 = 0; /* Variabile globale e volatile visibile sempre */
uint32_t last_telemetry_tick = 0;
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
static void Update_Phases(void);
static void HRTIM_Config(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* Calcola l'uscita u[k] del filtro digitale e aggiorna gli stati */
static inline float Controller_Update(control_elem_discrete *ctrl_var, float error)
{
  /* Equazione alle differenze: u[k] = b0*e[k] + b1*e[k-1] + b2*e[k-2] - a1*u[k-1] - a2*u[k-2] */
  float u0 = (ctrl_var->b0 * error)
           + (ctrl_var->b1 * ctrl_var->e1)
           + (ctrl_var->b2 * ctrl_var->e2)
           - (ctrl_var->a1 * ctrl_var->u1)
           - (ctrl_var->a2 * ctrl_var->u2);

  /* Saturazione e Anti-Windup (limita u0 prima di salvarlo nella storia) */
  if (u0 > ctrl_var->u_max) u0 = ctrl_var->u_max;
  if (u0 < ctrl_var->u_min) u0 = ctrl_var->u_min;

  /* Scorrimento della memoria per il prossimo ciclo di interrupt */
  ctrl_var->e2 = ctrl_var->e1;
  ctrl_var->e1 = error;
  ctrl_var->u2 = ctrl_var->u1;
  ctrl_var->u1 = u0;

  return u0;
}

/* Azzera la memoria del filtro (fondamentale per le fasi spente!) */
static inline void Controller_Reset(control_elem_discrete *ctrl_var, float init_output)
{
  ctrl_var->e1 = 0.0f;
  ctrl_var->e2 = 0.0f;
  ctrl_var->u1 = init_output;
  ctrl_var->u2 = init_output;
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
 // MX_HRTIM1_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

    /* 1. Hardware calibration of ADC1 and ADC2 in Single-Ended mode */
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
    {
      Error_Handler();
    }
    if (HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED) != HAL_OK)
    {
      Error_Handler();
    }

    char msg_init[] = "\r\nMisurazione automatica offset in corso...\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)msg_init, strlen(msg_init), 100);

    /* 2. Temporarily configure ADCs for Software Trigger to measure offsets without HRTIM/DMA */
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    HAL_ADC_Init(&hadc1);
    hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    HAL_ADC_Init(&hadc2);

    uint32_t sum_v = 0, sum_i[4] = {0};
    for (int s = 0; s < 64; s++)
    {
      /* Software trigger injected channels for Output Voltage */
      HAL_ADCEx_InjectedStart(&hadc2);
      HAL_ADCEx_InjectedPollForConversion(&hadc2, 10);
      sum_v += HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_1);

      /* Software trigger regular channels for Phase Currents (4 channels, discontinuous mode = 4 starts) */
      for (int i = 0; i < 4; i++)
      {
        HAL_ADC_Start(&hadc2);
        HAL_ADC_PollForConversion(&hadc2, 10);
        sum_i[i] += HAL_ADC_GetValue(&hadc2);
      }
    }

    Offset_OutputVoltage = (uint16_t)(sum_v / 64);
    for (int i = 0; i < 4; i++)
    {
      Offset_PhaseCurrents[i] = (uint16_t)(sum_i[i] / 64);
    }

    char msg_ok[128];
    snprintf(msg_ok, sizeof(msg_ok), "Offset registrati -> Vout: %u | Iph: [%u,%u,%u,%u]\r\nCollega la fonte. Controllo attivo!\r\n",
             Offset_OutputVoltage, Offset_PhaseCurrents[0], Offset_PhaseCurrents[1], Offset_PhaseCurrents[2], Offset_PhaseCurrents[3]);
    HAL_UART_Transmit(&huart2, (uint8_t*)msg_ok, strlen(msg_ok), 200);

    /* 3. Restore original ADC configurations (Hardware triggers) by re-calling MX functions */
    MX_ADC1_Init();
    MX_ADC2_Init();

    /* 4. Start DMA in Circular mode for the 4 Regular channels */
    /* From this point, the ADCs will automatically wait for HRTIM triggers */
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)PhaseVoltages_ADC1, 4) != HAL_OK)
    {
      Error_Handler();
    }
    if (HAL_ADC_Start_DMA(&hadc2, (uint32_t *)PhaseCurrents_ADC2, 4) != HAL_OK)
    {
      Error_Handler();
    }

    /* Disable unnecessary DMA interrupts */
    __HAL_DMA_DISABLE_IT(&hdma_adc1, DMA_IT_HT | DMA_IT_TC | DMA_IT_TE);
    __HAL_DMA_DISABLE_IT(&hdma_adc2, DMA_IT_HT | DMA_IT_TC | DMA_IT_TE);
    HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
    HAL_NVIC_DisableIRQ(DMA1_Channel2_IRQn);

    /* 5. Configure HRTIM1 and link physical pins (Triggers start firing NOW) */
    HRTIM_Config();
    HAL_HRTIM_MspPostInit(&hhrtim1);
    HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TD1);
    HAL_HRTIM_WaveformCountStart_IT(&hhrtim1, HRTIM_TIMERID_MASTER | HRTIM_TIMERID_TIMER_A | HRTIM_TIMERID_TIMER_B | HRTIM_TIMERID_TIMER_C | HRTIM_TIMERID_TIMER_D);

    /* Start UART listening */
    HAL_UART_Receive_IT(&huart2, &uart_rx_byte, 1);

    /* USER CODE END 2 */
  /* Infinite loop */
/* USER CODE BEGIN WHILE */
  while (1)
  {


    Update_Phases();

    /* Soft-Start update every 10 ms */
    if (HAL_GetTick() - last_ramp_tick >= 10)
    {
      last_ramp_tick = HAL_GetTick();

      if (Vout_Ramp < Vout_Ref)
      {
        Vout_Ramp += RAMP_STEP_V;
        if (Vout_Ramp > Vout_Ref) Vout_Ramp = Vout_Ref;
      }
      else if (Vout_Ramp > Vout_Ref)
      {
        Vout_Ramp -= RAMP_STEP_V;
        if (Vout_Ramp < Vout_Ref) Vout_Ramp = Vout_Ref;
      }
    }

    /* Telemetry output every 500 ms */
    if (HAL_GetTick() - last_telemetry_tick >= 500)
    {
      last_telemetry_tick = HAL_GetTick();

      if (ClosedLoopEnable)
      {
        int32_t v_raw = (int32_t)OutputVoltage_ADC2 - (int32_t)Offset_OutputVoltage;
        if (v_raw < 0) v_raw = 0;
        float v_val = (float)v_raw * ADC_TO_VOLTS;

        int32_t i1_raw = (int32_t)PhaseCurrents_ADC2[0] - (int32_t)Offset_PhaseCurrents[0];
        if (i1_raw < 0) i1_raw = 0;
        float i1_val = (float)i1_raw * ADC_TO_AMPS;


      }
    }

    HAL_Delay(10);
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
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2)
  {
    if (uart_rx_byte == '\r' || uart_rx_byte == '\n')
    {
      if (uart_rx_index > 0)
      {
        uart_rx_buffer[uart_rx_index] = '\0';
        for (uint8_t i = 0; i < uart_rx_index; i++)
          if (uart_rx_buffer[i] == ',') uart_rx_buffer[i] = '.';

        uart_rx_ready = 1; /* Signal the main loop to process */
      }
    }
    else if (uart_rx_index < sizeof(uart_rx_buffer) - 1)
    {
      uart_rx_buffer[uart_rx_index++] = (char)uart_rx_byte;
    }

    HAL_UART_Receive_IT(&huart2, &uart_rx_byte, 1);
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2)
  {
    __HAL_UART_CLEAR_OREFLAG(huart);
    __HAL_UART_CLEAR_FEFLAG(huart);
    __HAL_UART_CLEAR_NEFLAG(huart);
    HAL_UART_Receive_IT(&huart2, &uart_rx_byte, 1);
  }
}
/* Routine di Interrupt del Master Timer (eseguita a f_sw / (REPETITON_RATE + 1)) */
void HRTIM1_Master_IRQHandler(void)
{
  /* 1. Pulisci subito il flag di interrupt del Master Timer */
  __HAL_HRTIM_MASTER_CLEAR_IT(&hhrtim1, HRTIM_MASTER_IT_MREP);

  IrqCounter++; /* Se questo numero sale in Live Expressions, l'interrupt funziona! */

  /* 2. Leggi i valori già convertiti dai canali Injected (Tensione/Corrente uscita e Temp) */
  OutputCurrent_ADC1 = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1);
  OutputVoltage_ADC2 = HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_1);
  Temperature_ADC2   = HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_2);

  /* 3. Ri-avvia subito la conversione Injected per il prossimo ciclo di interrupt */
  HAL_ADCEx_InjectedStart(&hadc1);
  HAL_ADCEx_InjectedStart(&hadc2);


  if (ClosedLoopRequest && !ClosedLoopEnable)
  {
    /* Corrente media attuale per partire senza strappo */
    float i_avg = 0.0f;
    for (uint8_t i = 0; i < NumActivePhases; i++)
      i_avg += ((float)PhaseCurrents_ADC2[i] - (float)Offset_PhaseCurrents[i]) * ADC_TO_AMPS;
    i_avg /= (float)NumActivePhases;
    if (i_avg < 0.0f) i_avg = 0.0f;
    if (i_avg > I_PHASE_MAX) i_avg = I_PHASE_MAX;

    Controller_Reset(&VoltageLoop, i_avg);
    ClosedLoopEnable = 1;
    /* Inizializza Vout_Ramp alla tensione reale attuale per evitare gradini */
    int32_t v_init_raw = (int32_t)OutputVoltage_ADC2 - (int32_t)Offset_OutputVoltage;
    if (v_init_raw < 0) v_init_raw = 0;
    Vout_Ramp = (float)v_init_raw * ADC_TO_VOLTS;
    last_ramp_tick = HAL_GetTick();
  }

  if (ClosedLoopEnable == 1)
    {
      /* 1. Sottrazione offset Tensione di uscita */
      int32_t vout_raw = (int32_t)OutputVoltage_ADC2 - (int32_t)Offset_OutputVoltage;
      if (vout_raw < 0) vout_raw = 0;
      float vout_meas = (float)vout_raw * ADC_TO_VOLTS;
      float v_error   = Vout_Ref - vout_meas;

      I_Ref = Controller_Update(&VoltageLoop, v_error);

      /* 2. Sottrazione offset Correnti di fase e aggiornamento PI */
      for (uint8_t i = 0; i < 4; i++)
      {
        if (i < NumActivePhases)
        {
          int32_t iphase_raw = (int32_t)PhaseCurrents_ADC2[i] - (int32_t)Offset_PhaseCurrents[i];
          if (iphase_raw < 0) iphase_raw = 0;
          float iphase_meas = (float)iphase_raw * ADC_TO_AMPS;

          float i_error = I_Ref - iphase_meas;
          Duty_Phase[i] = Controller_Update(&CurrentLoop[i], i_error);
        }
        else
        {
          Duty_Phase[i] = Duty_Phase[0];
          Controller_Reset(&CurrentLoop[i], Duty_Phase[0]);
        }
      }
  }
  else
    {
      /* Modalità OPEN LOOP: usa TargetDutyPercent uguale per tutte le fasi */
      float d_open = TargetDutyPercent / 100.0f;
      if (d_open < DUTY_MIN) d_open = DUTY_MIN;
      if (d_open > DUTY_MAX) d_open = DUTY_MAX;

      Controller_Reset(&VoltageLoop, 0.0f);
      for (uint8_t i = 0; i < 4; i++)
      {
        Duty_Phase[i] = d_open;
        Controller_Reset(&CurrentLoop[i], d_open);
      }
    }

    /* 4. Converti i 4 Duty Cycle (0.02..0.80) in Ticks dell'HRTIM e aggiorna i 4 Timer */
    debug_cmp_ph1 = (uint32_t)(Duty_Phase[0] * (float)MULTIPHASE_PERIOD);
    debug_cmp_ph2 = (uint32_t)(Duty_Phase[1] * (float)MULTIPHASE_PERIOD);
    debug_cmp_ph3 = (uint32_t)(Duty_Phase[2] * (float)MULTIPHASE_PERIOD);
    debug_cmp_ph4 = (uint32_t)(Duty_Phase[3] * (float)MULTIPHASE_PERIOD);




  /* Aggiorna i 4 Duty Cycle indipendenti (CMP su Timer A, B, C, D) */
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, debug_cmp_ph1);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_1, debug_cmp_ph2);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, HRTIM_COMPAREUNIT_1, debug_cmp_ph3);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_1, debug_cmp_ph4);

    /* Aggiorna i 4 Trigger ADC esattamente a metà del Ton di ciascuna fase! */
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_2, debug_cmp_ph1 / 2);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_2, debug_cmp_ph2 / 2);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, HRTIM_COMPAREUNIT_2, debug_cmp_ph3 / 2);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_3, debug_cmp_ph4 / 2);
}


static void HRTIM_Config(void)
{
  HRTIM_TimeBaseCfgTypeDef timebase_config;
  HRTIM_TimerCfgTypeDef timer_config;
  HRTIM_OutputCfgTypeDef output_config;
  HRTIM_CompareCfgTypeDef compare_config;
  HRTIM_ADCTriggerCfgTypeDef adc_trigger_config;
  HRTIM_FaultCfgTypeDef fault_config;
  HRTIM_BurstModeCfgTypeDef burst_config;

  /* ----------------------------*/
  /* HRTIM Global initialization */
  /* ----------------------------*/
  /* Initialize the HRTIM handle */
  hhrtim1.Instance = HRTIM1;
  hhrtim1.Init.HRTIMInterruptResquests = HRTIM_IT_NONE;
  hhrtim1.Init.SyncOptions = HRTIM_SYNCOPTION_NONE;
  /* Initialize HRTIM */
  HAL_HRTIM_Init(&hhrtim1);

  /* HRTIM DLL calibration: periodic calibration, set period to 14�s */
  HAL_HRTIM_DLLCalibrationStart(&hhrtim1, HRTIM_CALIBRATIONRATE_14);
  /* Wait calibration completion*/
  if (HAL_HRTIM_PollForDLLCalibration(&hhrtim1, DLL_CALIBRATIONTIMEOUT) != HAL_OK)
  {
    Error_Handler(); /* if DLL or clock is not correctly set */
  }

  /* ---------------------------------------------------------------- */
  /* Master timer initialization: PWM frequency and mode (continuous) */

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
    /* Master timer initialization                                */
    /* Update on repetition, interrupt issued on repetition event */
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
    timer_config.InterruptRequests = HRTIM_MASTER_IT_MREP;
    HAL_HRTIM_WaveformTimerConfig(&hhrtim1, HRTIM_TIMERINDEX_MASTER, &timer_config);
  /* ---------------------------------------------------------------- */
  /* Ripristina il periodo originale per garantire il riarmo Single-Shot e il trigger ADC */
	timebase_config.Period = MULTIPHASE_PERIOD - 1 - ADC_CONVERSION_TIME;
	timebase_config.RepetitionCounter = REPETITON_RATE;
	timebase_config.PrescalerRatio = HRTIM_PRESCALERRATIO_MUL32;
	timebase_config.Mode = HRTIM_MODE_SINGLESHOT_RETRIGGERABLE;
	HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, &timebase_config);
	HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, &timebase_config);
	HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, &timebase_config);
	HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, &timebase_config);



  /* -------------------------------------------------------------- */
  /* TIMERA, TIMERB, TIMERC, TIMERD initialization: timer mode and PWM frequency */
  /* The 4 timers are running with identical parameters.            */
  /* -------------------------------------------------------------- */
  /* Rationale for period value: Timer A period event is used as event for ADC sampling */
  /* during phase shedding (for single-phase use case */

  //timebase_config.Period = MULTIPHASE_PERIOD - 1 - ADC_CONVERSION_TIME;
  timebase_config.Period = MULTIPHASE_PERIOD;
  timebase_config.RepetitionCounter = REPETITON_RATE; /* 1 ISR every REPETITON_RATE PWM periods */
  timebase_config.PrescalerRatio = HRTIM_PRESCALERRATIO_MUL32;
  timebase_config.Mode = HRTIM_MODE_SINGLESHOT_RETRIGGERABLE;
  HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, &timebase_config);
  HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, &timebase_config);
  HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, &timebase_config);
  HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, &timebase_config);

  /* --------------------------------------------------------------------- */
  /* TIMERA, TIMERB, TIMERC, TIMERD global configuration                   */
  /* Preload enabled, triggered by master, Fault1 enabled                  */
  /* Counter reset triggered by master: MSTPER for TIMA, MSTCMP1 for TIMB, */
  /* MSTCMP2 for TIMC, MSTCMP3 for TIMD                                    */
  /* All other parameter to default value                                  */
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
  timer_config.FaultEnable = HRTIM_TIMFAULTENABLE_NONE; /* Disabilitato per test Open Loop */
  timer_config.FaultLock = HRTIM_TIMFAULTLOCK_READWRITE;
  timer_config.DeadTimeInsertion = HRTIM_TIMDEADTIMEINSERTION_DISABLED;
  timer_config.DelayedProtectionMode = HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DISABLED;
  timer_config.UpdateTrigger= HRTIM_TIMUPDATETRIGGER_MASTER;
  timer_config.ResetTrigger = HRTIM_TIMRESETTRIGGER_MASTER_PER;
  HAL_HRTIM_WaveformTimerConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, &timer_config);

  timer_config.ResetTrigger = HRTIM_TIMRESETTRIGGER_MASTER_CMP1;
  HAL_HRTIM_WaveformTimerConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, &timer_config);

  timer_config.ResetTrigger = HRTIM_TIMRESETTRIGGER_MASTER_CMP2;
  HAL_HRTIM_WaveformTimerConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, &timer_config);

  timer_config.DelayedProtectionMode = HRTIM_TIMER_D_E_DELAYEDPROTECTION_DISABLED;
  timer_config.UpdateTrigger= HRTIM_TIMUPDATETRIGGER_MASTER;
  timer_config.ResetTrigger = HRTIM_TIMRESETTRIGGER_MASTER_CMP3;
  HAL_HRTIM_WaveformTimerConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, &timer_config);

  /* -------------------------------------------- */
  /* TA1, TB1, TC1, TD1 waveform description */
  /* -------------------------------------------- */
  output_config.Polarity = HRTIM_OUTPUTPOLARITY_HIGH;
  output_config.SetSource = HRTIM_OUTPUTSET_MASTERPER;
  output_config.ResetSource  = HRTIM_OUTPUTRESET_TIMCMP1;
  output_config.IdleMode = HRTIM_OUTPUTIDLEMODE_IDLE;
  output_config.IdleLevel = HRTIM_OUTPUTIDLELEVEL_INACTIVE;
  output_config.FaultLevel = HRTIM_OUTPUTFAULTLEVEL_INACTIVE;
  output_config.ChopperModeEnable = HRTIM_OUTPUTCHOPPERMODE_DISABLED;
  output_config.BurstModeEntryDelayed = HRTIM_OUTPUTBURSTMODEENTRY_REGULAR;

  /* TA1 waveform */
  HAL_HRTIM_WaveformOutputConfig(&hhrtim1,
                                 HRTIM_TIMERINDEX_TIMER_A,
                                 HRTIM_OUTPUT_TA1,
                                 &output_config);

  /* TB1 waveform */
  output_config.SetSource = HRTIM_OUTPUTSET_MASTERCMP1;
  output_config.ResetSource  = HRTIM_OUTPUTRESET_TIMCMP1;
  HAL_HRTIM_WaveformOutputConfig(&hhrtim1,
                                 HRTIM_TIMERINDEX_TIMER_B,
                                 HRTIM_OUTPUT_TB1,
                                 &output_config);

  /* TC1 waveform */
  output_config.SetSource = HRTIM_OUTPUTSET_MASTERCMP2;
  output_config.ResetSource  = HRTIM_OUTPUTRESET_TIMCMP1;
  HAL_HRTIM_WaveformOutputConfig(&hhrtim1,
                                 HRTIM_TIMERINDEX_TIMER_C,
                                 HRTIM_OUTPUT_TC1,
                                 &output_config);

  /* TD1 waveform */
  output_config.SetSource = HRTIM_OUTPUTSET_MASTERCMP3;
  output_config.ResetSource  = HRTIM_OUTPUTRESET_TIMCMP1;
  HAL_HRTIM_WaveformOutputConfig(&hhrtim1,
                                 HRTIM_TIMERINDEX_TIMER_D,
                                 HRTIM_OUTPUT_TD1,
                                 &output_config);

  /* Set compare registers for duty cycle on TA1 */
  compare_config.AutoDelayedMode = HRTIM_AUTODELAYEDMODE_REGULAR;
  compare_config.AutoDelayedTimeout = 0;
  compare_config.CompareValue = DUTY_CYCLE_INIT;
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1,
                                  HRTIM_TIMERINDEX_TIMER_A,
                                  HRTIM_COMPAREUNIT_1,
                                  &compare_config);

  /* Set compare registers for duty cycle on TB1 */
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1,
                                  HRTIM_TIMERINDEX_TIMER_B,
                                  HRTIM_COMPAREUNIT_1,
                                  &compare_config);

  /* Set compare registers for duty cycle on TC1 */
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1,
                                  HRTIM_TIMERINDEX_TIMER_C,
                                  HRTIM_COMPAREUNIT_1,
                                  &compare_config);

  /* Set compare registers for duty cycle on TD1 */
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1,
                                  HRTIM_TIMERINDEX_TIMER_D,
                                  HRTIM_COMPAREUNIT_1,
                                  &compare_config);

  /* Set compare registers for phase-shifts in master timer */
  /* Each compare is coding for the phase-shift of one phase */
  compare_config.CompareValue = PHASE_SHIFT_INIT;
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1,
                                  HRTIM_TIMERINDEX_MASTER,
                                  HRTIM_COMPAREUNIT_1,
                                  &compare_config);

  compare_config.CompareValue = 2 * PHASE_SHIFT_INIT;
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1,
                                  HRTIM_TIMERINDEX_MASTER,
                                  HRTIM_COMPAREUNIT_2,
                                  &compare_config);

  compare_config.CompareValue = 3 * PHASE_SHIFT_INIT;
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1,
                                  HRTIM_TIMERINDEX_MASTER,
                                  HRTIM_COMPAREUNIT_3,
                                  &compare_config);

  /* -------------------------------------------------- */
  /* Burst mode configuration for light load management */
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
  /* ADC trigger initialization (inc. for phase shedding management) */
  /* -------------------------------------------------------------- */
  compare_config.AutoDelayedMode = HRTIM_AUTODELAYEDMODE_REGULAR;
  compare_config.CompareValue = DUTY_CYCLE_INIT/2; /* Samples at 50% of Ton time */
  compare_config.AutoDelayedTimeout = 0;
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1,
                                  HRTIM_TIMERINDEX_TIMER_A,
                                  HRTIM_COMPAREUNIT_2,
                                  &compare_config);
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1,
                                  HRTIM_TIMERINDEX_TIMER_B,
                                  HRTIM_COMPAREUNIT_2,
                                  &compare_config);
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1,
                                  HRTIM_TIMERINDEX_TIMER_C,
                                  HRTIM_COMPAREUNIT_2,
                                  &compare_config);
  HAL_HRTIM_WaveformCompareConfig(&hhrtim1,
                                  HRTIM_TIMERINDEX_TIMER_D,
                                  HRTIM_COMPAREUNIT_3, //why not 2?
                                  &compare_config);

    /* Dummy trigger per configurazione a 3 FASI (serve 1 solo dummy trigger per fare la 4a conversione) */
  	compare_config.CompareValue = (MULTIPHASE_PERIOD / 3) - 1 - ADC_CONVERSION_TIME;
	HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, HRTIM_COMPAREUNIT_3, &compare_config);

	/* Dummy triggers per configurazione a 2 FASI (servono 2 dummy triggers: 3a e 4a conversione) */
	compare_config.CompareValue = (MULTIPHASE_PERIOD / 2) - 1 - ADC_CONVERSION_TIME;
	HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_3, &compare_config);

	compare_config.CompareValue = (MULTIPHASE_PERIOD / 2) - 1 - (ADC_CONVERSION_TIME * 2);
	HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_4, &compare_config);

	/* Dummy triggers per configurazione a 1 FASE (servono 3 dummy triggers: 2a, 3a su CMP3/CMP4 e 4a su TIMERA_PER) */
	compare_config.CompareValue = MULTIPHASE_PERIOD - 1 - (ADC_CONVERSION_TIME * 3);
	HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_3, &compare_config);

	compare_config.CompareValue = MULTIPHASE_PERIOD - 1 - (ADC_CONVERSION_TIME * 2);
	HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_4, &compare_config);


  adc_trigger_config.Trigger = HRTIM_ADCTRIGGEREVENT13_TIMERA_CMP2
                               + HRTIM_ADCTRIGGEREVENT13_TIMERB_CMP2
                               + HRTIM_ADCTRIGGEREVENT13_TIMERC_CMP2
                               + HRTIM_ADCTRIGGEREVENT13_TIMERD_CMP3;

  adc_trigger_config.UpdateSource = HRTIM_ADCTRIGGERUPDATE_MASTER;
  HAL_HRTIM_ADCTriggerConfig(&hhrtim1,
                             HRTIM_ADCTRIGGER_1,
                             &adc_trigger_config);


  /* ---------------------*/
  /* FAULT initialization */
  /* ---------------------*/
  fault_config.Filter = HRTIM_FAULTFILTER_NONE;
  fault_config.Lock = HRTIM_FAULTLOCK_READWRITE;
  fault_config.Polarity = HRTIM_FAULTPOLARITY_LOW;
  fault_config.Source = HRTIM_FAULTSOURCE_DIGITALINPUT;
  HAL_HRTIM_FaultConfig(&hhrtim1,
                        HRTIM_FAULT_1,
                        &fault_config);

  //HAL_HRTIM_FaultModeCtl(&hhrtim, HRTIM_FAULT_1, HRTIM_FAULTMODECTL_ENABLED); //disabilitato perchè non collego nulla sul fault PA12 e potrebbe bloccare le uscite
  HAL_NVIC_SetPriority(HRTIM1_Master_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(HRTIM1_Master_IRQn);
}


static void Update_Phases(void)
{
  HRTIM_ADCTriggerCfgTypeDef adc_trigger_config;
  /* Esegui solo se NumActivePhases è stato cambiato in Live Expressions */
  if (NumActivePhases != PrevActivePhases)
  {
    adc_trigger_config.UpdateSource = HRTIM_ADCTRIGGERUPDATE_MASTER;

    switch (NumActivePhases)
    {
      case 1: /* 1 FASE: Solo PH1 (PA8 - TA1) */
        HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TD1);
        HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1);

        adc_trigger_config.Trigger = HRTIM_ADCTRIGGEREVENT13_TIMERA_CMP2
                                   | HRTIM_ADCTRIGGEREVENT13_TIMERA_CMP3
                                   | HRTIM_ADCTRIGGEREVENT13_TIMERA_CMP4
                                   | HRTIM_ADCTRIGGEREVENT13_TIMERA_PERIOD;
        HAL_HRTIM_ADCTriggerConfig(&hhrtim1, HRTIM_ADCTRIGGER_1, &adc_trigger_config);
        PrevActivePhases = 1;
        break;

      case 2: /* 2 FASI: PH1 e PH2 sfasate di 180° */
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_1, (MULTIPHASE_PERIOD / 2));

        HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TD1);
        HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TB1);

        adc_trigger_config.Trigger = HRTIM_ADCTRIGGEREVENT13_TIMERA_CMP2
                                   | HRTIM_ADCTRIGGEREVENT13_TIMERB_CMP2
                                   | HRTIM_ADCTRIGGEREVENT13_TIMERB_CMP3
                                   | HRTIM_ADCTRIGGEREVENT13_TIMERB_CMP4;
        HAL_HRTIM_ADCTriggerConfig(&hhrtim1, HRTIM_ADCTRIGGER_1, &adc_trigger_config);
        PrevActivePhases = 2;
        break;

      case 3: /* 3 FASI: PH1, PH2, PH3 sfasate di 120° */
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_1, (MULTIPHASE_PERIOD / 3));
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_2, 2 * (MULTIPHASE_PERIOD / 3));

        HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TD1);
        HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TC1);

        adc_trigger_config.Trigger = HRTIM_ADCTRIGGEREVENT13_TIMERA_CMP2
                                   | HRTIM_ADCTRIGGEREVENT13_TIMERB_CMP2
                                   | HRTIM_ADCTRIGGEREVENT13_TIMERC_CMP2
                                   | HRTIM_ADCTRIGGEREVENT13_TIMERC_CMP3;
        HAL_HRTIM_ADCTriggerConfig(&hhrtim1, HRTIM_ADCTRIGGER_1, &adc_trigger_config);
        PrevActivePhases = 3;
        break;

      case 4: /* 4 FASI: PH1, PH2, PH3, PH4 sfasate di 90° */
      default:
        NumActivePhases = 4;
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_1, (MULTIPHASE_PERIOD / 4));
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_2, 2 * (MULTIPHASE_PERIOD / 4));
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_3, 3 * (MULTIPHASE_PERIOD / 4));

        HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TD1);

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
