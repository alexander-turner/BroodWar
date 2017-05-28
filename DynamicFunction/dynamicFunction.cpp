#include <iostream>
#include <vector>
#include <functional>
#include "dynamicFunction.h"
using namespace std;



int main()
{
	int f = 1;
	int e = 2;
	Functions fn;

	fn.invoke_all(f, e);

	return 0;
}

