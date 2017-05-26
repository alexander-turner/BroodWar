% Generates an x*y grid where all rewards are -1 - except for the top-right
% corner, which has a reward of 10. Returns a simulator for this grid.
function new_simulator = generate_grid_simulator(x, y)
numActions = 4;
numStates = x*y;

startState = numStates;
rewardState = 1;

transitions = cell(numActions, 1);
rewards = zeros(numStates, 1);

% initialize transition arrays
for action=1:numActions
    transitions{action} = zeros(numStates, numStates);
end

for state_y=1:y
    for state_x=1:x
        state = (state_y-1)*x + state_x;
        rewards(state) = -1;
        if state == rewardState % top-right corner
            rewards(state) = 10; 
        end

        % fill in transition probabilities
        for action=1:numActions % 1:=up, 2:=down, 3:=left, 4:=right
            % calculate next state
            newStateY = state_y;
            newStateX = state_x;
            if action == 1
                newStateY = state_y - 1;
            elseif action == 2
                newStateY = state_y + 1;
            elseif action == 3
                newStateX = state_x - 1;
            elseif action == 4 % lets us have a "stay still" option
                newStateX = state_x + 1;
            end

            if newStateY > y || newStateY <= 0 || ...
                    newStateX > x || newStateX <= 0 % exceeds bounds
                transitions{action}(state, state) = 1; % keep in graph
            else % a valid move
                newState = (newStateY-1)*x + newStateX;
                transitions{action}(state, newState) = 1;
            end
        end
    end
end

new_simulator = simulator(transitions, rewards, startState, rewardState);
end