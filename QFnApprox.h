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
		prevState.currentUnit = NULL;
		this->weights = { 1.0, 1.0, 1.0 };
	}

	/* 
	Differences:
	Why do we track prevaction/prevUnit?

	TODO: 
	Fix DPS function
	Fix state updating
	Fix weights for arbitrary number of features / actions
	Make estimateQ dynamic
	Run for arbitrary number of games - auto-restart
	Output scores to CSV for analysis
	Optional: open / save weight file
	*/
	// How to normalize rewards for bad situations? Granularity?
	/* Given a starting state, a set of actions and features, run GQ and return the feature weights. 
	   features is composed of |actions| vectors each containing k features. Optional filepath parameter allows for loading and saving weights.
	*/
	StateInfo QFunctionApproximation(std::vector<double(*)(StateInfo)> actions, std::vector<double(*)(StateInfo)> features, StateInfo state) {
		//std::cout << "In QFunctionApproximation()" << std::endl;
		std::vector<double> weights = this->weights; 
		this->currState = state;
		std::vector <StateInfo> orders; // set of commands executed each round
		

		/*if (filepath) {
			std::ifstream input_file(filePath); // check that this works. also, expand for actions / features?
			double tempVar;
			while (input_file >> tempVar)
			{
				weights.push_back(tempVar);
			}
		}
		else {*/
		//initializeWeights(weights, numActions, features.size()); // Fill with ones (dim: |actions| x k+1)
		//}

		if (Broodwar->isInGame()) {
			// update weights given the orders executed last time
			if (this->prevState.currentUnit != NULL)
				batchUpdateWeights(orders, this->currState, this->prevState, features, actions);	//weights doesnt currently return anything -AG
			// Check if this can be called once if prevState.currentUnit == NULL
			this->currState.friendlies = Broodwar->self()->getUnits();
			this->currState.enemies = Broodwar->enemy()->getUnits();
			for (auto &e : this->currState.enemies)
				this->currState.enemyHP[e] = e->getHitPoints();
			orders.clear(); // reset orders
			for (auto &u : this->currState.friendlies) { // calculate action for each unit
				if (!u->exists())
					continue;

				this->currState.friendlyHP[u] = u->getHitPoints();
				int prevAction = this->currState.actionInd;
				Unit prevTarget = this->currState.target;
				this->currState.currentUnit = u;
				this->currState.actionInd = selectAction(actions, features); // how to extract target? -> see next line -AG
				orders.push_back(this->currState);

				if (prevAction != this->currState.actionInd) // why do we need this?
				{
					actions.at(this->currState.actionInd)(this->currState); // execute action
				}

			}	
 			this->prevState = this->currState;
		}

		/*if (filepath) { // overwrite?
			std::ofstream file;
			file.open(filepath);
			file << weights;
			file.close();
		}*/
		this->weights = weights; //update weight vector in class
		return this->currState;
	}
	/*
	Select an action index in a state using an e-greedy algorithm.
	*/
	int selectAction(std::vector <double(*) (StateInfo)> actions, std::vector<double(*)(StateInfo)> features) {
		double epsilon = 0.5; // probability of choosing non-greedy action
		int greedyAction = selectGreedyAction(actions, features);

		//std::cout << "current target is: " << state.target << std::endl;
		int action = greedyAction;
		if ((double) std::rand() / (RAND_MAX) <= epsilon)
			while (action == greedyAction)
				action = rand() % actions.size();
		//std::cout << "greedy action is: " << greedyAction << "| " << "action chosen is: " << action << std::endl;

		return action;
	}

	/*
	Returns index of greedy action according to state, weights, and features and modifies unit target to the best found for that action.
	*/
	int selectGreedyAction(std::vector <double(*) (StateInfo)> actions, std::vector<double(*)(StateInfo)> features) {
		StateInfo state = this->currState;
		std::vector<double> weights = this->weights;
		int greedyAction = 0;
		double bestVal = -1 * DBL_MAX;
		Unit returnTarget;

		// run through all estimated Q-values
		for (int action = 0; action < actions.size(); action++) {
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
	void batchUpdateWeights(std::vector <StateInfo> orders, StateInfo currState, StateInfo prevState,
		std::vector <double(*) (StateInfo)> features, std::vector <double(*) (StateInfo)> actions) {
		std::vector<double> weights = this->weights;
		std::vector<double> weight_changes = initializeWeights(weights, actions.size(), features.size(), 0);
		
		for (int actionInd = 0; actionInd < (int) actions.size(); actionInd++) {
			std::vector<double> temp_weight = updateWeights(actionInd, actions.size(), currState, prevState, weights, features, actions);
			for (int i = 0; i < (int) weight_changes.size(); i++)
			{
				weight_changes[i] += temp_weight[i];
			}
		}

		for (int i = 0; i < (int) weights.size(); i++)
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

		std::vector<double> weight_changes = { 1.0, 1.0, 1.0 }; //initializeWeights(weights, numActions, features.size(), 0);
		
		int greedyAction = selectGreedyAction(actions, features); //need to create a new state before we can use it here
		for (int i = 0; i < (int) weight_changes.size(); i++) {
			double noisyGradient = reward(currState, prevState, features) + 
				discount*estimateQ(currState, greedyAction, weights, actions, features) - 
				estimateQ(prevState, greedyAction, weights, actions, features); // check which index to pass in
			noisyGradient *= learningRate;
			
			if (i == 0) // just add the gradient to the standalone weight
				weight_changes.at(actionInd) = noisyGradient;
			else  // multiply the feature by the gradient
				weight_changes.at(actionInd) =noisyGradient*features.at(i)(currState);
		}

		return weight_changes; 
	}

	double estimateQ(StateInfo state, int actionInd, std::vector<double> weights, std::vector <double(*) (StateInfo)> actions, std::vector <double(*) (StateInfo)> features) {
		double estimate = weights.at(actionInd);
		for (int i = 1; i < (int) weights.size(); i++) {
			double ftr = features.at(2)(state);		//made feature constant as we just want DPS-HP ratio feature
			double wt = weights.at(actionInd);
			estimate += wt * ftr;
		}
		return estimate;
	}

	/*
	Initialize an |actions|*(|features|+1) vector of the given value (1 by default).
	*/
	std::vector<double> initializeWeights(std::vector<double> weights, int numActions, int numFeatures, double val=1.0) {
		for (int i = 0; i < numActions; i++) {
			double newAction; // put this to a vector? How do we make this able to store multiple values
			for (int j = 0; j < numFeatures + 1; j++)
				newAction = val;
			weights.push_back(newAction);
		}
		return weights;
	}

	// Reward := change in our total hitpoint lead
	double reward(StateInfo currState, StateInfo prevState, std::vector <double(*) (StateInfo)> features)
	{
		// Current time step
		double R_curr = getHPDiff(currState);

		// Previous time step
		double R_prev = getHPDiff(prevState);

		double reward;
		if (R_prev >= 0)
			reward = R_curr - R_prev;
		else
			reward = R_curr + R_prev;
		std::cout << "Reward:" << reward << std::endl;
		return reward;
	}

	// Returns (total friendly HP - total enemy HP) for given state
	double getHPDiff(StateInfo state) {
		double HPDiff = 0;
		for (auto it : state.friendlyHP)
		{
			HPDiff += it.second;
		}
		for (auto it : state.enemyHP)
		{
			HPDiff -= it.second;
		}
		return HPDiff;
	}

	private:
		std::vector<double> weights;
		StateInfo currState;
		StateInfo prevState;
};
#endif // !QLEARN