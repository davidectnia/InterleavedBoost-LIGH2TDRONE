%% DDBC Design Parameters Evaluation
% Based on: 
% [1] WhatsApp Image 2026-03-02 at 16.39.15.jpeg (Design Specs)
% [2] Voltage_and_Current_Switching-Ripple_Cancelation...pdf (Topology & Formulas)

clear all
close all
clc

%% 1. Input Specifications
Vin_nom = 12;      % [V]
Vin_min = 10;      % [V]
Vin_max = 18;      % [V]
Pin = 100;         % [W] Nominal Input Power
fs = 400e3;        % [Hz] Switching Frequency
Ts = 1/fs;

%% 2. Output Specifications
Vout_nom = 44.4;   % [V] (12 cells * 3.7V)
Vout_max = 50.4;   % [V] (12 cells * 4.2V)
Vout_min = 38.4;   % [V] (12 cells * 3.2V)
Pout = Pin * 0.95; % Assuming 95% efficiency for sizing
Iout_nom = Pout / Vout_nom;
R_load = Vout_nom / Iout_nom; % Equivalent nominal load resistance

%% 3. Design for Zero Ripple at Nominal Point
% Nominal Gain
G_nom = Vout_nom / Vin_nom;

% Solve for the design duty cycle (D_bar) using the gain equation [cite: 114, 168]
% Formula derived from G = (1-D+D^2)/(D-D^2) at k = (1-D)/D
coeffs = [(G_nom + 1), -(G_nom + 1), 1];
d_roots = roots(coeffs);
D_bar = max(d_roots); % We select the higher duty cycle for boost operation [cite: 170]

% Proportionality constant k [cite: 124, 165]
k = (1 - D_bar) / D_bar;

%% 4. Inductor and Capacitor Sizing
% Target inductor ripple (e.g., 30% of average inductor current)
delta_iL_target = 0.30 * (Pin / Vin_nom / 2); 

% L1 Calculation [cite: 216]
L1 = (Vin_nom * D_bar * Ts) / delta_iL_target;
L2 = k * L1; % [cite: 124]

% Target output voltage ripple (e.g., 1% of Vout)
delta_Vout_target = 0.01 * Vout_nom;

% C1 Calculation [cite: 343]
C1 = (Iout_nom * D_bar * Ts) / delta_Vout_target;
C2 = k * C1; % [cite: 345]

%% 5. Results Display
fprintf('--- DDBC Design Parameters ---\n');
fprintf('Nominal Gain (G): %.2f\n', G_nom);
fprintf('Design Duty Cycle (D_bar): %.4f\n', D_bar);
fprintf('Constant k: %.4f\n', k);
fprintf('Duty Cycle 1 (D1): %.4f\n', D_bar);
fprintf('Duty Cycle 2 (D2 = k*D1): %.4f\n', k * D_bar);
fprintf('\n--- Component Values ---\n');
fprintf('Inductor L1: %.2f uH\n', L1 * 1e6);
fprintf('Inductor L2: %.2f uH\n', L2 * 1e6);
fprintf('Capacitor C1: %.2f uF\n', C1 * 1e6);
fprintf('Capacitor C2: %.2f uF\n', C2 * 1e6);
fprintf('Load Resistance (R): %.2f Ohm\n', R_load);