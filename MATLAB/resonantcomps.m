clear all
close all
clc


Vo=44.4;
Vin=12;
Po=100;
fs=400e3;
I0=Po/Vo;

RL=(Vo^2)/Po;
MCCM=Vo/Vin;

%RESONANT PARAMETERS
n=1
Q=MCCM;

%resonant frequency
f0=MCCM*fs/1.1;
%characteristic impedance
Z0=RL/Q;

%Duty cycle evaluation for h=0
%D=1-0.9092*fs/f0
D=1-0.9092*1.1/MCCM;

%Resonant Inductance(choose it lower than this value)
LR=RL/(2*pi*f0*MCCM);
%Resonant Capacitance(choose it higher than this value)
CR=Q/(2*pi*f0*RL);
%Peak switch current
ISM=MCCM*I0;
%Peak switch voltage
VSM=(MCCM/Q+1)*Vin;
%Peak diode current
IDM=2*MCCM*I0;
%Peak diode voltage
VDM=Vo;
