clear all
close all
clc

% Given parameters (replace these placeholder values with your actual data)
C1 = 330e-12;       % 330 pF in Farads
f_p1 = 16.8e3;         % Pole frequency in Hz (example value)
f_z1 = 730;        % Zero frequency in Hz (example value)
K_ci = -429;          % Gain/coefficient value (example value)

% Calculations based on your formulas
R2 = 1 / (2 * pi * f_p1 * C1);
C2 = 1 / (2 * pi * f_z1 * R2);
R1 = -R2 / K_ci;

% Display the results
fprintf('Calculated Component Values:\n');
fprintf('----------------------------\n');
fprintf('R1 = %.2f Ohms\n', R1);
fprintf('R2 = %.2f Ohms\n', R2);
fprintf('C2 = %.2e F (%.2f pF)\n', C2, C2 * 1e12);