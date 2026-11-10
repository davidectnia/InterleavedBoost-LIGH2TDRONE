/*
 * control_loop.c
 *
 *  Created on: Nov 3, 2026
 *      Author: Davide Catania
 */
#include "control_loop.h"
/* Pre-calculated Q8.24 coefficients (Float_Value * 16777216) */
#define BV0_Q24   4818129
#define BV1_Q24   139654
//#define BV1_Q24   0        /* Disable */
#define BV2_Q24  -4678440
//#define BV2_Q24   0        /* Disable */

#define AV1_Q24  -17163456
//#define AV1_Q24   0        /* Disable */

#define AV2_Q24   386175
//#define AV2_Q24   0        /* Disable */

#define BI0_Q24   125728
#define BI1_Q24   13170
#define BI2_Q24  -112557
#define AI1_Q24  -17899144
#define AI2_Q24   1122097

/* Global Variables (Q16.16) */
volatile int32_t Vout_Ref_Q16          = 2909798;  /* 44.4f * 65536 */
volatile uint16_t ticks_refresh_ramp   = 500;      /* Tipo corretto uint16_t per non andare in overflow! */
volatile int32_t Vout_Ramp_Q16         = 0;
volatile int32_t RAMP_STEP_V_Q16       = 6553;     /* 0.1f * 65536 */
volatile int32_t Duty_Phase_Q16[4]     = {16384, 16384, 16384, 16384}; /* 0.25f * 65536 */
volatile int32_t TargetDuty_Q16        = 16384;    /* 0.25f * 65536 */

/* Internal Variables, not exposed for debug operation*/
static volatile int32_t I_Ref_Q16 = 0;

/* --- FILTER ISTANCES --- */
static control_elem_discrete_q VoltageLoop = {
  .b0 = BV0_Q24, .b1 = BV1_Q24, .b2 = BV2_Q24, .a1 = AV1_Q24, .a2 = AV2_Q24,
  .e1 = 0, .e2 = 0, .u1 = 0, .u2 = 0,
  .u_min = 0, .u_max = I_PHASE_MAX_Q16
};
static control_elem_discrete_q CurrentLoop[4] = {
  { .b0 = BI0_Q24, .b1 = BI1_Q24, .b2 = BI2_Q24, .a1 = AI1_Q24, .a2 = AI2_Q24, .u_min = DUTY_MIN_Q16, .u_max = DUTY_MAX_Q16 },
  { .b0 = BI0_Q24, .b1 = BI1_Q24, .b2 = BI2_Q24, .a1 = AI1_Q24, .a2 = AI2_Q24, .u_min = DUTY_MIN_Q16, .u_max = DUTY_MAX_Q16 },
  { .b0 = BI0_Q24, .b1 = BI1_Q24, .b2 = BI2_Q24, .a1 = AI1_Q24, .a2 = AI2_Q24, .u_min = DUTY_MIN_Q16, .u_max = DUTY_MAX_Q16 },
  { .b0 = BI0_Q24, .b1 = BI1_Q24, .b2 = BI2_Q24, .a1 = AI1_Q24, .a2 = AI2_Q24, .u_min = DUTY_MIN_Q16, .u_max = DUTY_MAX_Q16 }
};

