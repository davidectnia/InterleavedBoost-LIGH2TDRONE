clear all
close all
clc

% =========================================================================
% INDUCTOR AND CONVERTER DESIGN
% =========================================================================
clear all
close all
clc

% Converter type: 1 = Buck, 2 = Boost, 3 = Buck-Boost, 4 = Flyback
converterType = 2;

% Converter symmetry: 0 = asymmetric (Buck/Boost/Flyback), 1 = symmetric (Push-Pull/Bridge)
converterSymmetry = 0;

% =========================================================================
% INPUT PARAMETERS
% =========================================================================
vo        = 44.4;           % Output voltage [V]
vomin     = 38.4;           % Minimum Output Voltage [V]
vomax     = 50.4;           % Minimum Output Voltage [V]
vin       = 12;
vinmin    = 10;             % Minimum input voltage [V]
vinmax    = 18;             % Minimum input voltage [V]
Io        = 100 / vo;       % Output current [A]
fs        = 400e3;          % Switching frequency [Hz]
Nps       = 0;              % Turns ratio (transformer; 0 if not applicable)
Bsat      = 0.41;           % Saturation flux density [T]
Kh        = 4e-5;           % Hysteresis loss coefficient
Ke        = 4e-10;          % Eddy current loss coefficient
Ta        = 30;             % Ambient temperature [°C]
delta_Tmax = 30;            % Maximum allowable temperature rise [°C]

%Diode Voltage
v_f = 1;

%MOSFET voltage
vds_on = 20e-3; %rds_on=2.5mOhm, Ilmax=8+0.8/2
% vin_v=linspace(vinmin, vinmax, 100);
% vo_v=linspace(vomin, vomax, 100);

% =========================================================================
% PRELIMINARY CALCULATIONS
% =========================================================================
% d_v       = (vo_v - vin_v + v_f) ./ (vo_v - vds_on + v_f);

% dmax1= max(d_v);
% dmin1 = min(d_v);


dmax      = 1 - vinmin / vomax;          % Ideal Duty cycle
dmin      = 1 - vinmax / vomin;
d         = 1 - vin / vo;

d_real_nom      = (vo - vin + v_f) / (vo - vds_on + v_f);
d_real_max      = (vomax - vinmin + v_f) / (vomax - vds_on + v_f);
d_real_min      = (vomin - vinmax + v_f) / (vomin - vds_on + v_f);
d_real_worst    = (vomin - vinmin + v_f) / (vomin - vds_on + v_f);

fl     = 400e3;                    % Inductor operating frequency [Hz]
%ildc   = 8;                        % Inductor DC current [A]
ildc   = 4;                         % Inductor DC maximum current (2 phases IBC) [A]
%ildc   = 2;                         % Inductor DC maximum current (4 phases IBC) [A]


deltailmax = 0.1 * ildc;           % Maximum Ripple accepted by Fuel Cell [A]
%Assuming the same ripple for all

% For d_real_worst we have the maximum current at the input to charge 
%Lmin  = vinmin*d_real_worst/(deltailmax*fl); % Minimum Inductance to comply with the ripple [H] @vinmin
% valid only for conventional boost converter

%L = 33e-6;                  % Commercial value of Inductance [H]
%Lmin = 27.33E-6;              % Minimum Conventional BC
L = 22E-6;                % 2-IBC
Lmin = 17.4E-6;                 % 2-IBC minimum
%L = 3.9E-6;               % 4-IBC
%Lmin = 3.8E-6;               % 4-IBC minimum

L=Lmin;

% We are always evaluating the worst case ripple in the specific situation
% when we are extracting the maximum current from the Fuel Cell, when the
% LiPO stack is completely discharged.

%Actual ripple with the chosen inductor value
%deltail = abs(vin - v_f - vomin) * (1 - d_real_worst) / fl / L;
deltail = vin * d  / fl / L;
%deltail = 4;

ilmax  = ildc + deltail / 2;       % Peak inductor current [A]
ilrms  = sqrt( ildc^2 + (deltail^2)/12);                        % Inductor RMS current [A]

