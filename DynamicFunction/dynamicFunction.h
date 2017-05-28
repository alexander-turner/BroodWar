#include <iostream>
#include <vector>
#include <functional>
using namespace std; 

void attack(int f, int e);
void heal(int f, int e);
void eat(int f, int e);

class Functions
{
	public:

		/*
		we only have to enumerate every function once, in the 
		constructor
		*/
		Functions() {
			void(*attackFunction) (int f, int e);
			attackFunction = &attack;
			functionVector.push_back(attackFunction);

			void(*healFunction) (int f, int e);
			healFunction = &heal;
			functionVector.push_back(healFunction);

			void(*eatFunction) (int f, int e);
			eatFunction = &eat;
			functionVector.push_back(eatFunction);

		}

		/*
		method to invoke every function in the vector
		can overload or change as we want
		*/
		void invoke_all(int f, int e)
		{
			for (int i = 0; i < this->functionVector.size(); i++) {
				this->functionVector.at(i)(f, e);
			}
		}

		//allows us to add functions from beyond class
		vector <void(*) (int, int)> getVec()
		{
			return this->functionVector;
		}
		void add(void(*fn) (int, int)) {
			functionVector.push_back(fn);
		}
		
	private:
		vector <void(*) (int, int)> functionVector;
};

/*
Functions to add - tried including these as member functions
but it cause type errors
*/
void attack(int f, int e)
{
	//TODO: Write attack()
	cout << f << " attacking " << e << endl;
}
void heal(int f, int e)
{
	//TODO: Write heal()
	cout << f << " healing " << e << endl;
}
void eat(int f, int e)
{
	//TODO: Write eat()
	cout << f << " eating " << e << endl;
}