% =========================================================================
% INDUCTOR AND CONVERTER DESIGN
% =========================================================================
clear all
close all
clc

% Converter type: 1 = Buck, 2 = Boost, 3 = Buck-Boost, 4 = Flyback, etc.
converterType = 2; 

% Converter symmetry: 0 = Buck, Boost, Flyback; 1 = Push-Pull, Bridge, etc.
converterSymmetry = 0;

% -------------------------------------------------------------------------
% INPUT DATA
% -------------------------------------------------------------------------
vo = 44.4;           % Tensión de salida
vinmin = 10;         % Tensión de entrada continua mínima
Io = 100/vo;           % Corriente de salida
fs = 400E3;      % Frecuencia de conmutación
Nps = 0;   % Relaciones de transformación
Bsat = 0.41;   % Densidad de flujo de saturación
Kh = 4e-5;   % Coeficiente de histéresis
Ke = 4e-10;  % Coeficiente de Eddy current
Ta = 30;      % Temperatura ambiente (ºC)
delta_Tmax = 30;      % Máximo aumento de temperatura (ºC)

% -------------------------------------------------------------------------
% PRELIMINARY CALCULATIONS
% -------------------------------------------------------------------------
% TODO: Formulas needed for the following variables
d = 1-vinmin/vo;               % Ciclo de trabajo
fl = 400e3;    % Frecuencia de trabajo de la bobina
ildc = 8;           % Valor de continua de corriente por la bobina

L = 27e-6;              % Inductancia
deltail = vo*(1-d)/fl/L;       % Rizado de corriente por la bobina

ilmax = ildc+deltail/2;          % Valor máximo de corriente por la bobina
ilrms = 8;          % Valor eficaz de corriente por la bobina

% -------------------------------------------------------------------------
% STEP 1: Core Size and Flux Density Variation
% -------------------------------------------------------------------------

Ku= 0.7; % Coeficiente de utilización de la ventana
K = 0.7;                % Factor de arrollamiento
AP1 = ((L*ilmax*ilrms*1e4)/(420*K*Bsat))^1.31;               % Valor mínimo del producto de áreas (AP)
AP2 = ((L*deltail*ilrms*1e4)/(130*K))^1.58 * (Kh*fl+Ke*fl^2)^0.66;               % Valor mínimo del producto de áreas (AP)

APmin=max(AP1, AP2);

% Core Data (From Catalog)
% -------------------------------------------------------------------------
% CORE WINDOW GEOMETRY CALCULATIONS
% -------------------------------------------------------------------------

% Given / from datasheet (cm)
window_h= 1.08;
window_w = 0.845/2;   

% Effective core data (already in your code)
Ae = 0.64;     % cm^2
Aw = window_h* window_w;

AP = Ae * Aw;


% Volume (given)
Vol = 2.43; % cm^3

fprintf('Medidas de la ventana:\n');
fprintf('  Window Height= %.3f cm\n', window_h);
fprintf('  Window Width = %.3f cm\n', window_w);

fprintf('\nÁreas:\n');
fprintf('  Ae = %.3f cm^2\n', Ae);
fprintf('  Aw = %.3f cm^2\n', Aw);
fprintf('  AP = %.3f cm^4\n', AP);
fprintf('  APmin = %.3f cm^4\n', APmin);
fprintf('  Vol = %.3f cm^3\n', Vol);

% -------------------------------------------------------------------------
% MEAN LENGTH PER TURN (MLT)
% -------------------------------------------------------------------------

diapat = 0.44; % cm (central leg diameter)

% Window limits (from geometry)
window_w_min = 0.45;
window_w_max = 0.935;

% Perimeters
perimetro_min = 2*pi*(window_w_min/2);
perimetro_max = 2*pi*(window_w_max/2);

% Mean Length per Turn
mlt = (perimetro_min + perimetro_max)/2;


delta_Bmax1 = Bsat;
delta_Bmax2 = (0.405 * AP^(-0.129))/((Kh*fs+Ke*fs^2)^0.417);

