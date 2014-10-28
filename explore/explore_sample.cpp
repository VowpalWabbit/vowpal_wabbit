// vw_explore.cpp : Defines the entry point for the console application.
//

#include "MWTExplorer.h"
#include <chrono>
#include <tuple>

using namespace std;
using namespace std::chrono;
using namespace MultiWorldTesting;

const int NUM_ACTIONS = 10;

u32 Stateful_Default_Policy1(int& parameters, Context& appContext)
{
	return parameters % NUM_ACTIONS + 1;
}
u32 Stateful_Default_Policy2(int& parameters, Context& appContext)
{
	return parameters % NUM_ACTIONS + 2;
}
void Stateful_Default_Scorer(int& parameters, Context& appContext, float scores[], u32 size)
{
	for (u32 i = 0; i < size; i++)
	{
		scores[i] = (float) (parameters + i);
	}
}

u32 Stateless_Default_Policy1(Context& appContext)
{
	return 99 % NUM_ACTIONS + 1;
}
u32 Stateless_Default_Policy2(Context& appContext)
{
	return 98 % NUM_ACTIONS + 1;
}
void Stateless_Default_Scorer(Context& appContext, float scores[], u32 size)
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
	int num_iter = 1000;
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
		mwt.Initialize_Epsilon_Greedy<int>(epsilon, Stateful_Default_Policy1, policy_params, NUM_ACTIONS);
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		time_init += iter < num_warmup ? 0 : duration_cast<chrono::microseconds>(t2 - t1).count();

		t1 = high_resolution_clock::now();
		Context appContext(features, num_features);
		for (int i = 0; i < num_interactions; i++)
		{
			mwt.Choose_Action(unique_key, appContext);
		}
		t2 = high_resolution_clock::now();
		time_choose += iter < num_warmup ? 0 : duration_cast<chrono::microseconds>(t2 - t1).count();

		t1 = high_resolution_clock::now();
		string logs = mwt.Get_All_Interactions();
		t2 = high_resolution_clock::now();
		time_serialized_log += iter < num_warmup ? 0 : duration_cast<chrono::microseconds>(t2 - t1).count();

		for (int i = 0; i < num_interactions; i++)
		{
			mwt.Choose_Action(unique_key, appContext);
		}

		t1 = high_resolution_clock::now();
		Interaction** interactions = nullptr;
		size_t n_inter = 0;
		mwt.Get_All_Interactions(n_inter, interactions);
		t2 = high_resolution_clock::now();
		time_typed_log += iter < num_warmup ? 0 : duration_cast<chrono::microseconds>(t2 - t1).count();
		for (size_t i = 0; i < n_inter; i++)
			delete interactions[i];
		delete[] interactions;
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

	if (argc < 2)
	  {
	    cerr << "arguments: {greedy,tau-first,bagging,softmax} [stateful]" << endl;
	    exit(1);
	  }
	
	bool stateful = false;
	if (argc == 3) {
	  if (strcmp(argv[2],"stateful")==0)
	    stateful = true;
	  else
	    {
	      cerr << "unknown policy type: " << argv[2] << endl;
	      exit(1);
	    }
	}
	      
	//arguments for individual explorers
	int policy_params = 101;//A more complex type in real applications.
	u32 num_bags = 2;
	Stateful<int>::Policy* bags[] = { Stateful_Default_Policy1, Stateful_Default_Policy2 };
	Policy* stateless_bags[] = { Stateless_Default_Policy1, Stateless_Default_Policy2 };
	int policy_params_bag_1 = 12;
	int policy_params_bag_2 = 24;
	int* params[] = { &policy_params_bag_1, &policy_params_bag_2 };	

	// Create a new MWT instance
	MWTExplorer mwt("test");

	//Initialize an explorer
	if (strcmp(argv[1],"greedy") == 0)
	  { 
	    float epsilon = .2f;
	    if (stateful) //Initialize Epsilon-Greedy explore algorithm using a default policy function that accepts parameters 
	      mwt.Initialize_Epsilon_Greedy<int>(epsilon, Stateful_Default_Policy1, policy_params, NUM_ACTIONS);
	    else //Initialize Epsilon-Greedy explore algorithm using a stateless default policy function 
	      mwt.Initialize_Epsilon_Greedy(epsilon, Stateless_Default_Policy1, NUM_ACTIONS);
	  }
	else if (strcmp(argv[1],"tau-first") == 0)
	  {
	    u32 tau = 5;
	    if (stateful) //Initialize Tau-First explore algorithm using a default policy function that accepts parameters 
	      mwt.Initialize_Tau_First<int>(tau, Stateful_Default_Policy1, policy_params, NUM_ACTIONS);
	    else // Initialize Tau-First explore algorithm using a stateless default policy function 
	      mwt.Initialize_Tau_First(tau, Stateless_Default_Policy1, NUM_ACTIONS);
	  }
	else if (strcmp(argv[1],"bagging") == 0)
	  {
	    if (stateful) // Initialize Bagging explore algorithm using a default policy function that accepts parameters
	      mwt.Initialize_Bagging<int>(num_bags, bags, *params, NUM_ACTIONS);
	    else //Initialize Bagging explore algorithm using a stateless default policy function 
	      mwt.Initialize_Bagging(num_bags, stateless_bags, NUM_ACTIONS);
	  }
	else if (strcmp(argv[1],"softmax") == 0)
	  {
	    float lambda = 0.5f;
	    if (stateful) //Initialize Softmax explore algorithm using a default scorer function that accepts parameters
	      mwt.Initialize_Softmax<int>(lambda, Stateful_Default_Scorer, policy_params, NUM_ACTIONS);
	    else 	    // Initialize Softmax explore algorithm using a stateless default scorer function 
	      mwt.Initialize_Softmax(lambda, Stateless_Default_Scorer, NUM_ACTIONS);
	  }
	else
	  {
	    cerr << "unknown exploration type: " << argv[1] << endl;
	    exit(1);
	  }
	 
	// Create Features & Context
	int num_features = 1;
	Feature features[1];//1 is the number of features.
	//a sparse feature representation
	features[0].Id = 32;
	features[0].Value = 0.5;

	Context appContext(features, num_features);

	// Now let MWT explore & choose an action
	string unique_key = "1001";
	u32 chosen_action = mwt.Choose_Action(unique_key, appContext);
	
	cout << "action = " << chosen_action << endl;
	
	// Get the logged data
	cout << mwt.Get_All_Interactions() << endl;

	return 0;
}
