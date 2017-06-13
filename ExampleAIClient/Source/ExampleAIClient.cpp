#include <BWAPI.h>
#include <BWAPI/Client.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <string>
#include "../functionWrappers.h"
#include "../QFnApprox.h"
using namespace BWAPI;

void reconnect()
{
	while (!BWAPIClient.connect())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds{ 1000 });
	}
}

// Writes the elements in results to the given filepath (same-directory 'results.csv' is the default)
void outputResultsToCSV(std::vector<double> results, std::string filepath = "") {
	if (filepath == "")
		filepath = "results.csv";

	std::ofstream myfile(filepath, std::ofstream::out);
	for (int i=0; i < (int) results.size(); i++) {
		myfile << results.at(i);
		myfile << ",";
	}
	myfile << "\n";
	myfile.close();
}

int main(int argc, const char* argv[])
{
	Functions f;
	QLearn qfn;
	std::vector <double(*) (StateInfo)> actionVector = f.getActions(), featureVector = f.getFeatures();
	std::vector <double> scores;
	  
	std::cout << "Connecting..." << std::endl;
	reconnect();
	while (true) {
		std::cout << "waiting to enter match" << std::endl;
		while (!Broodwar->isInGame())
		{
			BWAPI::BWAPIClient.update();
			if (!BWAPI::BWAPIClient.isConnected())
			{
				std::cout << "Reconnecting..." << std::endl;;
				reconnect();
			}
		}
		std::cout << "Starting match!" << std::endl;
		// Enable some cheat flags
		Broodwar->enableFlag(Flag::UserInput);
		// Enables complete map information
		Broodwar->enableFlag(Flag::CompleteMapInformation);
		  
		while (Broodwar->isInGame()) {
			//Broodwar->setLocalSpeed(0); // uncomment to speed simulation

			qfn.QFunctionApproximation(actionVector, featureVector);

			BWAPI::BWAPIClient.update();
			if (!BWAPI::BWAPIClient.isConnected()) {
				std::cout << "Reconnecting..." << std::endl;
				reconnect();
			}
		}
		scores.push_back(qfn.getScore()); // Store score
		outputResultsToCSV(scores);
	}

	std::cout << "Press ENTER to continue..." << std::endl;
	std::cin.ignore();
	return 0;
}