% =========================================================================
% STEP 1 — Core Size Selection and Flux Density Swing
% =========================================================================
Ku   = 0.7;   % Window utilisation factor
K    = 0.7;   % Winding fill factor

% Minimum area product — copper-loss-limited (Pcore << Pcu)
AP1 = ((L * ilmax * ilrms * 1e4) / (420 * K * Bsat))^1.31;

% Minimum area product — combined loss criterion (Pcore ≈ Pcu)
AP2 = ((L * deltail * ilrms * 1e4) / (130 * K))^1.58 * (Kh * fl + Ke * fl^2)^0.66;

APmin = max(AP1, AP2);

% -------------------------------------------------------------------------
% Core Data (from datasheet)
% -------------------------------------------------------------------------
window_h = 1.08;        % Window height [cm]
window_w = 0.845 / 2;   % Half window width [cm]  (one side of E-core)

Ae  = 0.52;             % Effective cross-sectional area [cm²]
                        % with center hole
Aw  = window_h * window_w; % Window area [cm²]
AP  = Ae * Aw;          % Area product [cm⁴]
Vol = 1.825;             % Core volume [cm³]
                        % with center hole

fprintf('============================================================\n');
fprintf('  CORE GEOMETRY\n');
fprintf('============================================================\n');
fprintf('  Window height : %6.3f cm\n', window_h);
fprintf('  Window width  : %6.3f cm\n', window_w);
fprintf('  Ae            : %6.3f cm²\n', Ae);
fprintf('  Aw            : %6.3f cm²\n', Aw);
fprintf('  AP            : %6.3f cm⁴   (required: %.3f cm⁴)\n', AP, APmin);
fprintf('  Core volume   : %6.3f cm³\n', Vol);

% =========================================================================
% MEAN LENGTH PER TURN (MLT)
% =========================================================================
diapat = 0.44;           % Central leg diameter [cm]

% OLD VERSION REVIEWED 12/05
%window_w_min = 0.45;     % Inner winding radius boundary [cm]
%window_w_max = 0.935;    % Outer winding radius boundary [cm]

%perimeter_min = 2 * pi * (window_w_min / 2);
%perimeter_max = 2 * pi * (window_w_max / 2);
%mlt = (perimeter_min + perimeter_max) / 2;  % Mean length per turn [cm]

% NEW VERSION WRITTEN ON 12/05

radius_min = 0.855 / 2;    %from the center to the window [cm]
radius_max = 1.76 / 2;     %from the center to the end of the window [cm]

mean_radius = (radius_min + radius_max)/2; %mean radius from the core center [cm]

mlt = 2 * pi * mean_radius; % Mean length per turn [cm]

% Maximum allowable flux density swing
delta_Bmax1 = Bsat;
delta_Bmax2 = (0.405 * AP^(-0.129)) / ((Kh * fs + Ke * fs^2)^0.417);
delta_Bmax  = min(delta_Bmax1, delta_Bmax2);

% =========================================================================
% STEP 2 — Number of Turns
% =========================================================================
Nmin1 = ceil(L * ilmax  * 1e4 / (Bsat      * Ae));  % Copper-loss-limited
Nmin2 = ceil(L * deltail * 1e4 / (delta_Bmax * Ae)); % Combined-loss criterion
N     = max(Nmin1, Nmin2);

delta_Breal = L * deltail / (N * Ae * 1e-4);  % Actual flux density swing [T]

% =========================================================================
% STEP 3 — Air-Gap Length
% =========================================================================
lg = (N^2) * Ae * 1e-2 * 4 * pi * 1e-7 / (2 * L);  % Air-gap length (E-core) [cm]

% =========================================================================
% STEP 4 — Winding Design
% =========================================================================

% --- Conductor selection from current density curves ---
J30 = 420 * AP^(-0.240);   % Current density at ΔT = 30 °C [A/cm²]
J15 = 297 * AP^(-0.240);   % Current density at ΔT = 15 °C [A/cm²]
Jmax = min(J30, J15);      % Conservative limit [A/cm²]

