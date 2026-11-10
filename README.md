# Four-Phases Interleaved Boost Converter - STM32 Firmware

![STM32](https://img.shields.io/badge/MCU-STM32F334-blue.svg)
![Control](https://img.shields.io/badge/Control-Dual%20Loop%20Current%20Mode-success.svg)
![Status](https://img.shields.io/badge/Status-Active-brightgreen.svg)

This repository contains the bare-metal C firmware for a custom **Four-Phases Interleaved Boost Converter**, powered by an STM32-F334R8 Nucleo Board. The system features a hard real-time digital control loop, dynamic phase shedding, and robust thermal protections, leveraging the STM32 High-Resolution Timer (HRTIM) for precise PWM generation and ADC synchronization.

## ⚙️ Hardware Overview

* **Target Board**: Custom "Four-Phases Interleaved Boost Converter"
* **Microcontroller**: STM32 (Cortex-M4 with Hardware FPU)
* **Switching Frequency ($f_{sw}$)**: 400 kHz per phase
* **Control Loop Frequency**: 50 kHz (Execute every 8 switching periods)
* **Sensors**: 
  * 1x Output Voltage, 1x Output Current (ADC Injected Channels)
  * 4x Inductor Phase Currents (ADC Regular Channels via DMA)
  * 1x NTC Temperature Sensor (Read via 12-bit ADC & Look-Up Table)

## 🏗️ Firmware Architecture

To ensure readability, maintainability, and scalability for future development, the firmware is strictly modularized into distinct logical units:

* `Core/Src/main.c`: Contains the Finite State Machine (FSM) managing Calibration, Open-Loop, Bumpless Transition, Closed-Loop, and Fault states.
* `Core/Src/control_loop.c`: Encapsulates the digital PI compensators (Tustin exact discretization), anti-windup logic, and the soft-start reference ramp.
* `Core/Src/analog_sensing.c`: Handles raw ADC to physical unit conversions (Voltage/Current gains), offset compensation, and $O(1)$ NTC temperature extraction via a 4096-element flash-stored LUT.
* `Core/Src/power_stage.c`: Abstracts the HRTIM hardware, managing the interleaved phase shifts ($90^\circ$, $120^\circ$, $180^\circ$), duty cycle updates, and dynamic phase shedding (1 to 4 active phases).
* `Core/Src/telemetry.c`: Manages asynchronous UART communications (115200 baud), handling real-time telemetry streaming and reference voltage updates.

## 🚀 Key Features

1. **Precise ADC Synchronization**: Analog sampling is triggered by the HRTIM exactly at $50\%$ of the $T_{on}$ interval of each phase, capturing the average inductor current and inherently rejecting switching noise without aliasing.
2. **Bumpless Transfer**: The transition from Open-Loop manual operation to Closed-Loop current-mode control is entirely bumpless. The integral memories of the digital IIR filters are pre-loaded with the actual duty cycle and measured currents right before closing the loop.
3. **Soft-Start Ramp**: To prevent inrush currents and integrator wind-up, the target voltage is reached through a digital ramp ($\Delta V$ per control cycle) governed by the high-priority ISR, completely independent of the asynchronous main loop.
4. **Thermal Protection (OTP)**: A background loop continuously monitors the power stage temperature. If $T_{NTC} \ge 100^\circ C$ (or if a sensor fault is detected, returning `-999.0`), the FSM immediately transitions to a `FAULT` state, disabling all HRTIM outputs.

## 🛠️ Usage & Debugging

### Compilation Flags
Ensure the project is compiled with Hardware FPU enabled (`-mfloat-abi=hard`) and optimization set to `-O3` or `-Ofast`. The control ISR performs heavily vectorized floating-point operations; without optimization, the FSM will suffer from SysTick starvation.

### Live Expressions (STM32CubeIDE)
During active debugging, the following global variables can be monitored or modified on the fly:
* `Vout_Meas_V`, `Iout_Meas_A`, `Iphase_Total_A`: Physical measurements.
* `temperature_deg_celsius`: Current NTC temperature.
* `ClosedLoopRequest`: Set to `1` to initiate the bumpless transfer to closed-loop.
* `TargetDutyPercent`: Adjust manual duty cycle in Open-Loop mode.

### UART Interface
Connect a Serial Terminal (115200 baud) to the telemetry port.
* **Telemetry**: Receives periodic strings formatted as `[CL] Vout: 44.40 V | Iout: 2.10 A ...`
* **Commands**: Send a float value terminated by `\n` (e.g., `48.5\n` or `48,5\r`) to dynamically update the voltage reference target (`Vout_Ref`).

---
*Developed for academic purposes in power electronics and digital control.*