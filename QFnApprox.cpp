#include <vector>
using namespace std;

std::vector<void(*)()> actions; // do we need to accomodate different units - buildings?
std::vector<vector<void(*)()>> features; // k vectors each containing |actions| features
std::vector<vector<double>> weights; // feature weights

double learningRate = 0.01;
double discount = 0.9;

/*
Select an action index in a state using an e-greedy algorithm.
CHECK TYPES.
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
