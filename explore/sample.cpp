// vw_explore.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MWT.h"

using namespace std;

u32 Stateful_Default_Policy(int* policy_params, Context* applicationContext)
{
	return *policy_params;
}

u32 Stateless_Default_Policy(Context* applicationContext)
{
	return 99;
}

int _tmain(int argc, _TCHAR* argv[])
{
	// Sample driver code
	string appId = "myapp";
	
	u32 num_actions = 10;

	// Create a new MWT instance
	MWTExplorer mwt(appId);

	float epsilon = .2f;
	float exploreBudget = .05f;

	int policy_params = 101;
	bool useStatefulFunc = true;
	if (useStatefulFunc)
	{
		mwt.Initialize_Epsilon_Greedy<int>(epsilon, Stateful_Default_Policy, &policy_params, num_actions);
	}
	else
	{
		mwt.Initialize_Epsilon_Greedy(epsilon, Stateless_Default_Policy, num_actions);
	}

	// Create Features & Context
	feature f[1];
	f[0].weight_index = 1;
	f[0].x = 0.5;

	Context ctx(f, 1);

	// Now let MWT explore & choose an action
	// todo: 
	pair<u32, u64> action_and_key = mwt.Choose_Action_And_Key(ctx);

	string unique_key = "1001";
	u32 chosen_action = mwt.Choose_Action(ctx, unique_key);
	
	cout << "Chosen Action ID with join key is: " << action_and_key.first << endl;
	cout << "Chosen Action ID is: " << chosen_action << endl;
	cout << mwt.Get_All_Interactions() << endl;

	return 0;
}