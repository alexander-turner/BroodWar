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
	void testF(StateInfo state)
	{
		Broodwar->sendText("Test");
	}

	/* TODO: 
	Fix DPS function
	Run for arbitrary number of games - auto-restart
	Output scores to CSV for analysis
	Open / save weight file
	*/
	// How to normalize rewards for bad situations? Granularity?
	/* Given a starting state, a set of actions and features, run GQ and return the feature weights. 
	   features is composed of |actions| vectors each containing k features. Optional filepath parameter allows for loading and saving weights.
	*/
	StateInfo QFunctionApproximation(std::vector<double(*)(StateInfo)> actions, std::vector<double(*)(StateInfo)> features, StateInfo state) {
		//std::cout << "In QFunctionApproximation()" << std::endl;
		int numActions = actions.size();
		int maxIter = 100; 
		std::vector<double> weights = this->weights; 
		
		this->currState = state;
		std::vector <StateInfo> orders; // set of commands executed each round
		
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
		//initializeWeights(weights, numActions, features.size()); // Fill with ones (dim: |actions| x k+1)
		//}

		// Run maxIter games
		//for (int i = 0; i < maxIter; i++) {
			if (Broodwar->isInGame()) {
				// update weights given the orders executed last time
				if (this->prevState.currentUnit != NULL)
					batchUpdateWeights(orders, this->currState, this->prevState, features, actions);	//weights doesnt currently return anything -AG
				this->currState.friendlies = Broodwar->self()->getUnits(); // do we have to keep calling this? -> I think we might bc the number of units might change per iteration -AG
				this->currState.enemies = Broodwar->enemy()->getUnits();
				for (auto &e : this->currState.enemies)
					this->currState.enemyHP[e] = e->getHitPoints();
				orders.clear(); // reset orders
				for (auto &u : this->currState.friendlies) { // Calculate action for each unit
					this->currState.friendlyHP[u] = u->getHitPoints();
					int prevAction = this->currState.actionInd;
					Unit prevTarget = this->currState.target;
					if (!u->exists())
						continue;
					this->currState.currentUnit = u;
					this->currState.actionInd = selectAction(actions, features); // how to extract target? -> see next line -AG
					orders.push_back(this->currState);	//didn't quite understand the purpose of this? -AG
					if (prevAction != this->currState.actionInd)
					{
						actions.at(this->currState.actionInd)(this->currState); // execute action
						//std::cout << "Changing from " << prevAction << " to " << newState.actionInd << std::endl;
					}

				}
				
 				this->prevState = this->currState;

			}
			//Broodwar->restartGame();
		//}

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
		//std::cout << "In selectAction()" << std::endl;
		StateInfo state = this->currState;
		std::vector<double> weights = this->weights;
		double epsilon = 0.5; // probability of choosing non-greedy action
		int greedyAction = selectGreedyAction(actions, weights, features);
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
	int selectGreedyAction(std::vector <double(*) (StateInfo)> actions, std::vector<double> weights, std::vector<double(*)(StateInfo)> features) {
		//std::cout << "In selectGreedyAction()" << std::endl;
		StateInfo state = this->currState;
		int greedyAction = 0;
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
					//std::cout << "target changed to: " << state.target << std::endl;
					greedyAction = action;
					
				}
			}
		}
		state.target = returnTarget;
		//std::cout << "returning target: " << state.target << std::endl;
		return greedyAction;
	}

	/*
	For each order executed (specified via StateInfo), calculate and apply batched changes to weights.
	*/
	void batchUpdateWeights(std::vector <StateInfo> orders, StateInfo currState, StateInfo prevState,
		std::vector <double(*) (StateInfo)> features, std::vector <double(*) (StateInfo)> actions) {
		//std::cout << "In batchUpdateWeights()" << std::endl;
		std::vector<double> weights = this->weights;
		std::vector<double> weight_changes = initializeWeights(weights, actions.size(), features.size(), 0);
		std::vector<double> temp_weight;	//temp weight vector
		for (int actionInd = 0; actionInd < actions.size(); actionInd++) {
			temp_weight = updateWeights(actionInd, actions.size(), currState, prevState, weights, features, actions);
			for (int i = 0; i < weight_changes.size(); i++)
			{
				weight_changes[i] += temp_weight[i];
			}
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

		std::vector<double> weight_changes = { 1.0, 1.0, 1.0 }; //initializeWeights(weights, numActions, features.size(), 0);
		
		int greedyAction = selectGreedyAction(actions, weights, features); //need to create a new state before we can use it here
		for (int i = 0; i < weight_changes.size(); i++) {
			double noisyGradient = reward(currState, prevState, features) + discount*estimateQ(currState, greedyAction, weights, actions, features) - estimateQ(prevState, greedyAction, weights, actions, features); // check which index to pass in
			noisyGradient *= learningRate;
			
			if (i == 0) // just add the gradient to the standalone weight
				weight_changes.at(actionInd) = noisyGradient;
			else  // multiply the feature by the gradient
				weight_changes.at(actionInd) =noisyGradient*features.at(i)(currState);
		}

		return weight_changes; 
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
			reward = R_curr - R_prev; // 50 + 30
		else
			reward = R_curr + R_prev;
		std::cout << "Reward differential:" << reward << std::endl;
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

	double estimateQ(StateInfo state, int actionInd, std::vector<double> weights, std::vector <double(*) (StateInfo)> actions, std::vector <double(*) (StateInfo)> features) {
		//std::cout << "In estimateQ()" << std::endl;
		double estimate = weights.at(actionInd);
		for (int i = 1; i < weights.size(); i++) {
			double ftr = features.at(2)(state);		//made feature constant as we just want DPS-HP ratio feature
			double wt = weights.at(actionInd);
			estimate += wt * ftr;
		}
		return estimate;
	}

	/*
	Initialize an |actions|*(|features|+1) vector of the given value (1 by default).
	*/
	std::vector<double>  initializeWeights(std::vector<double> weights, int numActions, int numFeatures, double val=1.0) {
		//std::cout << "In initializeWeights()" << std::endl;
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
		StateInfo currState;
		StateInfo prevState;
};
#endif // !QLEARN