scable_min        = ilrms / Jmax;                        % Minimum conductor cross-section [cm²]
diameter_cable_min = 2 * sqrt(scable_min / pi);          % Corresponding diameter [cm]
fprintf('\n============================================================\n');
fprintf('  CONDUCTOR SELECTION\n');
fprintf('============================================================\n');
fprintf('  Minimum conductor cross-section : %.4f cm² (d> %.4f cm)\n', scable_min, diameter_cable_min);

% Map to nearest larger AWG gauge
awgcable        = floor(-(log(10 * diameter_cable_min / 0.127)) * 39 / log(92) + 36);
diameter_awg_cable = 0.0127 * 92^((36 - awgcable) / 39);  % [cm]
scable          = pi * (diameter_awg_cable / 2)^2;         % [cm²]
fprintf('  Selected gauge                  : AWG %d  (d = %.3f cm)\n', awgcable, diameter_awg_cable);

% --- Skin effect check ---
T     = Ta + delta_Tmax;                                   % Maximum operating temperature [°C]
rho   = 1.724 * (1 + 0.0042 * (T - 20)) * 1e-8;           % Copper resistivity [Ω·m]
delta = sqrt(rho / (4 * pi^2 * 1e-7 * fl)) * 1e2;         % Skin depth [cm]
ratio = (2 * delta) / diameter_awg_cable;

fprintf('  Skin depth at %.0f kHz         : %.4f cm\n', fl/1e3, delta);

% =========================================================================
% STEP 4.1 — Litz Wire (if skin effect is significant)
% =========================================================================
if delta < diameter_awg_cable / 4
    % Solid wire would suffer unacceptable skin-effect losses → use Litz wire

    diameter_strand = 0.020;   % Strand diameter available in lab [cm] (≈ 0.2 mm / AWG 32)
    awg_strand      = floor(-(log(10 * diameter_strand / 0.127)) * 39 / log(92) + 36);
    scable_strand   = pi * (diameter_strand / 2)^2;   % Strand cross-section [cm²]

    fprintf('\n  → Skin depth < r_wire/2: Litz wire required.\n');
    fprintf('    Strand selected: AWG %d  (d = %.2f mm)\n', awg_strand, diameter_strand * 10);

    % Number of strands to match minimum conductor cross-section
    N_strands    = ceil(scable_min / scable_strand);
    scable_litz  = N_strands * scable_strand;          % Total Litz cross-section [cm²]

    % Equivalent bundle diameter (accounting for fill factor K)
    diameter_litz = sqrt(4 * scable_litz / (K * pi));

    % Add 10 % margin for insulation and inter-strand voids
    k_margin = 1.1;
    d_eff    = k_margin * diameter_litz;               % Effective bundle diameter [cm]

    % Window fit
    vertical_litz   = floor(window_h / d_eff);         % Turns per layer
    horizontal_litz = ceil(N / vertical_litz);          % Number of layers
    winding_h       = vertical_litz   * d_eff;          % Actual winding height [cm]
    winding_w       = horizontal_litz * d_eff;          % Actual winding width  [cm]

    fprintf('\n============================================================\n');
    fprintf('  LITZ WIRE WINDING\n');
    fprintf('============================================================\n');
    fprintf('  Strands per turn   : %d\n',   N_strands);
    fprintf('  Bundle diameter    : %.3f mm  (with fill factor, k = %.1f)\n', d_eff * 10, k_margin);
    fprintf('  Turns per layer    : %d\n',   vertical_litz);
    fprintf('  Number of layers   : %d\n',   horizontal_litz);
    fprintf('  Winding height     : %.3f cm  (window: %.3f cm)\n', winding_h, window_h);
    fprintf('  Winding width      : %.3f cm  (window: %.3f cm)\n', winding_w, window_w);

    if winding_w < window_w
        fprintf('  [OK] Winding fits within the core window.\n');

        % --- Wire length estimate ---
        L_base      = N * N_strands * mlt;                           % Base length [cm]
        k_layer     = 1 + 0.02 * (horizontal_litz - 1);             % Layer-radius correction (+2 % per layer)
        L_layers    = L_base * k_layer;
        L_leads     = 2 * 2;                                         % Lead allowance: 2 cm per terminal [cm]
        L_total     = (L_layers + L_leads) * 1.10 * 1.10;           % +10 % process + 10 % safety margin
        L_commercial = ceil(L_total / 10) * 10;                     % Round up to nearest 10 cm

        fprintf('\n  Wire Length Estimate:\n');
        fprintf('    Minimum length     : %.1f cm\n',  L_layers + L_leads);
        fprintf('    Recommended cut    : %.1f cm  (%.2f m)\n', L_total, L_total / 100);
        fprintf('    Cut %d strands of  : %.0f cm each  (%.2f cm)\n', N_strands, ceil(L_total / N_strands), L_total / N_strands);
        fprintf('    Order quantity     : %.0f cm  (%.2f m)\n', L_commercial, L_commercial / 100);
    else
        fprintf('  [FAIL] Winding exceeds core window width.\n');
    end

    scable = scable_litz;   % Update effective conductor cross-section

