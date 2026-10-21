% --- 1. Load Data from CSV ---
% Assuming the CSV has two columns: [Current, Voltage]
filename = 'h100_vi.csv'; 
data = readmatrix(filename);

% Extract I and V (Handling cases where columns might be swapped)
% We assume Column 1 = Current (A), Column 2 = Voltage (V)
I_measured = data(:, 1);
V_measured = data(:, 2);

% Sort data by current to ensure correct plotting and fitting
[I_measured, idx] = sort(I_measured);
V_measured = V_measured(idx);

% --- 2. Polynomial Curve Fitting ---
% A 3rd-degree polynomial is usually sufficient for the Horizon H100
degree = 4; 
p = polyfit(I_measured, V_measured, degree);

% --- 3. Generate the Function String ---
% This creates a readable string of the equation V(I)
fprintf('The fitted function is:\n');
fprintf('V(I) = %.4f*I^4 + %.4f*I^3 + %.4f*I^2 + %.4f*I + %.4f\n\n', p(1), p(2), p(3), p(4), p(5));

% --- 4. Validation Plot ---
I_fit = linspace(min(I_measured), max(I_measured), 100);
V_fit = polyval(p, I_fit);

figure;
plot(I_measured, V_measured, 'ro', 'DisplayName', 'CSV Data Points');
hold on;
%plot(I_fit, V_fit, 'b-', 'LineWidth', 2, 'DisplayName', ['Polynomial Fit (deg ' num2str(degree) ')']);
grid on;
xlabel('Current (A)');
ylabel('Voltage (V)');
title('Horizon H100 V-I Characterization');
legend('Location', 'northeast');
axis([0 10 0 20])
% --- 5. Exporting as a MATLAB Function Handle ---
% You can use this "f_VI" variable to calculate voltage in your simulation
f_VI = @(i) polyval(p, i);

% Example: Calculate voltage at 5A
v_at_5A = f_VI(5);
fprintf('Estimated Voltage at 5A: %.2f V\n', v_at_5A);