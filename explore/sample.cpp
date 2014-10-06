// vw_explore.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MWT.h"
#include <chrono>

using namespace std;
using namespace std::chrono;

const int NUM_ACTIONS = 10;

u32 Stateful_Default_Policy1(int* policy_params, Context* applicationContext)
{
	return *policy_params % NUM_ACTIONS + 1;
}
u32 Stateful_Default_Policy2(int* policy_params, Context* applicationContext)
{
	return *policy_params % NUM_ACTIONS + 2;
}
void Stateful_Default_Scorer(int* policy_params, Context* application_Context, float scores[], u32 size)
{
	for (u32 i = 0; i < size; i++)
	{
		scores[i] = *policy_params + i;
	}
}

u32 Stateless_Default_Policy1(Context* applicationContext)
{
	return 99 % NUM_ACTIONS + 1;
}
u32 Stateless_Default_Policy2(Context* applicationContext)
{
	return 98 % NUM_ACTIONS + 1;
}
void Stateless_Default_Scorer(Context* application_Context, float scores[], u32 size)
{
	for (u32 i = 0; i < size; i++)
	{
		scores[i] = 97 + i;
	}
}

void Clock_Explore()
{
	string appId = "myapp";
	float epsilon = .2f;
	int policy_params = 101;
	string unique_key = "key";
	int num_features = 10000;
	int num_iter = 5;
	int num_interactions = 200;

	double time_init = 0, time_choose = 0, time_serialized_log = 0, time_typed_log = 0;
	for (int iter = 0; iter < num_iter + 1; iter++)
	{
		high_resolution_clock::time_point t1 = high_resolution_clock::now();

		MWTExplorer mwt(appId);
		mwt.Initialize_Epsilon_Greedy<int>(epsilon, Stateful_Default_Policy1, &policy_params, NUM_ACTIONS);

		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		
		time_init += iter == 0 ? 0 : duration_cast<duration<double>>(t2 - t1).count();

		feature* features = new feature[num_features];
		for (int i = 0; i < num_features; i++)
		{
			features[i].weight_index = i + 1;
			features[i].x = 0.5;
		}

		t1 = high_resolution_clock::now();

		Context context(features, num_features);
		for (int i = 0; i < num_interactions / 2; i++)
		{
			mwt.Choose_Action_And_Key(context);
			mwt.Choose_Action(context, unique_key);
		}
		t2 = high_resolution_clock::now();
		time_choose += iter == 0 ? 0 : duration_cast<duration<double>>(t2 - t1).count();

		t1 = high_resolution_clock::now();

		string logs = mwt.Get_All_Interactions();
		
		t2 = high_resolution_clock::now();
		time_serialized_log += iter == 0 ? 0 : duration_cast<duration<double>>(t2 - t1).count();

		for (int i = 0; i < num_interactions / 2; i++)
		{
			mwt.Choose_Action_And_Key(context);
			mwt.Choose_Action(context, unique_key);
		}
		t1 = high_resolution_clock::now();

		Interaction** interactions = nullptr;
		size_t n_inter = 0;
		mwt.Get_All_Interactions(n_inter, interactions);
		
		t2 = high_resolution_clock::now();
		time_typed_log += iter == 0 ? 0 : duration_cast<duration<double>>(t2 - t1).count();
		delete features;

		cout << "Iteration " << iter + 1 << endl;
	}

	cout << "# iterations: " << num_iter << ", # interactions: " << num_interactions << ", # context features: " << num_features << endl;
	cout << "Init: " << time_init * 1000 / num_iter << " ms" << endl;
	cout << "Choose Action: " << time_choose * 1000 / (num_iter * num_interactions) << " ms" << endl;
	cout << "Get Serialized Log: " << time_serialized_log * 1000 / num_iter << " ms" << endl;
	cout << "Get Typed Log: " << time_typed_log * 1000 / num_iter << " ms" << endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	// Sample driver code
	string appId = "myapp";

	// Create a new MWT instance
	MWTExplorer mwt(appId);

	float epsilon = .2f;
	u32 tau = 5;
	u32 bags = 2;
	float lambda = 0.5f;

	int policy_params = 101;

	/*** Initialize Epsilon-Greedy explore algorithm using a default policy function that accepts parameters ***/
	//mwt.Initialize_Epsilon_Greedy<int>(epsilon, Stateful_Default_Policy1, &policy_params, NUM_ACTIONS);

	/*** Initialize Epsilon-Greedy explore algorithm using a stateless default policy function ***/
	//mwt.Initialize_Epsilon_Greedy(epsilon, Stateless_Default_Policy1, NUM_ACTIONS);

	/*** Initialize Tau-First explore algorithm using a default policy function that accepts parameters ***/
	//mwt.Initialize_Tau_First<int>(tau, Stateful_Default_Policy1, &policy_params, NUM_ACTIONS);

	/*** Initialize Tau-First explore algorithm using a stateless default policy function ***/
	//mwt.Initialize_Tau_First(tau, Stateless_Default_Policy1, NUM_ACTIONS);

	/*** Initialize Bagging explore algorithm using a default policy function that accepts parameters ***/
	StatefulFunctionWrapper<int>::Policy_Func* funcs[2] = { Stateful_Default_Policy1, Stateful_Default_Policy2 };
	int* params[2] = { &policy_params, &policy_params };
	mwt.Initialize_Bagging<int>(bags, funcs, params, NUM_ACTIONS);

	/*** Initialize Bagging explore algorithm using a stateless default policy function ***/
	//StatelessFunctionWrapper::Policy_Func* funcs[2] = { Stateless_Default_Policy1, Stateless_Default_Policy2 };
	//mwt.Initialize_Bagging(bags, funcs, NUM_ACTIONS);

	/*** Initialize Softmax explore algorithm using a default scorer function that accepts parameters ***/
	//mwt.Initialize_Softmax<int>(lambda, Stateful_Default_Scorer, &policy_params, NUM_ACTIONS);

	/*** Initialize Softmax explore algorithm using a stateless default scorer function ***/
	//mwt.Initialize_Softmax(lambda, Stateless_Default_Scorer, NUM_ACTIONS);

	// Create Features & Context
	feature features[1];
	features[0].weight_index = 1;
	features[0].x = 0.5;

	Context context(features, 1);

	// Now let MWT explore & choose an action
	pair<u32, u64> action_and_key = mwt.Choose_Action_And_Key(context);

	string unique_key = "1001";
	u32 chosen_action = mwt.Choose_Action(context, unique_key);
	
	// Get the logged data
	cout << mwt.Get_All_Interactions() << endl;

	return 0;
}