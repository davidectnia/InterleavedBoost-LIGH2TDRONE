% 1. Load LTspice Data (modify filename and column indices as needed)
data = readtable('ltspice_data.txt', 'FileType', 'text');
freq_ltspice  = data{:, 1};   % Frequency vector [Hz]
mag_ltspice   = data{:, 2};   % Magnitude [dB]
phase_ltspice = data{:, 3};   % Phase [deg]

% 2. Define IEEE Figure Parameters
figure('Units', 'inches', 'Position', [1, 1, 6.5, 4.0]);
fontName = 'Times New Roman';
fontSize = 10;

% 3. Evaluate Analytical Bode Response (Gci)
w_min = 2 * pi * freq_ltspice(1);
w_max = 2 * pi * freq_ltspice(end);
[mag_an, phase_an, w_an] = bode(Gci, {w_min, w_max});
freq_an = squeeze(w_an) / (2 * pi);
mag_an_dB = squeeze(20 * log10(mag_an));
phase_an_deg = squeeze(phase_an);

% 4. Create Single Plot with Double Y-Axis (yyaxis)
% --- Left Axis: Magnitude ---
yyaxis left
h1 = semilogx(freq_ltspice, mag_ltspice, '-', 'Color', [0.850 0.325 0.098], 'LineWidth', 1.5);
hold on;
h2 = semilogx(freq_an, mag_an_dB, '--', 'Color', [0.000 0.447 0.741], 'LineWidth', 1.2);
hold off;

ylabel('Magnitude [dB]', 'Interpreter', 'latex', 'FontName', fontName, 'FontSize', fontSize);
% Set color for left axis text/ticks to match IEEE standard neutral dark gray or black
ax = gca;
ax.YColor = [0 0 0]; 

% --- Right Axis: Phase ---
yyaxis right
h3 = semilogx(freq_ltspice, phase_ltspice, '-.', 'Color', [0.929 0.694 0.125], 'LineWidth', 1.2);
hold on;
h4 = semilogx(freq_an, phase_an_deg, ':', 'Color', [0.494 0.184 0.556], 'LineWidth', 1.2);
hold off;

ylabel('Phase [deg]', 'Interpreter', 'latex', 'FontName', fontName, 'FontSize', fontSize);
ax.YColor = [0 0 0];

% 5. General Plot Formatting (IEEE Style)
grid on;
xlabel('Frequency [Hz]', 'Interpreter', 'latex', 'FontName', fontName, 'FontSize', fontSize);
title('Bode Comparison ($G_{ci}$ vs. LTspice)', 'Interpreter', 'latex', 'FontName', fontName, 'FontSize', fontSize+1);

% Combine all handles for a single unified legend
legend([h1, h2, h3, h4], ...
    {'LTspice (Mag)', 'Analytical (Mag)', 'LTspice (Phase)', 'Analytical (Phase)'}, ...
    'Interpreter', 'latex', 'FontName', fontName, 'FontSize', fontSize, 'Location', 'best');

set(gca, 'FontName', fontName, 'FontSize', fontSize);