else
    fprintf('  Skin effect negligible — solid wire (AWG %d) is adequate.\n', awgcable);
end

% =========================================================================
% STEP 5 — Power Losses and Thermal Estimate
% =========================================================================
mlt_calc = diapat * pi * 1.3;   % Alternative MLT estimate (from central-leg perimeter)

rho_cu = 1.724 * (1 + 0.0042 * (T - 20)) * 1e-6;   % Copper resistivity at operating temperature [Ω·cm]
R      = rho_cu * mlt * N / scable;                  % Winding DC resistance [Ω] with litz wire
R2     = rho_cu * mlt * N / (2 * pi * (diameter_awg_cable/2) * delta) % Winding DC resistance [Ω] without litz wire(full copper)
%If we use the litz wire, we can approximate the overall loss by
%considering the total strands as a unique cable with section not affected
%by skin effect

% Core (magnetic) losses — Steinmetz equation
Pcore_cm3 = delta_Breal^2.4 * (Kh * fs + Ke * fs^2);   % Volumetric core loss [W/cm³]
Pcore_abs = Pcore_cm3 * Vol;                             % Total core loss [W]

% Copper losses
Pcu   = R * ilrms^2;   % [W]
Pcu_nolitz = R2 * ilrms^2; %[W]

Ptot  = Pcu + Pcore_abs;
Ptot_nolitz = Pcu_nolitz + Pcore_abs;

% Thermal resistance and temperature rise
Rth     = 23 * AP^(-0.37);    % Thermal resistance core–ambient [°C/W]
delta_T = Rth * Ptot;         % Expected temperature rise [°C]
delta_T_nolitz = Rth * Ptot_nolitz;         % Without Litz wire


fprintf('\n============================================================\n');
fprintf('  LOSS BUDGET\n');
fprintf('============================================================\n');
fprintf('  Core losses                  : %6.3f W\n', Pcore_abs);
fprintf('  Copper losses with Litz      : %6.3f W\n', Pcu);
fprintf('  Copper losses without Litz   : %6.3f W\n', Pcu_nolitz);
fprintf('  Total losses                 : %6.3f W\n', Ptot);
fprintf('  ΔT (estimated)               : %6.2f °C  (limit: %.0f °C)\n', delta_T, delta_Tmax);
fprintf('  ΔT (estimated) without Litz  : %6.2f °C  (limit: %.0f °C)\n', delta_T_nolitz, delta_Tmax);

% =========================================================================
% LEAKAGE INDUCTANCE (exact integration)
% =========================================================================
mu_0      = 4 * pi * 1e-7;
R_in_m    = (diapat / 2) / 100;           % Central leg radius [m]
b_eff_m   = winding_w / 100;              % Winding radial depth [m]
h_eff_m   = winding_h / 100;              % Winding axial height [m]
d_gap_m   = (window_w - winding_w) / 100; % Radial clearance to window wall [m]

