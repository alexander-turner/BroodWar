#include <vector>
#include <iostream>
using namespace BWAPI;

/* Given a starting state, a set of actions and features, run GQ and return the feature weights. 
   features is composed of |actions| vectors each containing k features. Optional filepath parameter allows for loading and saving weights.
*/
vector<vector<double>> QFunctionApproximation(vector<void(*)()> actions, vector<vector<void(*)()>> features, String filepath = NULL) {
	double learningRate = 0.01;
	double discount = 0.9;
	int numActions = actions.size();
	int maxIter = 100; 
	vector<vector<double>> weights; // FILL WITH ONES (dim: |actions| x k+1)

	if (filepath) {
		ifstream input_file(filePath); // check that this works. also, expand for actions / features?
		double tempVar;
		while (input_file >> tempVar)
		{
			weights.push_back(tempVar);
		}
	}

	// Run maxIter games
	for (int i = 0; i < maxIter; i++) {
		Unitset units = Broodwar->self()->getUnits();
		Unitset enemies = Broodwar->enemy()->getUnits();
		while (Broodwar->isInGame) { // move this to onframe?
			double lastHPDiff = getHPDiff(units, enemies);
			for (auto &u : units) { // Calculate action for each unit
				if (!u->exists())
					continue;
				void(*)() action = select_action(u, actions, weights, features);
				u.takeAction(state, action);
			}
			double newHPDiff = getHPDiff(units, enemies);
			double reward = newHPDiff - lastHPDiff; // potential issue: doesn't allow for differentiation of action quality. also how to normalize for bad situations?

			weights = update_weights(action, weights, features, reward, learningRate, discount);
		}
		Broodwar->restartGame();
	}

	if (filepath) {
		ofstream file;
		file.open(filepath);
		file << weights;
		file.close();
	}

	return weights;
}
/*
Select an action index in a state using an e-greedy algorithm.
*/
int selectAction(Unit u, vector<void(*)()> actions, vector<vector<double>> weights,
		vector<vector<void(*)()>> features) {
	double epsilon = 0.5; // probability of choosing non-greedy action
	int greedyAction = selectGreedyAction(u, actions, weights, features);
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
vector<vector<double>> updateWeights(vector<void(*)()> actions, int actionInd, vector<vector<double>> weights, 
		vector<vector<void(*)()>> features, double reward, double learningRate, double discount) {
	int greedyAction = selectGreedyAction(newState, actions, weights, features); 
	for (int i = 0; i < weights.size(); i++) {
		double noisyGradient = reward + discount*estimateQ(newState, actions, greedyAction, weights, features) - estimateQ(state, action, weights, features);
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
