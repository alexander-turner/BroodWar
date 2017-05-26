% Given a simulator and features, run GQ and return the weights.
function weights = Q_fn_approx(simulator, features)
% Initialize parameters
numActions = length(simulator.transitions);
learningRate = 0.01;
discount = 0.9;
maxIter = 100;
weights = ones(length(features) + 1, numActions);

for i=1:maxIter
    state = simulator.start_state;
    while ~ismember(simulator.terminal_states, state) % while not terminal
        action = select_action(state, numActions, weights, features);
        newState = simulator.take_action(state, action);
        weights = update_weights(state, action, newState, weights, ...
            features, simulator, learningRate, discount);
        state = newState;
    end
end
end

% Select an action in a state using an e-greedy algorithm.
function action = select_action(state, numActions, weights, features)
    epsilon = 0.5;
    greedy = select_greedy_action(state, numActions, weights, features);
    action = greedy;
    if rand() <= epsilon
        while action == greedy
            action = randi(numActions);
        end
    end
end

% Select greedy action according to estimated Q(s,a).
function greedy_action = select_greedy_action(state, numActions, weights, features)
bestVal = -inf;
for action=1:numActions
    estimate = estimate_Q(state, action, weights, features);
    if estimate > bestVal
        bestVal = estimate;
        greedy_action = action;
    end
end
end

% Update the weights based on a noisy sample (an action just taken).
function weights = update_weights(state, action, newState, weights, ...
    features, simulator, learningRate, discount)
    greedyAction = select_greedy_action(newState, length(simulator.transitions), weights, features);
for i=1:size(weights, 1)
    noisyGradient = simulator.rewards(state) ...
        + discount*estimate_Q(newState, greedyAction, weights, features) ...
        - estimate_Q(state, action, weights, features);
    noisyGradient = noisyGradient * learningRate; 
    if i == 1
        weights(i, action) = weights(i, action) + noisyGradient; 
    else
        weights(i, action) = weights(i, action) + noisyGradient*features{i-1}(state, action);
    end
end
end

% Approximate Q(state, action) using given weights and features.
function estimate = estimate_Q(state, action, weights, features)
estimate = weights(1, action);
for i=2:size(weights, 1) % already used first weight
   estimate = estimate + weights(i, action)*features{i-1}(state, action);
end
end