/* --- PRIVATE FUNCTIONS --- */
CCMRAM static inline int32_t Controller_Update(control_elem_discrete_q *ctrl, int32_t error) {
    /* 64-bit accumulator prevents overflow during Q8.24 * Q16.16 multiplication */
    int64_t acc = 0;
    acc += (int64_t)ctrl->b0 * error;
    acc += (int64_t)ctrl->b1 * ctrl->e1;
    acc += (int64_t)ctrl->b2 * ctrl->e2;
    acc -= (int64_t)ctrl->a1 * ctrl->u1;
    acc -= (int64_t)ctrl->a2 * ctrl->u2;

    /* Shift right by 24 to return to Q16.16 format */
    int32_t u0 = (int32_t)(acc >> 24);

/* --- ANTI-WINDUP  --- */
	if (u0 > ctrl->u_max) {
		u0 = ctrl->u_max;

		/* Overwrites Last Output with the maximum possible (u1, u2). */
		/* In this way, the integrator will stop overcharging by keeping at the maximum possible the output. */
		ctrl->u1 = ctrl->u_max;
		ctrl->u2 = ctrl->u_max;

		/* In windup conditions, the error must be put to zero to avoid saturation during startup */
		ctrl->e1 = 0;
		ctrl->e2 = 0;
	} else if (u0 < ctrl->u_min) {
		u0 = ctrl->u_min;
		ctrl->u1 = ctrl->u_min;
		ctrl->u2 = ctrl->u_min;
		ctrl->e1 = 0;
		ctrl->e2 = 0;
	} else {
		/* Linear Zone */
		ctrl->u2 = ctrl->u1;
		ctrl->u1 = u0;
		ctrl->e2 = ctrl->e1;
		ctrl->e1 = error;
	}

    return u0;
}

CCMRAM static inline void Controller_Reset(control_elem_discrete_q *ctrl, int32_t init_output) {
    if (init_output > ctrl->u_max) init_output = ctrl->u_max;
    if (init_output < ctrl->u_min) init_output = ctrl->u_min;
    ctrl->e1 = 0;
    ctrl->e2 = 0;
    ctrl->u1 = init_output;
    ctrl->u2 = init_output;
}

/* --- PUBLIC FUNCTIONS --- */
void ControlLoop_ResetAll(int32_t initial_duty_q16, int32_t initial_current_q16) {
    Controller_Reset(&VoltageLoop, initial_current_q16);
    for (uint8_t i = 0; i < 4; i++) {
        Controller_Reset(&CurrentLoop[i], initial_duty_q16);
    }
}

CCMRAM void ControlLoop_Execute(uint8_t closed_loop_enable, uint8_t num_active_phases,
                         int32_t vout_meas_q16, int32_t *iphase_meas_q16, int32_t target_duty_q16) {
    static uint16_t ramp_prescaler = 0;
    static uint8_t voltage_loop_decimator = 0;


    if (closed_loop_enable == 1) {
        if (++ramp_prescaler >= ticks_refresh_ramp) {
            if (Vout_Ramp_Q16 < Vout_Ref_Q16) {
                Vout_Ramp_Q16 += RAMP_STEP_V_Q16;
                if (Vout_Ramp_Q16 > Vout_Ref_Q16) Vout_Ramp_Q16 = Vout_Ref_Q16;
            } else if (Vout_Ramp_Q16 > Vout_Ref_Q16) {
                Vout_Ramp_Q16 -= RAMP_STEP_V_Q16;
                if (Vout_Ramp_Q16 < Vout_Ref_Q16) Vout_Ramp_Q16 = Vout_Ref_Q16;
            }
            ramp_prescaler = 0;
        }
        if (++voltage_loop_decimator >= 10) { /* Esegue a 5 kHz invece di 50 kHz */
            voltage_loop_decimator = 0;

            int32_t v_error = Vout_Ramp_Q16 - vout_meas_q16;
            /* 30mV Deadband definition (30mV * 65536 = ~1966 Q16.16) */
            if (v_error > -1966 && v_error < 1966) {
                v_error = 0; /* Errore zero -> l'integratore smette di accumulare */
            }
            I_Ref_Q16 = Controller_Update(&VoltageLoop, v_error);
        }

        for (uint8_t i = 0; i < 4; i++) {
            if (i < num_active_phases) {
                int32_t i_error = I_Ref_Q16 - iphase_meas_q16[i];
                Duty_Phase_Q16[i] = Controller_Update(&CurrentLoop[i], i_error);
            } else {
                Duty_Phase_Q16[i] = Duty_Phase_Q16[0];
            }
        }
    } else {
        Vout_Ramp_Q16 = vout_meas_q16;
        for (uint8_t i = 0; i < 4; i++) {
            Duty_Phase_Q16[i] = target_duty_q16;
        }
    }
}
