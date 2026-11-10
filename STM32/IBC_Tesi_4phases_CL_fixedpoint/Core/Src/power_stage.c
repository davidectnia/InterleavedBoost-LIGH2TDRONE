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

CCMRAM void PowerStage_ApplyDutyCycles(volatile int32_t *duty_array_q16) {
    /* Convert Q16.16 duties to HRTIM ticks (right shift by 16 divides by 65536) */
    uint32_t cmp_ph1 = (uint32_t)((duty_array_q16[0] * MULTIPHASE_PERIOD) >> 16);
    uint32_t cmp_ph2 = (uint32_t)((duty_array_q16[1] * MULTIPHASE_PERIOD) >> 16);
    uint32_t cmp_ph3 = (uint32_t)((duty_array_q16[2] * MULTIPHASE_PERIOD) >> 16);
    uint32_t cmp_ph4 = (uint32_t)((duty_array_q16[3] * MULTIPHASE_PERIOD) >> 16);

    /* Direct hardware register write (Bypassing __HAL_HRTIM_SETCOMPARE) */
    HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP1xR = cmp_ph1;
    HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_B].CMP1xR = cmp_ph2;
    HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP1xR = cmp_ph3;
    HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP1xR = cmp_ph4;

    /* Update ADC Triggers strictly at 50% Ton */
    HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP2xR = cmp_ph1 >> 1;
    HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_B].CMP2xR = cmp_ph2 >> 1;
    HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP2xR = cmp_ph3 >> 1;
    HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP3xR = cmp_ph4 >> 1;
}

void PowerStage_EmergencyStop(void) {
    HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TD1);
}
