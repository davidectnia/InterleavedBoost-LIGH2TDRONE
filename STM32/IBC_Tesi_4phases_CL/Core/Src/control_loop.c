/*
 * control_loop.c
 *
 *  Created on: Nov 3, 2026
 *      Author: Davide Catania
 */
#include "control_loop.h"
/* --- OUTER VOLTAGE LOOP COEFFICIENTS (Normalized for A0_V = 1.95497e5) --- */
#define A0_V  ( 1.95497e+05f)
#define BV0   ( 5.6143e+04f / A0_V)  /*  0.287181f */
#define BV1   ( 1.6273e+03f / A0_V)  /*  0.008324f */
#define BV2   (-5.4516e+04f / A0_V)  /* -0.278858f */
#define AV1   (-1.99997e+05f / A0_V) /* -1.023018f (1 + AV1 + AV2 = 0 for integral action) */
#define AV2   ( 4.5070e+03f / A0_V)  /*  0.023018f */

/* --- INNER CURRENT LOOP COEFFICIENTS (Normalized for A0_I = 2.14335e5) --- */
#define A0_I  ( 2.14335e+05f)
#define BI0   ( 1.6062e+03f / A0_I)  /*  0.007494f */
#define BI1   ( 168.3186f   / A0_I)  /*  0.000785f */
#define BI2   (-1.4379e+03f / A0_I)  /* -0.006709f */
#define AI1   (-2.2867e+05f / A0_I)  /* -1.066881f (1 + AI1 + AI2 = 0 for integral action) */
#define AI2   ( 1.4335e+04f / A0_I)  /*  0.066881f */

/* --- GLOBAL VARIABLES --- */
volatile float Vout_Ref          	= 44.4f;
volatile uint8_t ticks_refresh_ramp = 500;
volatile float Vout_Ramp         	= 0.0f;
volatile float RAMP_STEP_V       	= 0.1f;
volatile float Duty_Phase[4]     	= {0.25f, 0.25f, 0.25f, 0.25f};

volatile float debug_v_error     	= 0.0f;
volatile uint8_t stat_vloop_sat  	= 0;
volatile uint8_t stat_iloop_sat  	= 0;

/* Internal Variables, not exposed for debug operation*/
static volatile float I_Ref = 0.0f;

/* --- FILTER ISTANCES --- */
static control_elem_discrete VoltageLoop = {
  .b0 = BV0, .b1 = BV1, .b2 = BV2, .a1 = AV1, .a2 = AV2,
  .e1 = 0.0f, .e2 = 0.0f, .u1 = 0.0f, .u2 = 0.0f,
  .u_min = 0.0f, .u_max = I_PHASE_MAX
};

static control_elem_discrete CurrentLoop[4] = {
  { .b0 = BI0, .b1 = BI1, .b2 = BI2, .a1 = AI1, .a2 = AI2, .u_min = DUTY_MIN, .u_max = DUTY_MAX },
  { .b0 = BI0, .b1 = BI1, .b2 = BI2, .a1 = AI1, .a2 = AI2, .u_min = DUTY_MIN, .u_max = DUTY_MAX },
  { .b0 = BI0, .b1 = BI1, .b2 = BI2, .a1 = AI1, .a2 = AI2, .u_min = DUTY_MIN, .u_max = DUTY_MAX },
  { .b0 = BI0, .b1 = BI1, .b2 = BI2, .a1 = AI1, .a2 = AI2, .u_min = DUTY_MIN, .u_max = DUTY_MAX }
};

/* --- PRIVATE FUNCTIONS --- */
static inline float Controller_Update(control_elem_discrete *ctrl_var, float error) {
    float u0 = (ctrl_var->b0 * error) + (ctrl_var->b1 * ctrl_var->e1) + (ctrl_var->b2 * ctrl_var->e2)
             - (ctrl_var->a1 * ctrl_var->u1) - (ctrl_var->a2 * ctrl_var->u2);

    if (u0 > ctrl_var->u_max) u0 = ctrl_var->u_max;
    if (u0 < ctrl_var->u_min) u0 = ctrl_var->u_min;

    ctrl_var->e2 = ctrl_var->e1;
    ctrl_var->e1 = error;
    ctrl_var->u2 = ctrl_var->u1;
    ctrl_var->u1 = u0;

    return u0;
}

static inline void Controller_Reset(control_elem_discrete *ctrl_var, float init_output) {
    if (init_output > ctrl_var->u_max) init_output = ctrl_var->u_max;
    if (init_output < ctrl_var->u_min) init_output = ctrl_var->u_min;

    ctrl_var->e1 = 0.0f;
    ctrl_var->e2 = 0.0f;
    ctrl_var->u1 = init_output;
    ctrl_var->u2 = init_output;
}


/* --- PUBLIC FUNCTIONS --- */
void ControlLoop_ResetAll(float initial_duty, float initial_current) {
    Controller_Reset(&VoltageLoop, initial_current);
    for (uint8_t i = 0; i < 4; i++) {
        Controller_Reset(&CurrentLoop[i], initial_duty);
    }
}

void ControlLoop_Execute(uint8_t closed_loop_enable, uint8_t num_active_phases,
                         float vout_meas, float *iphase_meas, float target_duty_open_loop) {
    static uint16_t ramp_prescaler = 0;

    if (closed_loop_enable == 1) {
        /* Initial Ramp for startup  */
        if (++ramp_prescaler >= ticks_refresh_ramp) {
            if (Vout_Ramp < Vout_Ref) {
                Vout_Ramp += RAMP_STEP_V;
                if (Vout_Ramp > Vout_Ref) Vout_Ramp = Vout_Ref;
            } else if (Vout_Ramp > Vout_Ref) {
                Vout_Ramp -= RAMP_STEP_V;
                if (Vout_Ramp < Vout_Ref) Vout_Ramp = Vout_Ref;
            }
            ramp_prescaler = 0;
        }

        /* Voltage Loop */
        float v_error = Vout_Ramp - vout_meas;
        debug_v_error = v_error;
        I_Ref = Controller_Update(&VoltageLoop, v_error);

        /* Current Reference saturation of the exposed variable (can be seen from the debugger)*/
        stat_vloop_sat = (I_Ref >= VoltageLoop.u_max || I_Ref <= VoltageLoop.u_min) ? 1 : 0;

        /* Inner Current Loop */
        stat_iloop_sat = 0;
        for (uint8_t i = 0; i < 4; i++) {
            if (i < num_active_phases) {
                float i_error = I_Ref - iphase_meas[i];
                Duty_Phase[i] = Controller_Update(&CurrentLoop[i], i_error);
                if (Duty_Phase[i] >= CurrentLoop[i].u_max || Duty_Phase[i] <= CurrentLoop[i].u_min) {
                    stat_iloop_sat = 1;
                }
            } else {
                Duty_Phase[i] = Duty_Phase[0];
                Controller_Reset(&CurrentLoop[i], Duty_Phase[0]);
            }
        }
    } else {
        /* Open Loop */
        float d_open = target_duty_open_loop / 100.0f;
        if (d_open < DUTY_MIN) d_open = DUTY_MIN;
        if (d_open > DUTY_MAX) d_open = DUTY_MAX;

        Vout_Ramp = vout_meas; /* The ramp voltage is set to the measured voltage to avoid additional functions */

        for (uint8_t i = 0; i < 4; i++) {
            Duty_Phase[i] = d_open;
        }
    }
}
