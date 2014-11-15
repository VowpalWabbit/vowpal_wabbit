// explore.cpp : Timing code to measure performance of MWT Explorer library

#include "MWTExplorer.h"
#include <chrono>
#include <tuple>
#include <iostream>

using namespace std;
using namespace std::chrono;

using namespace MultiWorldTesting;

class MySimplePolicy : public IPolicy<SimpleContext>
{
public:
	u32 Choose_Action(SimpleContext& context)
	{
		return (u32)1;
	}
};

const u32 num_actions = 10;

void Clock_Explore()
{
	float epsilon = .2f;
	string unique_key = "key";
	int num_features = 1000;
	int num_iter = 10000;
	int num_warmup = 100;
	int num_interactions = 1;

	// pre-create features
	vector<Feature> features;
	for (int i = 0; i < num_features; i++)
	{
	  Feature f = {0.5, i+1};
	        features.push_back(f);
	}

	long long time_init = 0, time_choose = 0;
	for (int iter = 0; iter < num_iter + num_warmup; iter++)
	{
		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		StringRecorder<SimpleContext> recorder;
		MwtExplorer<SimpleContext> mwt("test", recorder);
	        MySimplePolicy default_policy; 
		EpsilonGreedyExplorer<SimpleContext> explorer(default_policy, epsilon, num_actions);
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		time_init += iter < num_warmup ? 0 : duration_cast<chrono::microseconds>(t2 - t1).count();

		t1 = high_resolution_clock::now();
		SimpleContext appContext(features);
		for (int i = 0; i < num_interactions; i++)
		{
		  mwt.Choose_Action(explorer, unique_key, appContext);
		}
		t2 = high_resolution_clock::now();
		time_choose += iter < num_warmup ? 0 : duration_cast<chrono::microseconds>(t2 - t1).count();
	}
	
	cout << "# iterations: " << num_iter << ", # interactions: " << num_interactions << ", # context features: " << num_features << endl;
	cout << "--- PER ITERATION ---" << endl;
	cout << "Init: " << (double)time_init / num_iter << " micro" << endl;
	cout << "Choose Action: " << (double)time_choose / (num_iter * num_interactions) << " micro" << endl;
	cout << "--- TOTAL TIME ---: " << (time_init + time_choose) << " micro" << endl;
}

int main(int argc, char* argv[])
{
  	Clock_Explore();
  	return 0;
}
