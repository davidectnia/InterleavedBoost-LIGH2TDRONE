/*
 * control_loop.h
 *
 *  Created on: Nov 3, 2026
 *      Author: Davide Catania
 */

#ifndef CONTROL_LOOP_H_
#define CONTROL_LOOP_H_

#include <stdint.h>

/* --- OPERATIONAL LIMITS --- */
#define I_PHASE_MAX  (8.0f)   /* Maximum Current per each phase, Maximum Current output for PEMFC H100 */
#define DUTY_MIN     (0.02f)  /* Minimum Duty Cycle = 2% */
#define DUTY_MAX     (0.77f)  /* Maximum Duty Cycle = 80% */

/**
 * @brief Structure definition for digital PI filter (regulator)
 */

typedef struct {
    float b0, b1, b2;
    float a1, a2;
    float e1, e2;
    float u1, u2;
    float u_min, u_max;
} control_elem_discrete;

/* --- PUBLIC VARIABLES FOR DEBUG IN CUBEMX--- */
extern volatile float Vout_Ref;
extern volatile float Vout_Ramp;
extern volatile float RAMP_STEP_V;
extern volatile float Duty_Phase[4];
extern volatile float debug_v_error;
extern volatile uint8_t stat_vloop_sat;
extern volatile uint8_t stat_iloop_sat;

/* --- PUBLIC FUNCTIONS PROTOTYPE --- */

/**
 * @brief Pre-store the memory of the filters to ensure Bumpless transfer
 * @param initial_duty Duty Cycle to set as a past output
 * @param initial_current Initial average current set as past output
 */
void ControlLoop_ResetAll(float initial_duty, float initial_current);

/**
 * @brief Execture the Control Loop (Inner Current + Outer Voltage Loop) or set the Open-Loop by fixing the duty cycle
 * @param closed_loop_enable Flag (1 = Closed Loop, 0 = Open Loop)
 * @param num_active_phases Number of active phases (1..4)
 * @param vout_meas Output Voltage measurement [V]
 * @param iphase_meas Phase Currents measurements array for the four phases [A]
 * @param target_duty_open_loop Duty cycle target value in Open Loop configuration
 */
void ControlLoop_Execute(uint8_t closed_loop_enable, uint8_t num_active_phases,
                         float vout_meas, float *iphase_meas, float target_duty_open_loop);
#endif /* CONTROL_LOOP_H_ */
