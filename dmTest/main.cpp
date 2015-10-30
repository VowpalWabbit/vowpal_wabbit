// stub for experimenting with minimal code snippets

#include <iostream>
#include <math.h>

using namespace std;

int main(int argc, char *argv[]) {

	float one(1.f);
	float minus1(-1.f);
	float infin(exp(10000.));
	float neginf(minus1 * infin);
	float result = exp(neginf);
	cerr << "inf is " << infin << "; neginf is " << neginf << "; result is " << result << endl;

	cin.ignore();

}