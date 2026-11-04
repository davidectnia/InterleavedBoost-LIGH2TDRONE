/*
 * power_stage.c
 *
 *  Created on: Nov 4, 2026
 *      Author: Davide Catania
 */

#include "power_stage.h"
#include "main.h"

extern HRTIM_HandleTypeDef hhrtim1;
static uint8_t PrevActivePhases = 0; // Aggiunta variabile locale di stato

void PowerStage_UpdatePhases(uint8_t active_phases) {

	  HRTIM_ADCTriggerCfgTypeDef adc_trigger_config = {0};
	  if (active_phases!= PrevActivePhases)
	  {
	    adc_trigger_config.UpdateSource = HRTIM_ADCTRIGGERUPDATE_MASTER;

	    switch (active_phases)
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
	        active_phases = 4;
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

void PowerStage_ApplyDutyCycles(volatile float *duty_array) {
    /* Convert float duties to timer ticks */
    uint32_t cmp_ph1 = (uint32_t)(duty_array[0] * (float)MULTIPHASE_PERIOD);
    uint32_t cmp_ph2 = (uint32_t)(duty_array[1] * (float)MULTIPHASE_PERIOD);
    uint32_t cmp_ph3 = (uint32_t)(duty_array[2] * (float)MULTIPHASE_PERIOD);
    uint32_t cmp_ph4 = (uint32_t)(duty_array[3] * (float)MULTIPHASE_PERIOD);

    /* Update Duty Cycles */
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, cmp_ph1);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_1, cmp_ph2);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, HRTIM_COMPAREUNIT_1, cmp_ph3);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_1, cmp_ph4);

    /* Update ADC Triggers strictly at 50% Ton */
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_2, cmp_ph1 / 2);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_2, cmp_ph2 / 2);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, HRTIM_COMPAREUNIT_2, cmp_ph3 / 2);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_3, cmp_ph4 / 2);
}

void PowerStage_EmergencyStop(void) {
    HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TD1);
}