delta_Bmax = min(delta_Bmax1, delta_Bmax2);
% -------------------------------------------------------------------------
% STEP 2: Number of Turns
% -------------------------------------------------------------------------
Nmin1 = ceil(L*ilmax*1e4/(Bsat*Ae));           % Pcore<<Pcu
Nmin2 = ceil(L*deltail*1e4/(delta_Bmax*Ae));     % Pcore circa Pcu
N= max(Nmin1, Nmin2);

delta_Breal = L*deltail/(N*Ae*1e-4); % Variación de B que se va a producir realmente
% -------------------------------------------------------------------------
% STEP 3: Airgap Length cm
% -------------------------------------------------------------------------
lg = (N^2) * Ae * (1e-2) * 4 * pi * (1e-7) /(2*L);        % Airgap E core [cm]

%The AWG recommended is 16 AWG since the inductor is characterized by a
%good airflow, the related area is 1.309 mm2. In the lab we should have the
%AWG17 which has a wire area of 1.04 mm2. I'll work with the AWG17

% -------------------------------------------------------------------------
% STEP 4: Winding Calculation
% -------------------------------------------------------------------------


i1rms = ilrms;       % Máxima densidad de corriente permitida
dim1 = 1.04;             % Diámetro del cable AWG17 [mm2]
A_cul1 = (dim1^2)* (pi/4)/100;     % Cable section

J1=i1rms/A_cul1;


% -------------------------------------------------------------------------
% STEP 4.1: Seccion del conductor
% -------------------------------------------------------------------------
J30= 420 * AP^(-0.240);%[A/cm2]

J15= 297 * AP^(-0.24);%[A/cm2]


%Jmax=min(J30, J15);%[A/cm2]
Jmax=J30; % Mariues says so because the AWG is much bigger with J30
%Discuss the difference that comes by choosing different values of J but
%the main reason is for building purposes, check Mariues TFG.


scable_min= ilrms/Jmax; %[cm2]Theoretical Minimum Section of the cable
diameter_cable_min=2 *sqrt(scable_min/pi); %[cm]
fprintf("...So the minimum cable section is %.2f cm2\n", scable_min);

%AWG and section are inverse proportional
awgcable = floor(-(log(10*diameter_cable_min/0.127))*39/log(92)+36); %Minimum commercially available
diameter_awg_cable = 0.0127 * 92^((36-awgcable)/39);%[cm]
scable = pi * (diameter_awg_cable/2)^2;%[cm2]


fprintf("Normalized cable cross-section AWG%d: %.2f cm\n", awgcable, diameter_awg_cable)

T=Ta+delta_Tmax; %Working temperature
rho = 1.724 * (1 + 0.0042 * (T - 20)) * 1e-8; %Cable resistivity
delta = sqrt(rho / (4 * pi^2 * 1e-7 * fl)) * 10^2; %Skin depth
ratio = (2 * delta) / diameter_awg_cable; %Ratio between skin depth and diameter
scable_nolitz = pi * delta * (diameter_awg_cable - delta); %[cm2]



