% Define the input parameters
V_out = 44.4;      % Output voltage [V]
Vin=12;
Vinmin=10;
Vinmax=18;
delta_i_in = 0.8;   % Input current ripple [A]
f_s = 400e3;       % Switching frequency [Hz]
D = 0.7297;         % Duty cycle (must be 0.5 <= D < 1)
Dmin=1-Vinmax/V_out;
Dmax=1-Vinmin/V_out;

d=linspace(Dmin, Dmax, 100);
P=100;
dv=0.1;          %Output Voltage Ripple [V]
Io=P/V_out;

L_boost = Vinmin*Dmax/(delta_i_in*f_s);
fprintf('Conventional Boost : The calculated inductance L is: %.3f uH\n', L_boost * 1e6);

L_2phase = max((V_out / (delta_i_in * f_s)) .* (2*d - 1) .* (1-d));
fprintf('2_PHASE : The calculated inductance L is: %.3f uH\n', L_2phase * 1e6);

L_3phase = max((V_out / (delta_i_in * f_s)) .* (3*d - 2) .* (1 - d));
fprintf('3_PHASE: The calculated inductance L is: %.3f uH\n', L_3phase * 1e6);

L_4phase = max((V_out / (delta_i_in * f_s)) .* (4*d - 3) .* (1 - d));
fprintf('4_PHASE: The calculated inductance L is: %.3f uH\n', L_4phase * 1e6);

C=Io*D/(dv*f_s);
fprintf('The calculated capacitance is: %.3f uF\n', C * 1e6);