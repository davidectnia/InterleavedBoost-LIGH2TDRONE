% Parameters
n_values = 1:4;             % Number of interleaved phases
d = linspace(0, 1, 1000);   % Duty cycle range from 0 to 1
figure;
hold on;
grid on;

colors = ['r', 'b', 'g', 'm']; % Distinct colors for n=1 to 4

for i = 1:length(n_values)
    n = n_values(i);
    ripple = zeros(size(d));
    
    % Evaluate the function for each interval x = 1, 2, ..., n
    for x = 1:n
        % Define the duty cycle interval for the current x
        lower_bound = (x - 1) / n;
        upper_bound = x / n;
        
        % Logical mask to find indices of d within the current interval
        mask = (d >= lower_bound) & (d <= upper_bound);
        
        % Apply the formula from the image
        % [n*d - (x-1)] * [x - n*d] / n
        ripple(mask) = (n * d(mask) - (x - 1)) .* (x - n * d(mask)) / n;
    end
    
    % Plot the result for the current n
    plot(d, ripple, 'LineWidth', 2, 'Color', colors(i), 'DisplayName', ['n = ' num2str(n)]);
end

% Formatting the plot
xlabel('Duty Cycle d(t)');
ylabel('Normalized Ripple \Deltai_{in} / (v_{out} T_s / L)');
title('Normalized Input Current Ripple for Interleaved Phases');
legend('show');
ylim([0, 0.3]); % Adjust limits for better visibility
hold off;