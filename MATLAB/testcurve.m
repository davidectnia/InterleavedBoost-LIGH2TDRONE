clear all
close all
clc
% --- 1. Import Setup for European CSV ---
filename = 'h100_data.csv'; 
img_file = 'zoom_H100.jpg'; 

% Initialize options for 2 variables
opts = delimitedTextImportOptions("NumVariables", 2);
opts.Delimiter = ";";

% Define the variable names and types EXPLICITLY as 'double'
opts.VariableNames = ["Current", "Voltage"];
opts.VariableTypes = ["double", "double"];

% NOW set the DecimalSeparator for the double variables
% This avoids the "Unknown Option" error
opts = setvaropts(opts, "DecimalSeparator", ",");

% --- 2. Load and Clean Data ---
data = readmatrix(filename, opts);

% Remove any NaN rows that might have slipped in (e.g., empty lines at the end)
data = data(~any(isnan(data),2), :);

I_meas = data(:, 1); 
V_meas = data(:, 2);

% --- 3. Polynomial Fitting (Degree 3) ---
p = polyfit(I_meas, V_meas, 3);
I_fit = linspace(0, 10, 100);
V_fit = polyval(p, I_fit);

% --- 4. Visualization with Background ---
img = imread(img_file);
x_range = [0 10]; % Matches Horizon H100 0-10A axis
y_range = [0 20]; % Matches Horizon H100 0-20V axis

figure('Color', 'w');
% Display image and map to physical units
imagesc(x_range, y_range, flipud(img)); 
set(gca, 'YDir', 'normal'); 
hold on;

% Plot your CSV points (Yellow/Red for high visibility)
plot(I_meas, V_meas, 'ro', 'MarkerSize', 4, 'MarkerFaceColor', 'y', ...
    'DisplayName', 'Data from CSV');

% Plot the Polynomial Model (Cyan dashed line)
plot(I_fit, V_fit, 'c--', 'LineWidth', 2, ...
    'DisplayName', '3^{rd} Degree Fit');

grid on;
axis([0 10 0 20]);
xlabel('Current I [A]');
ylabel('Voltage V [V]');

% Genera la stringa dell'equazione usando i coefficienti calcolati in 'p'
equazione_str = sprintf('V(I) = (%.4f)I^3 + (%.4f)I^2 + (%.4f)I + %.4f', p(1), p(2), p(3), p(4));

% Aggiunge il sottotitolo (usando un array di celle per creare due righe)
title({'Horizon H100', equazione_str}, 'FontSize', 12);
legend('Location', 'northeast');

% Print the final coefficients for your simulation
fprintf('Final Model: V(I) = %.4f*I^3 + %.4f*I^2 + %.4f*I + %.4f\n', ...
    p(1), p(2), p(3), p(4));