R_gap_mean_m = R_in_m + b_eff_m + d_gap_m / 2;  % Mean radius of the clearance region [m]

% Leakage contribution from the winding region (internal)
L_int = mu_0 * (N^2 / h_eff_m) * 2 * pi * b_eff_m * (R_in_m / 3 + b_eff_m / 12);

% Leakage contribution from the clearance region (external)
if d_gap_m > 0
    L_ext = mu_0 * (N^2 / h_eff_m) * 2 * pi * R_gap_mean_m * d_gap_m;
else
    L_ext = 0;
end

L_leak_tot = L_int + L_ext;

fprintf('\n============================================================\n');
fprintf('  LEAKAGE INDUCTANCE\n');
fprintf('============================================================\n');
fprintf('  Internal (winding) contribution : %7.2f nH\n', L_int     * 1e9);
fprintf('  External (clearance) contribution: %7.2f nH\n', L_ext    * 1e9);
fprintf('  Total leakage inductance         : %7.2f nH\n', L_leak_tot * 1e9);

% =========================================================================
% FINAL DESIGN SUMMARY
% =========================================================================
core_name = "B65811";
core_type = 2;

fprintf('\n============================================================\n');
fprintf('       INDUCTOR DESIGN — FINAL REPORT\n');
fprintf('============================================================\n');
fprintf('  Core             : %s  (Type E%d)\n', core_name, core_type);
fprintf('  Minimum Inductance       : %.2f uH\n', Lmin*1e6);
fprintf('  Inductance       : %.2f uH\n', L*1e6);
fprintf('  Inductor Ripple  : %.2f A\n', deltail);
fprintf('  Number of turns  : %d\n',    N);
fprintf('  Air-gap length   : %.3f cm\n', lg);
fprintf('------------------------------------------------------------\n');
fprintf('  WINDING\n');
fprintf('    Base gauge     : AWG %d\n', awgcable);
fprintf('    Litz wire      : AWG %d strands | %d total | %d layer(s) × %d turn(s)/layer\n', ...
        awg_strand, N_strands, horizontal_litz, vertical_litz);
fprintf('------------------------------------------------------------\n');
fprintf('  VERIFICATION\n');

% Area product
if AP > APmin
    fprintf('  [OK]   AP         : %.3f > %.3f cm⁴\n', AP, APmin);
else
    fprintf('  [FAIL] AP         : %.3f cm⁴ is below the minimum (%.3f cm⁴)\n', AP, APmin);
end

% Flux density swing
if delta_Breal < delta_Bmax
    fprintf('  [OK]   ΔB         : %.3f < %.3f T\n', delta_Breal, delta_Bmax);
else
    fprintf('  [FAIL] ΔB         : %.3f T exceeds limit (%.3f T) — saturation risk!\n', delta_Breal, delta_Bmax);
end

% Window fit
if winding_w < window_w
    fprintf('  [OK]   Window fit : %.3f < %.3f cm\n', winding_w, window_w);
else
    fprintf('  [FAIL] Window fit : winding (%.3f cm) exceeds window (%.3f cm)\n', winding_w, window_w);
end

% Thermal
if delta_T < delta_Tmax
    fprintf('  [OK]   Thermal    : ΔT = %.1f °C  (< %.0f °C)\n', delta_T, delta_Tmax);
else
    fprintf('  [FAIL] Thermal    : ΔT = %.1f °C exceeds limit (%.0f °C)\n', delta_T, delta_Tmax);
end

if delta_T_nolitz < delta_Tmax
    fprintf('  [OK]   Thermal    : ΔT = %.1f °C  (< %.0f °C)\n', delta_T_nolitz, delta_Tmax);
else
    fprintf('  [FAIL] Thermal    : ΔT = %.1f °C exceeds limit (%.0f °C)\n', delta_T_nolitz, delta_Tmax);
end

fprintf('============================================================\n');