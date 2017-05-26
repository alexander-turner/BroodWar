grid_sim = generate_grid_simulator(4, 4);
features = {@getX, @getY};
weights = Q_fn_approx(grid_sim, features);

% Returns the x-coordinate for a 4x4 grid simulator state.
function x = getX(state, action)
    x = mod(state-1, 4) + 1; % how do we get this on the fly?
end

% Returns the y-coordinate for a 4x4 grid simulator state.
function y = getY(state, action)
    y = floor(state-1 / 4) + 1;
end