/*
 * analog_sensing.h
 *
 *  Created on: Nov 3, 2026
 *      Author: Davide Catania
 */

#ifndef ANALOG_SENSING_H_
#define ANALOG_SENSING_H_

#include <stdint.h>

/* --- Public Variables (Physical Measurements) --- */
extern volatile float Vout_Meas_V;
extern volatile float Iout_Meas_A;
extern volatile float Iphase_Meas_A[4];
extern volatile float Iphase_Total_A;

/* Temperature reading from NTC sensor in degrees Celsius */
extern volatile float temperature_deg_celsius;

/* --- FUNCTION PROTOTYPES --- */
/**
 * @brief Temperature Reading
 */
float check_Temperature(uint32_t adc_temp);

/**
 * @brief Store the offset of the analog circuitry
 */
void AnalogSensing_InitOffsets(uint16_t offset_vout, uint16_t offset_iout, uint16_t *offset_iphase);

/**
 * @brief LSB to Voltage/Current Conversion. Called in the ISR
 */
void AnalogSensing_Update(uint16_t adc_vout, uint16_t adc_iout, uint16_t *adc_iphase, uint8_t num_active_phases);
#endif /* ANALOG_SENSING_H_ */
