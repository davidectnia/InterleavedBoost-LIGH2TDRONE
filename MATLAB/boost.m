clear all
close all 
clc

Vinmin=10;
Vinmax=18;
Vin=12;
Iin=8;

Vomax=50.4;
Vomin=38.4;
Vo=(Vomax+Vomin)/2;
Po=100;%12V in nominal, 8A out max 
deltaV=0.1;
deltaIL=0.1*Iin;
fs=400e3;

Dnom=1-Vin/Vo;
Dmin=1-Vinmax/Vo;
Dmax=1-Vinmin/Vo;

D2=1-Vinmin/Vomin;
R=(Vo^2)/Po;

Lmin1=(4/27)*R/(2*fs);
Lmin2=Vinmin*Dmax/(deltaIL*fs)
%Current Ripple Across CO
L1=5.2e-6 %my power electronics method which does not satisfy input current ripple
L2=27e-6 %noemi method


Io_avg=Vin*Iin/Vo;
Imax=Io_avg+deltaIL/2;

ESRmax=deltaV/Imax;

fz=5e3;
Comin1=1/(2*pi*ESRmax*fz); %ELECTROLYTHIC
Comin2=Iin*(1-Dmin)*Dmin/(deltaV*fs) ; %CERAMIC

Comin_charge=Iin*(1-D2)*D2/(deltaV*fs)