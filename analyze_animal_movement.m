function analyze_animal_movement(filename)

    data = readtable(filename, 'NumHeaderLines', 3);
    head1_x = data{:,4}; head1_y = data{:,5};
    body1_x = data{:,6}; body1_y = data{:,7};
    tail1_x = data{:,8}; tail1_y = data{:,9};

    head2_x = data{:,10}; head2_y = data{:,11};
    body2_x = data{:,12}; body2_y = data{:,13};
    tail2_x = data{:,14}; tail2_y = data{:,15};

    time = (1:length(head1_x))';

    window_size_smooth = 5;  
    smooth_head1_x = movmean(head1_x, window_size_smooth);
    smooth_head1_y = movmean(head1_y, window_size_smooth);
    smooth_body1_x = movmean(body1_x, window_size_smooth);
    smooth_body1_y = movmean(body1_y, window_size_smooth);
    smooth_tail1_x = movmean(tail1_x, window_size_smooth);
    smooth_tail1_y = movmean(tail1_y, window_size_smooth);

    smooth_head2_x = movmean(head2_x, window_size_smooth);
    smooth_head2_y = movmean(head2_y, window_size_smooth);
    smooth_body2_x = movmean(body2_x, window_size_smooth);
    smooth_body2_y = movmean(body2_y, window_size_smooth);
    smooth_tail2_x = movmean(tail2_x, window_size_smooth);
    smooth_tail2_y = movmean(tail2_y, window_size_smooth);    
    
    figure('Name', 'Movement Trajectory', 'NumberTitle', 'off');
    
    subplot(2,1,1);
    hold on;
    % plot(time, smooth_head1_x, 'r-', 'LineWidth', 2, 'DisplayName', 'Head1 X');
    % plot(time, smooth_head1_y, 'r--', 'LineWidth', 2, 'DisplayName', 'Head1 Y');
    plot(time*3, smooth_body1_x, 'b-', 'LineWidth', 2, 'DisplayName', 'Body1 X');
    plot(time*3, smooth_body1_y, 'b--', 'LineWidth', 2, 'DisplayName', 'Body1 Y');
    % plot(time, smooth_tail1_x, 'b-', 'LineWidth', 2, 'DisplayName', 'Tail1 X');
    % plot(time, smooth_tail1_y, 'b--', 'LineWidth', 2, 'DisplayName', 'Tail1 Y');

    % plot(time, smooth_head2_x, 'r-', 'LineWidth', 2, 'DisplayName', 'Head2 X');
    % plot(time, smooth_head2_y, 'r--', 'LineWidth', 2, 'DisplayName', 'Head2 Y');
    plot(time*3, smooth_body2_x, 'r-', 'LineWidth', 2, 'DisplayName', 'Body2 X');
    plot(time*3, smooth_body2_y, 'r--', 'LineWidth', 2, 'DisplayName', 'Body2 Y');
    % plot(time, smooth_tail2_x, 'b-', 'LineWidth', 2, 'DisplayName', 'Tail2 X');
    % plot(time, smooth_tail2_y, 'b--', 'LineWidth', 2, 'DisplayName', 'Tail2 Y');    
    hold off;
    
    title('Body Coordinates Over Time', 'FontSize', 12);
    xlabel('Time(s)', 'FontSize', 10);
    ylabel('Coordinate Value', 'FontSize', 10);
    legend('FontSize', 8, 'Location', 'best');
    grid on;
    set(gca, 'FontSize', 8);

    subplot(2,1,2);
    t = 1:length(smooth_head1_x);
    t_interp = linspace(1, length(t), 1000);  
    head1_x_interp = spline(t, smooth_head1_x, t_interp);
    head1_y_interp = spline(t, smooth_head1_y, t_interp);

    head2_x_interp = spline(t, smooth_head2_x, t_interp);
    head2_y_interp = spline(t, smooth_head2_y, t_interp);
    
    hold on;
    % plot(head1_x_interp, head1_y_interp, 'r-', 'LineWidth', 2, 'DisplayName', 'Head');
    plot(smooth_body1_x, smooth_body1_y, 'b-', 'LineWidth', 2, 'DisplayName', 'Body1');
    % plot(smooth_tail1_x, smooth_tail1_y, 'b-', 'LineWidth', 2, 'DisplayName', 'Tail');

    % plot(head2_x_interp, head2_y_interp, 'r-', 'LineWidth', 2, 'DisplayName', 'Head');
    plot(smooth_body2_x, smooth_body2_y, 'r-', 'LineWidth', 2, 'DisplayName', 'Body2');
    % plot(smooth_tail2_x, smooth_tail2_y, 'b-', 'LineWidth', 2, 'DisplayName', 'Tail');    
    hold off;
    
    title(' 2D Trajectory with Interpolation', 'FontSize', 12);
    xlabel('X Coordinate', 'FontSize', 10);
    ylabel('Y Coordinate', 'FontSize', 10);
    legend('FontSize', 8, 'Location', 'best');
    axis equal;
    grid on;
    set(gca, 'FontSize', 8);
    
    threshold = 50; window_size = 5;
    [is_moving1, displacement1] = detect_movement(smooth_head1_x, smooth_head1_y, threshold, window_size);
    [is_moving2, displacement2] = detect_movement(smooth_head2_x, smooth_head2_y, threshold, window_size);

    figure('Name', 'Movement Detection', 'NumberTitle', 'off');

    subplot(3,1,1);
    sigma = 2;  
    smooth_displacement1 = imgaussfilt(displacement1, sigma);
    smooth_displacement2 = imgaussfilt(displacement2, sigma);
    hold on;
    plot(time*3, smooth_displacement1, 'k-', 'LineWidth', 2);
    plot(time*3, smooth_displacement2, 'k--', 'LineWidth', 2);
    yline(threshold, 'r--', 'LineWidth', 1.5, 'DisplayName', 'Threshold');
    hold off;
    
    title('Displacement Over Time', 'FontSize', 12);
    xlabel('Time(s)', 'FontSize', 10);
    ylabel('Displacement (pixels)', 'FontSize', 10);
    legend('FontSize', 8);
    grid on;
    set(gca, 'FontSize', 8);

    subplot(3,1,2);
    bar(time*3, is_moving1, 'FaceColor', [0.2, 0.4, 0.8], 'EdgeColor', 'none', 'BarWidth', 1);
    ylim([0 1.5]);
    title('Movement State1 ', 'FontSize', 12);
    xlabel('Time (s)', 'FontSize', 10);
    ylabel('State', 'FontSize', 10);
    grid on;
    set(gca, 'FontSize', 8);

    subplot(3,1,3);
    bar(time*3, is_moving2, 'FaceColor', [0.2, 0.4, 0.8], 'EdgeColor', 'none', 'BarWidth', 1);
    ylim([0 1.5]);
    title('Movement State2 ', 'FontSize', 12);
    xlabel('Time (s)', 'FontSize', 10);
    ylabel('State', 'FontSize', 10);
    grid on;
    set(gca, 'FontSize', 8);
end

function [is_moving, displacement] = detect_movement(x, y, threshold, window_size)
    n = length(x);
    is_moving = false(n, 1);
    displacement = zeros(n, 1);
    for i = window_size:n
        dx = x(i) - x(i-window_size+1);
        dy = y(i) - y(i-window_size+1);
        displacement(i) = sqrt(dx^2 + dy^2);
        is_moving(i) = (displacement(i) > threshold);
    end
    displacement(1:window_size-1) = displacement(window_size);
    is_moving(1:window_size-1) = is_moving(window_size);
end