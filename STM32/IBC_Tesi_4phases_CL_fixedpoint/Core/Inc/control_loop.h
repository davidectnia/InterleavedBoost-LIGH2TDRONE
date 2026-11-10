/*
 * control_loop.h
 *
 *  Created on: Nov 3, 2026
 *      Author: Davide Catania
 */

#ifndef CONTROL_LOOP_H_
#define CONTROL_LOOP_H_

#include <stdint.h>

#ifndef CCMRAM
#define CCMRAM __attribute__((section(".ccmram")))
#endif

#define FLOAT_TO_Q16(x)  ((int32_t)((x) * 65536.0f))
#define Q16_TO_FLOAT(x)  ((float)(x) / 65536.0f)

/* Operational limits converted to Q16.16 */
#define I_PHASE_MAX_Q16  FLOAT_TO_Q16(8.0f)   /* 524288 8 A*/
#define DUTY_MIN_Q16     FLOAT_TO_Q16(0.02f)  /* 1310 80% duty cycle*/
#define DUTY_MAX_Q16     FLOAT_TO_Q16(0.80f)  /* 52428 2% duty cycle*/

/**
 * @brief Structure definition for digital PI filter (regulator)
 */
typedef struct {
    int32_t b0, b1, b2;       /* Coefficients in Q8.24 */
    int32_t a1, a2;           /* Coefficients in Q8.24 */
    int32_t e1, e2;           /* Errors in Q16.16 */
    int32_t u1, u2;           /* Outputs in Q16.16 */
    int32_t u_min, u_max;     /* Limits in Q16.16 */
} control_elem_discrete_q;


/* Public variables for system state */
extern volatile int32_t Vout_Ref_Q16;
extern volatile int32_t Vout_Ramp_Q16;
extern volatile int32_t Duty_Phase_Q16[4];
extern volatile int32_t TargetDuty_Q16;

/* --- PUBLIC FUNCTIONS PROTOTYPE --- */

/**
 * @brief Pre-store the memory of the filters to ensure Bumpless transfer
 * @param initial_duty Duty Cycle to set as a past output
 * @param initial_current Initial average current set as past output
 */
void ControlLoop_ResetAll(int32_t initial_duty_q16, int32_t initial_current_q16);


/**
 * @brief Execture the Control Loop (Inner Current + Outer Voltage Loop) or set the Open-Loop by fixing the duty cycle
 * @param closed_loop_enable Flag (1 = Closed Loop, 0 = Open Loop)
 * @param num_active_phases Number of active phases (1..4)
 * @param vout_meas Output Voltage measurement [V]
 * @param iphase_meas Phase Currents measurements array for the four phases [A]
 * @param target_duty_open_loop Duty cycle target value in Open Loop configuration
 */
void ControlLoop_Execute(uint8_t closed_loop_enable, uint8_t num_active_phases,
                         int32_t vout_meas_q16, int32_t *iphase_meas_q16, int32_t target_duty_q16);
#endif /* CONTROL_LOOP_H_ */
