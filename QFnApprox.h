#ifndef QLEARN
#define QLEARN
#include <vector>
#include <iostream>
#include <string>
#include "../BWAPI/functionwrappers.h"
using namespace BWAPI;

class QLearn
{
public:
	QLearn()
	{
		this->weights = { 1.0, 1.0, 1.0, 1.0, 1.0 };
	}



	// How to normalize rewards for bad situations? Granularity?
	/* Given a starting state, a set of actions and features, run GQ and return the feature weights. 
	   features is composed of |actions| vectors each containing k features. Optional filepath parameter allows for loading and saving weights.
	*/
	std::vector<double> QFunctionApproximation(std::vector<double(*)(StateInfo)> actions, std::vector<double(*)(StateInfo)> features, std::string filepath = NULL) {
		int numActions = actions.size();
		int maxIter = 100; 
		std::vector<double> weights = this->weights; 
		StateInfo currState, prevState; // Info on the current and previous states
		std::vector <double(*) (StateInfo)> orders; // set of commands executed each round

		/*if we wrap QFN in a class we can just do this in the constructor
		for now i've hard coded the values of the weight vector and commented out the file IO -AG */
		/*if (filepath) {
			std::ifstream input_file(filePath); // check that this works. also, expand for actions / features?
			double tempVar;
			while (input_file >> tempVar)
			{
				weights.push_back(tempVar);
			}
		}
		else {*/
		initializeWeights(weights, numActions, features.size()); // Fill with ones (dim: |actions| x k+1)
		//}

		// Run maxIter games
		for (int i = 0; i < maxIter; i++) {
			while (Broodwar->isInGame()) {
				// update weights given the orders executed last time
				if (prevState.currentUnit != NULL)
					batchUpdateWeights(orders, currState, prevState, weights, features, actions);	//weights doesnt currently return anything -AG
				currState.friendlies = Broodwar->self()->getUnits(); // do we have to keep calling this? -> I think we might bc the number of units might change per iteration -AG
				currState.enemies = Broodwar->enemy()->getUnits();
				orders.clear(); // reset orders
				for (auto &u : currState.friendlies) { // Calculate action for each unit
					if (!u->exists())
						continue;
					currState.currentUnit = u;
					currState.actionInd = selectAction(currState, actions, weights, features); // how to extract target? -> see next line -AG
					Unit target = u->getOrderTarget();
					//orders.push_back(currState);	//didn't quite understand the purpose of this? -AG
					actions.at(currState.actionInd)(currState); // execute action
				}
				prevState = currState;
			}
			Broodwar->restartGame();
		}

		/*if (filepath) { // overwrite?
			std::ofstream file;
			file.open(filepath);
			file << weights;
			file.close();
		}*/
		this->weights = weights; //update weight vector in class
		return weights;
	}
	/*
	Select an action index in a state using an e-greedy algorithm.
	*/
	int selectAction(StateInfo state, std::vector <double(*) (StateInfo)> actions, std::vector<double> weights,
		std::vector<double(*)(StateInfo)> features) {
		double epsilon = 0.5; // probability of choosing non-greedy action
		int greedyAction = selectGreedyAction(state, actions, weights, features);
		int action = greedyAction;
		if ((double) std::rand() / (RAND_MAX) <= epsilon)
			while (action == greedyAction)
				action = rand() % actions.size();
		return action;
	}

	/*
	Returns index of greedy action according to state, weights, and features and modifies unit target to the best found for that action.
	*/
	int selectGreedyAction(StateInfo state, std::vector <double(*) (StateInfo)> actions, std::vector<double> weights,
		std::vector<double(*)(StateInfo)> features) 
{
		int greedyAction;
		double bestVal = -1 * DBL_MAX;
		Unit returnTarget;
		int numActions = actions.size();
		// run through all estimated Q-values
		for (int action = 0; action < numActions; action++) {
			for (auto &target : state.enemies) { // try each possible target - WARNING: only works with attack(f,e)
				state.target = target;
				double estimate = estimateQ(state, greedyAction, weights, actions, features);
				if (estimate > bestVal) {
					bestVal = estimate;
					returnTarget = state.target;
					greedyAction = action;
				}
			}
		}
		state.target = returnTarget;
		return greedyAction;
	}

	/*
	For each order executed (specified via StateInfo), calculate and apply batched changes to weights.
	*/
	void batchUpdateWeights(std::vector <double(*) (StateInfo)> orders, StateInfo currState, StateInfo prevState,
		std::vector<double> weights, std::vector <double(*) (StateInfo)> features, std::vector <double(*) (StateInfo)> actions) {

		std::vector<double> weight_changes = initializeWeights(weights, actions.size(), features.size(), 0);
		std::vector<double> tweight;	//temp weight vector
		for (int actionInd = 0; actionInd < orders.size(); actionInd++) {
			tweight = updateWeights(actionInd, actions.size(), currState, prevState, weights, features, actions);
		}

		for (int i = 0; i < weights.size(); i++)
		{
			weights[i] += weight_changes[i];
		}
	}


	/*
	Perform TD update for each parameter after taking given action and return the changes.
	*/
	std::vector<double> updateWeights(int actionInd, int numActions, StateInfo currState, StateInfo prevState, std::vector<double> weights,
			std::vector <double(*) (StateInfo)> features, std::vector <double(*) (StateInfo)> actions) {
		// constants
		double learningRate = 0.01;
		double discount = 0.9;

		std::vector<double> weight_changes = initializeWeights(weights, numActions, features.size(), 0);
		
		int greedyAction = selectGreedyAction(currState, actions, weights, features); //need to create a new state before we can use it here
		for (int i = 0; i < weight_changes.size(); i++) {
			double noisyGradient = reward(currState, prevState) + discount*estimateQ(currState, greedyAction, weights, actions, features) - estimateQ(prevState, greedyAction, weights, actions, features); // check which index to pass in
			noisyGradient *= learningRate;
			if (i == 0) // just add the gradient to the standalone weight
				weight_changes.at(actionInd) = noisyGradient;
			else  // multiply the feature by the gradient
				weight_changes.at(actionInd) =noisyGradient*features.at(i)(currState);
		}

		return weight_changes; 
	}

	double reward(StateInfo currState, StateInfo prevState)
	{
		return 1.0;
	}

	double estimateQ(StateInfo state, int actionInd, std::vector<double> weights, std::vector <double(*) (StateInfo)> actions, std::vector <double(*) (StateInfo)> features) {
		double estimate = weights.at(actionInd);
		for (int i = 1; i < weights.size(); i++)
			estimate += weights.at(actionInd) * features.at(i - 1)(state);
		return estimate;
	}

	/*
	Initialize an |actions|*(|features|+1) vector of the given value (1 by default).
	*/
	std::vector<double>  initializeWeights(std::vector<double> weights, int numActions, int numFeatures, double val=1.0) {
		for (int i = 0; i < numActions; i++) {
			double newAction;
			for (int j = 0; j < numFeatures + 1; j++)
				newAction = val;
			weights.push_back(newAction);
		}
		return weights;
	}
	private:
		std::vector<double> weights;
};
#endif // !QLEARN
