% Define the coefficients (Highest degree first)
% p = [a, b, c, d] for a*x^3 + b*x^2 + c*x + d
p = [-0.019092251093906, 0.237483424531089, -1.380431541198065, 17.895062311398010];

% Generate 30 equally spaced points from 0 to 12
x = linspace(0, 12, 15);

% Calculate the Y values based on the polynomial curve
y = polyval(p, x);

% Initialize the base SPICE command string
spice_string = 'B1 n1 n2 V=TABLE {V(ctrl)} =';

% Loop through the points and append them in the (x, y) format
for i = 1:length(x)
    % Formatting to 4 decimal places to keep the string clean
    pair = sprintf(' (%.1f, %.2f)', x(i), y(i));
    spice_string = [spice_string, pair];
end

% Print the final string to the command window
disp('--- Final SPICE String ---');
disp(spice_string);

% Optional: Write to a text file for easy copying
fileID = fopen('spice_table.txt', 'w');
fprintf(fileID, '%s\n', spice_string);
fclose(fileID);
disp('Saved to spice_table.txt');