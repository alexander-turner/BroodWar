#ifndef QLEARN
#define QLEARN
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include "../BWAPI/functionwrappers.h"
using namespace BWAPI;

class QLearn
{
public:
	QLearn()
	{
		prevState.currentUnit = NULL;
	}

	/* 
	Differences:
	Store actions and features in class for simplicity?

	TODO: 
	Fix DPS function
	Fix state updating
	Store previous orders and fix updateWeights - DONE
	EstimateQ code doublecheck - where are actions?
	Doublecheck weight_changes.size() in updateWeights for 2d arrays
	Fix weights for arbitrary number of features / actions
	Make estimateQ dynamic
	Run for arbitrary number of games - auto-restart
	Output scores to CSV for analysis - DONE
	Optional: open / save weight file
	*/
	// How to normalize rewards for bad situations? Granularity?
	/* Given a starting state, a set of actions and features, run GQ and return the feature weights. 
	   features is composed of |actions| vectors each containing k features. Optional filepath parameter allows for loading and saving weights.
	*/
	StateInfo QFunctionApproximation(std::vector<double(*)(StateInfo)> actions, std::vector<double(*)(StateInfo)> features, StateInfo state, std::string filepath="") {
		this->actions = actions;
		this->features = features;
		this->currState = state;
		std::vector <StateInfo> orders; // set of commands executed each round
		
		if (filepath != "") {
			std::ifstream input_file(filepath); // check that this works. also, expand for actions / features?
			double tempVar;
			while (input_file >> tempVar)
				this->weights.push_back(tempVar);
		}
		else if (weights.size() == 0) {
			this->weights = initializeWeights(); // Fill with ones (dim: |actions| x k+1)
		}

		if (Broodwar->isInGame()) {
			this->currState.friendlies = Broodwar->self()->getUnits();
			this->currState.enemies = Broodwar->enemy()->getUnits();
			for (auto &e : this->currState.enemies)
				this->currState.enemyHP[e] = e->getHitPoints();
			// update weights given the orders executed last time
			if (this->prevState.currentUnit != NULL)
				batchUpdateWeights(orders, features, actions);	//weights doesnt currently return anything -AG
			// Check if this can be called once if prevState.currentUnit == NULL
			orders.clear(); // reset orders
			this->prevState = this->currState;
			for (auto &u : this->currState.friendlies) { // calculate action for each unit
				if (!u->exists())
					continue;

				this->currState.friendlyHP[u] = u->getHitPoints();
				this->currState.currentUnit = u;
				this->currState = selectAction(currState); // how to extract target? -> see next line -AG
				orders.push_back(this->currState);

				actions.at(this->currState.actionInd)(this->currState); // execute action
			}	
		}

		if (filepath != "") { // overwrite?
			std::ofstream file;
			file.open(filepath);
			for (int i = 0; i < (int) weights.size(); i++)
				file << weights.at(i);
			file.close();
		}
		return this->currState;
	}
	/*
	Use an e-greedy algorithm to modify and return the target and actionInd values for state.
	*/
	StateInfo selectAction(StateInfo state) {
		double epsilon = 0.5; // probability of choosing non-greedy action
		state = selectGreedyAction(state);
		int greedyAction = state.actionInd;
		Unit greedyTarget = state.target;
		std::unordered_map<int, Unit> unitmapping;
		int i = 0;
		for (auto &e : state.enemies) {
			unitmapping[i] = e;
			i++;
		}
		//std::cout << "current target is: " << state.target << std::endl;
		state.actionInd = greedyAction;
		state.target = greedyTarget;
		if ((double)std::rand() / (RAND_MAX) <= epsilon)
			while (state.actionInd == greedyAction && state.target == greedyTarget) {
				state.actionInd = rand() % (int) this->actions.size();
				state.target = unitmapping[rand() % i];
			}
		//std::cout << "greedy action is: " << greedyAction << "| " << "action chosen is: " << action << std::endl;

		return state;
	}

	/*
	Returns modified state including greedy action index according to state, weights, and features and modifies unit target to the best found for that action.
	*/
	StateInfo selectGreedyAction(StateInfo state) {
		double bestVal = -1 * DBL_MAX;
		Unit returnTarget;

		int greedyAction = 0;
		// run through all estimated Q-values
		for (int action = 0; action < (int) this->actions.size(); action++) {
			state.actionInd = action;
			for (auto &target : state.enemies) { // try each possible target - WARNING: only works with attack(f,e)
				state.target = target;
				double estimate = estimateQ(state);
				if (estimate > bestVal) {
					bestVal = estimate;
					returnTarget = state.target;
					greedyAction = action;
				}
			}
		}

		state.target = returnTarget;
		return state;
	}

	/*
	For each order executed (specified via StateInfo), calculate and apply batched changes to weights.
	*/
	void batchUpdateWeights(std::vector <StateInfo> orders, std::vector <double(*) (StateInfo)> features, std::vector <double(*) (StateInfo)> actions) {
		std::vector<double> weight_changes = initializeWeights(0);
		
		// For each order
		for (int i = 0; i < (int) orders.size(); i++) {
			std::vector<double> temp_weight = updateWeights(this->currState, orders[i]); // prevOrders[i]?
			for (int j = 0; j < (int) weight_changes.size(); j++)
				weight_changes[j] += temp_weight[j];
		}

		for (int i = 0; i < (int) this->weights.size(); i++)
			this->weights[i] += weight_changes[i];
	}


	/*
	Perform TD update for each parameter after taking given action and return the changes.
	*/
	std::vector<double> updateWeights(StateInfo currState, StateInfo prevState) {
		// constants
		double learningRate = 0.01;
		double discount = 0.9;

		int actionInd = prevState.actionInd; 
		std::vector<double> weight_changes = initializeWeights(0); 
		
		currState = selectGreedyAction(currState); 
		for (int i = 0; i < (int) weight_changes.size(); i++) {
			double noisyGradient = reward(currState, prevState) + 
				discount*estimateQ(currState) - 
				estimateQ(prevState); // check which index to pass in
			noisyGradient *= learningRate;
			
			if (i == 0) // just add the gradient to the standalone weight
				weight_changes.at(actionInd) = noisyGradient;
			else  // multiply the feature by the gradient
				weight_changes.at(actionInd) = noisyGradient*features.at(i)(prevState);
		}

		return weight_changes; 
	}

	double estimateQ(StateInfo state) { 
		int actionInd = state.actionInd;
		double estimate = this->weights.at(actionInd);
		for (int i = 1; i < (int) this->weights.size(); i++) {
			double ftr = this->features.at(2)(state);		//made feature constant as we just want DPS-HP ratio feature
			double wt = this->weights.at(actionInd);
			estimate += wt * ftr;
		}
		return estimate;
	}

	/*
	Initialize an |actions|*(|features|+1) vector of the given value (1 by default).
	*/
	std::vector<double> initializeWeights(double val=1.0) {
		std::vector<double> weights;
		for (int i = 0; i < (int) this->actions.size(); i++) {
			double newAction; // put this to a vector? How do we make this able to store multiple values
			for (int j = 0; j < (int) this->features.size() + 1; j++) {
				newAction = val;
				weights.push_back(newAction);
			}
		}
		return weights;
	}

	// Reward := change in our total hitpoint lead
	double reward(StateInfo currState, StateInfo prevState)
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
		std::vector<double(*) (StateInfo)> actions;
		std::vector<double(*) (StateInfo)> features;
		StateInfo currState;
		StateInfo prevState;
};		
#endif // !QLEARN