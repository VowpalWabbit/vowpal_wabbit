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

int main(int argc, char* argv[])
{
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
	      mwt.Initialize_Bagging<int>(num_bags, bags, params, NUM_ACTIONS);
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
	cout << mwt.Get_All_Interactions_As_String() << endl;

	return 0;
}
