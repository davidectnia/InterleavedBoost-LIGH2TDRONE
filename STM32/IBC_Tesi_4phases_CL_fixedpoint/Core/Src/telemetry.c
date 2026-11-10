/*
 * telemetry.c
 *
 *  Created on: Nov 4, 2026
 *      Author: Davide Catania
 */

#include "telemetry.h"
#include "control_loop.h"
#include "analog_sensing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* --- Module Private Variables--- */
static uint8_t  uart_rx_byte;
static char     uart_rx_buffer[16];
static uint8_t  uart_rx_index = 0;
static volatile uint8_t uart_rx_ready = 0;
static uint32_t last_telemetry_tick = 0;

/* Import Closed loop status for message*/
extern volatile uint8_t ClosedLoopEnable;

/* --- Public Functions --- */
void Telemetry_Init(UART_HandleTypeDef *huart) {
    HAL_UART_Receive_IT(huart, &uart_rx_byte, 1);
}

void Telemetry_Process(UART_HandleTypeDef *huart) {
    if (uart_rx_ready) {
        float new_ref = strtof(uart_rx_buffer, NULL);
        if (new_ref >= 5.0f && new_ref <= 60.0f) {
            Vout_Ref_Q16 = FLOAT_TO_Q16(new_ref); // Converted outside ISR
            char msg_ref[64];
            int ref_10 = (int)(new_ref * 10.0f + 0.5f);
            snprintf(msg_ref, sizeof(msg_ref), "Nuovo Vout_Ref: %d.%d V\r\n", ref_10 / 10, ref_10 % 10);
            HAL_UART_Transmit(huart, (uint8_t*)msg_ref, strlen(msg_ref), 50);
        }
        uart_rx_index = 0;
        uart_rx_ready = 0;
    }

    if (HAL_GetTick() - last_telemetry_tick >= 200) {
        last_telemetry_tick = HAL_GetTick();

        /* Convert Q16.16 to float purely for logging purposes */
        float vout = Q16_TO_FLOAT(Vout_Meas_Q16);
        float iout = Q16_TO_FLOAT(Iout_Meas_Q16);
        float iph1 = Q16_TO_FLOAT(Iphase_Meas_Q16[0]);
        float duty = Q16_TO_FLOAT(Duty_Phase_Q16[0]);

        int vout_10 = (int)(vout * 100.0f + 0.5f);
        int iout_10 = (int)(iout * 100.0f + 0.5f);
        int iph1_10 = (int)(iph1 * 100.0f + 0.5f);
        int duty_10 = (int)(duty * 1000.0f + 0.5f);

        char telem[120];
        snprintf(telem, sizeof(telem),
                 "[%s] Vout: %d.%02d V | Iout: %d.%02d A | Iph1: %d.%02d A | D1: %d.%d%%\r\n",
                 ClosedLoopEnable ? "CL" : "OL",
                 vout_10 / 100, vout_10 % 100,
                 iout_10 / 100, iout_10 % 100,
                 iph1_10 / 100, iph1_10 % 100,
                 duty_10 / 10,  duty_10 % 10);
        HAL_UART_Transmit(huart, (uint8_t*)telem, strlen(telem), 50);
    }
}

/* --- Callback Interrupt HAL  --- */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        if (uart_rx_byte == '\r' || uart_rx_byte == '\n') {
            if (uart_rx_index > 0) {
                uart_rx_buffer[uart_rx_index] = '\0';
                for (uint8_t i = 0; i < uart_rx_index; i++) {
                    if (uart_rx_buffer[i] == ',') uart_rx_buffer[i] = '.';
                }
                uart_rx_ready = 1;
            }
        } else if (uart_rx_index < sizeof(uart_rx_buffer) - 1) {
            uart_rx_buffer[uart_rx_index++] = (char)uart_rx_byte;
        }
        HAL_UART_Receive_IT(huart, &uart_rx_byte, 1);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        __HAL_UART_CLEAR_OREFLAG(huart);
        __HAL_UART_CLEAR_FEFLAG(huart);
        __HAL_UART_CLEAR_NEFLAG(huart);
        HAL_UART_Receive_IT(huart, &uart_rx_byte, 1);
    }
}
