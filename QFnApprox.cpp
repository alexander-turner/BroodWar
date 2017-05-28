#include <vector>
using namespace std;

/* Given a starting state, a set of actions and features, run GQ and return the feature weights. 
   features is composed of |actions| vectors each containing k features.
*/
vector<vector<double>> QFunctionApproximation(void(*) startState, vector<void(*)()> actions, vector<vector<void(*)()>> features) {
	double learningRate = 0.01;
	double discount = 0.9;
	int numActions = actions.size();
	int maxIter = 100; // how many games to run
	vector<vector<double>> weights; // FILL WITH ONES (dim: |actions| x k+1)
	// Load / save feature weights to resume training?

	for (int i = 0; i < maxIter; i++) {
		void(*) state = startState; // not sure about state representation
		while (!gameFinished) { // find actual code for this
			void(*)() action = select_action(state, numActions, weights, features);
			void(*) newState = game.takeAction(state, action);
			weights = update_weights(state, action, newState, weights, featuers, simulator, learningRate, discount);
			state = newState;
		}
	}

	return weights;
}
/*
Select an action index in a state using an e-greedy algorithm.
*/
int selectAction(void(*) state, vector<void(*)()> actions, 
	vector<vector<double>> weights, vector<vector<void(*)()>> features) {
	double epsilon = 0.5; // probability of choosing non-greedy action
	int greedyAction = selectGreedyAction(state, actions, weights, features);
	int action = greedyAction;
	if ((((double) rand() % 100) / 100) <= epsilon) // CHECK does division work here?
		while (action == greedy) 
			action = rand() % actions.size();
	return action;
}

/*
Returns index of greedy action according to state, weights, and features.
*/
int selectGreedyAction(void(*) state, vector<void(*)()> actions, 
	vector<vector<double>> weights, vector<vector<void(*)()>> features) {
	int greedyAction;
	double bestVal = -1 * DOUBLE_MAX;
	// run through all estimated Q-values
	for (int action = 0; action < actions.size(); action++) {
		double estimate = estimateQ(state, actions, actionInd, weights, features);
		if (estimate > bestVal) {
			bestVal = estimate;
			greedyAction = action;
		}
	}
	return greedyAction;
}

/*
Perform TD update for each parameter after taking given action.
*/
vector<vector<double>> updateWeights(void(*) state, vector<void(*)()> actions, int actionInd, void(*) newState, 
	vector<vector<double>> weights, vector<vector<void(*)()>> features, double learningRate, double discount) {
	int greedyAction = selectGreedyAction(newState, actions, weights, features); 
	for (int i = 0; i < weights.size(); i++) {
		double noisyGradient = reward(state) + discount*estimateQ(newState, actions, greedyAction, weights, features) - estimateQ(state, action, weights, features);
		noisyGradient *= learningRate;
		if (i == 0)
			weights[i][actionInd] += noisyGradient;
		else
			weights[i][actionInd] += noisyGradient*features[i][state, actionInd]; // features provides magnitude to the gradient
	}
	return weights;
}

double estimateQ(void(*) state, vector<void(*)()> actions, int actionInd, vector<vector<double>> weights, vector<vector<void(*)()>> features) {
	double estimate = weights[0][actionInd];
	for (int i = 1; i < weights.size(); i++)
		estimate += weights[i][actionInd] * features[i - 1][state, actions(actionInd)];
	return estimate;
}