if delta < diameter_awg_cable/4 %  If the skin depth is smaller than half radius, the section is too small and we need the Litz wire
    
    diameter_str_max=2*delta; %The worst case diameter

    awg_str_min=floor(-(log(10*diameter_str_max/0.127))*39/log(92)+36);
    awg_str_max=awgcable;
    
    diameter_strand = 0.020; %[cm] Minimum diameter found in lab 0.2mm
    awg_strand = floor(-(log(10*diameter_strand/0.127))*39/log(92)+36);
    fprintf('Selecting AWG%d (diameter %.2f mm)for the Litz wire... \n', awg_strand, diameter_strand*10);
    
    scable_strand = pi * (diameter_strand/2)^2; %[cm2]
    
    %measures in cm
    window_h = 1.08;
    window_w = 0.422;

    % Numero di strand
    N_strands = ceil(scable_min / scable_strand);
    
    % Sezione totale Litz
    scable_litz = N_strands * scable_strand;
    
    % Diametro equivalente (con fattore di riempimento K)
    diameter_litz = sqrt(4 * scable_litz / (K * pi));
    
    % Margine realistico (isolamento + vuoti)
    k_margin = 1.1;
    d_eff = k_margin * diameter_litz;
    
    % Numero spire in verticale
    vertical_litz = floor(window_h / d_eff);
    
    % Numero strati (orizzontale)
    horizontal_litz = ceil(N / vertical_litz);
    
    % Ingombri reali
    winding_h = vertical_litz * d_eff;
    winding_w = horizontal_litz * d_eff;
    
    fprintf('--- Litz winding ---\n');
    fprintf('Strands: %d\n', N_strands);
    fprintf('Equivalent diameter: %.3f mm (including Fill Factor due to air (K=1.1)\n', d_eff*10);
    fprintf('Vertical Layer(s): %d\t', vertical_litz);
    fprintf('Horizontal Layer(s): %d\n', horizontal_litz);
    fprintf('Winding height: %.3f cm (max %.3f)\t', winding_h, window_h);
    fprintf('Winding width: %.3f cm (max %.3f)\n', winding_w, window_w);

    if winding_w < window_w
        fprintf('[OK] Winding fits in the window\n');

        % -------------------------------------------------------------------------
        % LITZ WIRE LENGTH ESTIMATION (MINIMAL OUTPUT)
        % -------------------------------------------------------------------------
        
        % Inputs (assumed already defined)
        % N   = number of turns
        % mlt = mean length per turn [cm]
        % Nx  = number of layers (horizontal_litz)
        
        % --- Base length
        L_base = N*N_strands * mlt; %[cm]
        
        % --- Layer correction (+2% per layer)
        k_layer = 1 + 0.02 * (horizontal_litz - 1);
        L_layers = L_base * k_layer;%[cm]
        
        % --- Lead length (per side)
        lead_per_side = 2; % cm
        L_leads = 2 * lead_per_side;
        
        % --- Process margin (+10%)
        k_process = 1.10;
        L_process = (L_layers + L_leads) * k_process;
        
        % --- Safety margin (+10%)
        k_safety = 1.10;
        L_total = L_process * k_safety;
        
        % --- Commercial rounding (to nearest 10 cm)
        L_commercial = ceil(L_total / 10) * 10;
        
        % -------------------------------------------------------------------------
        % OUTPUT (essential only)
        % -------------------------------------------------------------------------
        fprintf('Min length: %.2f cm\n', L_layers + L_leads);
        fprintf('Recommended: %.2f cm (%.2f m)\n', L_total, L_total/100);
        fprintf("Cut: %d strands with %d cm(%.2f cm) length \n", N_strands, ceil(L_total/N_strands), L_total/N_strands)
        fprintf('Buy: %.2f cm (%.2f m)\n', L_commercial, L_commercial/100);
    else
        fprintf('[FAIL] Winding do not fit in the window\n');
    end
        scable = scable_litz;

else
    fprintf('Litz wire cannot be used\n');
end


% -------------------------------------------------------------------------
% STEP 5: Thermal/Magnetic Losses
% -------------------------------------------------------------------------
%The MLT is the average width per winding, the theoretical expression is
mlt_calc=diapat* pi *1.3;

rho = 1.724 * (1 + 0.0042 * (T - 20)) * 1e-6; % maximum core temperature
R = rho * mlt * N / scable; %Resistive component
R_nolitz = rho * mlt * N / scable_nolitz; %Resistive component



%Magnetic losses
fprintf("Processing magnetic losses ...\n");
Pcore_cm3 = delta_Breal^2.4 * (Kh*fs+Ke*fs^2);
Pcore_abs = Pcore_cm3 * Vol;
fprintf("Magnetic Losses: %.2f W \n", Pcore_abs);
Pcu=R * ilrms^2;
fprintf("Copper Losses: %.2f W \n", Pcu);
Ptot = Pcu + Pcore_abs;
fprintf("Total Losses: %.2f W \n", Ptot);

