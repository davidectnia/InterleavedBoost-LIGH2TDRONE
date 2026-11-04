/*
 * analog_sensing.c
 *
 *  Created on: Nov 3, 2026
 *      Author: Davide Catania
 */
#include "analog_sensing.h"
#include "ntc_lut.h" /* This header containes the lookup table generated from MATLAB*/
/* --- CONVERSION GAINS ---
 * Can be modified during adjustment phase until correct value is read
 */
volatile float Gain_Vout   = 0.01230807f;
volatile float Gain_Iout   = 0.00960000f;
volatile float Gain_Iphase = 0.00960000f;

/* --- Offset Analog Circuitry variables--- */
static uint16_t Offset_Vout = 0;
static uint16_t Offset_Iout = 0;
static uint16_t Offset_Iphase[4] = {0, 0, 0, 0};

/* --- Exposed variables. Can be seen from the debugger --- */
volatile float Vout_Meas_V        = 0.0f;
volatile float Iout_Meas_A        = 0.0f;
volatile float Iphase_Meas_A[4]   = {0.0f, 0.0f, 0.0f, 0.0f};
volatile float Iphase_Total_A     = 0.0f;
volatile uint16_t temperature_adc = 0;

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


void AnalogSensing_Update(uint16_t adc_vout, uint16_t adc_iout, uint16_t *adc_iphase, uint8_t num_active_phases) {



	/* 2. Output Voltage Conversion */
    int32_t vout_raw = (int32_t)adc_vout - (int32_t)Offset_Vout;
    if (vout_raw < 0) vout_raw = 0;
    Vout_Meas_V = (float)vout_raw * Gain_Vout;

    /* 3. Output Current Conversion */
    int32_t iout_raw = (int32_t)adc_iout - (int32_t)Offset_Iout;
    if (iout_raw < 0) iout_raw = 0;
    Iout_Meas_A = (float)iout_raw * Gain_Iout;

    /* 4. Phase Currents conversion */
    float i_tot = 0.0f;
    for (uint8_t i = 0; i < 4; i++) {
        int32_t iph_raw = (int32_t)adc_iphase[i] - (int32_t)Offset_Iphase[i];
        if (iph_raw < 0) iph_raw = 0;
        Iphase_Meas_A[i] = (float)iph_raw * Gain_Iphase;

        if (i < num_active_phases) {
            i_tot += Iphase_Meas_A[i];
        }
    }
    Iphase_Total_A = i_tot;
}
