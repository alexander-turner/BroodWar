#ifndef FUNCTIONS
#define FUNCTIONS


#include <iostream>
#include <vector>
#include <functional>
using namespace BWAPI;

struct StateInfo {
	Unit currentUnit;
	Unit target;
	Unitset friendlies;
	Unitset enemies;
	int actionInd; // which action has been chosen for currentUnit, target
};

double getHPtoDPSratio(StateInfo state);
double getHP(StateInfo state);
double getDPS(StateInfo state);
double attackEnemy(StateInfo state);
double moveToOrigin(StateInfo state);
class Functions
{
public:

	/*
	we only have to enumerate every function once, in the
	constructor
	*/
	Functions() {
		double(*getHPF) (StateInfo state);
		getHPF = &getHP;
		functionVector.push_back(getHPF);

		double(*getDPSF) (StateInfo state);
		getDPSF = &getDPS;
		functionVector.push_back(getDPSF);

		double(*DPStoHP) (StateInfo state);
		DPStoHP = &getHPtoDPSratio;
		functionVector.push_back(DPStoHP);

		double(*attackF) (StateInfo state);
		attackF = &attackEnemy;
		actionVector.push_back(attackF);

		double(*moveToOr) (StateInfo state);
		moveToOr = &moveToOrigin;
		actionVector.push_back(moveToOr);

	}

	/*
	method to invoke every function in the vector
	can overload or change as we want
	*/
	void invoke_all(StateInfo state)
	{
		for (double i = 0; i < this->functionVector.size(); i++) {
			this->functionVector.at(i)(state);
		}
	}

	//allows us to add functions from beyond class
	std::vector <double(*) (StateInfo)> getFeatures()
	{
		return this->functionVector;
	}
	std::vector <double(*) (StateInfo)> getActions()
	{
		return this->actionVector;
	}
	void addFeature(double(*fn) (StateInfo)) {
		functionVector.push_back(fn);
	}
	void addAction(double(*fn) (StateInfo)) {
		actionVector.push_back(fn);
	}

private:
	std::vector <double(*) (StateInfo)> functionVector;
	std::vector <double(*) (StateInfo)> actionVector;
};

/*
Functions to add - tried including these as member functions
but it cause type errors
*/
double getDPS(StateInfo state) {

	double dmg = 6 / 15;
	return dmg;
}

double getHP(StateInfo state) {
	Unit u = state.currentUnit;
	return u->getHitPoints();
}

double getHPtoDPSratio(StateInfo state) {
	Unit u = state.currentUnit;
	return u->getHitPoints();
	return getDPS(state) / u->getHitPoints();
}

double attackEnemy(StateInfo state) {

	Unit u = state.currentUnit;
	Unit e = state.target;
	u->attack(e);
	//UnitCommand::attack(u, e);
	std::cout << u << "attacking" << e << std::endl;
	//returns double so it can fit in double type vector of functions
	return 1.0;
}

double moveToOrigin(StateInfo state)
{
	Unit u = state.currentUnit;
	u->move(Positions::Origin);

	std::cout << u << "moving to" << Positions::Origin << std::endl;
	//returns double so it can fit in double type vector of functions
	return 1.0;
}

#endif // !FUNCTIONS