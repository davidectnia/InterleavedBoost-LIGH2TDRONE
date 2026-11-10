/*
 * power_stage.h
 *
 *  Created on: Nov 4, 2026
 *      Author: Davide Catania
 */

#ifndef POWER_STAGE_H_
#define POWER_STAGE_H_

#include <stdint.h>

void PowerStage_UpdatePhases(uint8_t active_phases);
void PowerStage_ApplyDutyCycles(volatile int32_t *duty_array_q16);
void PowerStage_EmergencyStop(void);

#endif /* POWER_STAGE_H_ */
