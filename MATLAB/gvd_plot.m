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
g_compv = Kcv *(1+s/w_zv)/(1+s/w_pv) /s;


gvc0 = R *(1-D)/kilx;
gvd = gvc0 *(1-s/w_rhpz) / (1+s/w_lhpp);
hv = hv0;

tv =g_compv *gvd * hv;
bode(tv)
margin(tv)
