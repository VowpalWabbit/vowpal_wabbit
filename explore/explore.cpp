// explore.cpp : Timing code to measure performance of MWT Explorer library

#include "MWTExplorer.h"
#include <chrono>
#include <tuple>

using namespace std;
using namespace std::chrono;

using namespace MultiWorldTesting;

const int NUM_ACTIONS = 10;

u32 Stateful_Default_Policy_1(int& parameters, BaseContext& appContext)
{
	return parameters % NUM_ACTIONS + 1;
}
u32 Stateful_Default_Policy_2(int& parameters, BaseContext& appContext)
{
	return parameters % NUM_ACTIONS + 2;
}
void Stateful_Default_Scorer_1(int& parameters, BaseContext& appContext, float scores[], u32 size)
{
	for (u32 i = 0; i < size; i++)
	{
		scores[i] = (float) (parameters + i);
	}
}

u32 Stateless_Default_Policy_1(BaseContext& appContext)
{
	return 99 % NUM_ACTIONS + 1;
}
u32 Stateless_Default_Policy_2(BaseContext& appContext)
{
	return 98 % NUM_ACTIONS + 1;
}
void Stateless_Default_Scorer_1(BaseContext& appContext, float scores[], u32 size)
{
	for (u32 i = 0; i < size; i++)
	{
		scores[i] = 97 + i;
	}
}

void Clock_Explore()
{
	float epsilon = .2f;
	int policy_params = 101;
	string unique_key = "key";
	int num_features = 1000;
	int num_iter = 10000;
	int num_warmup = 100;
	int num_interactions = 1;

	// pre-create features
	Feature* features = new Feature[num_features];
	for (int i = 0; i < num_features; i++)
	{
		features[i].Id = i + 1;
		features[i].Value = 0.5;
	}

	long long time_init = 0, time_choose = 0, time_serialized_log = 0, time_typed_log = 0;
	for (int iter = 0; iter < num_iter + num_warmup; iter++)
	{
		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		MWTExplorer mwt("test");
		mwt.Initialize_Epsilon_Greedy<int>(epsilon, Stateful_Default_Policy_1, policy_params, NUM_ACTIONS);
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		time_init += iter < num_warmup ? 0 : duration_cast<chrono::microseconds>(t2 - t1).count();

		t1 = high_resolution_clock::now();
		SimpleContext appContext(features, num_features);
		for (int i = 0; i < num_interactions; i++)
		{
		  mwt.Choose_Action(unique_key, appContext);
		}
		t2 = high_resolution_clock::now();
		time_choose += iter < num_warmup ? 0 : duration_cast<chrono::microseconds>(t2 - t1).count();

		t1 = high_resolution_clock::now();
		string logs = mwt.Get_All_Interactions_As_String();
		t2 = high_resolution_clock::now();
		time_serialized_log += iter < num_warmup ? 0 : duration_cast<chrono::microseconds>(t2 - t1).count();

		for (int i = 0; i < num_interactions; i++)
		{
		  mwt.Choose_Action(unique_key, appContext);
		}

		t1 = high_resolution_clock::now();
		vector<Interaction> interactions = mwt.Get_All_Interactions();
		t2 = high_resolution_clock::now();
		time_typed_log += iter < num_warmup ? 0 : duration_cast<chrono::microseconds>(t2 - t1).count();
	}
	
	delete[] features;

	cout << "# iterations: " << num_iter << ", # interactions: " << num_interactions << ", # context features: " << num_features << endl;
	cout << "--- PER ITERATION ---" << endl;
	cout << "Init: " << (double)time_init / num_iter << " micro" << endl;
	cout << "Choose Action: " << (double)time_choose / (num_iter * num_interactions) << " micro" << endl;
	cout << "Get Serialized Log: " << (double)time_serialized_log / num_iter << " micro" << endl;
	cout << "Get Typed Log: " << (double)time_typed_log / num_iter << " micro" << endl;
	cout << "--- TOTAL TIME ---: " << (time_init + time_choose + time_serialized_log + time_typed_log) << " micro" << endl;
}

int main(int argc, char* argv[])
{
  	Clock_Explore();
  	return 0;
}
