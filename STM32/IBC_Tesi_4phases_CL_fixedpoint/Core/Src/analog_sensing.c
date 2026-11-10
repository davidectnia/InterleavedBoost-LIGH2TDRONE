/*
 * analog_sensing.c
 *
 *  Created on: Nov 3, 2026
 *      Author: Davide Catania
 */
#include "analog_sensing.h"
#include "ntc_lut.h" /* This header containes the lookup table generated from MATLAB*/
#include "control_loop.h" /* To import the CCMRAM macros */

/* --- CONVERSION GAINS (Scaled to Q16.16 format) --- 
 * V_gain_Q16 = 0.01230807 * 65536 = 806.6 -> 807
 * I_gain_Q16 = 0.0096 * 65536 = 629.1 -> 629
 */
static const int32_t GAIN_VOUT_Q16   = 807;
static const int32_t GAIN_IOUT_Q16   = 629;
static const int32_t GAIN_IPHASE_Q16 = 629;

/* --- Offset Analog Circuitry variables--- */
static uint16_t Offset_Vout = 0;
static uint16_t Offset_Iout = 0;
static uint16_t Offset_Iphase[4] = {0, 0, 0, 0};

/* --- Exposed variables. Can be seen from the debugger --- */
volatile int32_t Vout_Meas_Q16        = 0;
volatile int32_t Iout_Meas_Q16        = 0;
volatile int32_t Iphase_Meas_Q16[4]   = {0, 0, 0, 0};
volatile int32_t Iphase_Total_Q16     = 0;
volatile float temperature_deg_celsius = 0.0f;


/* --- Function Implementations --- */

float check_Temperature(uint32_t adc_temp){
	/* 1. Temperature reading from LUT*/
	/* Saturation to avoid out of range readings */
	if (adc_temp > 4095) return -999.0f;
	temperature_deg_celsius = (float)NTC_TEMP_LUT[adc_temp] / 10.0f;

	return temperature_deg_celsius;
}
void AnalogSensing_InitOffsets(uint16_t offset_vout, uint16_t offset_iout, uint16_t *offset_iphase) {
    Offset_Vout = offset_vout;
    Offset_Iout = offset_iout;
    for (int i = 0; i < 4; i++) {
        Offset_Iphase[i] = offset_iphase[i];
    }
}


CCMRAM void AnalogSensing_Update(uint16_t adc_vout, uint16_t adc_iout, uint16_t *adc_iphase, uint8_t num_active_phases) {
    int32_t vout_raw = (int32_t)adc_vout - Offset_Vout;
    Vout_Meas_Q16 = (vout_raw > 0) ? (vout_raw * GAIN_VOUT_Q16) : 0;

    int32_t iout_raw = (int32_t)adc_iout - Offset_Iout;
    Iout_Meas_Q16 = (iout_raw > 0) ? (iout_raw * GAIN_IOUT_Q16) : 0;

    int32_t i_tot = 0;
    for (uint8_t i = 0; i < 4; i++) {
        int32_t iph_raw = (int32_t)adc_iphase[i] - Offset_Iphase[i];
        Iphase_Meas_Q16[i] = (iph_raw > 0) ? (iph_raw * GAIN_IPHASE_Q16) : 0;
        
        if (i < num_active_phases) {
            i_tot += Iphase_Meas_Q16[i];
        }
    }
    Iphase_Total_Q16 = i_tot;
}
