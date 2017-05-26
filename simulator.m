classdef simulator < handle
    properties
        transitions;
        rewards;
        start_state;
        terminal_states = [];
    end
    
    methods
        function new_simulator = simulator(transitions, rewards, start_state, terminal_states)
            new_simulator.transitions = transitions;
            new_simulator.rewards = rewards;
            new_simulator.start_state = start_state;
            new_simulator.terminal_states = terminal_states;
        end
        
        % Returns the avg reward for evaluating the given policy n times.
        % Each trial stops after entering one of the specified terminal
        % states; if the states are not correctly specified, infinite
        % looping may occur.
        function avg_reward = evaluate_policy_n(simulator, policy, n)
           avg_reward = 0;
           
           for trial = 1 : n
              avg_reward = avg_reward + evaluate_policy(simulator, policy);
           end
           
           % Make it an average
           avg_reward = avg_reward / n;
        end
        
        % Returns the reward for evaluating the policy in the simulator. If
        % terminal states are not correctly specified, infinite looping
        % may occur.
        function reward = evaluate_policy(simulator, policy)
            state = simulator.start_state;
            reward = simulator.rewards(state);
            
            % While we aren't in a terminating state, follow the policy and
            % tally the reward.
            while ~ismember(state, simulator.terminal_states)
                state = take_action(simulator, state, policy(state));
                reward = reward + simulator.rewards(state);
            end
        end
        
        % Takes the given action in the given state and returns the resultant
        % state.
        % Assumes the transitions table is well-formed (transitions{a}(state, :)
        % sums to 1 for all a, state. If not, it will choose the last non-zero probability state. 
        function new_state = take_action(simulator, state, action)
            trimmed_transitions = simulator.transitions{action};
            roll = rand(); % roll the dice
            current_prob_sum = 0; % sum of probabilities thus far

            for end_state = 1 : length(trimmed_transitions)
                if trimmed_transitions(state, end_state) > 0
                    new_state = end_state; % failsafe in case of rounding errors
                end

                current_prob_sum = current_prob_sum + trimmed_transitions(state, end_state);
                if roll <= current_prob_sum % see which state we end up in
                    new_state = end_state;
                    return;
                end
            end
        end
    end
end