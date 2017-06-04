#include <vector>
#include <iostream>
#include "../BWAPI/functionwrappers.h"
using namespace BWAPI;

// How to normalize rewards for bad situations? Granularity?
/* Given a starting state, a set of actions and features, run GQ and return the feature weights. 
   features is composed of |actions| vectors each containing k features. Optional filepath parameter allows for loading and saving weights.
*/
vector<vector<double>> QFunctionApproximation(vector<void(*)()> actions, vector<vector<void(*)()>> features, String filepath = NULL) {
	int numActions = actions.size();
	int maxIter = 100; 
	std::vector<std::vector<double>> weights; 
	StateInfo currState, prevState; // Info on the current and previous states
	std::vector <double(*) (StateInfo)> orders; // set of commands executed each round

	if (filepath) {
		std::ifstream input_file(filePath); // check that this works. also, expand for actions / features?
		double tempVar;
		while (input_file >> tempVar)
		{
			weights.push_back(tempVar);
		}
	}
	else {
		initializeWeights(weights, numActions, features.size()); // Fill with ones (dim: |actions| x k+1)
	}

	// Run maxIter games
	for (int i = 0; i < maxIter; i++) {
		while (Broodwar->isInGame) {
			// update weights given the orders executed last time
			if (prevState != NULL)
				weights = batchUpdateWeights(orders, currState, prevState, weights, features);
			currState.friendlies = Broodwar->self()->getUnits(); // do we have to keep calling this?
			currState.enemies = Broodwar->enemy()->getUnits();
			orders.clear(); // reset orders
			for (auto &u : units) { // Calculate action for each unit
				if (!u->exists())
					continue;
				currState.currentUnit = u;
				currState.actionInd = select_action(currState, numActions, weights, features); // how to extract target?
				orders.push_back(currState);
				actionVector.at(actionInd)(currState); // execute action
			}
			prevState = currState.copy();
		}
		Broodwar->restartGame();
	}

	if (filepath) { // overwrite?
		std::ofstream file;
		file.open(filepath);
		file << weights;
		file.close();
	}

	return weights;
}
/*
Select an action index in a state using an e-greedy algorithm.
*/
int selectAction(StateInfo state, std::vector <double(*) (StateInfo)> actions, std::vector<std::vector<double>> weights,
		vector<vector<void(*)()>> features) {
	double epsilon = 0.5; // probability of choosing non-greedy action
	int greedyAction = selectGreedyAction(state, actions, weights, features);
	int action = greedyAction;
	if ((double) std::rand() / (RAND_MAX) <= epsilon)
		while (action == greedy) 
			action = rand() % actions.size();
	return action;
}

/*
Returns index of greedy action according to state, weights, and features.
*/
int selectGreedyAction(StateInfo state, int numActions, std::vector<std::vector<double>> weights, 
	std::vector<std::vector<void(*)()>> features) {
	int greedyAction;
	double bestVal = -1 * DOUBLE_MAX;
	// run through all estimated Q-values
	for (int action = 0; action < numActions; action++) {
		for (auto &target in state.enemies) { // try each possible target - WARNING: only works with attack
			state.enemies = state.target;
			double estimate = estimateQ(state, action, weights, features);
			if (estimate > bestVal) {
				bestVal = estimate;
				greedyAction = action;
			}
		}
	}
	return greedyAction;
}

/*
For each order executed (specified via StateInfo), calculate and apply batched changes to weights.
*/
void batchedUpdateWeights(std::vector <StateInfo> orders, StateInfo currState,
	std::vector<std::vector<double>> weights, std::vector <double(*) (StateInfo)> features) {
	std::vector<std::vector<double>> weight_changes = initializeWeights(weights, numActions, features.size(), 0);

	for (auto &order in orders) {
		weight_changes += updateWeights(order.actionInd, currState, order, weights, features);
	}

	weights += weight_changes;
}


/*
Perform TD update for each parameter after taking given action and return the changes.
*/
void updateWeights(int actionInd, int numActions, StateInfo currState, StateInfo prevState, std::vector<std::vector<double>> weights,
		std::vector <double(*) (StateInfo)> features) {
	// constants
	double learningRate = 0.01;
	double discount = 0.9;

	std::vector<std::vector<double>> weight_changes = initializeWeights(weights, numActions, features.size(), 0);

	int greedyAction = selectGreedyAction(newState, weights, features); 
	for (int i = 0; i < weight_changes.size(); i++) {
		double noisyGradient = reward(currState, prevState) + discount*estimateQ(currState, greedyAction, weights, features) - estimateQ(prevState, action, weights, features); // check which index to pass in
		noisyGradient *= learningRate;
		if (i == 0) // just add the gradient to the standalone weight
			(weight_changes.at(i)).at(actionInd).assign(noisyGradient);
		else  // multiply the feature by the gradient
			(weight_changes.at(i)).at(actionInd).assign(noisyGradient*features.at(i)(currState));
	}

	return weight_changes; 
}

double estimateQ(StateInfo state, int actionInd, vector<vector<double>> weights, std::vector <double(*) (StateInfo)> features) {
	double estimate = (weights.at(0)).at(actionInd);
	for (int i = 1; i < weights.size(); i++)
		estimate += (weights.at(i)).at(actionInd) * features.at(i - 1)(state);
	return estimate;
}

/*
Initialize an |actions|*(|features|+1) vector of the given value (1 by default).
*/
void initializeWeights(std::vector<std::vector<double>> weights, int numActions, int numFeatures, double val=1.0) {
	for (int i = 0; i < numActions; i++) {
		std::vector<double> newAction;
		for (int j = 0; j < numFeatures + 1; j++)
			newAction.push_back(val);
		weights.push_back(newAction);
	}
}