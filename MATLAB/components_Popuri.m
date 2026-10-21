%Improved Interleaved Boost Converter with
%Soft-Switching: Analysis and Experimental
%Validation- Popuri

% Given Inputs
Vo = 44.4;          % Output voltage (V)
Vin = 12;           % Input voltage (V)
delta_Vo = 0.100;   % Output voltage ripple (V)
fs = 400e3;         % Switching frequency (Hz)
i_in = 8;           % Input current (A)
Ca = 1e-9;          % Resonant capacitor Ca (F) - FIXED INPUT

% Assumed/Missing Inputs (Replace with your actual design requirement)
delta_iin = 0.8;    % Assumed Input current ripple (A) - STILL MISSING

% 1. Calculate Duty Cycle (D) and Voltage Gain (M)
D = 1 - (Vin / Vo);
M = Vo / Vin;

% 2. Calculate Output Current (Io) and Output Resistance (Ro)
Io = i_in / M;
Ro = Vo / Io;

% 3. Calculate Output Capacitor (Co)
Co = (D * Vo) / (Ro * delta_Vo * fs);

% 4. Calculate Input Inductors (L1, L2)
% Assuming L = L1 = L2
L = (Vin * D) / (delta_iin * fs); 

% 5. Calculate Resonant Elements (La, Cb, Lb)
Q = 3;              % Q factor is equal to 3
Zo = Ro / Q;        % Characteristic impedance

% Since Zo = sqrt(La / Ca), we can solve for La:
La = Ca * (Zo^2);

% The converter is symmetrical, so Lb = La and Cb = Ca
Lb = La;
Cb = Ca;

% Display Results
fprintf('Calculated Component Values:\n');
fprintf('Duty Cycle (D): %.3f\n', D);
fprintf('Load Resistance (Ro): %.2f Ohms\n', Ro);
fprintf('Output Capacitor (Co): %.2f uF\n', Co * 1e6);
fprintf('Input Inductors (L1, L2): %.2f uH\n', L * 1e6);
fprintf('Resonant Inductors (La, Lb): %.2f uH\n', La * 1e6);
fprintf('Resonant Capacitors (Ca, Cb): %.2f nF\n', Ca * 1e9);