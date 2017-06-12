#ifndef FUNCTIONS
#define FUNCTIONS


#include <iostream>
#include <vector>
#include <functional>
#include <unordered_map>
using namespace BWAPI;

struct StateInfo {
	Unit currentUnit;
	Unit target;
	Unitset friendlies;
	Unitset enemies;
	int actionInd; // which action has been chosen for currentUnit, target
	std::unordered_map<Unit, int> friendlyHP;
	std::unordered_map<Unit, int> enemyHP;
	
};

double getTargetHP(StateInfo state);
double getTargetDPS(StateInfo state);
double getTargetDistance(StateInfo statE);
double getDPStoHPratio(StateInfo state);

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
		double(*DPStoHP) (StateInfo state);
		DPStoHP = &getDPStoHPratio;
		functionVector.push_back(DPStoHP);

		/*double(*distance) (StateInfo state);
		distance = &getTargetDistance;
		functionVector.push_back(distance);*/

		double(*attackF) (StateInfo state);
		attackF = &attackEnemy;
		actionVector.push_back(attackF);

		/*double(*moveToOr) (StateInfo state);
		moveToOr = &moveToOrigin;
		actionVector.push_back(moveToOr);*/

	}

	/*
	method to invoke every function in the vector
	can overload or change as we want
	*/
	void invoke_all(StateInfo state)
	{
		for (int i = 0; i < (int) this->functionVector.size(); i++) {
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

/* FEATURES */
double getTargetHP(StateInfo state) {
	Unit u = state.target;
	if (!u)
		return 1.0;
	return u->getHitPoints();
}

double getTargetDPS(StateInfo state) {
	Unit u = state.target;
	if (!u)
		return 0.0;
	WeaponType weapon = u->getType().groundWeapon();
	int dmgamount = weapon.damageAmount();
	int hits = u->getType().maxGroundHits();
	int cd = u->getType().groundWeapon().damageCooldown();
	double ticksPerSecond = 14.93; // normal game tickrate
	double dmg = (((double) dmgamount * hits * ticksPerSecond) / cd);
	return dmg;
}

double getTargetDistance(StateInfo state) {
	Unit u = state.currentUnit;
	Unit e = state.target;
	if (!u || !e)
		return 1.0;
	std::cout << u->getDistance(e) << std::endl;
	return u->getDistance(e);
}

double getDPStoHPratio(StateInfo state) {
	if (!state.target)
		return 0.0;
	return getTargetDPS(state) / getTargetHP(state);
}

/* ACTIONS */
double attackEnemy(StateInfo state) {

	Unit u = state.currentUnit;
	Unit e = state.target;

	UnitCommand::attack(u, e);
	
	//returns double so it can fit in double type vector of functions
	return 1.0;
}

double moveToOrigin(StateInfo state)
{
	Unit u = state.currentUnit;
	u->move(Positions::Origin);

	return 1.0;
}

#endif // !FUNCTIONS