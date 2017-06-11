#ifndef QLEARN
#define QLEARN
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
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
	Paper: How to normalize rewards for bad situations? Granularity?

	TODO: 
	Fix DPS function - DONE
	Fix state updating
	Fix action vector being size=1
	Fix for 2d weights
	Store previous orders and fix updateWeights - DONE
	EstimateQ code doublecheck - where are actions?
	Doublecheck weight_changes.size() in updateWeights for 2d arrays
	Fix weights for arbitrary number of features / actions - DONE (OK)
	Run for arbitrary number of games - auto-restart
	Output scores to CSV for analysis - DONE
	Optional: open / save weight file
	*/

	/* Given a starting state, a set of actions and features, run GQ and update the action-feature weights.
	   weights has dimensions: |features|+1, |actions|. Optional filepath parameter allows for loading and saving weights.
	*/
	StateInfo QFunctionApproximation(std::vector<double(*)(StateInfo)> actions, std::vector<double(*)(StateInfo)> features, std::string filepath="") {
		this->actions = actions;
		this->features = features;
		this->orders.clear(); // reset orders
		
		if (filepath != "") {
			std::ifstream input_file(filepath); 
			double tempVar;
			/*while (input_file >> tempVar)
				this->weights.push_back(tempVar);*/ // TODO: Make compatible with 2d-vec
		}
		else if (this->weights.size() == 0) {
			this->weights = initializeWeights(); // Fill with ones (dim: |actions| x k+1)
		}

		this->currState.friendlies = Broodwar->self()->getUnits();
		this->currState.enemies = Broodwar->enemy()->getUnits();
		//for (auto &e : this->currState.enemies)
		//	this->currState.enemyHP[e] = e->getHitPoints();

		// update weights given the orders executed last time
		if (this->prevState.currentUnit != NULL)
			batchUpdateWeights(this->orders);
		this->orders.clear(); // reset orders
		this->prevState = this->currState;
		for (auto &u : this->currState.friendlies) { // calculate action for each unit
			if (!u->exists())
				continue;

			//this->currState.friendlyHP[u] = u->getHitPoints();
			this->currState.currentUnit = u;
			this->currState.actionInd = selectAction();
			this->orders.push_back(this->currState);

			actions.at(this->currState.actionInd)(this->currState); // execute action
		}

		if (filepath != "") { // overwrite?
			std::ofstream file;
			file.open(filepath);
			for (int i = 0; i < (int) this->weights.size(); i++)
				for (int j = 0; j < (int) this->weights[i].size(); j++)
				file << this->weights[i][j];
			file.close();
		}
		return this->currState;
	}
	/*
	Select an action index in a state using an e-greedy algorithm.
	*/
	int selectAction() {
		double epsilon = 0.5; // probability of choosing non-greedy action
		int greedyAction = selectGreedyAction();
		Unit greedyTarget = this->currState.target;

		std::unordered_map<int, Unit> unitmapping;
		int i = 0;
		for (auto &e : this->currState.enemies) {
			unitmapping[i] = e;
			i++;
		}

		int action = greedyAction;
		Unit target = greedyTarget;
		if ((double) std::rand() / (RAND_MAX) <= epsilon)
			while (action == greedyAction && target == greedyTarget) {
				action = rand() % (int) this->actions.size();
				target = unitmapping[rand() % i];
			}
		//std::cout << "greedy action is: " << greedyAction << "| " << "action chosen is: " << action << std::endl;

		this->currState.target = target;
		return action;
	}

	/*
	Returns index of greedy action according to state, weights, and features and modifies unit target to the best found for that action.
	*/
	int selectGreedyAction() {
		StateInfo state = this->currState;
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

		this->currState.target = returnTarget;
		return greedyAction;
	}

	/*
	For each order executed (specified via StateInfo), calculate and apply batched changes to weights.
	*/
	void batchUpdateWeights(std::vector <StateInfo> orders) {
		std::vector<std::vector<double>> weight_changes = initializeWeights(0);
		
		// For each order
		for (int i = 0; i < (int) orders.size(); i++) {
			std::vector<std::vector<double>> temp_weight = updateWeights(this->currState, orders[i]); 
			for (int j = 0; j < (int) weight_changes.size(); j++)
				weight_changes[j][orders[i].actionInd] += temp_weight[j][orders[i].actionInd]; // check this?
		}

		for (int i = 0; i < (int) this->weights.size(); i++)
			for (int j = 0; j < (int) this->weights[i].size(); j++)
				this->weights[i][j] += weight_changes[i][j];
	}


	/*
	Perform TD update for each parameter after taking given action and return the changes.
	*/
	std::vector<std::vector<double>> updateWeights(StateInfo currState, StateInfo prevState) {
		// constants
		double learningRate = 0.01;
		double discount = 0.9;

		int actionInd = prevState.actionInd; 
		std::vector<std::vector<double>> weight_changes = initializeWeights(0); 
		
		int greedyAction = selectGreedyAction(); 
		currState.actionInd = greedyAction;
		for (int i = 0; i < (int) weight_changes.size(); i++) {
			double noisyGradient = reward(currState, prevState) + 
				discount*estimateQ(currState) - 
				estimateQ(prevState); // check which index to pass in
			noisyGradient *= learningRate;
			std::cout << "Gradient:" << noisyGradient << std::endl;
			
			if (i == 0) // just add the gradient to the standalone weight
				weight_changes.at(i).at(actionInd) = noisyGradient;
			else  // multiply the feature by the gradient
				weight_changes.at(i).at(actionInd) = noisyGradient*this->features.at(i)(prevState);
		}

		return weight_changes; 
	}

	/*
	Initialize an |actions|*(|features|+1) vector of the given value (1 by default).
	*/
	std::vector<std::vector<double>> initializeWeights(double val = 1.0) {
		std::vector<std::vector<double>> weights;
		for (int i = 0; i < (int) this->features.size() + 1; i++) {
			std::vector<double> featureVec;
			for (int j = 0; j < (int) this->actions.size(); j++)
				featureVec.push_back(val);
			weights.push_back(featureVec);
		}
		return weights;
	}

	double estimateQ(StateInfo state) {
		int actionInd = state.actionInd;
		double estimate = (this->weights.at(0)).at(actionInd);
		for (int i = 1; i < (int) this->weights.size(); i++)
			estimate += this->weights.at(i).at(actionInd) * this->features.at(i - 1)(state);
		return estimate;
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
		for (auto &u : state.friendlies)
			HPDiff += u->getHitPoints();
		for (auto &u : state.enemies)
			HPDiff -= u->getHitPoints();
		return HPDiff;
	}

	double getScore() {
		return getHPDiff(this->currState);
	}

	private:
		std::vector < std::vector <double> > weights;
		std::vector<double(*) (StateInfo)> actions;
		std::vector<double(*) (StateInfo)> features;
		std::vector <StateInfo> orders;
		StateInfo currState;
		StateInfo prevState;
};
#endif // !QLEARN