Pcu_nolitz= R_nolitz *ilrms^2;
Ptot_nolitz = Pcu_nolitz + Pcore_abs;

%Core warming estimation
Rth = 23*AP^-0.37;
delta_T = Rth*Ptot; %overall temperature change
delta_Tnolitz = Rth*Ptot_nolitz; %overall temperature change with no litz wire


% --- EXACT LEAKAGE INDUCTANCE CALCULATION ---
mu_0 = 4 * pi * 1e-7;
R_in = diapat / 2; % Core center leg radius in cm
b_eff = winding_w; % Actual winding width in cm
h_eff = winding_h; % Actual winding height in cm
d_gap = window_w - b_eff; % Empty space in the window in cm

% Convert dimensions to meters for Henry calculation
R_in_m = R_in / 100;
b_eff_m = b_eff / 100;
h_eff_m = h_eff / 100;
d_gap_m = d_gap / 100;

% Mean radius of the empty space
R_gap_mean_m = R_in_m + b_eff_m + (d_gap_m / 2);

% Internal (wire) contribution using exact integration
L_int = mu_0 * (N^2 / h_eff_m) * 2 * pi * b_eff_m * ((R_in_m / 3) + (b_eff_m / 12));

% External (empty space) contribution
if d_gap_m > 0
    L_ext = mu_0 * (N^2 / h_eff_m) * 2 * pi * R_gap_mean_m * d_gap_m;
else
    L_ext = 0;
end

L_leak_tot = L_int + L_ext;
fprintf("Parasitic L due to internal wire:  %.2f nH\n", L_int*1e9);
fprintf("Parasitic L due to airgap:         %.2f nH\n", L_ext*1e9);
fprintf('Exact Leakage Inductance:          %.2f nH\n', L_leak_tot*1e9);
tipo=2;
nucleo= "B65811";
fprintf('==========================================\n');
fprintf('       INDUCTOR DESIGN FINAL REPORT       \n');
fprintf('==========================================\n');
fprintf('Core Selected:      %s (Type %d)\n', nucleo, tipo);
fprintf('Inductance:         %.2e H\n', L);
fprintf('Number of Turns:    %d\n', N);
fprintf('Air Gap (lg):       %.3f cm\n', lg);
fprintf('------------------------------------------\n');
fprintf('WINDING DETAILS:\n');
fprintf('Wire Gauge (AWG):   %d\n', awgcable);
fprintf('Litz Wire:          ( AWG%d) TOTAL strands:%d \t X:%d layers\t Y:%d layers\n', awg_strand, N_strands, horizontal_litz, vertical_litz);
fprintf('------------------------------------------\n');
fprintf('VERIFICATION CHECKS:\n');

% Check Area Product
if AP > APmin
    fprintf('[OK] AP: %.3f > %.3f cm^4\n', AP, APmin);
else
    fprintf('[FAIL] AP: %.3f is too small!\n', AP);
end

% Check Flux Density
if delta_Breal < delta_Bmax
    fprintf('[OK] deltaB: %.3f < %.3f T\n', delta_Breal, delta_Bmax);
else
    fprintf('[FAIL] Saturation Risk! deltaB too high.\n');
end

% Check Window Fit
if winding_w < window_w
    fprintf('[OK] Winding fits: %.3f < %.3f cm\n',winding_w , window_w);
else
    fprintf('[FAIL] Winding too wide for core window.\n');
end


% Check Temperature Rise
if delta_T < delta_Tmax
    fprintf('[OK] Thermal: deltaT = %.2f C (< %.d C)\n', delta_T, delta_Tmax);
else
    fprintf('[FAIL] Overheating! deltaT exceeds limit.\n');
end

if delta_Tnolitz < delta_Tmax
    fprintf('[OK] NO LITZ Thermal: deltaT = %.2f C (< %.d C)\n', delta_Tnolitz, delta_Tmax);
else
    fprintf('[FAIL] WITH NO LITZ Overheating! deltaT exceeds limit.\n');
end
fprintf('==========================================\n');