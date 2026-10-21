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

D=Dnom;            % To design the controller select the nominal duty cycle
%Control to output transfer function
w_rhpz = (1-D)^2 * R/Leq;
gvd0 = Vin /(1-D)^3;

%Transfer Function properties of the inductor current
w_lhpz = 2/(R*C);           %left hand plane zero 
w0 = (1-D)/(sqrt(Leq*C));   %natural frequency
q = R*(1-D)*sqrt(C/Leq);      %quality factor
gid0 = 2*Vin/(R*(1-D)^3);   %dc gain 

%Transfer function definition
% Define the transfer function (assuming gid0, w_lhpz, q, and w0 are already defined)
% Define the transfer function (assuming gid0, w_lhpz, q, and w0 are defined)
s = tf('s');
Gid = gid0 * (1 + s/w_lhpz) / (1 + s/(q*w0) + (s/w0)^2);

% Generate the Bode plot
bode(Gid);
grid on;

% 1. Extract frequencies
[~, w_peak] = getPeakGain(Gid);
[wn_poles, ~] = damp(pole(Gid));
[wn_zeros, ~] = damp(zero(Gid));

% 2. Apply vertical lines to subplots with dynamic labels
ax = findall(gcf, 'Type', 'axes');

for i = 1:length(ax)
    % 1. Apply IEEE standard formatting to the axes
    set(ax(i), 'FontName', 'Times New Roman', 'FontSize', 10, ...
        'TickLabelInterpreter', 'latex', 'XGrid', 'on', 'YGrid', 'on');
        
    % 2. Highlight Resonance Peak
    
    
    % 3. Highlight Poles
    for p = unique(wn_poles)'
        if p > 0 
            % Using f_0 since \omega_0 is rad/s and you are displaying Hz
            pole_label = sprintf('$$f_0 = %.2f$$ Hz', p/(2*pi));
            xline(ax(i), p, '--', pole_label, ...
                'Color', [0 0.4470 0.7410], 'LineWidth', 1.5, ... % High-contrast Blue
                'LabelVerticalAlignment', 'middle', ...
                'LabelHorizontalAlignment', 'left', ...
                'Interpreter', 'latex', 'FontSize', 9, 'FontName', 'Times New Roman');
        end
    end
    
    % 4. Highlight Zeros
    for z = unique(wn_zeros)'
        if z > 0
            % Using f_{lhpz} for consistency in units
            zero_label = sprintf('$$f_{lhpz} = %.2f$$ Hz', z/(2*pi));
            xline(ax(i), z, '-.', zero_label, ...
                'Color', [0.4660 0.6740 0.1880], 'LineWidth', 1.5, ... % High-contrast Green
                'LabelVerticalAlignment', 'top', ...
                'LabelHorizontalAlignment', 'left', ...
                'Interpreter', 'latex', 'FontSize', 9, 'FontName', 'Times New Roman');
        end
    end
end
%Transfer function of the Hall-Effect sensor 

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

% Funzione di trasferimento del sensore
H_s = Hsense / (1 + s/(2*pi*50e3));

% Calcolo della costante wp0c 
w_p0c = wcross / (Hsense * (1/Vm) * mag_gid * k);

% Funzione del compensatore (Tipo 2)
Gci = (w_p0c / s) * (1 + s/w_zc) / (1 + s/w_pc);

% figure;
% bode(Gci);
% title('Bode Diagram - Compensator Gci');
% grid on;

% 1. Read and Parse the LTspice Data File
fileName = 'compensator.txt';
lines = readlines(fileName);

freq_ltspice = [];
mag_ltspice = [];
phase_ltspice = [];

for i = 2:length(lines)
    str = strip(lines(i));
    if strlength(str) == 0
        continue;
    end
    
    % Regex to extract scientific numbers from strings like "(6.70e+01dB,9.91e+01°)"
    nums = regexp(str, '[-+]?[0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?', 'match');
    
    if length(nums) >= 3
        freq_ltspice(end+1, 1) = str2double(nums{1});
        mag_ltspice(end+1, 1)  = str2double(nums{2});
        phase_ltspice(end+1, 1) = str2double(nums{3});
    end
end

% 2. Define IEEE Figure Parameters
figure('Units', 'inches', 'Position', [1, 1, 6.5, 4.0]);
fontName = 'Times New Roman';
fontSize = 10;
colorMag = [0.000 0.447 0.741];   % IEEE-safe Blue for Magnitude
colorPhase = [0.850 0.325 0.098]; % IEEE-safe Rust Red for Phase

% 3. Evaluate Analytical Bode Response (Gci)
w_min = 2 * pi * freq_ltspice(1);
w_max = 2 * pi * freq_ltspice(end);
[mag_an, phase_an, w_an] = bode(Gci, {w_min, w_max});
freq_an = squeeze(w_an) / (2 * pi);
mag_an_dB = squeeze(20 * log10(mag_an));
phase_an_deg = squeeze(phase_an);

% 4. Create Single Plot with Double Y-Axis (yyaxis)
% --- Left Axis: Magnitude (All Blue) ---
yyaxis left
h1 = semilogx(freq_ltspice, mag_ltspice, '-', 'Color', colorMag, 'LineWidth', 1.5);
hold on;
h2 = semilogx(freq_an, mag_an_dB, '--', 'Color', colorMag, 'LineWidth', 1.5);
hold off;

ylabel('Magnitude [dB]', 'Interpreter', 'latex', 'FontName', fontName, 'FontSize', fontSize);
ax = gca;
ax.YColor = colorMag; % Colors the left axis blue to match the magnitude curves

% --- Right Axis: Phase (All Red) ---
yyaxis right
h3 = semilogx(freq_ltspice, phase_ltspice, '-', 'Color', colorPhase, 'LineWidth', 1.5);
hold on;
h4 = semilogx(freq_an, phase_an_deg, '--', 'Color', colorPhase, 'LineWidth', 1.5);
hold off;

ylabel('Phase [deg]', 'Interpreter', 'latex', 'FontName', fontName, 'FontSize', fontSize);
ax.YColor = colorPhase; % Colors the right axis red to match the phase curves

% 5. General Plot Formatting (IEEE Style)
grid on;
xlabel('Frequency [Hz]', 'Interpreter', 'latex', 'FontName', fontName, 'FontSize', fontSize);
title('Bode Comparison ($G_{ci}$ vs. LTspice)', 'Interpreter', 'latex', 'FontName', fontName, 'FontSize', fontSize+1);

% Unified legend clearly distinguishing sources and variables
legend([h1, h2, h3, h4], ...
    {'Mag: LTspice', 'Mag: Analytical', 'Phase: LTspice', 'Phase: Analytical'}, ...
    'Interpreter', 'latex', 'FontName', fontName, 'FontSize', fontSize, 'Location', 'best');

set(gca, 'FontName', fontName, 'FontSize', fontSize);
% Funzione di trasferimento ad anello aperto
Tci = Gci * (1/Vm) * Gid * H_s;

figure;
bode(Tci);
title('Bode Diagram - Open Loop Tci');
margin(Tci); % Questa funzione ti mostrerà in automatico il Margine di Fase!
grid on;