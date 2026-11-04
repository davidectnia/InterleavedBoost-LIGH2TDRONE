/*
 * telemetry.h
 *
 *  Created on: Nov 4, 2026
 *      Author: Davide Catania
 */

#ifndef TELEMETRY_H_
#define TELEMETRY_H_

#include "stm32f3xx_hal.h" /* Necessary for UART_HandleTypeDef */

/**
 * @brief Start listening in Interrupt mode
 * @param huart UART pointer
 */
void Telemetry_Init(UART_HandleTypeDef *huart);

/**
 * @brief Analyse the messages received periodically
 * @param huart UART Pointer
 */
void Telemetry_Process(UART_HandleTypeDef *huart);

#endif /* INC_TELEMETRY_H_ */
