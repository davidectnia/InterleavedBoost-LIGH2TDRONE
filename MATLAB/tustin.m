clear all
close all
clc

V_out = 44.4;      % Output voltage [V]
V_outmax = 50.4;
V_outmin = 38.4;

Vin=12;
Pin=100;

N=1                %Number of Interleaved stages, worst case 1(single phase boost), best 4(4 phases IBC)
R=V_out^2/Pin;     %Equivalent Load value to get 44.4V
L=27e-6;           %Inductance
Leq=L/N;           %Equivalent Inductance considering the interleaving stage

C=47e-6;           %Output Capacitance

f_s = 400e3;       % Switching frequency [Hz]
Dnom = 1-Vin/V_out;         % Duty cycle (must be 0.5 <= D < 1)
Dmin=1-Vin/V_outmin;
Dmax=1-Vin/V_outmax;
hv0= 3/50.4;       % gain of the output voltage sensor 3V/50.4 V (max adc input/max output voltage)
D=Dnom;            % To design the controller select the nominal duty cycle


kilx = 429;        % gain of the inner current loop compensator
w_cv=600*2*pi;           %voltage loop 
w_rhpz = (1-D)^2 * R/Leq;
w_lhpp = 2/(R*C);           %left hand plane pole

w_pv = w_rhpz;
w_zv = w_lhpp;

Kcv = kilx *w_cv /hv0/(R*(1-D));

s = tf('s');
Gcv = Kcv *(1+s/w_zv)/(1+s/w_pv) /s;

%Evaluation of the Inner Current Loop compensator K-factor method
w_lhpz = 2/(R*C);           %left hand plane zero 
q = R*(1-D)*sqrt(C/Leq);      %quality factor
w0 = (1-D)/(sqrt(Leq*C));   %natural frequency
gid0 = 2*Vin/(R*(1-D)^3);   %dc gain 
Gid = gid0 * (1 + s/w_lhpz) / (1 + s/(q*w0) + (s/w0)^2);

% 2 set the phase margin
pm=60;

% 3 current loop transfer function phase at wc
wcross = 2*pi*3.5e3;
[mag_gid, phase] = bode(Gid, wcross);


hall_phase = atand(wcross/(2*pi*50e3));

plantphase_tot = phase - hall_phase;
% 4 evaluate the total phase gain
aufa = pm - 180 - plantphase_tot +90;

% 5 evaluate the K-factor
k=tand(45+aufa/2);

% 6. Calcolo dello zero (in rad/s)
w_zc = wcross / k;

% 7. Calcolo del polo ad alta frequenza (in rad/s)
w_pc = wcross * k;

% 8. Calcolo del guadagno dell'integratore (in rad/s)
Sens = 0.066; %V/A
Gcond = 5.71; %gain of the conditioning circuitry
Vm = 3;       %peak value of pwm ramp
Hsense = Sens * Gcond;
% Calcolo della costante wp0c 
w_p0c = wcross / (Hsense * (1/Vm) * mag_gid * k);
% Funzione del compensatore (Tipo 2)
Gci = (w_p0c / s) * (1 + s/w_zc) / (1 + s/w_pc);


% Discretizzazione secondo Tustin dei compensatori analogici
Ts = 1 / 50e3; % 50 kHz = 400 kHz / 8 (REPETITON_RATE = 7)
Ci_z = c2d(Gci, Ts, 'tustin'); % Nuovi coefficienti a e b per l'anello di corrente
Cv_z = c2d(Gcv, Ts, 'tustin'); % Nuovi coefficienti a e b per l'anello di tensione

[bi, ai] = tfdata(Ci_z, 'v');
[bv, av] = tfdata(Cv_z, 'v');
